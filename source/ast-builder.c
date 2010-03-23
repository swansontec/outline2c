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
  b->stack = malloc(b->stack_size*sizeof(AstNode));
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
 * Pushes an node onto the stack.
 * @return 0 for success
 */
int ast_builder_push(AstBuilder *b, AstType type, void *p)
{
  AstNode node;
  node.p = p;
  node.type = type;

  if (!p) return 1;

  /* Grow, if needed: */
  if (b->stack_size <= b->stack_top) {
    size_t new_size = 2*b->stack_size;
    AstNode *new_stack = realloc(b->stack, new_size*sizeof(AstNode));
    if (!new_stack) return 1;
    b->stack_size = new_size;
    b->stack = new_stack;
  }
  b->stack[b->stack_top] = node;
  ++b->stack_top;
  return 0;
}

AstNode ast_builder_pop(AstBuilder *b)
{
  --b->stack_top;
  return b->stack[b->stack_top];
}

AstNode ast_builder_peek(AstBuilder *b)
{
  return (b->stack[b->stack_top - 1]);
}

/**
 * Searches the stack for "x in y" nodes. If the symbol defined in the node
 * matches the passed-in string, the function returns the depth at which the
 * node occurred. The topmost "x in y" node would be 0, the "x in y" node below
 * it (possibly with other stuff between) would be 1, etc.. Returns -1 if there
 * is no match.
 */
int ast_builder_find_symbol(AstBuilder *b, String symbol)
{
  size_t top = b->stack_top;
  int level = 0;

  while (top) {
    --top;
    if (b->stack[top].type == AST_IN) {
      AstIn *in = b->stack[top].p;
      if (string_equal(in->symbol, symbol))
        return level;
      else
        ++level;
    }
  }
  return -1;
}

/*
 * Functions for assembling an AST. All functions return 0 on success.
 */
int ast_build_file(AstBuilder *b)
{
  return ast_builder_push(b, AST_FILE,
    ast_file_new(&b->pool,
      ast_to_code(ast_builder_pop(b))));
}

int ast_build_code(AstBuilder *b, size_t node_n)
{
  size_t i;
  AstCodeNode *nodes;

  nodes = pool_alloc(&b->pool, node_n*sizeof(AstCodeNode));
  if (!nodes) return 1;

  b->stack_top -= node_n;
  for (i = 0; i < node_n; ++i)
    nodes[i] = ast_to_code_node(b->stack[b->stack_top + i]);

  return ast_builder_push(b, AST_CODE,
    ast_code_new(&b->pool, nodes, nodes + node_n));
}

int ast_build_code_text(AstBuilder *b, String code)
{
  return ast_builder_push(b, AST_CODE_TEXT,
    ast_code_text_new(&b->pool,
      pool_string_copy(&b->pool, code)));
}

int ast_build_include(AstBuilder *b)
{
  return ast_builder_push(b, AST_INCLUDE,
    ast_include_new(&b->pool,
      ast_to_file(ast_builder_pop(b))));
}

int ast_build_outline(AstBuilder *b, String name)
{
  return ast_builder_push(b, AST_OUTLINE,
    ast_outline_new(&b->pool,
      pool_string_copy(&b->pool, name),
      ast_to_outline_list(ast_builder_pop(b))));
}

int ast_build_outline_list(AstBuilder *b, size_t item_n)
{
  size_t i;
  AstOutlineItem **items;

  items = pool_alloc(&b->pool, item_n*sizeof(AstOutlineItem *));
  if (!items) return 1;

  b->stack_top -= item_n;
  for (i = 0; i < item_n; ++i)
    items[i] = ast_to_outline_item(b->stack[b->stack_top + i]);

  return ast_builder_push(b, AST_OUTLINE_LIST,
    ast_outline_list_new(&b->pool, items, items + item_n));
}

int ast_build_outline_item(AstBuilder *b, String name, size_t tag_n)
{
  size_t i;
  AstOutlineTag **tags;
  AstOutlineList *children;

  children = ast_builder_peek(b).type == AST_OUTLINE_LIST ?
    ast_to_outline_list(ast_builder_pop(b)) : 0;

  tags = pool_alloc(&b->pool, tag_n*sizeof(AstOutlineTag*));
  if (!tags) return 1;

  b->stack_top -= tag_n;
  for (i = 0; i < tag_n; ++i)
    tags[i] = ast_to_outline_tag(b->stack[b->stack_top + i]);

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

int ast_build_map(AstBuilder *b, String name, size_t line_n)
{
  size_t i;
  AstMapLine **lines;

  lines = pool_alloc(&b->pool, line_n*sizeof(AstMapLine*));
  if (!lines) return 1;

  b->stack_top -= line_n;
  for (i = 0; i < line_n; ++i)
    lines[i] = ast_to_map_line(b->stack[b->stack_top + i]);

  return ast_builder_push(b, AST_MAP,
    ast_map_new(&b->pool,
      pool_string_copy(&b->pool, name),
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

int ast_build_for(AstBuilder *b)
{
  AstIn *in;
  AstFilter *filter;
  AstCode *code;

  code = ast_to_code(ast_builder_pop(b));

  filter = ast_builder_peek(b).type == AST_FILTER ?
    ast_to_filter(ast_builder_pop(b)) : 0;

  in = ast_to_in(ast_builder_pop(b));

  return ast_builder_push(b, AST_FOR,
    ast_for_new(&b->pool, in, filter, code));
}

int ast_build_in(AstBuilder *b, String symbol, String name)
{
  return ast_builder_push(b, AST_IN,
    ast_in_new(&b->pool,
      pool_string_copy(&b->pool, symbol),
      name.p ? pool_string_copy(&b->pool, name) : string_null()));
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

int ast_build_symbol(AstBuilder *b, int level)
{
  return ast_builder_push(b, AST_SYMBOL,
    ast_symbol_new(&b->pool,
      level));
}

int ast_build_lookup(AstBuilder *b, String name)
{
  return ast_builder_push(b, AST_LOOKUP,
    ast_lookup_new(&b->pool,
      ast_to_symbol(ast_builder_pop(b)),
      name));
}
