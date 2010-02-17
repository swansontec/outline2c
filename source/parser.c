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

#include "parser.h"
#include "outline.h"
#include "match.h"
#include "lexer.h"
#include "file.h"
#include "string.h"
#include "debug.h"
#include "file.h"
#include <stdio.h>

int parse_source_file(Context *ctx);
int parse_top(Context *ctx);

int parse_include(Context *ctx);

int parse_outline(Context *ctx);
int parse_outline_node(Context *ctx, OutlineBuilder *b);

int parse_match_top(Context *ctx);
int parse_match(Context *ctx, Match **match, Match *outer);
int parse_match_line(Context *ctx, Match **match, Match *outer);
int parse_match_entry(Context *ctx, MatchBuilder *b);

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
 * Prints an informative message.
 */
void
info(Context *ctx, char const *message)
{
#ifdef DEBUG
  printf("%s:%d: info: %s\n", ctx->filename, ctx->marker.line + 1, message);
#endif
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
 * Prepares a fresh context structure.
 */
Context context_init(String file, char const *filename, FileW *out)
{
  static Outline root = {0};

  Context ctx;
  ctx.file = file;
  ctx.filename = filename;
  ctx.out = out;

  ctx.cursor = cursor_init(file.p);
  outline_builder_init(&ctx.root, &root);
  return ctx;
}

/**
 * Reads the next token, filtering out atmosphere and converting token types.
 */
static void advance(Context *ctx, int want_space)
{
  /* Advance and eat spaces, if requested: */
  do {
    ctx->marker = ctx->cursor;
    ctx->token = lex(&ctx->cursor, ctx->file.end);
  } while (!want_space &&
    (ctx->token == LEX_WHITESPACE || ctx->token == LEX_COMMENT));
}

int
parser_start(String aIn, char const *filename, FileW *aOut)
{
  int rv;

  Context ctx = context_init(aIn, filename, aOut);
  rv = parse_source_file(&ctx);
  ENSURE_SUCCESS(rv);

#ifdef DEBUG
  printf("--- Outline: ---\n");
  outline_dump(ctx.root.outline, 0);
#endif
  return 0;
}

/**
 * Processes a source file, writing its contents to the output file. This file
 * should be called with the cursor pointing to the start of the file; unlike
 * the other parser methods, it is incorrect to advance the cursor before
 * calling.
 */
int parse_source_file(Context *ctx)
{
  int rv;
  char const *start;

  start = ctx->cursor.p;
  do {
    advance(ctx, 1);
    /* Escape code: */
    if (ctx->token == LEX_ESCAPE_O2C) {
      /* Write the input so far: */
      rv = file_w_write(ctx->out, start, ctx->marker.p);
      if (rv) {
        printf("Error writing to the output file.");
        return 1;
      }

      /* Process the next token: */
      info(ctx, "Got escape code");
      advance(ctx, 0);
      rv = parse_top(ctx);
      ENSURE_SUCCESS(rv);
      start = ctx->cursor.p;
    }
  } while (ctx->token != LEX_END);

  /* Write the remaining input: */
  rv = file_w_write(ctx->out, start, ctx->cursor.p);
  if (rv) {
    printf("Error writing to the output file.");
    return 1;
  }
  return 0;
}

/**
 * Handles the bit right after an @o2c escape code.
 */
int parse_top(Context *ctx)
{
  int rv;

  /* Unexpected end of input: */
  if (ctx->token == LEX_END) {
    printf("Unexpected end of input.");
    return 1;
  /* Nested {} block: */
  } else if (ctx->token == LEX_BRACE_OPEN) {
    advance(ctx, 0);
    while (ctx->token != LEX_BRACE_CLOSE) {
      rv = parse_top(ctx);
      ENSURE_SUCCESS(rv);
      advance(ctx, 0);
    }
    return 0;
  /* Keywords: */
  } else if (ctx->token == LEX_IDENTIFIER) {
    String temp = string_init(ctx->marker.p, ctx->cursor.p);
    if (string_equal(temp, string_init_l("include", 7))) {
      info(ctx, "Got include rule");
      advance(ctx, 0);
      return parse_include(ctx);
    } else if (string_equal(temp, string_init_l("outline", 7))) {
      info(ctx, "Got outline rule");
      advance(ctx, 0);
      return parse_outline(ctx);
    } else if (string_equal(temp, string_init_l("match", 5))) {
      info(ctx, "Got match rule");
      advance(ctx, 0);
      return parse_match_top(ctx);
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
int parse_include(Context *ctx)
{
  int rv;
  Context c;
  char *filename;
  FileR file;

  if (ctx->token != LEX_STRING) {
    error(ctx, "An include statment expects a quoted filename.");
    return 1;
  }

  /* Prepare a context for the new file: */
  filename = string_to_c(string_init(ctx->marker.p + 1, ctx->cursor.p - 1));
  rv = file_r_open(&file, filename);
  if (rv) {
    error(ctx, "Could not open the included file.");
    return 1;
  }
  c.file = string_init(file.p, file.end);
  c.filename = filename;
  c.out = ctx->out;
  c.cursor = cursor_init(file.p);
  c.root = ctx->root;
#ifdef DEBUG
  printf("The filename is: \"%s\"\n", filename);
#endif

  /* Parse the input file: */
  advance(&c, 0);
  while (c.token != LEX_END) {
    rv = parse_top(&c);
    ENSURE_SUCCESS(rv);
    advance(&c, 0);
  }

  /* Free the context stuff: */
  free(filename);
  /* TODO: Closing the file would invalidate all string pointers in the AST.
   find some better way to handle this problem. */
/*  file_r_close(&file); */
  ctx->root = c.root;

  advance(ctx, 0);
  if (ctx->token != LEX_SEMICOLON) {
    error(ctx, "An include stament must end with a semicolon.");
    return 1;
  }
  return 0;
}

/**
 * Handles the outline keyword.
 */
int parse_outline(Context *ctx)
{
  int rv;

  /* Multiple outlines between braces: */
  if (ctx->token == LEX_BRACE_OPEN) {
    advance(ctx, 0);
    while (ctx->token != LEX_BRACE_CLOSE) {
      OutlineBuilder temp;
      rv = outline_builder_init_new(&temp);
      ENSURE_MEMORY(!rv);
      rv = parse_outline_node(ctx, &temp);
      ENSURE_SUCCESS(rv);
      outline_builder_insert_outline(&ctx->root, temp.outline);
      advance(ctx, 0);
    }
    return 0;
  /* Single outline: */
  } else {
    OutlineBuilder temp;
    rv = outline_builder_init_new(&temp);
    ENSURE_MEMORY(!rv);
    rv = parse_outline_node(ctx, &temp);
    ENSURE_SUCCESS(rv);
    outline_builder_insert_outline(&ctx->root, temp.outline);
    return 0;
  }
}

/**
 * Handles individual entries within an outline statement.
 */
int parse_outline_node(Context *ctx, OutlineBuilder *b)
{
  int rv;

  /* Handle the works making up the node: */
  while (ctx->token == LEX_IDENTIFIER || ctx->token == LEX_STRING) {
    rv = outline_builder_add_word(b, ctx->marker.p, ctx->cursor.p);
    ENSURE_MEMORY(!rv);
    advance(ctx, 0);
  }
  /* Handle any sub-nodes: */
  if (ctx->token == LEX_BRACE_OPEN) {
    advance(ctx, 0);
    while (ctx->token != LEX_BRACE_CLOSE) {
      OutlineBuilder temp;
      rv = outline_builder_init_new(&temp);
      ENSURE_MEMORY(!rv);
      rv = parse_outline_node(ctx, &temp);
      ENSURE_SUCCESS(rv);
      outline_builder_insert_outline(b, temp.outline);
      advance(ctx, 0);
    }
    return 0;
  /* Otherwise, the node must end with a semicolon: */
  } else if (ctx->token == LEX_SEMICOLON) {
    return 0;
  /* Anything else is an error: */
  } else {
    error(ctx, "An outline can only end with a semicolon or an opening brace.");
    return 1;
  }
}

/**
 * Handles a top-level match keyword. This function calls parse_match to
 * perform the actual parsing. Then, it feeds the return value into the code-
 * generation algorithm.
 */
int parse_match_top(Context *ctx)
{
  int rv;
  Match *match = 0;

  rv = parse_match(ctx, &match, 0);
  ENSURE_SUCCESS(rv);

  if (match) {
#ifdef DEBUG
    match_dump(match, 0);
#endif
    match_search(match, ctx->root.outline->children, ctx->out);
  }
  return 0;
}

/**
 * Parses the match keyword. The plan, here, is to build a list of match
 * structures representing the entries in the pattern. Then, return the list to
 * the caller.
 */
int parse_match(Context *ctx, Match **match, Match *outer)
{
  int rv;

  /* Multiple matches between braces: */
  if (ctx->token == LEX_BRACE_OPEN) {
    advance(ctx, 0);
    while (ctx->token != LEX_BRACE_CLOSE) {
      rv = parse_match_line(ctx, match, outer);
      ENSURE_SUCCESS(rv);
      advance(ctx, 0);
    }
    return 0;
  /* Single match: */
  } else {
    rv = parse_match_line(ctx, match, outer);
    ENSURE_SUCCESS(rv);
    return 0;
  }
}

/**
 * Parses a line within a match statement. A line must have a pattern and a
 * code block.
 */
int parse_match_line(Context *ctx, Match **match, Match *outer)
{
  int rv;
  MatchBuilder b;

  rv = match_builder_init(&b, outer);
  ENSURE_MEMORY(!rv);
  rv = parse_match_entry(ctx, &b);
  ENSURE_SUCCESS(rv);
  b.match->next = *match;
  *match = b.match;

  return 0;
}

/**
 * Handles individual entries within an outline statement.
 */
int parse_match_entry(Context *ctx, MatchBuilder *b)
{
  int rv;
  char const *start;
  int indent;

  do {
    /* A literal word to match: */
    if (ctx->token == LEX_IDENTIFIER) {
      rv = match_builder_add_pattern_word(b, ctx->marker.p, ctx->cursor.p);
      ENSURE_MEMORY(!rv);
      advance(ctx, 0);
    /* A word replacement to match: */
    } else if (ctx->token == LEX_LESS) {
      advance(ctx, 0);
      if (ctx->token == LEX_IDENTIFIER) {
        rv = match_builder_add_pattern_replace(b, ctx->marker.p, ctx->cursor.p);
        ENSURE_MEMORY(!rv);
        advance(ctx, 0);
        if (ctx->token != LEX_GREATER) {
          error(ctx, "A replacement rule must end with a > character.");
          return 1;
        }
      } else if (ctx->token == LEX_GREATER) {
        rv = match_builder_add_pattern_replace(b, 0, 0);
        ENSURE_MEMORY(!rv);
      }
      advance(ctx, 0);
    /* Bad closing character: */
    } else if (ctx->token == LEX_BRACE_CLOSE || ctx->token == LEX_SEMICOLON) {
      error(ctx, "A match rule must end with a code block");
      return 1;
    } else if (ctx->token != LEX_BRACE_OPEN) {
      error(ctx, "A match rule can only contain words and wildcards.");
      return 1;
    }
  } while (ctx->token != LEX_BRACE_OPEN);

  /* At this point, the match pattern is now stored as a linked list. The next
   * step is to process the source code block: */
  start = ctx->cursor.p;
  indent = 1;
  do {
    advance(ctx, 1);
    /* Sub-match expression: */
    if (ctx->token == LEX_ESCAPE_O2C) {
      rv = match_builder_add_code_code(b, start, ctx->marker.p);
      ENSURE_MEMORY(!rv);

      advance(ctx, 0);
      {
        String temp = string_init(ctx->marker.p, ctx->cursor.p);
        if (!string_equal(temp, string_init_l("match", 5))) {
          error(ctx, "Only the match keyword is allowed within code blocks.");
          return 1;
        }
      }
      advance(ctx, 0);
      {
        Match *match;
        rv = parse_match(ctx, &match, b->match);
        ENSURE_SUCCESS(rv);
        rv = match_builder_add_code_match(b, match);
        ENSURE_MEMORY(!rv);
      }
      start = ctx->cursor.p;
    /* A symbol to substitute: */
    } else if (ctx->token == LEX_IDENTIFIER) {
      String *p = match_find_symbol(b->match, string_init(ctx->marker.p, ctx->cursor.p));
      if (p) {
        rv = match_builder_add_code_code(b, start, ctx->marker.p);
        ENSURE_MEMORY(!rv);
        rv = match_builder_add_code_replace(b, p);
        ENSURE_MEMORY(!rv);
        start = ctx->cursor.p;
      }
    /* Opening brace: */
    } else if (ctx->token == LEX_BRACE_OPEN) {
      ++indent;
    /* Closing brace: */
    } else if (ctx->token == LEX_BRACE_CLOSE) {
      --indent;
    /* Unexpected end of file: */
    } else if (ctx->token == LEX_END) {
      error(ctx, "Unexpected end of input in match code block.");
      return 1;
    }
    /* Anything else is C code; continue with the loop. */
  } while (indent);
  rv = match_builder_add_code_code(b, start, ctx->marker.p);
  ENSURE_MEMORY(!rv);

  return 0;
}
