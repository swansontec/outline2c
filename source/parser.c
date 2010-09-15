/*
 * Copyright 2010 William R. Swanson
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
/*
 * This is a recursive descent parser which scans through the input, building
 * up an AST representing the file contents.
 *
 * The context structure holds important information about the parser state. In
 * particular, it holds the read cursor. Parser functions expect to be called
 * with the read cursor pointing to the first input character to consume.
 * Parser functions return with the cursor pointing one-past the last consumed
 * character.
 */

#include "parser.h"
#include "lexer.h"
#include <stdio.h>

int parse_escape(Context *ctx, AstBuilder *b);

int parse_include(Context *ctx, AstBuilder *b);

int parse_outline(Context *ctx, AstBuilder *b);
int parse_outline_item(Context *ctx, AstBuilder *b);

int parse_map(Context *ctx, AstBuilder *b);
int parse_map_line(Context *ctx, AstBuilder *b);

int parse_for(Context *ctx, AstBuilder *b);

int parse_filter(Context *ctx, AstBuilder *b);

/**
 * All parser functions return 1 for success and 0 for failure. This
 * macro checks a return code and bails out if it indicates an error.
 */
#define CHECK(r) do { if (!r) return 0; } while(0)

/**
 * Verifies that a memory-allocating call succeeded.
 */
#define CHECK_MEM(r) do { if (!r) { fprintf(stderr, "Out of memory on line %d!\n", __LINE__); return 0; } } while(0)

/**
 * Parses a block of code in the host language, looking for escape sequences
 * and replacement symbols.
 *
 * When this function is called, the cursor should point to either the start
 * of the file or the opening brace. When the function returns, the cursor
 * will point to either the end of the file or the character after the closing
 * brace.
 *
 * @param scoped If this parameter is non-zero, the parser will stop at the
 * first unbalanced }. Otherwise, the parser will stop at the end of the file.
 */
int parse_code(Context *ctx, AstBuilder *b, int scoped)
{
  char const *start_c, *start;
  Token token;
  Symbol *symbol;
  int indent = 1;
  ListBuilder nodes = list_builder_init();

#define WRITE_CODE \
  if (start_c != start) \
    CHECK_MEM(list_builder_add(&nodes, &b->pool, AST_CODE_TEXT, \
      ast_code_text_new(&b->pool, string_init(start_c, start))));

  start_c = ctx->cursor;
  start = ctx->cursor; token = lex(&ctx->cursor, ctx->file.end);

code:
  /* We are in a block of host-language code. Select a course of action: */
  if (token == LEX_END) goto done;
  if (token == LEX_PASTE) goto paste;
  if (token == LEX_ESCAPE_O2C) goto escape;
  if (scoped && token == LEX_IDENTIFIER) {
    symbol = ast_builder_scope_find(b, string_init(start, ctx->cursor));
    if (symbol)
      goto symbol;
  } else if (scoped && token == LEX_BRACE_R) {
    if (!--indent)
      goto done;
  } else if (scoped && token == LEX_BRACE_L) {
    ++indent;
  }
  start = ctx->cursor; token = lex(&ctx->cursor, ctx->file.end);
  goto code;

paste:
  WRITE_CODE

  /* Token pasting: */
  start_c = ctx->cursor;
  start = ctx->cursor; token = lex(&ctx->cursor, ctx->file.end);
  goto code;

escape:
  WRITE_CODE

  /* "\ol" escape sequences: */
  CHECK(parse_escape(ctx, b));
  CHECK_MEM(list_builder_add2(&nodes, &b->pool, ast_builder_pop(b)));

  start_c = ctx->cursor;
  start = ctx->cursor; token = lex(&ctx->cursor, ctx->file.end);
  goto code;

symbol:
  WRITE_CODE

  /* Symbol replacement: */
  if (symbol->type != AST_OUTLINE_ITEM)
    return context_error(ctx, "Wrong type - only outline items may be embedded in C code.\n");

  /* Is there a lookup modifier? */
  start_c = ctx->cursor;
  start = ctx->cursor; token = lex(&ctx->cursor, ctx->file.end);
  if (token == LEX_BANG) {
    start = ctx->cursor; token = lex(&ctx->cursor, ctx->file.end);
    if (token == LEX_IDENTIFIER) {
      Symbol *lookup = ast_builder_scope_find(b, string_init(start, ctx->cursor));
      if (lookup) {
        if (lookup->type != AST_MAP)
          return context_error(ctx, "Wrong type - expecting a map here.\n");
        CHECK_MEM(list_builder_add(&nodes, &b->pool, AST_CALL,
          ast_call_new(&b->pool, lookup, symbol)));
      } else {
        CHECK_MEM(list_builder_add(&nodes, &b->pool, AST_LOOKUP,
          ast_lookup_new(&b->pool, symbol, string_init(start, ctx->cursor))));
      }
      start_c = ctx->cursor;
      start = ctx->cursor; token = lex(&ctx->cursor, ctx->file.end);
    } else {
      CHECK_MEM(list_builder_add(&nodes, &b->pool, AST_SYMBOL_REF,
        ast_symbol_ref_new(&b->pool, symbol)));
    }
  } else {
    CHECK_MEM(list_builder_add(&nodes, &b->pool, AST_SYMBOL_REF,
      ast_symbol_ref_new(&b->pool, symbol)));
  }
  goto code;

done:
  WRITE_CODE

  /* End-of-code: */
  if (scoped && token == LEX_END)
    return context_error(ctx, "Unexpected end of input in code block.");
  CHECK_MEM(ast_builder_push(b, AST_CODE,
    ast_code_new(&b->pool, nodes.first)));
  return 1;
}

