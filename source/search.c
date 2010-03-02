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
 * This algorithm for comparing a pattern against an outline is very bad. This
 * is a parsing problem, and requires a general parsing algorithm to solve.
 */

#include "search.h"
#include "outline.h"
#include <assert.h>

int ast_pattern_item_match(AstPatternItem pattern, OutlineItem *o);

/**
 * Returns non-zero if a pattern has the specified symbol in its assignment
 * list.
 */
String *ast_pattern_has_assign(AstPattern *pattern, String symbol)
{
  AstPatternItem *item;

  item = pattern->items;
  while (item < pattern->items_end) {
    if (item->type == AST_PATTERN_ASSIGN) {
      AstPatternAssign *p = item->p;
      if (string_equal(p->symbol, symbol))
        return &p->symbol;
    }
    ++item;
  }
  return 0;
}

/**
 * Compares a pattern against an outline block.
 * @return non-zero if the two are equal
 */
int ast_pattern_compare(AstPattern *pattern, Outline *outline)
{
  AstPatternItem *item;
  OutlineItem *word;

  if (pattern->items_end - pattern->items != outline->word_n)
    return 0;

  item = pattern->items;
  word = outline->words;
  while (item < pattern->items_end) {
    if (!ast_pattern_item_match(*item, word))
      return 0;
    ++item;
    word = word->next;
  }

  return 1;
}

int ast_pattern_wild_match(AstPatternWild *p, OutlineItem *o);
int ast_pattern_symbol_match(AstPatternSymbol *p, OutlineItem *o);
int ast_pattern_assign_match(AstPatternAssign *p, OutlineItem *o);

/**
 * Compares a single pattern item against a single outline word
 */
int ast_pattern_item_match(AstPatternItem item, OutlineItem *o)
{
  switch (item.type) {
  case AST_PATTERN_WILD:    return ast_pattern_wild_match(item.p, o);
  case AST_PATTERN_SYMBOL:  return ast_pattern_symbol_match(item.p, o);
  case AST_PATTERN_ASSIGN:  return ast_pattern_assign_match(item.p, o);
  default: assert(0); return 0;
  }
}

int ast_pattern_wild_match(AstPatternWild *p, OutlineItem *o)
{
  return 1;
}

int ast_pattern_symbol_match(AstPatternSymbol *p, OutlineItem *o)
{
  return string_equal(p->symbol, string_init(o->p, o->end));
}

int ast_pattern_assign_match(AstPatternAssign *p, OutlineItem *o)
{
  if (!ast_pattern_item_match(p->pattern, o))
    return 0;
  p->symbol = string_init(o->p, o->end);
  return 1;
}
