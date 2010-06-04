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
 * particular, it holds the next token. To advance from one token to the next,
 * the parser calls the advance() function.
 *
 * Each parser function expects to be called with the context pre-advanced. In
 * other words, the current token has not been processesd yet. Parser functions
 * leave the context in an un-advanced state, meaning that the calling function
 * must call advance() to get the next un-processed token.
 */

#include "parser.h"
#include "lexer.h"
#include "file.h"
#include <stdio.h>

typedef struct context Context;

/**
 * Contains everything related to the current input scanner state.
 */
struct context
{
  String file;
  char const *filename;
  Cursor cursor;
  Cursor marker;
  Token token;
};

int parse_code(Context *ctx, AstBuilder *b, int scoped);
int parse_escape(Context *ctx, AstBuilder *b);

int parse_include(Context *ctx, AstBuilder *b);

int parse_outline(Context *ctx, AstBuilder *b);
int parse_outline_item(Context *ctx, AstBuilder *b);

int parse_map(Context *ctx, AstBuilder *b);
int parse_map_line(Context *ctx, AstBuilder *b);

int parse_for(Context *ctx, AstBuilder *b);

int parse_filter(Context *ctx, AstBuilder *b);

int parse_symbol_new(Context *ctx, AstBuilder *b);

/**
 * All parser functions return 0 for success and non-zero for failure. This
 * macro checks a return code and bails out if it indicates an error.
 */
#define ENSURE_SUCCESS(rv) do { if (rv) return rv; } while(0)

/**
 * Verifies that a call to the AstBuilder succeeded.
 */
#define ENSURE_BUILD(b) do { if (b) { fprintf(stderr, "Out of memory on line %d!\n", __LINE__); return 1; } } while(0)

/**
 * Prepares a fresh context structure.
 */
Context context_init(String file, char const *filename)
{
  Context ctx;
  ctx.file = file;
  ctx.filename = filename;
  ctx.cursor = cursor_init(file.p);
  ctx.marker = ctx.cursor;
  ctx.token = LEX_START;
  return ctx;
}

/**
 * Prints an error message.
 */
void
error(Context *ctx, char const *message)
{
  fprintf(stderr, "%s:%d:%d: error: %s\n", ctx->filename, ctx->marker.line + 1, ctx->marker.column + 1, message);
}

/**
 * Reads the next token, optionally filtering out whitespace & comments.
 */
static void advance(Context *ctx, int want_space)
{
  do {
    ctx->marker = ctx->cursor;
    ctx->token = lex(&ctx->cursor, ctx->file.end);
  } while (!want_space &&
    (ctx->token == LEX_WHITESPACE || ctx->token == LEX_COMMENT));
}

/**
 * Processes a source file, adding its contents to the AST.
 */
int parse_file(char const *filename, AstBuilder *b)
{
  int rv;
  FileR file;
  Context ctx;

  rv = file_r_open(&file, filename);
  if (rv) {
    fprintf(stderr, "error: Could not open file %s\n", filename);
    return 1;
  }
  ctx = context_init(string_init(file.p, file.end), filename);

  /* Parse the input file: */
  rv = parse_code(&ctx, b, 0);
  ENSURE_SUCCESS(rv);
  ENSURE_BUILD(ast_build_file(b));

  file_r_close(&file);
  return 0;
}

/**
 * Parses a block of code in the host language, looking for escape sequences
 * and replacement symbols.
 *
 * When this function is called, the current token should be either the start
 * of the file or the opening brace. When this function returns, the current
 * token will be either the end of the file or the closing brace.
 *
 * @param scoped If this parameter is non-zero, the parser will stop at the
 * first unbalanced }. Otherwise, the parser will stop at the end of the file.
 */