/**
 * Handles the bit right after an \ol escape code.
 */
int parse_escape(Context *ctx, AstBuilder *b)
{
  char const *start;
  Token token;

  /* Unexpected end of input: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token == LEX_END) {
    return context_error(ctx, "Unexpected end of input.");
  /* Keywords: */
  } else if (token == LEX_IDENTIFIER) {
    String temp = string_init(start, ctx->cursor);
    if (string_equal(temp, string_init_l("include", 7))) {
      return parse_include(ctx, b);
    } else if (string_equal(temp, string_init_l("outline", 7))) {
      return parse_outline(ctx, b);
    } else if (string_equal(temp, string_init_l("map", 3))) {
      return parse_map(ctx, b);
    } else if (string_equal(temp, string_init_l("for", 3))) {
      return parse_for(ctx, b);
    } else {
      Symbol *symbol;
      Dynamic value;

      /* Assignment? */
      token = lex_next(&start, &ctx->cursor, ctx->file.end);
      if (token != LEX_EQUALS)
        return context_error(ctx, "No idea what this keyword is.");

      CHECK(parse_escape(ctx, b));
      value = ast_builder_pop(b);

      symbol = ast_builder_scope_add(b, temp);
      CHECK_MEM(symbol);
      symbol->type = value.type;

      CHECK_MEM(ast_builder_push(b, AST_SET,
        ast_set_new(&b->pool, symbol, value)));
      return 1;
    }
  } else {
    return context_error(ctx, "No idea what token this is.");
  }
}

/**
 * Parses the "include" directive
 */
int parse_include(Context *ctx, AstBuilder *b)
{
  char const *start;
  Token token;
  String old_file;
  String old_filename;
  char const *old_cursor;
  AstCode *code;

  /* File name: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token != LEX_STRING)
    return context_error(ctx, "An include statment expects a quoted filename.");

  /* Save the context: */
  old_file = ctx->file;
  old_filename = ctx->filename;
  old_cursor = ctx->cursor;

  /* Process the file's contents: */
  ctx->filename = string_init(start + 1, ctx->cursor - 1);
  ctx->file = string_load(ctx->filename);
  if (!string_size(ctx->file)) {
    ctx->file = old_file;
    ctx->filename = old_filename;
    return context_error(ctx, "Could not open the included file.");
  }
  ctx->cursor = ctx->file.p;
  CHECK(parse_code(ctx, b, 0));
  code = ast_to_code(ast_builder_pop(b));
  string_free(ctx->file);

  /* Restore the context: */
  ctx->file = old_file;
  ctx->filename = old_filename;
  ctx->cursor = old_cursor;

  /* Closing semicolon: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token != LEX_SEMICOLON)
    return context_error(ctx, "An include stament must end with a semicolon.");

  CHECK_MEM(ast_builder_push(b, AST_INCLUDE,
    ast_include_new(&b->pool, code)));
  return 1;
}

/**
 * Parses a list of outline items.
 */
int parse_outline(Context *ctx, AstBuilder *b)
{
  char const *start;
  Token token;
  ListBuilder items = list_builder_init();

  /* Opening brace: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token != LEX_BRACE_L)
    return context_error(ctx, "An outline must start with an opening {.");

  /* Items: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  while (token != LEX_BRACE_R) {
    ctx->cursor = start;
    CHECK(parse_outline_item(ctx, b));
    CHECK_MEM(list_builder_add2(&items, &b->pool, ast_builder_pop(b)));
    token = lex_next(&start, &ctx->cursor, ctx->file.end);
  }

  CHECK_MEM(ast_builder_push(b, AST_OUTLINE,
    ast_outline_new(&b->pool, items.first)));
  return 1;
}

/**
 * Parses an individual item within an outline, including its tags and
 * children.
 */
int parse_outline_item(Context *ctx, AstBuilder *b)
{
  char const *start;
  Token token;
  String last = string_null();
  ListBuilder tags = list_builder_init();
  AstOutline *children = 0;

  /* Handle the words making up the item: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  while (token == LEX_IDENTIFIER) {
    if (string_size(last)) {
      CHECK_MEM(list_builder_add(&tags, &b->pool, AST_OUTLINE_TAG,
        ast_outline_tag_new(&b->pool, last, 0)));
    }
    last = string_init(start, ctx->cursor);
    token = lex_next(&start, &ctx->cursor, ctx->file.end);
    if (token == LEX_EQUALS) {
      AstCode *code;

      /* Opening brace: */
      token = lex_next(&start, &ctx->cursor, ctx->file.end);
      if (token != LEX_BRACE_L)
        return context_error(ctx, "A tag's value must be a code block.");

      /* Value: */
      CHECK(parse_code(ctx, b, 1));
      code = ast_to_code(ast_builder_pop(b));

      CHECK_MEM(list_builder_add(&tags, &b->pool, AST_OUTLINE_TAG,
        ast_outline_tag_new(&b->pool, last, code)));

      last = string_null();
      token = lex_next(&start, &ctx->cursor, ctx->file.end);
    }
  }
  if (!string_size(last))
    return context_error(ctx, "An outline item must have a name.");

  /* Is there a sub-outline? */
  if (token == LEX_BRACE_L) {
    ctx->cursor = start;
    CHECK(parse_outline(ctx, b));
    children = ast_to_outline(ast_builder_pop(b));
  } else if (token != LEX_SEMICOLON) {
    return context_error(ctx, "An outline can only end with a semicolon or an opening brace.");
  }

  CHECK_MEM(ast_builder_push(b, AST_OUTLINE_ITEM,
    ast_outline_item_new(&b->pool, tags.first, last, children)));
  return 1;
}

