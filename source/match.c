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
 * Gets the "next" pointer from any type of pattern structure.
 */
Pattern pattern_get_next(Pattern pattern)
{
  Pattern none = {0};
  switch (pattern.type) {
  case PATTERN_WORD:    return ((PatternWord *)pattern.p)->next;
  case PATTERN_REPLACE: return ((PatternReplace *)pattern.p)->next;
  }
  return none;
}

/**
 * Sets the "next" pointer on any type of pattern structure.
 */
void pattern_set_next(Pattern pattern, Pattern next)
{
  switch (pattern.type) {
  case PATTERN_WORD:    ((PatternWord *)pattern.p)->next = next;    break;
  case PATTERN_REPLACE: ((PatternReplace *)pattern.p)->next = next; break;
  }
}

/**
 * Allocates and initializes a PatternWord structure.
 */
Pattern pattern_word_new(char const *p, char const *end)
{
  Pattern pattern = {0};
  PatternWord *temp = malloc(sizeof(PatternWord));
  if (!temp) return pattern;
  temp->p = p;
  temp->end = end;
  temp->next.p = 0;

  pattern.p = temp;
  pattern.type = PATTERN_WORD;
  return pattern;
}

/**
 * Allocates and initializes a PatternReplace structure.
 */
Pattern pattern_replace_new(char const *p, char const *end)
{
  Pattern pattern = {0};
  PatternReplace *temp = malloc(sizeof(PatternReplace));
  if (!temp) return pattern;
  temp->p = p;
  temp->end = end;
  temp->next.p = 0;

  pattern.p = temp;
  pattern.type = PATTERN_REPLACE;
  return pattern;
}

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
 * Allocates and initializes an empty Match structure.
 */
Match *match_new(Match *outer)
{
  Match *temp = malloc(sizeof(Match));
  if (!temp) return 0;
  temp->pattern.p = 0;
  temp->pattern_n = 0;
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
        if (string_equal(string_init(p->p, p->end), symbol))
          return (String*)&p->p;
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
      if (match_compare(m, outline)) {
        rv = match_generate(m, outline->children, file);
        if (rv) return 1;
        break;
      }
      m = m->next;
    }
    outline = outline->next;
  }
  return 0;
}

/**
 * Compares a match pattern against an outline block.
 * @return non-zero if the two are equal
 */
int match_compare(Match *match, Outline *outline)
{
  Pattern pattern;
  OutlineWord *word;

  if (outline->word_n != match->pattern_n)
    return 0;

  pattern = match->pattern;
  word = outline->words;
  while (pattern.p) {
    if (pattern.type == PATTERN_WORD) {
      PatternWord *p = pattern.p;
      if (!string_equal(string_init(p->p, p->end), string_init(word->p, word->end)))
        return 0;
    } else if (pattern.type == PATTERN_REPLACE) {
      PatternReplace *p = pattern.p;
      p->p = word->p;
      p->end = word->end;
    }
    pattern = pattern_get_next(pattern);
    word = word->next;
  }

  return 1;
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

/**
 * Initializes a match builder, constructing the initial Match structure.
 * @return 0 for success.
 */
int match_builder_init(MatchBuilder *b, Match *outer)
{
  b->match = match_new(outer);
  if (!b->match) return 1;
  b->pattern_last.p = 0;
  b->code_last.p = 0;
  return 0;
}

/**
 * Appends an item to the end of the Match structure's pattern.
 */
static void match_builder_insert_pattern(MatchBuilder *b, Pattern pattern)
{
  if (b->pattern_last.p)
    pattern_set_next(b->pattern_last, pattern);
  else
    b->match->pattern = pattern;
  b->pattern_last = pattern;
}

/**
 * Appends a new PatternWord structure to the end of the Match structure's
 * pattern.
 * @return 0 for success.
 */
int match_builder_add_pattern_word(MatchBuilder *b, char const *p, char const *end)
{
  Pattern temp = pattern_word_new(p, end);
  if (!temp.p) return 1;
  match_builder_insert_pattern(b, temp);
  ++b->match->pattern_n;
  return 0;
}

/**
 * Appends a new PatternReplace structure to the end of the Match structure's
 * pattern.
 * @return 0 for success.
 */
int match_builder_add_pattern_replace(MatchBuilder *b, char const *p, char const *end)
{
  Pattern temp = pattern_replace_new(p, end);
  if (!temp.p) return 1;
  match_builder_insert_pattern(b, temp);
  ++b->match->pattern_n;
  return 0;
}

/**
 * Appends an item to the end of the Match structure's code-generation template.
 */
static void match_builder_insert_code(MatchBuilder *b, Code code)
{
  if (b->code_last.p)
    code_set_next(b->code_last, code);
  else
    b->match->code = code;
  b->code_last = code;
}

/**
 * Appends a new CodeCode structure to the end of the Match structure's code-
 * generation template.
 * @return 0 for success.
 */
int match_builder_add_code_code(MatchBuilder *b, char const *p, char const *end)
{
  Code temp;
  if (p == end)
    return 0;
  temp = code_code_new(p, end);
  if (!temp.p) return 1;
  match_builder_insert_code(b, temp);
  return 0;
}

/**
 * Appends a new CodeReplace structure to the end of the Match structure's
 * code-generation template.
 * @return 0 for success.
 */
int match_builder_add_code_replace(MatchBuilder *b, String *symbol)
{
  Code temp = code_replace_new(symbol);
  if (!temp.p) return 1;
  match_builder_insert_code(b, temp);
  return 0;
}

/**
 * Appends a new CodeMatch structure to the end of the Match structure's code-
 * generation template.
 * @return 0 for success.
 */
int match_builder_add_code_match(MatchBuilder *b, Match *match)
{
  Code temp = code_match_new(match);
  if (!temp.p) return 1;
  match_builder_insert_code(b, temp);
  return 0;
}