int parse_code(Context *ctx, AstBuilder *b, int scoped)
{
  int rv;
  AstSymbolNew *symbol;
  int indent = 1;
  char const *start;

  advance(ctx, 1);
  start = ctx->marker.p;
  ENSURE_BUILD(ast_builder_push_start(b));

code:
  /* We are in a block of host-language code. Select a course of action: */
  if (ctx->token == LEX_END) goto done;
  if (ctx->token == LEX_PASTE) goto paste;
  if (ctx->token == LEX_ESCAPE_O2C) goto escape;
  if (scoped && ctx->token == LEX_IDENTIFIER) {
    symbol = ast_builder_find_symbol(b, string_init(ctx->marker.p, ctx->cursor.p));
    if (symbol)
      goto symbol;
  } else if (scoped && ctx->token == LEX_BRACE_R) {
    if (!--indent)
      goto done;
  } else if (scoped && ctx->token == LEX_BRACE_L) {
    ++indent;
  }
  advance(ctx, 1);
  goto code;

paste:
  ENSURE_BUILD(ast_build_code_text(b, string_init(start, ctx->marker.p)));

  /* Handle token pasting: */
  advance(ctx, 1);
  start = ctx->marker.p;
  goto code;

escape:
  ENSURE_BUILD(ast_build_code_text(b, string_init(start, ctx->marker.p)));

  /* Handle @o2c escape sequences: */
  advance(ctx, 0);
  rv = parse_escape(ctx, b);
  ENSURE_SUCCESS(rv);
  advance(ctx, 1);
  start = ctx->marker.p;
  goto code;

symbol:
  ENSURE_BUILD(ast_build_code_text(b, string_init(start, ctx->marker.p)));

  /* Handle symbol replacement: */
  ENSURE_BUILD(ast_build_symbol_ref(b, symbol));
  advance(ctx, 1);
  start = ctx->marker.p;
  /* Is there a lookup modifier? */
  if (ctx->token == LEX_BANG) {
    advance(ctx, 1);
    if (ctx->token == LEX_IDENTIFIER) {
      symbol = ast_builder_find_symbol(b, string_init(ctx->marker.p, ctx->cursor.p));
      if (symbol) {
        ENSURE_BUILD(ast_build_symbol_ref(b, symbol));
        ENSURE_BUILD(ast_build_call(b));
      } else {
        ENSURE_BUILD(ast_build_lookup(b, string_init(ctx->marker.p, ctx->cursor.p)));
      }
      advance(ctx, 1);
      start = ctx->marker.p;
    }
  }
  goto code;

done:
  ENSURE_BUILD(ast_build_code_text(b, string_init(start, ctx->marker.p)));

  /* Handle end-of-code: */
  if (scoped && ctx->token == LEX_END) {
    error(ctx, "Unexpected end of input in code block.");
    return 1;
  }
  ENSURE_BUILD(ast_build_code(b));
  return 0;
}

/**
 * Handles the bit right after an \ol escape code.
 */
int parse_escape(Context *ctx, AstBuilder *b)
{
  int rv;

  /* Unexpected end of input: */
  if (ctx->token == LEX_END) {
    error(ctx, "Unexpected end of input.");
    return 1;
  /* Nested {} block: */
  } else if (ctx->token == LEX_BRACE_L) {
    advance(ctx, 0);
    while (ctx->token != LEX_BRACE_R) {
      rv = parse_escape(ctx, b);
      ENSURE_SUCCESS(rv);
      advance(ctx, 0);
    }
    return 0;
  /* Keywords: */
  } else if (ctx->token == LEX_IDENTIFIER) {
    String temp = string_init(ctx->marker.p, ctx->cursor.p);
    if (string_equal(temp, string_init_l("include", 7))) {
      advance(ctx, 0);
      return parse_include(ctx, b);
    } else if (string_equal(temp, string_init_l("outline", 7))) {
      advance(ctx, 0);
      return parse_outline(ctx, b);
    } else if (string_equal(temp, string_init_l("map", 3))) {
      advance(ctx, 0);
      return parse_map(ctx, b);
    } else if (string_equal(temp, string_init_l("for", 3))) {
      advance(ctx, 0);
      return parse_for(ctx, b);
    } else {
      /* Assignment? */
      advance(ctx, 0);
      if (ctx->token != LEX_EQUALS) {
        error(ctx, "No idea what this keyword is.");
        return 1;
      }

      ENSURE_BUILD(ast_build_symbol_new(b, temp));

      advance(ctx, 0);
      rv = parse_escape(ctx, b);
      ENSURE_SUCCESS(rv);

      ENSURE_BUILD(ast_build_set(b));
      return 0;
    }
  } else {
    error(ctx, "No idea what token this is.");
    return 1;
  }
}

