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

#include "pattern.h"
#include "outline.h"
#include <stdlib.h>
#include <assert.h>

/**
 * Gets the "next" pointer from any type of pattern structure.
 */
Pattern pattern_get_next(Pattern pattern)
{
  switch (pattern.type) {
  case PATTERN_WILD:    return ((PatternWild *)pattern.p)->next;
  case PATTERN_WORD:    return ((PatternWord *)pattern.p)->next;
  case PATTERN_REPLACE: return ((PatternReplace *)pattern.p)->next;
  default: assert(0); return pattern;
  }
}

/**
 * Sets the "next" pointer on any type of pattern structure.
 */
void pattern_set_next(Pattern pattern, Pattern next)
{
  switch (pattern.type) {
  case PATTERN_WILD:    ((PatternWild *)pattern.p)->next = next;    break;
  case PATTERN_WORD:    ((PatternWord *)pattern.p)->next = next;    break;
  case PATTERN_REPLACE: ((PatternReplace *)pattern.p)->next = next; break;
  default: assert(0);
  }
}

/**
 * Compares a single pattern item against a single outline word
 */
int pattern_match(Pattern pattern, OutlineItem *o)
{
  switch (pattern.type) {
  case PATTERN_WILD:    return pattern_wild_match(pattern.p, o);
  case PATTERN_WORD:    return pattern_word_match(pattern.p, o);
  case PATTERN_REPLACE: return pattern_replace_match(pattern.p, o);
  default: assert(0); return 0;
  }
}

/**
 * Compares a pattern against an outline block.
 * @return non-zero if the two are equal
 */
int pattern_compare(Pattern pattern, Outline *outline)
{
  OutlineItem *word;

  word = outline->words;
  while (pattern.p && word) {
    if (!pattern_match(pattern, word))
      return 0;
    pattern = pattern_get_next(pattern);
    word = word->next;
  }

  return (!pattern.p && !word) ? 1 : 0;
}

/**
 * Allocates and initializes a PatternWild structure.
 */
Pattern pattern_wild_new()
{
  Pattern pattern = {0};
  PatternWild *temp = malloc(sizeof(PatternWild));
  if (!temp) return pattern;
  temp->next.p = 0;

  pattern.p = temp;
  pattern.type = PATTERN_WILD;
  return pattern;
}

int pattern_wild_match(PatternWild *p, OutlineItem *o)
{
  return 1;
}

/**
 * Allocates and initializes a PatternWord structure.
 */
Pattern pattern_word_new(char const *p, char const *end)
{
  Pattern pattern = {0};
  PatternWord *temp = malloc(sizeof(PatternWord));
  if (!temp) return pattern;
  temp->symbol = string_init(p, end);
  temp->next.p = 0;

  pattern.p = temp;
  pattern.type = PATTERN_WORD;
  return pattern;
}

int pattern_word_match(PatternWord *p, OutlineItem *o)
{
  return string_equal(p->symbol, string_init(o->p, o->end));
}

/**
 * Allocates and initializes a PatternReplace structure.
 */
Pattern pattern_replace_new(String symbol, Pattern pattern)
{
  Pattern item = {0};
  PatternReplace *temp = malloc(sizeof(PatternReplace));
  if (!temp) return item;
  temp->symbol = symbol;
  temp->pattern = pattern;
  temp->next.p = 0;

  item.p = temp;
  item.type = PATTERN_REPLACE;
  return item;
}

int pattern_replace_match(PatternReplace *p, OutlineItem *o)
{
  if (!pattern_match(p->pattern, o))
    return 0;
  p->symbol = string_init(o->p, o->end);
  return 1;
}

/**
 * Prepares a pattern builder structure
 */
void pattern_builder_init(PatternBuilder *b)
{
  b->first.p = 0;
  b->last.p = 0;
}

/**
 * Appends an item to the end of the pattern.
 */
static void pattern_builder_append(PatternBuilder *b, Pattern pattern)
{
  if (b->last.p)
    pattern_set_next(b->last, pattern);
  else
    b->first = pattern;
  b->last = pattern;
}

/**
 * Appends a new PatternWild structure to the end of the pattern.
 * @return 0 for success.
 */
int pattern_builder_add_wild(PatternBuilder *b)
{
  Pattern temp = pattern_wild_new();
  if (!temp.p) return 1;
  pattern_builder_append(b, temp);
  return 0;
}

/**
 * Appends a new PatternWord structure to the end of the pattern.
 * @return 0 for success.
 */
int pattern_builder_add_word(PatternBuilder *b, char const *p, char const *end)
{
  Pattern temp = pattern_word_new(p, end);
  if (!temp.p) return 1;
  pattern_builder_append(b, temp);
  return 0;
}

/**
 * Appends a new PatternReplace structure to the end of the pattern.
 * @return 0 for success.
 */
int pattern_builder_add_replace(PatternBuilder *b, String symbol, Pattern pattern)
{
  Pattern temp = pattern_replace_new(symbol, pattern);
  if (!temp.p) return 1;
  pattern_builder_append(b, temp);
  return 0;
}
