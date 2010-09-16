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

/**
 * Pushes a list start marker onto the stack.
 */
int ast_builder_push_start(AstBuilder *b)
{
  return ast_builder_push(b, TYPE_END, b /* HACK */);
}

Dynamic ast_builder_pop(AstBuilder *b)
{
  --b->stack_top;
  return b->stack[b->stack_top];
}

Dynamic ast_builder_peek(AstBuilder *b)
{
  return (b->stack[b->stack_top - 1]);
}

/**
 * Finds the length of the list at the top of the stack.
 */
static size_t ast_builder_count(AstBuilder *b)
{
  size_t i = b->stack_top;
  while (0 < i && b->stack[i - 1].type != TYPE_END)
    --i;
  return i == 0 ? 0 : b->stack_top - i;
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
int ast_build_code(AstBuilder *b)
{
  size_t i;
  size_t node_n;
  AstCodeNode *nodes;

  node_n = ast_builder_count(b);
  nodes = pool_alloc(&b->pool, node_n*sizeof(AstCodeNode));
  if (!nodes) return 0;

  b->stack_top -= node_n;
  for (i = 0; i < node_n; ++i)
    nodes[i] = ast_to_code_node(b->stack[b->stack_top + i]);
  --b->stack_top;

  return ast_builder_push(b, AST_CODE,
    ast_code_new(&b->pool, nodes, nodes + node_n));
}

int ast_build_code_text(AstBuilder *b, String code)
{
  if (!string_size(code))
    return 1;

  return ast_builder_push(b, AST_CODE_TEXT,
    ast_code_text_new(&b->pool,
      pool_string_copy(&b->pool, code)));
}

int ast_build_include(AstBuilder *b)
{
  return ast_builder_push(b, AST_INCLUDE,
    ast_include_new(&b->pool,
      ast_to_code(ast_builder_pop(b))));
}

int ast_build_outline(AstBuilder *b)
{
  size_t i;
  size_t item_n;
  AstOutlineItem **items;

  item_n = ast_builder_count(b);
  items = pool_alloc(&b->pool, item_n*sizeof(AstOutlineItem *));
  if (!items) return 0;

  b->stack_top -= item_n;
  for (i = 0; i < item_n; ++i)
    items[i] = ast_to_outline_item(b->stack[b->stack_top + i]);
  --b->stack_top;

  return ast_builder_push(b, AST_OUTLINE,
    ast_outline_new(&b->pool, items, items + item_n));
}

int ast_build_outline_item(AstBuilder *b, String name)
{
  size_t i;
  size_t tag_n;
  AstOutlineTag **tags;
  AstOutline *children;

  children = ast_builder_peek(b).type == AST_OUTLINE ?
    ast_to_outline(ast_builder_pop(b)) : 0;

  tag_n = ast_builder_count(b);
  tags = pool_alloc(&b->pool, tag_n*sizeof(AstOutlineTag*));
  if (!tags) return 0;

  b->stack_top -= tag_n;
  for (i = 0; i < tag_n; ++i)
    tags[i] = ast_to_outline_tag(b->stack[b->stack_top + i]);
  --b->stack_top;

  return ast_builder_push(b, AST_OUTLINE_ITEM,
    ast_outline_item_new(&b->pool,
      tags,
      tags + tag_n,
      pool_string_copy(&b->pool, name),
      children));
}

int ast_build_outline_tag(AstBuilder *b, String symbol)
{
  AstCode *code;

  code = ast_builder_peek(b).type == AST_CODE ?
    ast_to_code(ast_builder_pop(b)) : 0;

  return ast_builder_push(b, AST_OUTLINE_TAG,
    ast_outline_tag_new(&b->pool,
      pool_string_copy(&b->pool, symbol),
      code));
}

int ast_build_map(AstBuilder *b, Symbol *item)
{
  size_t i;
  size_t line_n;
  AstMapLine **lines;

  line_n = ast_builder_count(b);
  lines = pool_alloc(&b->pool, line_n*sizeof(AstMapLine*));
  if (!lines) return 0;

  b->stack_top -= line_n;
  for (i = 0; i < line_n; ++i)
    lines[i] = ast_to_map_line(b->stack[b->stack_top + i]);
  --b->stack_top;

  return ast_builder_push(b, AST_MAP,
    ast_map_new(&b->pool,
      item,
      lines, lines + line_n));
}

int ast_build_map_line(AstBuilder *b)
{
  AstFilter *filter;
  AstCode *code;

  code = ast_to_code(ast_builder_pop(b));
  filter = ast_to_filter(ast_builder_pop(b));

  return ast_builder_push(b, AST_MAP_LINE,
    ast_map_line_new(&b->pool, filter, code));
}

int ast_build_for(AstBuilder *b, Symbol *item, Symbol *outline, int reverse, int list)
{
  AstFilter *filter;
  AstCode *code;

  code = ast_to_code(ast_builder_pop(b));

  filter = ast_builder_peek(b).type == AST_FILTER ?
    ast_to_filter(ast_builder_pop(b)) : 0;

  return ast_builder_push(b, AST_FOR,
    ast_for_new(&b->pool,
      item,
      outline,
      filter,
      reverse,
      list,
      code));
}

int ast_build_filter(AstBuilder *b)
{
  return ast_builder_push(b, AST_FILTER,
    ast_filter_new(&b->pool,
      ast_to_filter_node(ast_builder_pop(b))));
}

int ast_build_filter_tag(AstBuilder *b, String tag)
{
  return ast_builder_push(b, AST_FILTER_TAG,
    ast_filter_tag_new(&b->pool,
      pool_string_copy(&b->pool, tag)));
}

int ast_build_filter_not(AstBuilder *b)
{
  return ast_builder_push(b, AST_FILTER_NOT,
    ast_filter_not_new(&b->pool,
      ast_to_filter_node(ast_builder_pop(b))));
}

int ast_build_filter_any(AstBuilder *b)
{
  return ast_builder_push(b, AST_FILTER_ANY,
    ast_filter_any_new(&b->pool));
}

int ast_build_filter_and(AstBuilder *b)
{
  return ast_builder_push(b, AST_FILTER_AND,
    ast_filter_and_new(&b->pool,
      ast_to_filter_node(ast_builder_pop(b)),
      ast_to_filter_node(ast_builder_pop(b))));
}

int ast_build_filter_or(AstBuilder *b)
{
  return ast_builder_push(b, AST_FILTER_OR,
    ast_filter_or_new(&b->pool,
      ast_to_filter_node(ast_builder_pop(b)),
      ast_to_filter_node(ast_builder_pop(b))));
}

int ast_build_set(AstBuilder *b, Symbol *symbol)
{
  Dynamic value = ast_builder_pop(b);

  return ast_builder_push(b, AST_SET,
    ast_set_new(&b->pool,
      symbol,
      value));
}

int ast_build_symbol_ref(AstBuilder *b, Symbol *symbol)
{
  return ast_builder_push(b, AST_SYMBOL_REF,
    ast_symbol_ref_new(&b->pool,
      symbol));
}

int ast_build_call(AstBuilder *b, Symbol *f, Symbol *data)
{
  return ast_builder_push(b, AST_CALL,
    ast_call_new(&b->pool,
      f,
      data));
}

int ast_build_lookup(AstBuilder *b, Symbol *symbol, String name)
{
  return ast_builder_push(b, AST_LOOKUP,
    ast_lookup_new(&b->pool,
      symbol,
      pool_string_copy(&b->pool, name)));
}
