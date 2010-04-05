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
  if (generate_code(out, &scope, file->code))
    return 1;

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
  if (string_size(p->in->name)) {
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
  if (p->in->reverse) {
    for (item = items->items_end - 1; item != items->items - 1; --item) {
      if (!p->filter || test_filter(p->filter, *item)) {
        Scope scope = scope_init(p->code, s, *item);
        if (generate_code(out, &scope, p->code))
          return 1;
      }
    }
  } else {
    for (item = items->items; item != items->items_end; ++item) {
      if (!p->filter || test_filter(p->filter, *item)) {
        Scope scope = scope_init(p->code, s, *item);
        if (generate_code(out, &scope, p->code))
          return 1;
        if (p->in->list && item != items->items_end - 1) {
          char c = ',';
          file_w_write(out, &c, &c + 1);
        }
      }
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
  int rv;

  /* If a tag satisfies the lookup, generate that: */
  rv = generate_lookup_tag(out, s, p);
  if (rv == 1) return 0;
  if (rv == -1) return 1;

  /* If that didn't work, try the built-in transforms: */
  if (generate_lookup_builtin(out, s, p))
    return 0;

  /* If that didn't work, go on to search for maps: */
  rv = generate_lookup_map(out, s, p);
  if (rv == 1) return 0;
  if (rv == -1) return 1;

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
 * Otherwise, the function returns 0. Returns -1 for errors.
 */
int generate_lookup_tag(FileW *out, Scope *s, AstLookup *p)
{
  AstOutlineItem *item = scope_get_item(s, p->symbol->level);
  AstOutlineTag **tag;

  for (tag = item->tags; tag != item->tags_end; ++tag) {
    if ((*tag)->value && string_equal((*tag)->name, p->name)) {
      if (generate_code(out, s, (*tag)->value))
        return -1;
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
  } else if (string_equal(p->name, string_init_l("lower", 5))) {
    return !generate_lower(out, scope_get_item(s, p->symbol->level)->name);
  } else if (string_equal(p->name, string_init_l("upper", 5))) {
    return !generate_upper(out, scope_get_item(s, p->symbol->level)->name);
  } else if (string_equal(p->name, string_init_l("camel", 5))) {
    return !generate_camel(out, scope_get_item(s, p->symbol->level)->name);
  } else if (string_equal(p->name, string_init_l("mixed", 5))) {
    return !generate_mixed(out, scope_get_item(s, p->symbol->level)->name);
  }

  return 0;
}

/**
 * Searches the current scope for map statements that match the lookup's name.
 * If one does, generate that and return 1. Otherwise, return 0. Returns -1
 * for errors.
 */
int generate_lookup_map(FileW *out, Scope *s, AstLookup *p)
{
  AstOutlineItem *item = scope_get_item(s, p->symbol->level);
  AstMap *map;
  AstMapLine **line;

  map = scope_find_map(s, p->name);
  if (!map)
    return 0;

  for (line = map->lines; line != map->lines_end; ++line) {
    if (test_filter((*line)->filter, item)) {
      if (generate_code(out, s, (*line)->code))
        return -1;
      return 1;
    }
  }

  {
    char *temp = string_to_c(p->name);
    fprintf(stderr, "Could not match against map \"%s\".\n", temp);
    free(temp);
  }
  return -1;
}

/**
 * Writes a string to the output file, converting it to lower_case
 */
int generate_lower(FileW *out, String s)
{
  String inner = strip_symbol(s);
  String word = scan_symbol(inner, inner.p);

  write_leading(out, s, inner);
  while (string_size(word)) {
    write_lower(out, word);
    word = scan_symbol(inner, word.end);
    if (string_size(word)) {
      char c = '_';
      file_w_write(out, &c, &c + 1);
    }
  }
  write_trailing(out, s, inner);

  return 0;
}

/**
 * Writes a string to the output file, converting it to UPPER_CASE
 */
int generate_upper(FileW *out, String s)
{
  String inner = strip_symbol(s);
  String word = scan_symbol(inner, inner.p);

  write_leading(out, s, inner);
  while (string_size(word)) {
    write_upper(out, word);
    word = scan_symbol(inner, word.end);
    if (string_size(word)) {
      char c = '_';
      file_w_write(out, &c, &c + 1);
    }
  }
  write_trailing(out, s, inner);

  return 0;
}

/**
 * Writes a string to the output file, converting it to CamelCase
 */
int generate_camel(FileW *out, String s)
{
  String inner = strip_symbol(s);
  String word = scan_symbol(inner, inner.p);

  write_leading(out, s, inner);
  while (string_size(word)) {
    write_cap(out, word);
    word = scan_symbol(inner, word.end);
  }
  write_trailing(out, s, inner);

  return 0;
}

/**
 * Writes a string to the output file, converting it to mixedCase
 */
int generate_mixed(FileW *out, String s)
{
  String inner = strip_symbol(s);
  String word = scan_symbol(inner, inner.p);

  write_leading(out, s, inner);
  if (string_size(word)) {
    write_lower(out, word);
    word = scan_symbol(inner, word.end);
  }
  while (string_size(word)) {
    write_cap(out, word);
    word = scan_symbol(inner, word.end);
  }
  write_trailing(out, s, inner);

  return 0;
}

/**
 * Removes the leading and trailing underscores from an identifier.
 */
String strip_symbol(String s)
{
  while (s.p < s.end && *s.p == '_')
    ++s.p;
  while (s.p < s.end && s.end[-1] == '_')
    --s.end;
  return s;
}

/**
 * Locates individual words within an indentifier. The identifier must have its
 * leading and trailing underscores stripped off before being passed to this
 * function. As always, the only valid symbols within an indentifier are
 * [_a-zA-Z0-9]
 * @param s the input string to break into words
 * @param p a pointer into string s, which marks the first character to begin
 * scanning at.
 * @return the next located word, or a null string upon reaching the end of the
 * input
 */
String scan_symbol(String s, char const *p)
{
  char const *start;

  /* Trim underscores between words: */
  while (p < s.end && *p == '_')
    ++p;
  if (p == s.end)
    return string_null();
  start = p;

  /* Numbers? */
  if ('0' <= *p && *p <= '9') {
    do {
      ++p;
    } while (p < s.end && '0' <= *p && *p <= '9');
    return string_init(start, p);
  }

  /* Lower-case letters? */
  if ('a' <= *p && *p <= 'z') {
    do {
      ++p;
    } while (p < s.end && 'a' <= *p && *p <= 'z');
    return string_init(start, p);
  }

  /* Upper-case letters? */
  if ('A' <= *p && *p <= 'Z') {
    do {
      ++p;
    } while (p < s.end && 'A' <= *p && *p <= 'Z');
    /* Did the last upper-case letter start a lower-case word? */
    if (p < s.end && 'a' <= *p && *p <= 'z') {
      --p;
      if (p == start) {
        do {
          ++p;
        } while (p < s.end && 'a' <= *p && *p <= 'z');
      }
    }
    return string_init(start, p);
  }

  /* Anything else is a bug */
  assert(0);
  return string_null();
}

/**
 * Writes leading underscores to a file, if any.
 * @param s the entire string, including leading and trailing underscores.
 * @param inner the inner portion of the string after underscores have been
 * stripped.
 * @return 0 for success
 */
int write_leading(FileW *out, String s, String inner)
{
  if (s.p != inner.p)
    return file_w_write(out, s.p, inner.p);
  return 0;
}

int write_trailing(FileW *out, String s, String inner)
{
  if (inner.end != s.end)
    return file_w_write(out, inner.end, s.end);
  return 0;
}

/**
 * Writes a word to a file in lower case.
 */
int write_lower(FileW *out, String s)
{
  char const *p;
  for (p = s.p; p != s.end; ++p) {
    char c = 'A' <= *p && *p <= 'Z' ? *p - 'A' + 'a' : *p;
    if (file_w_write(out, &c, &c + 1)) return 1;
  }
  return 0;
}

/**
 * Writes a word to a file in UPPER case.
 */
int write_upper(FileW *out, String s)
{
  char const *p;
  for (p = s.p; p != s.end; ++p) {
    char c = 'a' <= *p && *p <= 'z' ? *p - 'a' + 'A' : *p;
    if (file_w_write(out, &c, &c + 1)) return 1;
  }
  return 0;
}

/**
 * Writes a word to a file in Capitalized case.
 */
int write_cap(FileW *out, String s)
{
  char const *p;
  for (p = s.p; p != s.end; ++p) {
    char c = (p == s.p) ?
      ('a' <= *p && *p <= 'z' ? *p - 'a' + 'A' : *p) :
      ('A' <= *p && *p <= 'Z' ? *p - 'A' + 'a' : *p) ;
    if (file_w_write(out, &c, &c + 1)) return 1;
  }
  return 0;
}
