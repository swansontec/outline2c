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
#include "context.h"
#include "lexer.h"
#include "file.h"
#include <stdio.h>

int parse_code(Context *ctx, AstBuilder *b, int scoped);
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
 * Processes a source file, adding its contents to the AST.
 */
int parse_file(String filename, AstBuilder *b)
{
  FileR file;
  Context ctx;
  char *s;

  s = string_to_c(filename);
  if (!file_r_open(&file, s)) {
    fprintf(stderr, "error: Could not open source file \"%s\"\n", s);
    return 0;
  }
  ctx = context_init(string_init(file.p, file.end), s);

  /* Parse the input file: */
  CHECK(parse_code(&ctx, b, 0));

  file_r_close(&file);
  free(s);
  return 1;
}

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

  CHECK_MEM(ast_builder_push_start(b));
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
  CHECK_MEM(ast_build_code_text(b, string_init(start_c, start)));

  /* Token pasting: */
  start_c = ctx->cursor;
  start = ctx->cursor; token = lex(&ctx->cursor, ctx->file.end);
  goto code;

escape:
  CHECK_MEM(ast_build_code_text(b, string_init(start_c, start)));

  /* "\ol" escape sequences: */
  CHECK(parse_escape(ctx, b));

  start_c = ctx->cursor;
  start = ctx->cursor; token = lex(&ctx->cursor, ctx->file.end);
  goto code;

symbol:
  CHECK_MEM(ast_build_code_text(b, string_init(start_c, start)));

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
        CHECK_MEM(ast_build_call(b, lookup, symbol));
      } else {
        CHECK_MEM(ast_build_lookup(b, symbol, string_init(start, ctx->cursor)));
      }
      start_c = ctx->cursor;
      start = ctx->cursor; token = lex(&ctx->cursor, ctx->file.end);
    } else {
      CHECK_MEM(ast_build_symbol_ref(b, symbol));
    }
  } else {
    CHECK_MEM(ast_build_symbol_ref(b, symbol));
  }
  goto code;

done:
  CHECK_MEM(ast_build_code_text(b, string_init(start_c, start)));

  /* End-of-code: */
  if (scoped && token == LEX_END)
    return context_error(ctx, "Unexpected end of input in code block.");
  CHECK_MEM(ast_build_code(b));
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
  /* Nested {} block: */
  } else if (token == LEX_BRACE_L) {
    token = lex_next(&start, &ctx->cursor, ctx->file.end);
    while (token != LEX_BRACE_R) {
      ctx->cursor = start;
      CHECK(parse_escape(ctx, b));
      token = lex_next(&start, &ctx->cursor, ctx->file.end);
    }
    return 1;
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

      /* Assignment? */
      token = lex_next(&start, &ctx->cursor, ctx->file.end);
      if (token != LEX_EQUALS)
        return context_error(ctx, "No idea what this keyword is.");

      CHECK(parse_escape(ctx, b));

      symbol = ast_builder_scope_add(b, temp);
      CHECK_MEM(symbol);
      symbol->type = ast_builder_peek(b).type;

      CHECK_MEM(ast_build_set(b, symbol));
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

  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token != LEX_STRING)
    return context_error(ctx, "An include statment expects a quoted filename.");

  /* Process the file's contents: */
  CHECK(parse_file(string_init(start + 1, ctx->cursor - 1), b));
  CHECK_MEM(ast_build_include(b));

  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token != LEX_SEMICOLON)
    return context_error(ctx, "An include stament must end with a semicolon.");

  return 1;
}

/**
 * Parses a list of outline items.
 */
int parse_outline(Context *ctx, AstBuilder *b)
{
  char const *start;
  Token token;

  /* Opening brace: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token != LEX_BRACE_L)
    return context_error(ctx, "An outline must start with an opening {.");

  /* Items: */
  CHECK_MEM(ast_builder_push_start(b));
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  while (token != LEX_BRACE_R) {
    ctx->cursor = start;
    CHECK(parse_outline_item(ctx, b));
    token = lex_next(&start, &ctx->cursor, ctx->file.end);
  }

  CHECK_MEM(ast_build_outline(b));
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

  /* Handle the words making up the item: */
  CHECK_MEM(ast_builder_push_start(b));
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  while (token == LEX_IDENTIFIER) {
    if (string_size(last)) {
      CHECK_MEM(ast_build_outline_tag(b, last));
    }
    last = string_init(start, ctx->cursor);
    token = lex_next(&start, &ctx->cursor, ctx->file.end);
    if (token == LEX_EQUALS) {
      /* Opening brace: */
      token = lex_next(&start, &ctx->cursor, ctx->file.end);
      if (token != LEX_BRACE_L)
        return context_error(ctx, "A tag's value must be a code block.");

      /* Value: */
      CHECK(parse_code(ctx, b, 1));

      CHECK_MEM(ast_build_outline_tag(b, last));

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
  } else if (token != LEX_SEMICOLON) {
    return context_error(ctx, "An outline can only end with a semicolon or an opening brace.");
  }

  CHECK_MEM(ast_build_outline_item(b, last));
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
  CHECK_MEM(ast_builder_push_start(b));
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  while (token != LEX_BRACE_R) {
    ctx->cursor = start;
    CHECK(parse_map_line(ctx, b));
    token = lex_next(&start, &ctx->cursor, ctx->file.end);
  }

  ast_builder_scope_pop(b);
  CHECK_MEM(ast_build_map(b, item));
  return 1;
}

/**
 * Parses an individual line within a map statement.
 */
int parse_map_line(Context *ctx, AstBuilder *b)
{
  char const *start;
  Token token;

  /* Filter: */
  CHECK(parse_filter(ctx, b));

  /* Opening brace: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token != LEX_BRACE_L)
    return context_error(ctx, "A line within a \"map\" staement must end with a code block.");

  /* Code: */
  CHECK(parse_code(ctx, b, 1));

  CHECK_MEM(ast_build_map_line(b));
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
  String symbol;
  int reverse = 0;
  int list = 0;

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
    symbol = string_init(start, ctx->cursor);

    /* "with" modifier: */
    if (string_equal(symbol, string_init_l("with", 4))) {
      CHECK(parse_filter(ctx, b));
      goto modifier;

    /* "reverse" modifier: */
    } else if (string_equal(symbol, string_init_l("reverse", 7))) {
      reverse = 1;
      goto modifier;

    /* "list" modifier: */
    } else if (string_equal(symbol, string_init_l("list", 4))) {
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

  ast_builder_scope_pop(b);
  CHECK_MEM(ast_build_for(b, item, outline, reverse, list));
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

  CHECK_MEM(ast_build_filter(b));
  return 1;
}
