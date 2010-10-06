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
#include "filter.h"
#include <assert.h>

int generate_code_node(FILE *out, AstCodeNode node);
int generate_for(FILE *out, AstFor *p);
int generate_variable(FILE *out, AstVariable *p);
int generate_call(FILE *out, AstCall *p);
int generate_lookup(FILE *out, AstLookup *p);
int generate_lookup_tag(FILE *out, AstLookup *p);
int generate_lookup_builtin(FILE *out, AstLookup *p);

int generate_lower(FILE *out, String s);
int generate_upper(FILE *out, String s);
int generate_camel(FILE *out, String s);
int generate_mixed(FILE *out, String s);

String strip_symbol(String s);
String scan_symbol(String s, char const *p);
int write_leading(FILE *out, String s, String inner);
int write_trailing(FILE *out, String s, String inner);
int write_lower(FILE *out, String s);
int write_upper(FILE *out, String s);
int write_cap(FILE *out, String s);

/* Writes bytes to a file, and returns 0 for failure. */
#define file_write(file, p, end) (fwrite(p, 1, end - p, file) == end - p)
#define file_putc(file, c) (fputc(c, file) != EOF)

/**
 * Processes source code, writing the result to the output file.
 */
int generate_code(FILE *out, ListNode *node)
{
  for (; node; node = node->next)
    CHECK(generate_code_node(out, ast_to_code_node(*node)));
  return 1;
}

/**
 * Processes source code, writing the result to the output file.
 */
int generate_code_node(FILE *out, AstCodeNode node)
{
  if (node.type == AST_CODE_TEXT) {
    AstCodeText *p = node.p;
    CHECK(file_write(out, p->code.p, p->code.end));
  } else if (node.type == AST_FOR) {
    CHECK(generate_for(out, node.p));
  } else if (node.type == AST_VARIABLE) {
    CHECK(generate_variable(out, node.p));
  } else if (node.type == AST_CALL) {
    CHECK(generate_call(out, node.p));
  } else if (node.type == AST_LOOKUP) {
    CHECK(generate_lookup(out, node.p));
  } else {
    assert(0);
  }
  return 1;
}

/**
 * Performs code-generation for a for statement node
 */
int generate_for(FILE *out, AstFor *p)
{
  AstOutline *outline;
  int need_comma = 0;

  /* Find the outline list to process: */
  if (p->outline.type == AST_OUTLINE) {
    outline = p->outline.p;
  } else {
    AstVariable *v = p->outline.p;
    outline = v->value->children;
  }
  if (!outline)
    return 1;

  /* Process the list: */
  if (p->reverse) {
    /* Warning: O(n^2) reversing algorithm */
    ListNode *item, *last = 0;
    while (outline->items != last) {
      item = outline->items;
      while (item->next != last)
        item = item->next;
      last = item;

      p->item->value = ast_to_outline_item(*item);
      if (!p->filter || test_filter(p->filter, p->item->value)) {
        if (p->list && need_comma)
          CHECK(file_putc(out, ','));
        CHECK(generate_code(out, p->code));
        need_comma = 1;
      }
    }
  } else {
    ListNode *item;
    for (item = outline->items; item; item = item->next) {
      p->item->value = ast_to_outline_item(*item);
      if (!p->filter || test_filter(p->filter, p->item->value)) {
        if (p->list && need_comma)
          CHECK(file_putc(out, ','));
        CHECK(generate_code(out, p->code));
        need_comma = 1;
      }
    }
  }

  return 1;
}

/**
 * Performs code-generation for a variable lookup
 */
int generate_variable(FILE *out, AstVariable *p)
{
  AstOutlineItem *item = p->value;

  CHECK(file_write(out, item->name.p, item->name.end));
  return 1;
}

/**
 * Performs code-generation for a map call
 */
int generate_call(FILE *out, AstCall *p)
{
  AstOutlineItem *item = p->item->value;
  ListNode *line;
  char *temp;

  /* Match against the map: */
  for (line = p->map->lines; line; line = line->next) {
    AstMapLine *l = ast_to_map_line(*line);
    if (test_filter(l->filter, item)) {
      p->map->item->value = item;
      CHECK(generate_code(out, l->code));
      return 1;
    }
  }

  /* Nothing matched: */
  temp = string_to_c(p->item->value->name);
  fprintf(stderr, "error: Could not match item \"%s\" against map.\n", temp);
  free(temp);
  return 0;
}

/**
 * Performs code-generation for a lookup node.
 */
int generate_lookup(FILE *out, AstLookup *p)
{
  int rv;

  /* If a tag satisfies the lookup, generate that: */
  rv = generate_lookup_tag(out, p);
  if (rv == 1) return 1;
  if (rv == 0) return 0;

  /* If that didn't work, try the built-in transforms: */
  if (generate_lookup_builtin(out, p))
    return 1;

  {
    char *temp = string_to_c(p->name);
    fprintf(stderr, "Could not find a transform named %s.\n", temp);
    free(temp);
  }
  return 0;
}