/**
 * Handles the "include" directive
 */
int parse_include(Context *ctx, AstBuilder *b)
{
  int rv;
  char *filename;

  if (ctx->token != LEX_STRING) {
    error(ctx, "An include statment expects a quoted filename.");
    return 1;
  }
  filename = string_to_c(string_init(ctx->marker.p + 1, ctx->cursor.p - 1));

  /* Process the file's contents: */
  rv = parse_file(filename, b);
  ENSURE_SUCCESS(rv);
  ENSURE_BUILD(ast_build_include(b));
  free(filename);

  advance(ctx, 0);
  if (ctx->token != LEX_SEMICOLON) {
    error(ctx, "An include stament must end with a semicolon.");
    return 1;
  }
  return 0;
}

/**
 * Handles a list of outline items.
 */
int parse_outline(Context *ctx, AstBuilder *b)
{
  int rv;

  /* Opening brace: */
  if (ctx->token != LEX_BRACE_L) {
    error(ctx, "An outline must start with an opening {.");
    return 1;
  }

  advance(ctx, 0);
  ENSURE_BUILD(ast_builder_push_start(b));
  while (ctx->token != LEX_BRACE_R) {
    rv = parse_outline_item(ctx, b);
    ENSURE_SUCCESS(rv);
    advance(ctx, 0);
  }
  ENSURE_BUILD(ast_build_outline(b));

  return 0;
}

/**
 * Handles an individual item within an outline, including its tags and
 * children.
 */
int parse_outline_item(Context *ctx, AstBuilder *b)
{
  int rv;
  String last = string_null();

  /* Handle the words making up the item: */
  ENSURE_BUILD(ast_builder_push_start(b));
  while (ctx->token == LEX_IDENTIFIER) {
    if (string_size(last)) {
      ENSURE_BUILD(ast_build_outline_tag(b, last));
    }
    last = string_init(ctx->marker.p, ctx->cursor.p);
    advance(ctx, 0);
    if (ctx->token == LEX_EQUALS) {
      /* Opening brace: */
      advance(ctx, 0);
      if (ctx->token != LEX_BRACE_L) {
        error(ctx, "A tag's value must be a code block.");
        return 1;
      }

      rv = parse_code(ctx, b, 1);
      ENSURE_SUCCESS(rv);
      advance(ctx, 0);

      ENSURE_BUILD(ast_build_outline_tag(b, last));
      last = string_null();
    }
  }
  if (!string_size(last)) {
    error(ctx, "An outline item must have a name.");
    return 1;
  }

  /* Handle any sub-items: */
  if (ctx->token == LEX_BRACE_L) {
    rv = parse_outline(ctx, b);
    ENSURE_SUCCESS(rv);
    ENSURE_BUILD(ast_build_outline_item(b, last));
    return 0;
  /* Otherwise, the item must end with a semicolon: */
  } else if (ctx->token == LEX_SEMICOLON) {
    ENSURE_BUILD(ast_build_outline_item(b, last));
    return 0;
  /* Anything else is an error: */
  } else {
    error(ctx, "An outline can only end with a semicolon or an opening brace.");
    return 1;
  }
}

/**
 * Parses a map statement.
 */
