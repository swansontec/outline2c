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

#include "ast-builder.h"
#include <assert.h>

int ast_builder_init(AstBuilder *b)
{
  pool_init(&b->pool, 0x10000); /* 64K block size */
  b->stack_size = 32;
  b->stack = malloc(b->stack_size*sizeof(AstItem));
  if (!b->stack) return 1;
  b->stack_top = 0;
  return 0;
}

void ast_builder_free(AstBuilder *b)
{
  pool_free(&b->pool);
  free(b->stack);
}

/**
 * Pushes an item onto the stack.
 * @return 0 for success
 */
int ast_builder_push(AstBuilder *b, AstType type, void *p)
{
  AstItem item;
  item.p = p;
  item.type = type;

  if (!p) return 1;

  /* Grow, if needed: */
  if (b->stack_size <= b->stack_top) {
    size_t new_size = 2*b->stack_size;
    AstItem *new_stack = realloc(b->stack, new_size*sizeof(AstItem));
    if (!new_stack) return 1;
    b->stack_size = new_size;
    b->stack = new_stack;
  }
  b->stack[b->stack_top] = item;
  ++b->stack_top;
  return 0;
}

AstItem ast_builder_pop(AstBuilder *b)
{
  --b->stack_top;
  return b->stack[b->stack_top];
}

AstItem ast_builder_peek(AstBuilder *b)
{
  return (b->stack[b->stack_top - 1]);
}

/*
 * Functions for assembling an AST. All functions return 0 on success.
 */
int ast_build_pattern(AstBuilder *b, size_t item_n)
{
  size_t i;
  AstPatternItem *items;

  items = pool_alloc(&b->pool, item_n*sizeof(AstPatternItem));
  if (!items) return 1;

  b->stack_top -= item_n;
  for (i = 0; i < item_n; ++i)
    items[i] = ast_to_pattern_item(b->stack[b->stack_top + i]);

  return ast_builder_push(b, AST_PATTERN,
    ast_pattern_new(&b->pool, items, items + item_n));
}

int ast_build_pattern_wild(AstBuilder *b)
{
  return ast_builder_push(b, AST_PATTERN_WILD,
    ast_pattern_wild_new(&b->pool));
}

int ast_build_pattern_any_symbol(AstBuilder *b)
{
  return ast_builder_push(b, AST_PATTERN_ANY_SYMBOL,
    ast_pattern_any_symbol_new(&b->pool));
}

int ast_build_pattern_any_string(AstBuilder *b)
{
  return ast_builder_push(b, AST_PATTERN_ANY_STRING,
    ast_pattern_any_string_new(&b->pool));
}

int ast_build_pattern_any_number(AstBuilder *b)
{
  return ast_builder_push(b, AST_PATTERN_ANY_NUMBER,
    ast_pattern_any_number_new(&b->pool));
}

int ast_build_pattern_rule(AstBuilder *b, AstRule *rule)
{
  return ast_builder_push(b, AST_PATTERN_RULE,
    ast_pattern_rule_new(&b->pool,
      rule));
}

int ast_build_pattern_symbol(AstBuilder *b, String symbol)
{
  return ast_builder_push(b, AST_PATTERN_SYMBOL,
    ast_pattern_symbol_new(&b->pool,
      pool_string_copy(&b->pool, symbol)));
}

int ast_build_pattern_string(AstBuilder *b, String string)
{
  return ast_builder_push(b, AST_PATTERN_STRING,
    ast_pattern_string_new(&b->pool,
      pool_string_copy(&b->pool, string)));
}

int ast_build_pattern_number(AstBuilder *b, String number)
{
  return ast_builder_push(b, AST_PATTERN_NUMBER,
    ast_pattern_number_new(&b->pool,
      pool_string_copy(&b->pool, number)));
}

int ast_build_pattern_assign(AstBuilder *b, String symbol)
{
  return ast_builder_push(b, AST_PATTERN_ASSIGN,
    ast_pattern_assign_new(&b->pool,
      pool_string_copy(&b->pool, symbol),
      ast_to_pattern_item(ast_builder_pop(b))));
}
