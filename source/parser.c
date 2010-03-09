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
 * up a tree of "@o2c outline" statements it finds. When it encouters an "@o2c
 * match" statement, it builds a temporary tree describing the match. Then, it
 * immediately processes the tree against the outline, writing the generated
 * code to the output file.
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

/*#define DEBUG */

#include "parser.h"
#include "lexer.h"
#include "ast-builder.h"
#include "search.h"
#include "debug.h"
#include "file.h"
#include "string.h"
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

int parse_file(char const *filename, AstBuilder *b);
int parse_code(Context *ctx, AstBuilder *b, int scoped);
int parse_escape(Context *ctx, AstBuilder *b);

int parse_include(Context *ctx, AstBuilder *b);

int parse_outline(Context *ctx, AstBuilder *b);
int parse_outline_list(Context *ctx, AstBuilder *b);
int parse_outline_item(Context *ctx, AstBuilder *b);

int parse_match(Context *ctx, AstBuilder *b);
int parse_match_line(Context *ctx, AstBuilder *b);

int parse_pattern(Context *ctx, AstBuilder *b);
int parse_pattern_item(Context *ctx, AstBuilder *b);

/**
 * All parser functions return 0 for success and non-zero for failure. This
 * macro checks a return code and bails out if it indicates an error.
 */
#define ENSURE_SUCCESS(rv) do { if (rv) return rv; } while(0)

/**
 * Verifies that a pointer is non-null. If the pointer is null, prints an error
 * message and bails.
 */
#define ENSURE_MEMORY(p) do { if (!p) { printf("Out of memory!\n"); return 1; } } while(0)

/**
 * Verifies that a call to the AstBuilder succeeded.
 */
#define ENSURE_BUILD(b) do { if (b) { printf("Out of memory!\n"); return 1; } } while(0)

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
  ctx.token = LEX_WHITESPACE;
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

int
parser_start(String file, char const *filename, FileW *out)
{
  int rv;

  Context ctx = context_init(file, filename);
  AstBuilder b;
  rv = ast_builder_init(&b);
  ENSURE_MEMORY(!rv);

  rv = parse_code(&ctx, &b, 0);
  ENSURE_SUCCESS(rv);

  {
    AstOutlineList list;
    AstCode *code = ast_builder_pop(&b).p;
    outline_list_from_file(&list, (AstNode*)code->nodes, (AstNode*)code->nodes_end);
    ast_code_generate(code, &list, out);
    outline_list_free(&list);
#ifdef DEBUG
  printf("--- Outline: ---\n");
    AstCodeNode *node;
    for (node = code->nodes; node < code->nodes_end; ++node) {
      if (node->type == AST_OUTLINE)
        dump_outline(node->p);
    }
#endif
  }

  ast_builder_free(&b);
  return 0;
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
 * and replacement keywords. This function should be called without advancing
 * the cursor to the first token, as is normal for the other parser methods.
 * @param scoped If this parameter is non-zero, the parser will stop at the
 * first unbalanced }. Otherwise, the parser will stop at the end of the file.
 */
int parse_code(Context *ctx, AstBuilder *b, int scoped)
{
  int rv;
  int indent = 1;
  size_t node_n = 0;
  char const *start;

  start = ctx->cursor.p;
  while (ctx->token != LEX_END && indent) {
    advance(ctx, 1);
    /* Sub-match expression: */
    if (ctx->token == LEX_ESCAPE_O2C) {
      /* Write the input so far: */
      ENSURE_BUILD(ast_build_code_text(b, string_init(start, ctx->marker.p))); ++node_n;

      /* Process the expression: */
      advance(ctx, 0);
      rv = parse_escape(ctx, b); ++node_n;
      ENSURE_SUCCESS(rv);
      start = ctx->cursor.p;

    /* Possibly a symbol to substitute: */
    } else if (ctx->token == LEX_IDENTIFIER) {
      AstPatternAssign *p = ast_builder_find_assign(b, string_init(ctx->marker.p, ctx->cursor.p));
      if (p) {
        ENSURE_BUILD(ast_build_code_text(b, string_init(start, ctx->marker.p))); ++node_n;
        ENSURE_BUILD(ast_build_symbol(b, p)); ++node_n;
        start = ctx->cursor.p;
      }
    /* Opening brace: */
    } else if (scoped && ctx->token == LEX_BRACE_OPEN) {
      ++indent;
    /* Closing brace: */
    } else if (scoped && ctx->token == LEX_BRACE_CLOSE) {
      --indent;
    }
    /* Anything else is C code; continue with the loop. */
  }
  if (scoped && ctx->token == LEX_END) {
    error(ctx, "Unexpected end of input in code block.");
    return 1;
  }
  ENSURE_BUILD(ast_build_code_text(b, string_init(start, ctx->marker.p))); ++node_n;
  ENSURE_BUILD(ast_build_code(b, node_n));

  return 0;
}

