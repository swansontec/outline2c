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
#include "file.h"
#include <stdio.h>
#include <assert.h>

/**
 * Opens the file given in filename, parses it, processes it, and writes the
 * results to the output file.
 */
int generate(FileW *out, char const *filename)
{
  int rv;
  int debug = 0;
  AstBuilder b;
  AstFile *file;
  Scope scope;

  rv = ast_builder_init(&b);
  if (rv) {
    fprintf(stderr, "Out of memory!\n");
    return 1;
  }

  rv = parse_file(filename, &b);
  if (rv) return rv;
  file = ast_to_file(ast_builder_pop(&b));

  if (debug) {
    printf("--- AST: ---\n");
    dump_code(file->code, 0);
  }

  scope = scope_init(file->code, 0, 0);
  generate_code(out, &scope, file->code);

  ast_builder_free(&b);
  return 0;
}

/**
 * Processes source code, writing the result to the output file.
 */
int generate_code(FileW *out, Scope *s, AstCode *p)
{
  int rv;
  AstCodeNode *node;

  for (node = p->nodes; node != p->nodes_end; ++node) {
    if (node->type == AST_CODE_TEXT) {
      AstCodeText *p = node->p;
      rv = file_w_write(out, p->code.p, p->code.end);
      if (rv) return rv;
    } else if (node->type == AST_INCLUDE) {
    } else if (node->type == AST_OUTLINE) {
    } else if (node->type == AST_FOR) {
      rv = generate_for(out, s, node->p);
      if (rv) return rv;
    } else if (node->type == AST_SYMBOL) {
      rv = generate_symbol(out, s, node->p);
      if (rv) return rv;
    } else {
      assert(0);
    }
  }
  return 0;
}

/**
 * Performs code-generation for a for statement node
 */
int generate_for(FileW *out, Scope *s, AstFor *p)
{
  AstOutline *outline;
  AstOutlineItem **item;

  outline = scope_find_outline(s, p->in->name);
  if (!outline) {
    char *temp = string_to_c(p->in->name);
    fprintf(stderr, "Could not find outline %s.\n", temp);
    free(temp);
    return 1;
  }

  for (item = outline->children->items; item != outline->children->items_end; ++item) {
    if (!p->filter || test_filter(p->filter, *item)) {
      Scope scope = scope_init(p->code, s, *item);
      generate_code(out, &scope, p->code);
    }
  }

  return 0;
}

/**
 * Performs code-generation for a symbol node
 */
int generate_symbol(FileW *out, Scope *s, AstSymbol *p)
{
  int i;
  Scope *scope = s;
  AstOutlineTag *tag;

  for (i = 0; i < p->level; ++i)
    scope = scope->outer;

  tag = scope->item->tags_end[-1];
  file_w_write(out, tag->symbol.p, tag->symbol.end);
  return 0;
}
