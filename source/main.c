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
int main_generate(Pool *pool, ListNode *code, Options *opt)
{
  String filename = string_copy(pool, opt->name_out);
  FILE *file_out = fopen(filename.p, "wb");
  if (!file_out) {
    fprintf(stderr, "error: Could not open output file \"%s\"\n", filename.p);
    return 1;
  }

  CHECK(generate_code(pool, file_out, code));

  fclose(file_out);
  return 1;
}

/**
 * Program entry point. Constructs and launches the main program object.
 */
int main(int argc, char *argv[])
{
  Pool pool = pool_init(0x10000); /* 64K block size */
  Options opt = options_init();
  Source *in;
  Scope scope = scope_init(0);
  ListBuilder code = list_builder_init(&pool);

  /* Read the options: */
  if (!options_parse(&opt, argc, argv)) {
    options_usage(argv[0]);
    goto error;
  }

  /* Determine output file name: */
  if (!string_size(opt.name_out)) {
    if (string_rmatch(opt.name_in, string_from_k(".ol")) != 3) {
      fprintf(stderr, "error: If no output file is specified, the input file name must end with \".ol\".\n");
      goto error;
    }
    opt.name_out = string(opt.name_in.p, opt.name_in.end - 3);
  }

  /* Input stream: */
  in = source_load(&pool, opt.name_in);
  if (!in) {
    fprintf(stderr, "error: Could not open source file \"%s\"\n", opt.name_in.p);
    goto error;
  }

  /* Keywords: */
  scope_add(&scope, &pool, string_from_k("macro"), dynamic(type_keyword,
    keyword_new(&pool, parse_macro)));
  scope_add(&scope, &pool, string_from_k("outline"), dynamic(type_keyword,
    keyword_new(&pool, parse_outline)));
  scope_add(&scope, &pool, string_from_k("union"), dynamic(type_keyword,
    keyword_new(&pool, parse_union)));
  scope_add(&scope, &pool, string_from_k("map"), dynamic(type_keyword,
    keyword_new(&pool, parse_map)));
  scope_add(&scope, &pool, string_from_k("for"), dynamic(type_keyword,
    keyword_new(&pool, parse_for)));
  scope_add(&scope, &pool, string_from_k("include"), dynamic(type_keyword,
    keyword_new(&pool, parse_include)));

  /* Do outline2c stuff: */
  if (!parse_code(&pool, in, &scope, out_list_builder(&code))) goto error;
  if (opt.debug) {
    printf("--- AST: ---\n");
    dump_code(code.first, 0);
    printf("\n");
  }
  if (!main_generate(&pool, code.first, &opt)) goto error;

  /* Clean up: */
  pool_free(&pool);
  return 0;

error:
  pool_free(&pool);
  return 1;
}
