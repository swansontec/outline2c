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
#include "file.h"
#include <assert.h>

int ast_pattern_item_match(AstPatternItem pattern, OutlineItem *o);

/**
 * Compares a match and its siblings against an outline and its siblings.
 * Generates code for the first match to to match against a particular outline.
 * @return 0 for success
 */
int ast_match_search(AstMatch *match, Outline *outline, FileW *file)
{
  int rv;

  while (outline) {
    AstMatchLine **line = match->lines;
    while (line < match->lines_end) {
      if (ast_pattern_compare((*line)->pattern, outline)) {
        rv = ast_code_generate((*line)->code, outline->children, file);
        if (rv) return rv;
        break;
      }
      ++line;
    }
    outline = outline->next;
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

/**
 * Generates code for a match block.
 * @return 0 for success
 */
int ast_code_generate(AstCode *code, Outline *outline, FileW *file)
{
  int rv;
  char end[] = "\n";
  AstCodeItem *item;

  item = code->items;
  while (item < code->items_end) {
    if (item->type == AST_C) {
      AstC *p = item->p;
      rv = file_w_write(file, p->code.p, p->code.end);
      if (rv) return rv;
    } else if (item->type == AST_MATCH) {
      AstMatch *p = item->p;
      rv = ast_match_search(p, outline, file);
      if (rv) return rv;
    } else if (item->type == AST_CODE_SYMBOL) {
      AstCodeSymbol *p = item->p;
      rv = file_w_write(file, p->symbol->symbol.p, p->symbol->symbol.end);
      if (rv) return rv;
    } else {
      assert(0);
    }
    ++item;
  }
  file_w_write(file, end, end+1);
  return 0;
}