/**
 * Handles the bit right after an @o2c escape code.
 */
int parse_escape(Context *ctx, AstBuilder *b)
{
  int rv;

  /* Unexpected end of input: */
  if (ctx->token == LEX_END) {
    error(ctx, "Unexpected end of input.");
    return 1;
  /* Nested {} block: */
  } else if (ctx->token == LEX_BRACE_OPEN) {
    advance(ctx, 0);
    while (ctx->token != LEX_BRACE_CLOSE) {
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
    } else if (string_equal(temp, string_init_l("match", 5))) {
      advance(ctx, 0);
      return parse_match(ctx, b);
    } else {
      error(ctx, "No idea what this keyword is.\n");
      return 1;
    }
  } else {
    error(ctx, "No idea what token this is.\n");
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
 * Handles the top-level outline keyword.
 */
int parse_outline(Context *ctx, AstBuilder *b)
{
  int rv;
  String name;

  /* Outline name: */
  if (ctx->token != LEX_IDENTIFIER) {
    error(ctx, "An outline stament must begin with a name.");
    return 1;
  }
  name = string_init(ctx->marker.p, ctx->cursor.p);
  advance(ctx, 0);

  /* Opening brace: */
  if (ctx->token != LEX_BRACE_OPEN) {
    error(ctx, "An opening { must come after the name of an outline.");
    return 1;
  }

  /* Outline: */
  rv = parse_outline_list(ctx, b);
  ENSURE_SUCCESS(rv);
  ENSURE_BUILD(ast_build_outline(b, name));
  return 0;
}

/**
 * Handles a list of outline items.
 */
int parse_outline_list(Context *ctx, AstBuilder *b)
{
  int rv;
  size_t child_n = 0;

  advance(ctx, 0);
  while (ctx->token != LEX_BRACE_CLOSE) {
    rv = parse_outline_item(ctx, b); ++child_n;
    ENSURE_SUCCESS(rv);
    advance(ctx, 0);
  }
  ENSURE_BUILD(ast_build_outline_list(b, child_n));

  return 0;
}

/**
 * Handles an individual item within an outline, including its tags and
 * children.
 */
int parse_outline_item(Context *ctx, AstBuilder *b)
{
  int rv;
  size_t item_n = 0;

  /* Handle the words making up the item: */
  while (1) {
    if (ctx->token == LEX_IDENTIFIER) {
      ENSURE_BUILD(ast_build_outline_symbol(b, string_init(ctx->marker.p, ctx->cursor.p))); ++item_n;
    } else if (ctx->token == LEX_STRING) {
      ENSURE_BUILD(ast_build_outline_string(b, string_init(ctx->marker.p, ctx->cursor.p))); ++item_n;
    } else if (ctx->token == LEX_NUMBER) {
      ENSURE_BUILD(ast_build_outline_number(b, string_init(ctx->marker.p, ctx->cursor.p))); ++item_n;
    } else {
      break;
    }
    advance(ctx, 0);
  }

  /* Handle any sub-items: */
  if (ctx->token == LEX_BRACE_OPEN) {
    rv = parse_outline_list(ctx, b);
    ENSURE_SUCCESS(rv);
    ENSURE_BUILD(ast_build_outline_item(b, item_n));
    return 0;
  /* Otherwise, the item must end with a semicolon: */
  } else if (ctx->token == LEX_SEMICOLON) {
    ENSURE_BUILD(ast_build_outline_item(b, item_n));
    return 0;
  /* Anything else is an error: */
  } else {
    error(ctx, "An outline can only end with a semicolon or an opening brace.");
    return 1;
  }
}

/**
 * Parses the match keyword. The plan, here, is to build a list of match
 * structures representing the entries in the pattern. Then, return the list to
 * the caller.
 */
int parse_match(Context *ctx, AstBuilder *b)
{
  int rv;

  /* Multiple matches between braces: */
  if (ctx->token == LEX_BRACE_OPEN) {
    size_t lines = 0;
    advance(ctx, 0);
    while (ctx->token != LEX_BRACE_CLOSE) {
      rv = parse_match_line(ctx, b);
      ENSURE_SUCCESS(rv);
      advance(ctx, 0);
      ++lines;
    }
    ENSURE_BUILD(ast_build_match(b, lines));
    return 0;
  /* Single match: */
  } else {
    rv = parse_match_line(ctx, b);
    ENSURE_SUCCESS(rv);
    ENSURE_BUILD(ast_build_match(b, 1));
    return 0;
  }
}

/**
 * Parses a line within a match statement. A line must have a pattern and a
 * code block.
 */
int parse_match_line(Context *ctx, AstBuilder *b)
{
  int rv;

  /* Parse the pattern: */
  rv = parse_pattern(ctx, b);
  ENSURE_SUCCESS(rv);

  /* Check for bad closing characters: */
  if (ctx->token == LEX_BRACE_CLOSE || ctx->token == LEX_SEMICOLON) {
    error(ctx, "A match rule must end with a code block");
    return 1;
  } else if (ctx->token != LEX_BRACE_OPEN) {
    error(ctx, "A match rule can only contain words and wildcards.");
    return 1;
  }

  /* Parse the code: */
  rv = parse_code(ctx, b, 1);
  ENSURE_SUCCESS(rv);

  ENSURE_BUILD(ast_build_match_line(b));
  return 0;
}

/**
 * Handles pattern syntax. The context will be advanced when this function
 * returns.
 */
int parse_pattern(Context *ctx, AstBuilder *b)
{
  int rv;
  size_t node_n = 0;

  while (1) {
    rv = parse_pattern_item(ctx, b);
    if (rv == -1) {
      ENSURE_BUILD(ast_build_pattern(b, node_n));
      return 0;
    } else if (rv) {
      return rv;
    } else {
      ++node_n;
    }
  }
}

/**
 * Parses a single node within a match pattern. The context will be advanced
 * when this function returns.
 * @return 0 for success, -1 for unknown symbol, 1 for definite errors
 */
int parse_pattern_item(Context *ctx, AstBuilder *b)
{
  int rv;

  /* An identifier could be either a terminal symbol or an assignmen rule: */
  if (ctx->token == LEX_IDENTIFIER) {
    String s = string_init(ctx->marker.p, ctx->cursor.p);
    advance(ctx, 0);
    /* Replacement rule: */
    if (ctx->token == LEX_EQUALS) {
      advance(ctx, 0);
      rv = parse_pattern_item(ctx, b);
      if (rv) {
        error(ctx, "A replacement rule must end with a sub-pattern.");
        return 1;
      }
      if (ast_builder_peek(b).type == AST_PATTERN_ASSIGN) {
        error(ctx, "A replacement rule cannot contain another replacement rule.");
        return 1;
      }
      ENSURE_BUILD(ast_build_pattern_assign(b, s));
      return 0;
    }
    /* Literal: */
    ENSURE_BUILD(ast_build_pattern_symbol(b, s));
    return 0;
  }

  /* A rule invocation is surrounded by <>: */
  if (ctx->token == LEX_LESS) {
    advance(ctx, 0);
    if (ctx->token == LEX_GREATER) {
      ENSURE_BUILD(ast_build_pattern_wild(b));
      advance(ctx, 0);
      return 0;
    } else if (ctx->token == LEX_IDENTIFIER) {
      error(ctx, "Rule invocations are not supported at this time.");
      return 1;
      /*
      rv = pattern_builder_add_replace(b, ctx->marker.p, ctx->cursor.p);
      ENSURE_MEMORY(!rv);
      advance(ctx, 0);
      if (ctx->token != LEX_GREATER) {
        error(ctx, "A rule invocation must end with a > character.");
        return 1;
      }
      advance(ctx, 0);
      return 0;
      */
    } else {
      error(ctx, "A rule name should appear here.");
      return 1;
    }
  }

  return -1;
}