/**
 * Searches for a tag with the specified name in an outline item. If the tag
 * exists and has a value, the function emits the value and returns 1.
 * Otherwise, the function returns -1. Returns 0 for errors.
 */
int generate_lookup_tag(FILE *out, AstLookup *p)
{
  AstOutlineItem *item = p->item->value;
  ListNode *tag;

  for (tag = item->tags; tag; tag = tag->next) {
    AstOutlineTag *t = ast_to_outline_tag(*tag);
    if (t->value && string_equal(t->name, p->name)) {
      CHECK(generate_code(out, t->value));
      return 1;
    }
  }

  return -1;
}

/**
 * If the lookup name matches one of the built-in transformations, generate
 * that and return 1. Otherwise, return 0.
 */
int generate_lookup_builtin(FILE *out, AstLookup *p)
{
  AstOutlineItem *item = p->item->value;

  if (string_equal(p->name, string_init_l("quote", 5))) {
    CHECK(file_putc(out, '"'));
    CHECK(file_write(out, item->name.p, item->name.end));
    CHECK(file_putc(out, '"'));
    return 1;
  } else if (string_equal(p->name, string_init_l("lower", 5))) {
    return generate_lower(out, item->name);
  } else if (string_equal(p->name, string_init_l("upper", 5))) {
    return generate_upper(out, item->name);
  } else if (string_equal(p->name, string_init_l("camel", 5))) {
    return generate_camel(out, item->name);
  } else if (string_equal(p->name, string_init_l("mixed", 5))) {
    return generate_mixed(out, item->name);
  }

  return 0;
}

/**
 * Writes a string to the output file, converting it to lower_case
 */
int generate_lower(FILE *out, String s)
{
  String inner = strip_symbol(s);
  String word = scan_symbol(inner, inner.p);

  write_leading(out, s, inner);
  while (string_size(word)) {
    write_lower(out, word);
    word = scan_symbol(inner, word.end);
    if (string_size(word))
      CHECK(file_putc(out, '_'));
  }
  write_trailing(out, s, inner);

  return 1;
}

/**
 * Writes a string to the output file, converting it to UPPER_CASE
 */
int generate_upper(FILE *out, String s)
{
  String inner = strip_symbol(s);
  String word = scan_symbol(inner, inner.p);

  write_leading(out, s, inner);
  while (string_size(word)) {
    write_upper(out, word);
    word = scan_symbol(inner, word.end);
    if (string_size(word))
      CHECK(file_putc(out, '_'));
  }
  write_trailing(out, s, inner);

  return 1;
}

/**
 * Writes a string to the output file, converting it to CamelCase
 */
int generate_camel(FILE *out, String s)
{
  String inner = strip_symbol(s);
  String word = scan_symbol(inner, inner.p);

  write_leading(out, s, inner);
  while (string_size(word)) {
    write_cap(out, word);
    word = scan_symbol(inner, word.end);
  }
  write_trailing(out, s, inner);

  return 1;
}

/**
 * Writes a string to the output file, converting it to mixedCase
 */
int generate_mixed(FILE *out, String s)
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

  return 1;
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
 * @return 0 for failure
 */
int write_leading(FILE *out, String s, String inner)
{
  if (s.p != inner.p)
    return file_write(out, s.p, inner.p);
  return 1;
}

int write_trailing(FILE *out, String s, String inner)
{
  if (inner.end != s.end)
    return file_write(out, inner.end, s.end);
  return 1;
}

/**
 * Writes a word to a file in lower case.
 */
int write_lower(FILE *out, String s)
{
  char const *p;
  for (p = s.p; p != s.end; ++p) {
    char c = 'A' <= *p && *p <= 'Z' ? *p - 'A' + 'a' : *p;
    CHECK(file_putc(out, c));
  }
  return 1;
}

/**
 * Writes a word to a file in UPPER case.
 */
int write_upper(FILE *out, String s)
{
  char const *p;
  for (p = s.p; p != s.end; ++p) {
    char c = 'a' <= *p && *p <= 'z' ? *p - 'a' + 'A' : *p;
    CHECK(file_putc(out, c));
  }
  return 1;
}

/**
 * Writes a word to a file in Capitalized case.
 */
int write_cap(FILE *out, String s)
{
  char const *p;
  for (p = s.p; p != s.end; ++p) {
    char c = (p == s.p) ?
      ('a' <= *p && *p <= 'z' ? *p - 'a' + 'A' : *p) :
      ('A' <= *p && *p <= 'Z' ? *p - 'A' + 'a' : *p) ;
    CHECK(file_putc(out, c));
  }
  return 1;
}
