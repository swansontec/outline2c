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
 * A stack for building filters using Dijkstra's shunting-yard algorithm
 */
typedef struct {
  Dynamic *stack;
  size_t stack_size;
  size_t stack_top;
} FilterBuilder;

int filter_builder_init(FilterBuilder *b)
{
  b->stack_size = 32;
  b->stack = malloc(b->stack_size*sizeof(Dynamic));
  CHECK_MEM(b->stack);
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
  if (!node.p) return 0;

  /* Grow, if needed: */
  if (b->stack_size <= b->stack_top) {
    size_t new_size = 2*b->stack_size;
    Dynamic *new_stack = realloc(b->stack, new_size*sizeof(Dynamic));
    CHECK_MEM(new_stack);
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
  AstFilterTag *self = pool_alloc(pool, sizeof(AstFilterTag));
  CHECK_MEM(self);
  self->tag = string_copy(pool, tag);
  CHECK_MEM(string_size(self->tag));

  return filter_builder_push(b, AST_FILTER_TAG, self);
}

int filter_build_any(FilterBuilder *b, Pool *pool)
{
  AstFilterAny *self = pool_alloc(pool, sizeof(AstFilterAny));
  CHECK_MEM(self);

  return filter_builder_push(b, AST_FILTER_ANY, self);
}

int filter_build_not(FilterBuilder *b, Pool *pool)
{
  AstFilterNot *self = pool_alloc(pool, sizeof(AstFilterNot));
  CHECK_MEM(self);
  self->test = ast_to_filter_node(filter_builder_pop(b));
  assert(self->test.p);

  return filter_builder_push(b, AST_FILTER_NOT, self);
}

int filter_build_and(FilterBuilder *b, Pool *pool)
{
  AstFilterAnd *self = pool_alloc(pool, sizeof(AstFilterAnd));
  CHECK_MEM(self);
  self->test_a = ast_to_filter_node(filter_builder_pop(b));
  assert(self->test_a.p);
  self->test_b = ast_to_filter_node(filter_builder_pop(b));
  assert(self->test_b.p);

  return filter_builder_push(b, AST_FILTER_AND, self);
}

int filter_build_or(FilterBuilder *b, Pool *pool)
{
  AstFilterOr *self = pool_alloc(pool, sizeof(AstFilterOr));
  CHECK_MEM(self);
  self->test_a = ast_to_filter_node(filter_builder_pop(b));
  assert(self->test_a.p);
  self->test_b = ast_to_filter_node(filter_builder_pop(b));
  assert(self->test_b.p);

  return filter_builder_push(b, AST_FILTER_OR, self);
}
