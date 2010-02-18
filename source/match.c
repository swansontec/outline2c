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
 * An AST representation of the match keyword and its sub-elements.
 */

#include "match.h"
#include "string.h"
#include "outline.h"
#include "file.h"
#include <stdlib.h>

/**
 * Gets the "next" pointer from any type of code-generation template structure.
 */
Code code_get_next(Code code)
{
  Code nil = {0};
  switch (code.type) {
  case CODE_CODE:    return ((CodeCode*)code.p)->next;
  case CODE_REPLACE: return ((CodeReplace*)code.p)->next;
  case CODE_MATCH:   return ((CodeMatch*)code.p)->next;
  }
  return nil;
}

/**
 * Sets the "next" pointer on any type of code-generation template structure.
 */
void code_set_next(Code code, Code next)
{
  switch (code.type) {
  case CODE_CODE:    ((CodeCode *)code.p)->next = next;    break;
  case CODE_REPLACE: ((CodeReplace *)code.p)->next = next; break;
  case CODE_MATCH:   ((CodeMatch *)code.p)->next = next;   break;
  }
}

/**
 * Allocates and initializes a CodeCode structure.
 */
Code code_code_new(char const *p, char const *end)
{
  Code code = {0};
  CodeCode *temp = malloc(sizeof(CodeCode));
  if (!temp) return code;
  temp->p = p;
  temp->end = end;
  temp->next.p = 0;

  code.p = temp;
  code.type = CODE_CODE;
  return code;
}

/**
 * Allocates and initializes a CodeReplace structure.
 */
Code code_replace_new(String *symbol)
{
  Code code = {0};
  CodeReplace *temp = malloc(sizeof(CodeReplace));
  if (!temp) return code;
  temp->symbol = symbol;

  code.p = temp;
  code.type = CODE_REPLACE;
  return code;
}

/**
 * Allocates and initializes a CodeMatch structure.
 */
Code code_match_new(Match *match)
{
  Code code = {0};
  CodeMatch *temp = malloc(sizeof(CodeMatch));
  if (!temp) return code;
  temp->match = match;

  code.p = temp;
  code.type = CODE_MATCH;
  return code;
}

/**
 * Prepares a code builder structure
 */
void code_builder_init(CodeBuilder *b)
{
  b->first.p = 0;
  b->last.p = 0;
}

static void code_builder_append(CodeBuilder *b, Code code)
{
  if (b->last.p)
    code_set_next(b->last, code);
  else
    b->first = code;
  b->last = code;
}

int code_builder_add_code(CodeBuilder *b, char const *p, char const *end)
{
  Code temp = code_code_new(p, end);
  if (!temp.p) return 1;
  code_builder_append(b, temp);
  return 0;
}

int code_builder_add_replace(CodeBuilder *b, String *symbol)
{
  Code temp = code_replace_new(symbol);
  if (!temp.p) return 1;
  code_builder_append(b, temp);
  return 0;
}

int code_builder_add_match(CodeBuilder *b, Match *match)
{
  Code temp = code_match_new(match);
  if (!temp.p) return 1;
  code_builder_append(b, temp);
  return 0;
}

/**
 * Allocates and initializes an empty Match structure.
 */
Match *match_new(Match *outer)
{
  Match *temp = malloc(sizeof(Match));
  if (!temp) return 0;
  temp->pattern.p = 0;
  temp->code.p    = 0;
  temp->outer     = outer;
  temp->next      = 0;
  return temp;
}

/**
 * Finds a symbol within the match's replacement scope.
 */
String *match_find_symbol(Match *match, String symbol)
{
  Match *m = match;
  while (m) {
    Pattern pattern = m->pattern;
    while (pattern.p) {
      if (pattern.type == PATTERN_REPLACE) {
        PatternReplace *p = pattern.p;
        if (string_equal(p->symbol, symbol))
          return &p->symbol;
      }
      pattern = pattern_get_next(pattern);
    }
    m = m->outer;
  }
  return 0;
}

/**
 * Compares a match and its siblings against an outline and its siblings.
 * Generates code for the first match to to match against a particular outline.
 * @return 0 for success
 */
int match_search(Match *match, Outline *outline, FileW *file)
{
  int rv;

  while (outline) {
    Match *m = match;
    while (m) {
      if (pattern_compare(m->pattern, outline)) {
        rv = match_generate(m, outline->children, file);
        if (rv) return rv;
        break;
      }
      m = m->next;
    }
    outline = outline->next;
  }
  return 0;
}

/**
 * Generates code for a match block.
 * @return 0 for success
 */
int match_generate(Match *match, Outline *outline, FileW *file)
{
  int rv;
  char end[] = "\n";

  Code code = match->code;
  while (code.p) {
    if (code.type == CODE_CODE) {
      CodeCode *p = code.p;
      rv = file_w_write(file, p->p, p->end);
      if (rv) return rv;
    } else if (code.type == CODE_REPLACE) {
      CodeReplace *p = code.p;
      rv = file_w_write(file, p->symbol->p, p->symbol->end);
      if (rv) return rv;
    } if (code.type == CODE_MATCH) {
      CodeMatch *p = code.p;
      rv = match_search(p->match, outline, file);
      if (rv) return rv;
    }
    code = code_get_next(code);
  }
  file_w_write(file, end, end+1);
  return 0;
}
