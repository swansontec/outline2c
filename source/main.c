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

/**
 * Performs code-generation into the output file given in the options.
 */
int generate(ListNode *code, Options *opt)
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

int main_context_init(Pool *pool, Source *in, Scope *scope, Options *opt)
{
  /* Pool: */
  CHECK_MEM(pool_init(pool, 0x10000)); /* 64K block size */

  /* Input stream: */
  if (!source_load(in, pool, opt->name_in)) {
    fprintf(stderr, "error: Could not open source file \"");
    fwrite(opt->name_in.p, string_size(opt->name_in), 1, stderr);
    fprintf(stderr, "\"\n");
    return 0;
  }

  /* Scope: */
  *scope = scope_init(0);

  /* Keywords: */
  CHECK(scope_add(scope, pool, string_init_k("macro"), TYPE_KEYWORD,
    keyword_new(pool, parse_macro)));
  CHECK(scope_add(scope, pool, string_init_k("outline"), TYPE_KEYWORD,
    keyword_new(pool, parse_outline)));
  CHECK(scope_add(scope, pool, string_init_k("union"), TYPE_KEYWORD,
    keyword_new(pool, parse_union)));
  CHECK(scope_add(scope, pool, string_init_k("map"), TYPE_KEYWORD,
    keyword_new(pool, parse_map)));
  CHECK(scope_add(scope, pool, string_init_k("for"), TYPE_KEYWORD,
    keyword_new(pool, parse_for)));
  CHECK(scope_add(scope, pool, string_init_k("include"), TYPE_KEYWORD,
    keyword_new(pool, parse_include)));

  return 1;
}

void main_context_free(Source *in, Pool *pool)
{
  pool_free(pool);
}

/**
 * Program entry point. Constructs and launches the main program object.
 */
int main(int argc, char *argv[])
{
  Options opt = options_init();
  Source in = {0};
  Pool pool = {0};
  Scope scope;
  ListBuilder code = list_builder_init(&pool);

  /* Read the options: */
  if (!options_parse(&opt, argc, argv)) {
    fprintf(stderr, "Usage: %s [-d] [-o output-file] <input-file>\n", argv[0]);
    return 1;
  }

  /* Determine output file name: */
  if (!string_size(opt.name_out)) {
    if (string_rmatch(opt.name_in, string_init_k(".ol")) != 3) {
      fprintf(stderr, "error: If no output file is specified, the input file name must end with \".ol\".\n");
      return 1;
    }
    opt.name_out = string_init(opt.name_in.p, opt.name_in.end - 3);
  }

  /* Do outline2c stuff: */
  if (!main_context_init(&pool, &in, &scope, &opt)) goto error;
  if (!parse_code(&pool, &in, &scope, out_list_builder(&code), 0)) goto error;
  if (opt.debug) {
    printf("--- AST: ---\n");
    dump_code(code.first, 0);
  }
  if (!generate(code.first, &opt)) goto error;

  main_context_free(&in, &pool);
  return 0;

error:
  main_context_free(&in, &pool);
  return 1;
}