int parse_map(Context *ctx, AstBuilder *b)
{
  int rv;

  /* Map name: */
  if (ctx->token != LEX_IDENTIFIER) {
    error(ctx, "An map stament must begin with a name.");
    return 1;
  }
  ENSURE_BUILD(ast_build_symbol_new(b, string_init(ctx->marker.p, ctx->cursor.p)));
  advance(ctx, 0);

  /* Opening brace: */
  if (ctx->token != LEX_BRACE_L) {
    error(ctx, "An opening { must come after the name of a map.");
    return 1;
  }

  /* Lines: */
  advance(ctx, 0);
  ENSURE_BUILD(ast_builder_push_start(b));
  while (ctx->token != LEX_BRACE_R) {
    rv = parse_map_line(ctx, b);
    ENSURE_SUCCESS(rv);
    advance(ctx, 0);
  }

  ENSURE_BUILD(ast_build_map(b));
  return 0;
}

/**
 * Parses an individual line within a map statement.
 */
int parse_map_line(Context *ctx, AstBuilder *b)
{
  int rv;

  rv = parse_filter(ctx, b);
  ENSURE_SUCCESS(rv);

  /* Opening brace: */
  if (ctx->token != LEX_BRACE_L) {
    error(ctx, "A line within a \"map\" staement must end with a code block.");
    return 1;
  }

  /* Parse the code: */
  rv = parse_code(ctx, b, 1);
  ENSURE_SUCCESS(rv);

  ENSURE_BUILD(ast_build_map_line(b));
  return 0;
}

/**
 * Parses a for ... in ... construction.
 */
int parse_for(Context *ctx, AstBuilder *b)
{
  int rv;
  AstSymbolNew *symbol;
  String token;
  int reverse = 0;
  int list = 0;

  rv = parse_symbol_new(ctx, b);
  ENSURE_SUCCESS(rv);
  advance(ctx, 0);

  /* in keyword: */
  if (ctx->token != LEX_IDENTIFIER || !string_equal(
    string_init(ctx->marker.p, ctx->cursor.p),
    string_init_l("in", 2))
  ) {
    error(ctx, "Expecting the \"in\" keyword here.");
    return 1;
  }
  advance(ctx, 0);

  /* Outline name: */
  if (ctx->token != LEX_IDENTIFIER) {
    error(ctx, "An outline name must come after the \"in\" keyword.");
    return 1;
  }
  symbol = ast_builder_find_symbol(b, string_init(ctx->marker.p, ctx->cursor.p));
  if (!symbol) {
    error(ctx, "Could not find an outline with this name.");
    return 1;
  }
  ENSURE_BUILD(ast_build_symbol_ref(b, symbol));
  advance(ctx, 0);

  /* Behavior modification keywords: */
modifier:
  if (ctx->token != LEX_IDENTIFIER)
    goto modifier_end;

  /* "with" modifier: */
  token = string_init(ctx->marker.p, ctx->cursor.p);
  if (string_equal(token, string_init_l("with", 4))) {
    advance(ctx, 0);
    rv = parse_filter(ctx, b);
    ENSURE_SUCCESS(rv);
    goto modifier;

  /* "reverse" modifier: */
  } else if (string_equal(token, string_init_l("reverse", 7))) {
    reverse = 1;
    advance(ctx, 0);
    goto modifier;

  /* "list" modifier: */
  } else if (string_equal(token, string_init_l("list", 4))) {
    list = 1;
    advance(ctx, 0);
    goto modifier;
  }
modifier_end:

  /* Opening brace: */
  if (ctx->token != LEX_BRACE_L) {
    error(ctx, "A \"for\" staement must end with a code block.");
    return 1;
  }

  /* Parse the code: */
  rv = parse_code(ctx, b, 1);
  ENSURE_SUCCESS(rv);

  ENSURE_BUILD(ast_build_for(b, reverse, list));
  return 0;
}

