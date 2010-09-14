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
