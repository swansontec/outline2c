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
#include "filter.h"
#include <assert.h>
#include <stdio.h>

int parse_escape(Context *ctx, OutRoutine or);

int parse_include(Context *ctx, OutRoutine or);

int parse_outline(Context *ctx, OutRoutine or);
int parse_outline_item(Context *ctx, OutRoutine or);

int parse_map(Context *ctx, OutRoutine or);
int parse_map_line(Context *ctx, OutRoutine or);

int parse_for(Context *ctx, OutRoutine or);

int parse_filter(Context *ctx, OutRoutine or);

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
int parse_code(Context *ctx, OutRoutine or, int scoped)
{
  char const *start_c, *start;
  Token token;
  Dynamic out;
  AstVariable *variable;
  int indent = 1;
  ListBuilder nodes = list_builder_init();
  AstCode *self = pool_alloc(ctx->pool, sizeof(AstCode));
  CHECK_MEM(self);

#define WRITE_CODE \
  if (start_c != start) \
    CHECK(list_builder_add(&nodes, ctx->pool, AST_CODE_TEXT, \
      ast_code_text_new(ctx->pool, string_init(start_c, start))));

  start_c = ctx->cursor;
  start = ctx->cursor; token = lex(&ctx->cursor, ctx->file.end);

code:
  /* We are in a block of host-language code. Select a course of action: */
  if (token == LEX_END) goto done;
  if (token == LEX_PASTE) goto paste;
  if (token == LEX_ESCAPE_O2C) goto escape;
  if (scoped && token == LEX_IDENTIFIER) {
    if (context_scope_get(ctx, &out, string_init(start, ctx->cursor)))
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
  CHECK(parse_escape(ctx, dynamic_out(&out)));
  if (out.type != TYPE_END) {
    if (!ast_is_code_node(out.type))
      return context_error(ctx, "Wrong type - expecting a code node here.");
    CHECK(list_builder_add2(&nodes, ctx->pool, out));
  }

  start_c = ctx->cursor;
  start = ctx->cursor; token = lex(&ctx->cursor, ctx->file.end);
  goto code;

symbol:
  WRITE_CODE

  /* Variable lookup: */
  if (out.type != AST_VARIABLE)
    return context_error(ctx, "Wrong type - only outline items may be embedded in C code.\n");
  variable = ast_to_variable(out);

  /* Is there a lookup modifier? */
  start_c = ctx->cursor;
  start = ctx->cursor; token = lex(&ctx->cursor, ctx->file.end);
  if (token == LEX_BANG) {
    start = ctx->cursor; token = lex(&ctx->cursor, ctx->file.end);
    if (token == LEX_IDENTIFIER) {
      if (context_scope_get(ctx, &out, string_init(start, ctx->cursor))) {
        if (out.type != AST_MAP)
          return context_error(ctx, "Wrong type - expecting a map here.\n");
        CHECK(list_builder_add(&nodes, ctx->pool, AST_CALL,
          ast_call_new(ctx->pool, variable, ast_to_map(out))));
      } else {
        CHECK(list_builder_add(&nodes, ctx->pool, AST_LOOKUP,
          ast_lookup_new(ctx->pool, variable, string_init(start, ctx->cursor))));
      }
      start_c = ctx->cursor;
      start = ctx->cursor; token = lex(&ctx->cursor, ctx->file.end);
    } else {
      CHECK(list_builder_add(&nodes, ctx->pool, AST_VARIABLE, variable));
    }
  } else {
    CHECK(list_builder_add(&nodes, ctx->pool, AST_VARIABLE, variable));
  }
  goto code;

done:
  WRITE_CODE

  /* End-of-code: */
  if (scoped && token == LEX_END)
    return context_error(ctx, "Unexpected end of input in code block.");
  self->nodes = nodes.first;

  CHECK(or.code(or.data, AST_CODE, self));
  return 1;
}

/**
 * Handles the bit right after an \ol escape code.
 */
int parse_escape(Context *ctx, OutRoutine or)
{
  char const *start;
  Token token;
  Dynamic out;

  /* Unexpected end of input: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token == LEX_END) {
    return context_error(ctx, "Unexpected end of input.");
  /* Keywords: */
  } else if (token == LEX_IDENTIFIER) {
    String temp = string_init(start, ctx->cursor);
    if (string_equal(temp, string_init_l("include", 7))) {
      return parse_include(ctx, or);
    } else if (string_equal(temp, string_init_l("outline", 7))) {
      return parse_outline(ctx, or);
    } else if (string_equal(temp, string_init_l("map", 3))) {
      return parse_map(ctx, or);
    } else if (string_equal(temp, string_init_l("for", 3))) {
      return parse_for(ctx, or);
    } else {
      /* Assignment? */
      token = lex_next(&start, &ctx->cursor, ctx->file.end);
      if (token != LEX_EQUALS)
        return context_error(ctx, "No idea what this keyword is.");

      CHECK(parse_escape(ctx, dynamic_out(&out)));
      CHECK(context_scope_add(ctx, temp, out.type, out.p));

      return 1;
    }
  } else {
    return context_error(ctx, "No idea what token this is.");
  }
}

