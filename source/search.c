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

#include "search.h"
#include <assert.h>
#include <stdlib.h>

int test_filter_node(AstFilterNode test, AstOutlineItem *item);
int test_filter_tag(AstFilterTag *test, AstOutlineItem *item);
int test_filter_not(AstFilterNot *test, AstOutlineItem *item);
int test_filter_and(AstFilterAnd *test, AstOutlineItem *item);
int test_filter_or(AstFilterOr *test, AstOutlineItem *item);

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
  case AST_FILTER_ANY: return 1;
  case AST_FILTER_NOT: return test_filter_not(test.p, item);
  case AST_FILTER_AND: return test_filter_and(test.p, item);
  case AST_FILTER_OR:  return test_filter_or(test.p, item);
  default: assert(0); return 0;
  }
}

int test_filter_tag(AstFilterTag *test, AstOutlineItem *item)
{
  AstOutlineTag **tag;

  for (tag = item->tags; tag != item->tags_end; ++tag) {
    if (string_equal((*tag)->name, test->tag))
      return 1;
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

static AstOutline *code_find_outline(AstCode *code, String name)
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
  AstOutline *outline;
  while (s) {
    outline = code_find_outline(s->code, name);
    if (outline) return outline;
    s = s->outer;
  }
  return 0;
}

static AstMap *code_find_map(AstCode *code, String name)
{
  AstCodeNode *node;
  AstMap *map;

  for (node = code->nodes; node != code->nodes_end; ++node) {
    if (node->type == AST_MAP) {
      map = node->p;
      if (string_equal(map->name, name))
        return map;
    } else if (node->type == AST_INCLUDE) {
      AstInclude *include = node->p;
      map = code_find_map(include->file->code, name);
      if (map)
        return map;
    }
  }
  return 0;
}

AstMap *scope_find_map(Scope *s, String name)
{
  AstMap *map;
  while (s) {
    map = code_find_map(s->code, name);
    if (map) return map;
    s = s->outer;
  }
  return 0;
}
