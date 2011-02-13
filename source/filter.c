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

int test_filter_tag(AstFilterTag *test, AstOutlineItem *item)
{
  ListNode *tag;

  for (tag = item->tags; tag; tag = tag->next) {
    if (string_equal(ast_to_outline_tag(tag->d)->name, test->tag))
      return 1;
  }

  return 0;
}

int test_filter_not(AstFilterNot *test, AstOutlineItem *item)
{
  return !test_filter(test->test, item);
}

int test_filter_and(AstFilterAnd *test, AstOutlineItem *item)
{
  return
    test_filter(test->test_a, item) &&
    test_filter(test->test_b, item);
}

int test_filter_or(AstFilterOr *test, AstOutlineItem *item)
{
  return
    test_filter(test->test_a, item) ||
    test_filter(test->test_b, item);
}

/**
 * Determines whether an outline item satisfies a particular filter expression.
 */
int test_filter(Dynamic test, AstOutlineItem *item)
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
 * Pushes a node onto the stack.
 */
static int filter_builder_push(FilterBuilder *b, Dynamic node)
{
  if (!dynamic_ok(node)) return 0;

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
  AstFilterTag *self = pool_new(pool, AstFilterTag);
  CHECK_MEM(self);
  self->tag = string_copy(pool, tag);
  CHECK_MEM(string_size(self->tag));

  return filter_builder_push(b, dynamic(AST_FILTER_TAG, self));
}

int filter_build_any(FilterBuilder *b, Pool *pool)
{
  return filter_builder_push(b, dynamic_init(AST_FILTER_ANY, 0));
}

int filter_build_not(FilterBuilder *b, Pool *pool)
{
  AstFilterNot *self = pool_new(pool, AstFilterNot);
  CHECK_MEM(self);
  self->test = filter_builder_pop(b);
  assert(dynamic_ok(self->test));

  return filter_builder_push(b, dynamic(AST_FILTER_NOT, self));
}

int filter_build_and(FilterBuilder *b, Pool *pool)
{
  AstFilterAnd *self = pool_new(pool, AstFilterAnd);
  CHECK_MEM(self);
  self->test_a = filter_builder_pop(b);
  assert(dynamic_ok(self->test_a));
  self->test_b = filter_builder_pop(b);
  assert(dynamic_ok(self->test_b));

  return filter_builder_push(b, dynamic(AST_FILTER_AND, self));
}

int filter_build_or(FilterBuilder *b, Pool *pool)
{
  AstFilterOr *self = pool_new(pool, AstFilterOr);
  CHECK_MEM(self);
  self->test_a = filter_builder_pop(b);
  assert(dynamic_ok(self->test_a));
  self->test_b = filter_builder_pop(b);
  assert(dynamic_ok(self->test_b));

  return filter_builder_push(b, dynamic(AST_FILTER_OR, self));
}
