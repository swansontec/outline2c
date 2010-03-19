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
 * Determines whether an outline item satisfies a particular filter expression.
 */
int test_filter(AstFilter *test, AstOutlineItem *item)
{
  return test_filter_node(test->test, item);
}

int test_filter_node(AstFilterNode test, AstOutlineItem *item)
{
  switch (test.type) {
  case AST_FILTER_TAG: return test_filter_tag(test.p, item);
  case AST_FILTER_NOT: return test_filter_not(test.p, item);
  case AST_FILTER_AND: return test_filter_and(test.p, item);
  case AST_FILTER_OR:  return test_filter_or(test.p, item);
  default: assert(0); return 0;
  }
}

int test_filter_tag(AstFilterTag *test, AstOutlineItem *item)
{
  AstOutlineNode *node;

  for (node = item->nodes; node != item->nodes_end; ++node) {
    if (node->type == AST_OUTLINE_SYMBOL) {
      AstOutlineSymbol *p = node->p;
      if (string_equal(p->symbol, test->tag))
        return 1;
    }
  }

  return 0;
}

int test_filter_not(AstFilterNot *test, AstOutlineItem *item)
{
  return !test_filter_node(test->test, item);
}

int test_filter_and(AstFilterAnd *test, AstOutlineItem *item)
{
  return
    test_filter_node(test->test_a, item) &&
    test_filter_node(test->test_b, item);
}

int test_filter_or(AstFilterOr *test, AstOutlineItem *item)
{
  return
    test_filter_node(test->test_a, item) ||
    test_filter_node(test->test_b, item);
}

Scope scope_init(AstCode *code, Scope *outer, AstOutlineItem *item)
{
  Scope self;
  self.code = code;
  self.outer = outer;
  self.item = item;
  return self;
}

AstOutline *code_find_outline(AstCode *code, String name)
{
  AstCodeNode *node;
  AstOutline *outline;

  for (node = code->nodes; node != code->nodes_end; ++node) {
    if (node->type == AST_OUTLINE) {
      outline = node->p;
      if (string_equal(outline->name, name))
        return outline;
    } else if (node->type == AST_INCLUDE) {
      AstInclude *include = node->p;
      outline = code_find_outline(include->file->code, name);
      if (outline)
        return outline;
    }
  }
  return 0;
}

AstOutline *scope_find_outline(Scope *s, String name)
{
  return code_find_outline(s->code, name);
}

/**
 * Releases the memory held by an outline list. This only needs to be called
 * for lists allocated by outline_list_from_file.
 */
void outline_list_free(AstOutlineList *self)
{
  free(self->items);
}

static size_t count_outlines(AstFile *file)
{
  size_t count = 0;
  AstCodeNode *node;

  for (node = file->code->nodes; node != file->code->nodes_end; ++node) {
    if (node->type == AST_OUTLINE) {
      AstOutline *outline = node->p;
      count += outline->children->items_end - outline->children->items;
    } else if (node->type == AST_INCLUDE) {
      AstInclude *include = node->p;
      count += count_outlines(include->file);
    }
  }
  return count;
}

static size_t copy_outlines(AstOutlineItem **list, AstFile *file)
{
  size_t i = 0;
  AstCodeNode *node;

  for (node = file->code->nodes; node != file->code->nodes_end; ++node) {
    if (node->type == AST_OUTLINE) {
      AstOutline *outline = node->p;
      AstOutlineItem **child;
      for (child = outline->children->items; child < outline->children->items_end; ++child)
        list[i++] = *child;
    } else if (node->type == AST_INCLUDE) {
      AstInclude *include = node->p;
      i += copy_outlines(list + i, include->file);
    }
  }
  return i;
}

/**
 * Searches a file for all available outline nodes and adds them to a single
 * master list.
 */
int outline_list_from_file(AstOutlineList *self, AstFile *file)
{
  AstOutlineItem **list;
  size_t count = 0;

  self->items = 0;
  self->items_end = 0;

  /* Count the outlines: */
  count = count_outlines(file);
  if (!count) return 0;

  /* Build the list: */
  list = malloc(count*sizeof(AstOutlineItem*));
  if (!list) return 1;
  copy_outlines(list, file);

  self->items = list;
  self->items_end = list + count;
  return 0;
}