/**
 * Parses a map statement.
 */
int parse_map(Context *ctx, AstBuilder *b)
{
  char const *start;
  Token token;
  Symbol *item;
  ListBuilder lines = list_builder_init();

  CHECK_MEM(ast_builder_scope_new(b));

  /* Map name: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token != LEX_IDENTIFIER)
    return context_error(ctx, "An map stament must begin with a name.");
  item = ast_builder_scope_add(b, string_init(start, ctx->cursor));
  CHECK_MEM(item);
  item->type = AST_OUTLINE_ITEM;

  /* Opening brace: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token != LEX_BRACE_L)
    return context_error(ctx, "An opening { must come after the name of a map.");

  /* Lines: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  while (token != LEX_BRACE_R) {
    ctx->cursor = start;
    CHECK(parse_map_line(ctx, b));
    CHECK_MEM(list_builder_add2(&lines, &b->pool, ast_builder_pop(b)));
    token = lex_next(&start, &ctx->cursor, ctx->file.end);
  }

  ast_builder_scope_pop(b);
  CHECK_MEM(ast_builder_push(b, AST_MAP,
    ast_map_new(&b->pool, item, lines.first)));
  return 1;
}

/**
 * Parses an individual line within a map statement.
 */
int parse_map_line(Context *ctx, AstBuilder *b)
{
  char const *start;
  Token token;
  AstFilter *filter;
  AstCode *code;

  /* Filter: */
  CHECK(parse_filter(ctx, b));
  filter = ast_to_filter(ast_builder_pop(b));

  /* Opening brace: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token != LEX_BRACE_L)
    return context_error(ctx, "A line within a \"map\" staement must end with a code block.");

  /* Code: */
  CHECK(parse_code(ctx, b, 1));
  code = ast_to_code(ast_builder_pop(b));

  CHECK_MEM(ast_builder_push(b, AST_MAP_LINE,
    ast_map_line_new(&b->pool, filter, code)));
  return 1;
}

/**
 * Parses a for ... in ... construction.
 */
int parse_for(Context *ctx, AstBuilder *b)
{
  char const *start;
  Token token;
  Symbol *item;
  Symbol *outline;
  AstFilter *filter = 0;
  int reverse = 0;
  int list = 0;
  AstCode *code;

  CHECK_MEM(ast_builder_scope_new(b));

  /* Variable name: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token != LEX_IDENTIFIER)
    return context_error(ctx, "Expecting a new symbol name here.");
  item = ast_builder_scope_add(b, string_init(start, ctx->cursor));
  CHECK_MEM(item);
  item->type = AST_OUTLINE_ITEM;

  /* "in" keyword: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token != LEX_IDENTIFIER ||
    !string_equal(string_init(start, ctx->cursor), string_init_l("in", 2)))
    return context_error(ctx, "Expecting the \"in\" keyword here.");

  /* Outline name: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token != LEX_IDENTIFIER)
    return context_error(ctx, "An outline name must come after the \"in\" keyword.");
  outline = ast_builder_scope_find(b, string_init(start, ctx->cursor));
  if (!outline)
    return context_error(ctx, "Could not find an outline with this name.");
  if (outline->type != AST_OUTLINE && outline->type != AST_OUTLINE_ITEM)
    return context_error(ctx, "Wrong type - the for statement expects an outline.\n");

  /* Behavior modification keywords: */
modifier:
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token == LEX_IDENTIFIER) {
    String s = string_init(start, ctx->cursor);

    /* "with" modifier: */
    if (string_equal(s, string_init_l("with", 4))) {
      CHECK(parse_filter(ctx, b));
      filter = ast_to_filter(ast_builder_pop(b));
      goto modifier;

    /* "reverse" modifier: */
    } else if (string_equal(s, string_init_l("reverse", 7))) {
      reverse = 1;
      goto modifier;

    /* "list" modifier: */
    } else if (string_equal(s, string_init_l("list", 4))) {
      list = 1;
      goto modifier;
    } else {
      return context_error(ctx, "Invalid \"for\" statement modifier.");
    }
  } else if (token != LEX_BRACE_L) {
    return context_error(ctx, "A \"for\" staement must end with a code block.");
  }

  /* Code: */
  CHECK(parse_code(ctx, b, 1));
  code = ast_to_code(ast_builder_pop(b));

  ast_builder_scope_pop(b);
  CHECK_MEM(ast_builder_push(b, AST_FOR,
    ast_for_new(&b->pool, item, outline, filter, reverse, list, code)));
  return 1;
}

