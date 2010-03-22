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
    } else if (node->type == AST_MAP) {
    } else if (node->type == AST_FOR) {
      rv = generate_for(out, s, node->p);
      if (rv) return rv;
    } else if (node->type == AST_SYMBOL) {
      rv = generate_symbol(out, s, node->p);
      if (rv) return rv;
    } else if (node->type == AST_LOOKUP) {
      rv = generate_lookup(out, s, node->p);
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
  AstOutlineList *items;
  AstOutlineItem **item;

  /* Find the outline list to process: */
  if (p->in->name.p) {
    AstOutline *outline = scope_find_outline(s, p->in->name);
    if (!outline) {
      char *temp = string_to_c(p->in->name);
      fprintf(stderr, "Could not find outline %s.\n", temp);
      free(temp);
      return 1;
    }
    items = outline->children;
  } else {
    if (!s->item) {
      fprintf(stderr, "There is no outer for loop.\n");
      return 1;
    }
    items = s->item->children;
  }
  if (!items)
    return 0;

  /* Process the list: */
  for (item = items->items; item != items->items_end; ++item) {
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
  AstOutlineItem *item = scope_get_item(s, p->level);
  file_w_write(out, item->name.p, item->name.end);
  return 0;
}

/**
 * Performs code-generation for a lookup node.
 */
int generate_lookup(FileW *out, Scope *s, AstLookup *p)
{
  /* If a tag satisfies the lookup, generate that: */
  if (generate_lookup_tag(out, s, p))
    return 0;

  /* If that didn't work, go try the built-in transforms: */
  if (generate_lookup_builtin(out, s, p))
    return 0;

  /* If that didn't work, go on to search for maps: */
  if (generate_lookup_map(out, s, p))
    return 0;

  {
    char *temp = string_to_c(p->name);
    fprintf(stderr, "Could not find a transform named %s.\n", temp);
    free(temp);
  }
  return 1;
}

/**
 * Searches for a tag with the specified name in an outline item. If the tag
 * exists and has a value, the function emits the value and returns 1.
 * Otherwise, the function returns 0.
 */
int generate_lookup_tag(FileW *out, Scope *s, AstLookup *p)
{
  AstOutlineItem *item = scope_get_item(s, p->symbol->level);
  AstOutlineTag **tag;

  for (tag = item->tags; tag != item->tags_end; ++tag) {
    if ((*tag)->value && string_equal((*tag)->name, p->name)) {
      generate_code(out, s, (*tag)->value);
      return 1;
    }
  }

  return 0;
}

/**
 * If the lookup name matches one of the built-in transformations, generate
 * that and return 1. Otherwise, return 0.
 */
int generate_lookup_builtin(FileW *out, Scope *s, AstLookup *p)
{
  if (string_equal(p->name, string_init_l("quote", 5))) {
    char q = '"';
    file_w_write(out, &q, &q + 1);
    generate_symbol(out, s, p->symbol);
    file_w_write(out, &q, &q + 1);
    return 1;
  }

  return 0;
}

/**
 * Searches the current scope for map statements that match the lookup's name.
 * If one does, generate that and return 1. Otherwise, return 0.
 */
int generate_lookup_map(FileW *out, Scope *s, AstLookup *p)
{
  AstOutlineItem *item = scope_get_item(s, p->symbol->level);
  AstMap *map;

  map = scope_find_map(s, p->name);
  if (map) {
    AstMapLine **line;
    for (line = map->lines; line != map->lines_end; ++line) {
      if (test_filter((*line)->filter, item)) {
        generate_code(out, s, (*line)->code);
        return 1;
      }
    }
  }

  return 0;
}
