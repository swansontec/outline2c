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
#include "file.h"
#include <assert.h>
#include <stdlib.h>

/**
 * Releases the memory held by an outline list. This only needs to be called
 * for lists allocated by outline_list_from_file.
 */
void outline_list_free(OutlineList *self)
{
  free(self->p);
}

/**
 * Searches a file for all available outline items and adds them to a single
 * master list.
 */
int outline_list_from_file(OutlineList *self, AstItem *items, AstItem *items_end)
{
  AstItem *item;
  AstOutline **list;
  size_t list_n = 0;
  size_t i = 0;

  self->p = 0;
  self->end = 0;

  /* Count the outlines: */
  item = items;
  while (item < items_end) {
    if (item->type == AST_OUTLINE)
      ++list_n;
    ++item;
  }

  /* Build the list: */
  if (!list_n) return 0;
  list = malloc(list_n*sizeof(AstOutline*));
  if (!list) return 1;

  item = items;
  while (item < items_end) {
    if (item->type == AST_OUTLINE)
      list[i++] = item->p;
    ++item;
  }

  self->p = list;
  self->end = list + list_n;
  return 0;
}

/**
 * Initializes an outline list with the childern of the given outline.
 */
OutlineList outline_list_from_outline(AstOutline *outline)
{
  OutlineList temp;
  temp.p = outline->children;
  temp.end = outline->children_end;
  return temp;
}

/**
 * Compares a match against a list of outlines.
 * Generates code for the first match to to match against a particular outline.
 * @return 0 for success
 */
int ast_match_search(AstMatch *match, OutlineList outlines, FileW *file)
{
  int rv;
  AstOutline **outline;

  outline = outlines.p;
  while (outline < outlines.end) {
    AstMatchLine **line = match->lines;
    while (line < match->lines_end) {
      if (match_pattern((*line)->pattern, *outline)) {
        rv = ast_code_generate((*line)->code, outline_list_from_outline(*outline), file);
        if (rv) return rv;
        break;
      }
      ++line;
    }
    ++outline;
  }
  return 0;
}

/**
 * Generates code for a match block.
 * @return 0 for success
 */
int ast_code_generate(AstCode *code, OutlineList outlines, FileW *file)
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
      rv = ast_match_search(p, outlines, file);
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

/**
 * Compares a pattern against an outline.
 * @return non-zero for a match
 */
int match_pattern(AstPattern *pattern, AstOutline *outline)
{
  AstPatternItem *pi;
  AstOutlineItem *oi;

  if (pattern->items_end - pattern->items != outline->items_end - outline->items)
    return 0;

  pi = pattern->items;
  oi = outline->items;
  while (pi < pattern->items_end) {
    if (!match_pattern_item(*pi, *oi))
      return 0;
    ++pi;
    ++oi;
  }
  return 1;
}

/**
 * Compares a single pattern item against a single outline item
 * @return non-zero for a match
 */
int match_pattern_item(AstPatternItem pi, AstOutlineItem oi)
{
  switch (pi.type) {
  case AST_PATTERN_WILD:    return match_pattern_wild(pi.p, oi);
  case AST_PATTERN_SYMBOL:  return match_pattern_symbol(pi.p, oi);
  case AST_PATTERN_ASSIGN:  return match_pattern_assign(pi.p, oi);
  default: assert(0); return 0;
  }
}

int match_pattern_wild(AstPatternWild *p, AstOutlineItem oi)
{
  return 1;
}

int match_pattern_symbol(AstPatternSymbol *p, AstOutlineItem oi)
{
  return oi.type == AST_OUTLINE_SYMBOL &&
    string_equal(p->symbol, ((AstOutlineSymbol*)oi.p)->symbol);
}

int match_pattern_assign(AstPatternAssign *p, AstOutlineItem oi)
{
  if (!match_pattern_item(p->pattern, oi))
    return 0;
  /* TODO: This whole next line is an ill-concieved hack. The cast is not
  really safe, and the whole concept of storing the value in the symbol is
  wrong. Also, matching could eventually generate arbitraily complex code, so
  we can't assume the outline item is even what we want in the first place.*/
  p->symbol = ((AstOutlineSymbol*)oi.p)->symbol;
  return 1;
}