/**
 * Parses a filter definition. Uses the standard shunting-yard algorithm with
 * the following order of precedence: () ! & |
 */
int parse_filter(Context *ctx, AstBuilder *b)
{
  char const *start;
  Token token;
  enum operators { NOT, AND, OR, LPAREN } stack[32];
  int top = 0;

want_term:
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token == LEX_IDENTIFIER) {
    CHECK_MEM(ast_build_filter_tag(b, string_init(start, ctx->cursor)));
    goto want_operator;

  } else if (token == LEX_STAR) {
    CHECK_MEM(ast_build_filter_any(b));
    goto want_operator;

  } else if (token == LEX_BANG) {
    stack[top++] = NOT;
    goto want_term;

  } else if (token == LEX_PAREN_L) {
    stack[top++] = LPAREN;
    goto want_term;

  } else {
    return context_error(ctx, "There seems to be a missing term here.");
  }

want_operator:
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token == LEX_AMP) {
    for (; top && stack[top-1] <= AND; --top) {
      if (stack[top-1] == NOT) {
        CHECK_MEM(ast_build_filter_not(b));
      } else if (stack[top-1] == AND) {
        CHECK_MEM(ast_build_filter_and(b));
      }
    }
    stack[top++] = AND;
    goto want_term;

  } else if (token == LEX_PIPE) {
    for (; top && stack[top-1] <= OR; --top) {
      if (stack[top-1] == NOT) {
        CHECK_MEM(ast_build_filter_not(b));
      } else if (stack[top-1] == AND) {
        CHECK_MEM(ast_build_filter_and(b));
      } else if (stack[top-1] == OR) {
        CHECK_MEM(ast_build_filter_or(b));
      }
    }
    stack[top++] = OR;
    goto want_term;

  } else if (token == LEX_PAREN_R) {
    for (; top && stack[top-1] < LPAREN; --top) {
      if (stack[top-1] == NOT) {
        CHECK_MEM(ast_build_filter_not(b));
      } else if (stack[top-1] == AND) {
        CHECK_MEM(ast_build_filter_and(b));
      } else if (stack[top-1] == OR) {
        CHECK_MEM(ast_build_filter_or(b));
      }
    }
    if (!top)
      return context_error(ctx, "No maching opening parenthesis.");
    --top;
    goto want_operator;

  } else if (token == LEX_BANG || token == LEX_PAREN_L) {
    return context_error(ctx, "There seems to be a missing operator here.");

  } else {
    ctx->cursor = start;
    goto done;
  }

done:
  for (; top; --top) {
    if (stack[top-1] == NOT) {
      CHECK_MEM(ast_build_filter_not(b));
    } else if (stack[top-1] == AND) {
      CHECK_MEM(ast_build_filter_and(b));
    } else if (stack[top-1] == OR) {
      CHECK_MEM(ast_build_filter_or(b));
    } else if (stack[top-1] == LPAREN) {
      return context_error(ctx, "No maching closing parenthesis.");
    }
  }

  CHECK_MEM(ast_builder_push(b, AST_FILTER,
    ast_filter_new(&b->pool, ast_to_filter_node(ast_builder_pop(b)))));
  return 1;
}
