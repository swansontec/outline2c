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

#include "filter.h"
#include <assert.h>

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
  ListNode *tag;

  for (tag = item->tags; tag; tag = tag->next) {
    if (string_equal(ast_to_outline_tag(*tag)->name, test->tag))
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

/**
 * Initializes the AST-building stack.
 * @return 0 for failure
 */
int filter_builder_init(FilterBuilder *b)
{
  b->stack_size = 32;
  b->stack = malloc(b->stack_size*sizeof(Dynamic));
  if (!b->stack) return 0;
  b->stack_top = 0;
  return 1;
}

void filter_builder_free(FilterBuilder *b)
{
  free(b->stack);
}

/**
 * Pushes an node onto the stack.
 * @return 0 for failure
 */
static int filter_builder_push(FilterBuilder *b, Type type, void *p)
{
  Dynamic node;
  node.p = p;
  node.type = type;

  if (!p) return 0;

  /* Grow, if needed: */
  if (b->stack_size <= b->stack_top) {
    size_t new_size = 2*b->stack_size;
    Dynamic *new_stack = realloc(b->stack, new_size*sizeof(Dynamic));
    if (!new_stack) return 0;
    b->stack_size = new_size;
    b->stack = new_stack;
  }
  b->stack[b->stack_top] = node;
  ++b->stack_top;
  return 1;
}

Dynamic filter_builder_pop(FilterBuilder *b)
{
  --b->stack_top;
  return b->stack[b->stack_top];
}

/*
 * Functions for assembling a filter.
 */
int filter_build_tag(FilterBuilder *b, Pool *pool, String tag)
{
  return filter_builder_push(b, AST_FILTER_TAG,
    ast_filter_tag_new(pool, tag));
}

int filter_build_not(FilterBuilder *b, Pool *pool)
{
  AstFilterNode test = ast_to_filter_node(filter_builder_pop(b));

  return filter_builder_push(b, AST_FILTER_NOT,
    ast_filter_not_new(pool, test));
}

int filter_build_any(FilterBuilder *b, Pool *pool)
{
  return filter_builder_push(b, AST_FILTER_ANY,
    ast_filter_any_new(pool));
}

int filter_build_and(FilterBuilder *b, Pool *pool)
{
  AstFilterNode test_a = ast_to_filter_node(filter_builder_pop(b));
  AstFilterNode test_b = ast_to_filter_node(filter_builder_pop(b));

  return filter_builder_push(b, AST_FILTER_AND,
    ast_filter_and_new(pool, test_a, test_b));
}

int filter_build_or(FilterBuilder *b, Pool *pool)
{
  AstFilterNode test_a = ast_to_filter_node(filter_builder_pop(b));
  AstFilterNode test_b = ast_to_filter_node(filter_builder_pop(b));

  return filter_builder_push(b, AST_FILTER_OR,
    ast_filter_or_new(pool, test_a, test_b));
}
