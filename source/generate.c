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

#include "generate.h"
#include "parser.h"
#include "ast-builder.h"
#include "search.h"
#include "debug.h"
#include <stdio.h>

#define DEBUG /* */

/**
 * Opens the file given in filename, parses it, processes it, and writes the
 * results to the output file.
 */
int generate(char const *filename, FileW *out)
{
  int rv;
  int debug = 0;
  AstBuilder b;
  AstFile *file;

  rv = ast_builder_init(&b);
  if (rv) {
    fprintf(stderr, "Out of memory!\n");
    return 1;
  }

  rv = parse_file(filename, &b);
  if (rv) return rv;
  file = ast_to_file(ast_builder_pop(&b));

  {
    AstOutlineList list;
    outline_list_from_file(&list, file);
    ast_code_generate(file->code, &list, out);
    outline_list_free(&list);
  }

  if (debug) {
    printf("--- AST: ---\n");
    dump_code(file->code, 0);
  }

  ast_builder_free(&b);
  return 0;
}
