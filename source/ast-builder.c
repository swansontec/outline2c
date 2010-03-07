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
 * Searches for an assignment statement in scope which has a name matching the
 * given symbol. This search is insanely inefficient, and could really use
 * some sort of fast data structure to accelerate it.
 * @return a pointer to the found node, or 0 for no match
 */
AstPatternAssign *ast_builder_find_assign(AstBuilder *b, String symbol)
{
  size_t top = b->stack_top;
  AstPatternAssign *temp;

  while (top) {
    --top;
    if (b->stack[top].type == AST_PATTERN) {
      temp = ast_pattern_find_assign(b->stack[top].p, symbol);
      if (temp) return temp;
    }
  }
  return 0;
}

/**
 * Returns non-zero if a pattern has the specified symbol in its assignment
 * list.
 */
AstPatternAssign *ast_pattern_find_assign(AstPattern *pattern, String symbol)
{
  AstPatternNode *node;

  for (node = pattern->nodes; node != pattern->nodes_end; ++node) {
    if (node->type == AST_PATTERN_ASSIGN) {
      AstPatternAssign *p = node->p;
      if (string_equal(p->symbol, symbol))
        return p;
    }
  }
  return 0;
}

/*
 * Functions for assembling an AST. All functions return 0 on success.
 */
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

int ast_build_outline(AstBuilder *b, String name)
{
  return ast_builder_push(b, AST_OUTLINE,
    ast_outline_new(&b->pool,
      name,
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

int ast_build_outline_item(AstBuilder *b, size_t node_n)
{
  size_t i;
  AstOutlineNode *nodes;
  AstOutlineList *children;

  children = ast_builder_peek(b).type == AST_OUTLINE_LIST ?
    ast_to_outline_list(ast_builder_pop(b)) : 0;

  nodes = pool_alloc(&b->pool, node_n*sizeof(AstOutlineNode));
  if (!nodes) return 1;

  b->stack_top -= node_n;
  for (i = 0; i < node_n; ++i)
    nodes[i] = ast_to_outline_node(b->stack[b->stack_top + i]);

  return ast_builder_push(b, AST_OUTLINE_ITEM,
    ast_outline_item_new(&b->pool, nodes, nodes + node_n, children));
}

int ast_build_outline_symbol(AstBuilder *b, String symbol)
{
  return ast_builder_push(b, AST_OUTLINE_SYMBOL,
    ast_outline_symbol_new(&b->pool,
      pool_string_copy(&b->pool, symbol)));
}

int ast_build_outline_string(AstBuilder *b, String string)
{
  return ast_builder_push(b, AST_OUTLINE_STRING,
    ast_outline_string_new(&b->pool,
      pool_string_copy(&b->pool, string)));
}

int ast_build_outline_number(AstBuilder *b, String number)
{
  return ast_builder_push(b, AST_OUTLINE_NUMBER,
    ast_outline_number_new(&b->pool,
      pool_string_copy(&b->pool, number)));
}

int ast_build_match(AstBuilder *b, size_t line_n)
{
  size_t i;
  AstMatchLine **lines;

  lines = pool_alloc(&b->pool, line_n*sizeof(AstMatchLine *));
  if (!lines) return 1;

  b->stack_top -= line_n;
  for (i = 0; i < line_n; ++i)
    lines[i] = ast_to_match_line(b->stack[b->stack_top + i]);

  return ast_builder_push(b, AST_MATCH,
    ast_match_new(&b->pool, lines, lines + line_n));
}

int ast_build_match_line(AstBuilder *b)
{
  return ast_builder_push(b, AST_MATCH_LINE,
    ast_match_line_new(&b->pool,
      ast_to_pattern(ast_builder_pop(b)),
      ast_to_code(ast_builder_pop(b))));
}

int ast_build_pattern(AstBuilder *b, size_t node_n)
{
  size_t i;
  AstPatternNode *nodes;

  nodes = pool_alloc(&b->pool, node_n*sizeof(AstPatternNode));
  if (!nodes) return 1;

  b->stack_top -= node_n;
  for (i = 0; i < node_n; ++i)
    nodes[i] = ast_to_pattern_node(b->stack[b->stack_top + i]);

  return ast_builder_push(b, AST_PATTERN,
    ast_pattern_new(&b->pool, nodes, nodes + node_n));
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
      ast_to_pattern_node(ast_builder_pop(b))));
}

int ast_build_code_symbol(AstBuilder *b, AstPatternAssign *symbol)
{
  return ast_builder_push(b, AST_CODE_SYMBOL,
    ast_code_symbol_new(&b->pool,
      symbol));
}

int ast_build_code_upper(AstBuilder *b)
{
  return ast_builder_push(b, AST_CODE_UPPER,
    ast_code_upper_new(&b->pool,
      ast_to_code_symbol_node(ast_builder_pop(b))));
}

int ast_build_code_lower(AstBuilder *b)
{
  return ast_builder_push(b, AST_CODE_LOWER,
    ast_code_lower_new(&b->pool,
      ast_to_code_symbol_node(ast_builder_pop(b))));
}

int ast_build_code_camel(AstBuilder *b)
{
  return ast_builder_push(b, AST_CODE_CAMEL,
    ast_code_camel_new(&b->pool,
      ast_to_code_symbol_node(ast_builder_pop(b))));
}

int ast_build_code_mixed(AstBuilder *b)
{
  return ast_builder_push(b, AST_CODE_MIXED,
    ast_code_mixed_new(&b->pool,
      ast_to_code_symbol_node(ast_builder_pop(b))));
}

int ast_build_code_string(AstBuilder *b)
{
  return ast_builder_push(b, AST_CODE_STRING,
    ast_code_string_new(&b->pool,
      ast_to_code_symbol_node(ast_builder_pop(b))));
}