/**
 * Compares a match against a list of outlines.
 * Generates code for the first match to to match against a particular outline.
 * @param outlines The list of outlines to search. May be NULL.
 * @return 0 for success
 */
int ast_match_search(AstMatch *match, AstOutlineList *outlines, FileW *out)
{
  int rv;
  AstOutlineItem **outline;
  AstMatchLine **line;

  if (!outlines)
    return 0;

  for (outline = outlines->items; outline != outlines->items_end; ++outline) {
    for (line = match->lines; line != match->lines_end; ++line) {
      if (match_pattern((*line)->pattern, *outline)) {
        char end[] = "\n";
        rv = ast_code_generate((*line)->code, (*outline)->children, out);
        if (rv) return rv;
        file_w_write(out, end, end+1);
        break;
      }
    }
  }
  return 0;
}

/**
 * Generates code for a match block.
 * @return 0 for success
 */
int ast_code_generate(AstCode *code, AstOutlineList *outlines, FileW *out)
{
  int rv;
  AstCodeNode *node;

  for (node = code->nodes; node != code->nodes_end; ++node) {
    if (node->type == AST_CODE_TEXT) {
      AstCodeText *p = node->p;
      rv = file_w_write(out, p->code.p, p->code.end);
      if (rv) return rv;
    } else if (node->type == AST_INCLUDE) {
    } else if (node->type == AST_OUTLINE) {
    } else if (node->type == AST_FOR) {
    } else if (node->type == AST_MATCH) {
      AstMatch *p = node->p;
      rv = ast_match_search(p, outlines, out);
      if (rv) return rv;
    } else if (node->type == AST_SYMBOL) {
    } else if (node->type == AST_REPLACE) {
      AstReplace *p = node->p;
      rv = file_w_write(out, p->symbol->symbol.p, p->symbol->symbol.end);
      if (rv) return rv;
    } else {
      assert(0);
    }
  }
  return 0;
}

/**
 * Compares a pattern against an outline.
 * @return non-zero for a match
 */
int match_pattern(AstPattern *pattern, AstOutlineItem *outline)
{
  AstPatternNode *pn;
  AstOutlineNode *on;

  if (pattern->nodes_end - pattern->nodes != outline->nodes_end - outline->nodes)
    return 0;

  pn = pattern->nodes;
  on = outline->nodes;
  while (pn < pattern->nodes_end) {
    if (!match_pattern_item(*pn, *on))
      return 0;
    ++pn;
    ++on;
  }
  return 1;
}

/**
 * Compares a single pattern node against a single outline node
 * @return non-zero for a match
 */
int match_pattern_item(AstPatternNode pn, AstOutlineNode on)
{
  switch (pn.type) {
  case AST_PATTERN_WILD:    return match_pattern_wild(pn.p, on);
  case AST_PATTERN_SYMBOL:  return match_pattern_symbol(pn.p, on);
  case AST_PATTERN_ASSIGN:  return match_pattern_assign(pn.p, on);
  default: assert(0); return 0;
  }
}

int match_pattern_wild(AstPatternWild *p, AstOutlineNode on)
{
  return 1;
}

int match_pattern_symbol(AstPatternSymbol *p, AstOutlineNode on)
{
  return on.type == AST_OUTLINE_SYMBOL &&
    string_equal(p->symbol, ((AstOutlineSymbol*)on.p)->symbol);
}

int match_pattern_assign(AstPatternAssign *p, AstOutlineNode on)
{
  if (!match_pattern_item(p->pattern, on))
    return 0;
  /* TODO: This whole next line is an ill-concieved hack. The cast is not
  really safe, and the whole concept of storing the value in the symbol is
  wrong. Also, matching could eventually generate arbitraily complex code, so
  we can't assume the outline node is even what we want in the first place.*/
  p->symbol = ((AstOutlineSymbol*)on.p)->symbol;
  return 1;
}