/**
 * Parses a filter definition. Uses the standard shunting-yard algorithm with
 * the following order of precedence: () ! & |
 */
int parse_filter(Context *ctx, AstBuilder *b)
{
  enum operators { NOT, AND, OR, LPAREN } stack[32];
  int top = 0;

want_term:
  if (ctx->token == LEX_IDENTIFIER) {
    ENSURE_BUILD(ast_build_filter_tag(b, string_init(ctx->marker.p, ctx->cursor.p)));
    advance(ctx, 0);
    goto want_operator;

  } else if (ctx->token == LEX_STAR) {
    ENSURE_BUILD(ast_build_filter_any(b));
    advance(ctx, 0);
    goto want_operator;

  } else if (ctx->token == LEX_BANG) {
    stack[top++] = NOT;
    advance(ctx, 0);
    goto want_term;

  } else if (ctx->token == LEX_PAREN_L) {
    stack[top++] = LPAREN;
    advance(ctx, 0);
    goto want_term;

  } else {
    error(ctx, "There seems to be a missing term here.");
    return 1;
  }

want_operator:
  if (ctx->token == LEX_AMP) {
    for (; top && stack[top-1] <= AND; --top) {
      if (stack[top-1] == NOT) {
        ENSURE_BUILD(ast_build_filter_not(b));
      } else if (stack[top-1] == AND) {
        ENSURE_BUILD(ast_build_filter_and(b));
      }
    }
    stack[top++] = AND;
    advance(ctx, 0);
    goto want_term;

  } else if (ctx->token == LEX_PIPE) {
    for (; top && stack[top-1] <= OR; --top) {
      if (stack[top-1] == NOT) {
        ENSURE_BUILD(ast_build_filter_not(b));
      } else if (stack[top-1] == AND) {
        ENSURE_BUILD(ast_build_filter_and(b));
      } else if (stack[top-1] == OR) {
        ENSURE_BUILD(ast_build_filter_or(b));
      }
    }
    stack[top++] = OR;
    advance(ctx, 0);
    goto want_term;

  } else if (ctx->token == LEX_PAREN_R) {
    for (; top && stack[top-1] < LPAREN; --top) {
      if (stack[top-1] == NOT) {
        ENSURE_BUILD(ast_build_filter_not(b));
      } else if (stack[top-1] == AND) {
        ENSURE_BUILD(ast_build_filter_and(b));
      } else if (stack[top-1] == OR) {
        ENSURE_BUILD(ast_build_filter_or(b));
      }
    }
    if (!top) {
      error(ctx, "No maching opening parenthesis.");
      return 1;
    }
    --top;
    advance(ctx, 0);
    goto want_operator;

  } else if (ctx->token == LEX_BANG || ctx->token == LEX_PAREN_L) {
    error(ctx, "There seems to be a missing operator here.");
    return 1;

  } else {
    goto done;
  }

done:
  for (; top; --top) {
    if (stack[top-1] == NOT) {
      ENSURE_BUILD(ast_build_filter_not(b));
    } else if (stack[top-1] == AND) {
      ENSURE_BUILD(ast_build_filter_and(b));
    } else if (stack[top-1] == OR) {
      ENSURE_BUILD(ast_build_filter_or(b));
    } else if (stack[top-1] == LPAREN) {
      error(ctx, "No maching closing parenthesis.");
    }
  }
  ENSURE_BUILD(ast_build_filter(b));

  return 0;
}

/**
 * Parses the introduction of a new symbol name.
 */
int parse_symbol_new(Context *ctx, AstBuilder *b)
{
  String symbol;

  if (ctx->token != LEX_IDENTIFIER) {
    error(ctx, "Expecting a new symbol name here.");
    return 1;
  }
  symbol = string_init(ctx->marker.p, ctx->cursor.p);

  ENSURE_BUILD(ast_build_symbol_new(b, symbol));
  return 0;
}