/**
 * Parses the "include" directive
 */
int parse_include(Context *ctx, OutRoutine or)
{
  char const *start;
  Token token;
  Dynamic out;
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
  CHECK(parse_code(ctx, dynamic_out(&out), 0));
  code = ast_to_code(out);
  string_free(ctx->file);

  /* Restore the context: */
  ctx->file = old_file;
  ctx->filename = old_filename;
  ctx->cursor = old_cursor;

  /* Closing semicolon: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token != LEX_SEMICOLON)
    return context_error(ctx, "An include stament must end with a semicolon.");

  return 1;
}

/**
 * Parses a list of outline items.
 */
int parse_outline(Context *ctx, OutRoutine or)
{
  char const *start;
  Token token;
  Dynamic out;
  ListBuilder items = list_builder_init();
  AstOutline *self = pool_alloc(ctx->pool, sizeof(AstOutline));
  CHECK_MEM(self);

  /* Opening brace: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token != LEX_BRACE_L)
    return context_error(ctx, "An outline must start with an opening {.");

  /* Items: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  while (token != LEX_BRACE_R) {
    ctx->cursor = start;
    CHECK(parse_outline_item(ctx, dynamic_out(&out)));
    CHECK(list_builder_add2(&items, ctx->pool, out));
    token = lex_next(&start, &ctx->cursor, ctx->file.end);
  }
  self->items = items.first;

  CHECK(or.code(or.data, AST_OUTLINE, self));
  return 1;
}

/**
 * Parses an individual item within an outline, including its tags and
 * children.
 */
int parse_outline_item(Context *ctx, OutRoutine or)
{
  char const *start;
  Token token;
  Dynamic out;
  String last = string_null();
  ListBuilder tags = list_builder_init();
  AstOutlineItem *self = pool_alloc(ctx->pool, sizeof(AstOutlineItem));
  CHECK_MEM(self);

  /* Handle the words making up the item: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  while (token == LEX_IDENTIFIER) {
    if (string_size(last)) {
      CHECK(list_builder_add(&tags, ctx->pool, AST_OUTLINE_TAG,
        ast_outline_tag_new(ctx->pool, last, 0)));
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
      CHECK(parse_code(ctx, dynamic_out(&out), 1));
      code = ast_to_code(out);

      CHECK(list_builder_add(&tags, ctx->pool, AST_OUTLINE_TAG,
        ast_outline_tag_new(ctx->pool, last, code)));

      last = string_null();
      token = lex_next(&start, &ctx->cursor, ctx->file.end);
    }
  }
  if (!string_size(last))
    return context_error(ctx, "An outline item must have a name.");
  self->tags = tags.first;
  self->name = pool_string_copy(ctx->pool, last);
  CHECK_MEM(string_size(self->name));

  /* Is there a sub-outline? */
  self->children = 0;
  if (token == LEX_BRACE_L) {
    ctx->cursor = start;
    CHECK(parse_outline(ctx, dynamic_out(&out)));
    self->children = ast_to_outline(out);
  } else if (token != LEX_SEMICOLON) {
    return context_error(ctx, "An outline can only end with a semicolon or an opening brace.");
  }

  CHECK(or.code(or.data, AST_OUTLINE_ITEM, self));
  return 1;
}

/**
 * Parses a map statement.
 */
int parse_map(Context *ctx, OutRoutine or)
{
  char const *start;
  Token token;
  Dynamic out;
  ListBuilder lines = list_builder_init();
  AstMap *self = pool_alloc(ctx->pool, sizeof(AstMap));
  CHECK_MEM(self);

  CHECK(context_scope_push(ctx));

  /* Map name: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token != LEX_IDENTIFIER)
    return context_error(ctx, "An map stament must begin with a name.");
  CHECK(self->item = ast_variable_new(ctx->pool, string_init(start, ctx->cursor)));
  CHECK(context_scope_add(ctx, self->item->name, AST_VARIABLE, self->item));
  assert(self->item);

  /* Opening brace: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token != LEX_BRACE_L)
    return context_error(ctx, "An opening { must come after the name of a map.");

  /* Lines: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  while (token != LEX_BRACE_R) {
    ctx->cursor = start;
    CHECK(parse_map_line(ctx, dynamic_out(&out)));
    CHECK(list_builder_add2(&lines, ctx->pool, out));
    token = lex_next(&start, &ctx->cursor, ctx->file.end);
  }
  self->lines = lines.first;

  context_scope_pop(ctx);
  CHECK(or.code(or.data, AST_MAP, self));
  return 1;
}

/**
 * Parses an individual line within a map statement.
 */
int parse_map_line(Context *ctx, OutRoutine or)
{
  char const *start;
  Token token;
  Dynamic out;
  AstMapLine *self = pool_alloc(ctx->pool, sizeof(AstMapLine));
  CHECK_MEM(self);

  /* Filter: */
  CHECK(parse_filter(ctx, dynamic_out(&out)));
  self->filter = ast_to_filter(out);
  assert(self->filter);

  /* Opening brace: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token != LEX_BRACE_L)
    return context_error(ctx, "A line within a \"map\" staement must end with a code block.");

  /* Code: */
  CHECK(parse_code(ctx, dynamic_out(&out), 1));
  self->code = ast_to_code(out);
  assert(self->code);

  CHECK(or.code(or.data, AST_MAP_LINE, self));
  return 1;
}

/**
 * Parses a for ... in ... construction.
 */
int parse_for(Context *ctx, OutRoutine or)
{
  char const *start;
  Token token;
  Dynamic out;
  AstFor *self = pool_alloc(ctx->pool, sizeof(AstFor));
  CHECK_MEM(self);

  CHECK(context_scope_push(ctx));

  /* Variable name: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token != LEX_IDENTIFIER)
    return context_error(ctx, "Expecting a new symbol name here.");
  CHECK(self->item = ast_variable_new(ctx->pool, string_init(start, ctx->cursor)));
  CHECK(context_scope_add(ctx, self->item->name, AST_VARIABLE, self->item));
  assert(self->item);

  /* "in" keyword: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token != LEX_IDENTIFIER ||
    !string_equal(string_init(start, ctx->cursor), string_init_l("in", 2)))
    return context_error(ctx, "Expecting the \"in\" keyword here.");

  /* Outline name: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token != LEX_IDENTIFIER)
    return context_error(ctx, "An outline name must come after the \"in\" keyword.");
  if (!context_scope_get(ctx, &out, string_init(start, ctx->cursor)))
    return context_error(ctx, "Could not find an outline with this name.");
  if (!ast_is_for_node(out.type))
    return context_error(ctx, "Wrong type - the for statement expects an outline.\n");
  self->outline = ast_to_for_node(out);
  assert(self->outline.p);

  /* Behavior modification keywords: */
  self->filter = 0;
  self->reverse = 0;
  self->list = 0;
modifier:
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token == LEX_IDENTIFIER) {
    String s = string_init(start, ctx->cursor);

    /* "with" modifier: */
    if (string_equal(s, string_init_l("with", 4))) {
      CHECK(parse_filter(ctx, dynamic_out(&out)));
      self->filter = ast_to_filter(out);
      goto modifier;

    /* "reverse" modifier: */
    } else if (string_equal(s, string_init_l("reverse", 7))) {
      self->reverse = 1;
      goto modifier;

    /* "list" modifier: */
    } else if (string_equal(s, string_init_l("list", 4))) {
      self->list = 1;
      goto modifier;
    } else {
      return context_error(ctx, "Invalid \"for\" statement modifier.");
    }
  } else if (token != LEX_BRACE_L) {
    return context_error(ctx, "A \"for\" staement must end with a code block.");
  }

  /* Code: */
  CHECK(parse_code(ctx, dynamic_out(&out), 1));
  self->code = ast_to_code(out);
  assert(self->code);

  context_scope_pop(ctx);
  CHECK(or.code(or.data, AST_FOR, self));
  return 1;
}

/**
 * Parses a filter definition. Uses the standard shunting-yard algorithm with
 * the following order of precedence: () ! & |
 */
int parse_filter(Context *ctx, OutRoutine or)
{
  char const *start;
  Token token;
  FilterBuilder fb;
  enum operators { NOT, AND, OR, LPAREN } stack[32];
  int top = 0;
  AstFilter *self = pool_alloc(ctx->pool, sizeof(AstFilter));
  CHECK_MEM(self);

  CHECK(filter_builder_init(&fb));

want_term:
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token == LEX_IDENTIFIER) {
    CHECK(filter_build_tag(&fb, ctx->pool, string_init(start, ctx->cursor)));
    goto want_operator;

  } else if (token == LEX_STAR) {
    CHECK(filter_build_any(&fb, ctx->pool));
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
        CHECK(filter_build_not(&fb, ctx->pool));
      } else if (stack[top-1] == AND) {
        CHECK(filter_build_and(&fb, ctx->pool));
      }
    }
    stack[top++] = AND;
    goto want_term;

  } else if (token == LEX_PIPE) {
    for (; top && stack[top-1] <= OR; --top) {
      if (stack[top-1] == NOT) {
        CHECK(filter_build_not(&fb, ctx->pool));
      } else if (stack[top-1] == AND) {
        CHECK(filter_build_and(&fb, ctx->pool));
      } else if (stack[top-1] == OR) {
        CHECK(filter_build_or(&fb, ctx->pool));
      }
    }
    stack[top++] = OR;
    goto want_term;

  } else if (token == LEX_PAREN_R) {
    for (; top && stack[top-1] < LPAREN; --top) {
      if (stack[top-1] == NOT) {
        CHECK(filter_build_not(&fb, ctx->pool));
      } else if (stack[top-1] == AND) {
        CHECK(filter_build_and(&fb, ctx->pool));
      } else if (stack[top-1] == OR) {
        CHECK(filter_build_or(&fb, ctx->pool));
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
      CHECK(filter_build_not(&fb, ctx->pool));
    } else if (stack[top-1] == AND) {
      CHECK(filter_build_and(&fb, ctx->pool));
    } else if (stack[top-1] == OR) {
      CHECK(filter_build_or(&fb, ctx->pool));
    } else if (stack[top-1] == LPAREN) {
      return context_error(ctx, "No maching closing parenthesis.");
    }
  }

  self->test = ast_to_filter_node(filter_builder_pop(&fb));
  assert(self->test.p);

  CHECK(or.code(or.data, AST_FILTER, self));
  filter_builder_free(&fb);
  return 1;
}
