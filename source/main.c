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

#include "parser.h"
#include "debug.h"
#include "generate.h"
#include <string.h>

typedef struct options Options;

/*
 * Holds outline2c command-line options.
 */
struct options
{
  int debug;
  String name_in;
  String name_out;
};

void options_init(Options *self)
{
  self->debug = 0;
  self->name_in = string_null();
  self->name_out = string_null();
}

/**
 * Processes the command-line options, filling in the members of the Options
 * structure corresponding to the switches.
 * @return 0 for failure
 */
int options_parse(Options *self, int argc, char *argv[])
{
  int arg = 1;

  while (arg < argc) {
    String s = string_from_c(argv[arg]);

    /* Debug: */
    if (!strcmp(argv[arg], "-d") || !strcmp(argv[arg], "--debug")) {
      self->debug = 1;

    /* Output filename: */
    } else if (!strcmp(argv[arg], "-o")) {
      ++arg;
      if (argc <= arg) return 0;
      self->name_out = string_from_c(argv[arg]);

    /* Output filename, smushed: */
    } else if (2 == string_match(s, string_init_l("-o", 2))) {
      self->name_out = string_init(s.p + 2, s.end);

    /* Input filename: */
    } else {
      if (string_size(self->name_in)) return 0;
      self->name_in = s;
    }
    ++arg;
  }
  return 1;
}

/**
 * Performs code-generation into the output file given in the options.
 */
int generate(AstCode *code, Options *opt)
{
  char *s;
  FILE *file_out;

  s = string_to_c(opt->name_out);
  CHECK_MEM(s);
  file_out = fopen(s, "wb");
  if (!file_out) {
    fprintf(stderr, "error: Could not open output file \"%s\"\n", s);
    free(s);
    return 1;
  }
  free(s);

  CHECK(generate_code(file_out, code));

  fclose(file_out);
  return 1;
}

int main_context_init(Context *ctx, Pool *pool, Options *opt)
{
  /* Pool: */
  CHECK_MEM(pool_init(pool, 0x10000)); /* 64K block size */
  ctx->pool = pool;

  /* Scope: */
  ctx->scope = 0;
  context_scope_push(ctx);
  CHECK(ctx->scope);

  /* Input stream: */
  ctx->filename = opt->name_in;
  ctx->file = string_load(ctx->filename);
  if (!string_size(ctx->file)) {
    fprintf(stderr, "error: Could not open source file \"");
    fwrite(ctx->filename.p, string_size(ctx->filename), 1, stderr);
    fprintf(stderr, "\"\n");
    return 0;
  }
  ctx->cursor = ctx->file.p;

  return 1;
}

void main_context_free(Context *ctx)
{
  string_free(ctx->file);
  if(ctx->pool) pool_free(ctx->pool);
}

/**
 * Program entry point. Constructs and launches the main program object.
 */
int main(int argc, char *argv[])
{
  Options opt;
  Context ctx = {0};
  Pool pool;

  /* Read the options: */
  options_init(&opt);
  if (!options_parse(&opt, argc, argv)) {
    fprintf(stderr, "Usage: %s [-d] [-o output-file] <input-file>\n", argv[0]);
    return 1;
  }

  /* Determine output file name: */
  if (!string_size(opt.name_out)) {
    if (string_rmatch(opt.name_in, string_init_l(".ol", 3)) != 3) {
      fprintf(stderr, "error: If no output file is specified, the input file name must end with \".ol\".\n");
      return 1;
    }
    opt.name_out = string_init(opt.name_in.p, opt.name_in.end - 3);
  }

  /* Do outline2c stuff: */
  if (!main_context_init(&ctx, &pool, &opt)) goto error;
  if (!parse_code(&ctx, 0)) goto error;
  if (opt.debug) {
    printf("--- AST: ---\n");
    dump_code(ast_to_code(ctx.out), 0);
  }
  if (!generate(ast_to_code(ctx.out), &opt)) goto error;

  main_context_free(&ctx);
  return 0;

error:
  main_context_free(&ctx);
  return 1;
}
