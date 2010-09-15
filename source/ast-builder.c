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

/**
 * Initializes the AST-building stack.
 * @return 0 for failure
 */
int ast_builder_init(AstBuilder *b)
{
  if (!pool_init(&b->pool, 0x10000)) /* 64K block size */
    return 0;
  b->stack_size = 32;
  b->stack = malloc(b->stack_size*sizeof(Dynamic));
  if (!b->stack) return 0;
  b->stack_top = 0;
  b->scope = scope_new(&b->pool, 0);
  if (!b->scope) return 0;
  return 1;
}

void ast_builder_free(AstBuilder *b)
{
  pool_free(&b->pool);
  free(b->stack);
}

/**
 * Pushes an node onto the stack.
 * @return 0 for failure
 */
int ast_builder_push(AstBuilder *b, Type type, void *p)
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

Dynamic ast_builder_pop(AstBuilder *b)
{
  --b->stack_top;
  return b->stack[b->stack_top];
}

/**
 * Constructs a new scope. The current scope becomes the outer scope.
 * @return the new scope, or 0 for failure.
 */
Scope *ast_builder_scope_new(AstBuilder *b)
{
  Scope *s = scope_new(&b->pool, b->scope);
  if (!s) return 0;
  b->scope = s;
  return s;
}

/**
 * Adds a symbol to the current scope.
 * @return the new symbol, or 0 for failure.
 */
Symbol *ast_builder_scope_add(AstBuilder *b, String symbol)
{
  return scope_add(b->scope, &b->pool, pool_string_copy(&b->pool,symbol));
}

void ast_builder_scope_pop(AstBuilder *b)
{
  assert(b->scope->outer);
  b->scope = b->scope->outer;
}

Symbol *ast_builder_scope_find(AstBuilder *b, String symbol)
{
  return scope_find(b->scope, symbol);
}

/*
 * Functions for assembling an AST. All functions return 0 on success.
 */
int ast_build_filter_tag(AstBuilder *b, String tag)
{
  return ast_builder_push(b, AST_FILTER_TAG,
    ast_filter_tag_new(&b->pool, tag));
}

int ast_build_filter_not(AstBuilder *b)
{
  AstFilterNode test = ast_to_filter_node(ast_builder_pop(b));

  return ast_builder_push(b, AST_FILTER_NOT,
    ast_filter_not_new(&b->pool, test));
}

int ast_build_filter_any(AstBuilder *b)
{
  return ast_builder_push(b, AST_FILTER_ANY,
    ast_filter_any_new(&b->pool));
}

int ast_build_filter_and(AstBuilder *b)
{
  AstFilterNode test_a = ast_to_filter_node(ast_builder_pop(b));
  AstFilterNode test_b = ast_to_filter_node(ast_builder_pop(b));

  return ast_builder_push(b, AST_FILTER_AND,
    ast_filter_and_new(&b->pool, test_a, test_b));
}

int ast_build_filter_or(AstBuilder *b)
{
  AstFilterNode test_a = ast_to_filter_node(ast_builder_pop(b));
  AstFilterNode test_b = ast_to_filter_node(ast_builder_pop(b));

  return ast_builder_push(b, AST_FILTER_OR,
    ast_filter_or_new(&b->pool, test_a, test_b));
}
