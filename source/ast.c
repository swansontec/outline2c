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

#include "ast.h"
#include <assert.h>

int ast_is_code_node(Type type)
{
  return
    type == AST_CODE_TEXT ||
    type == AST_INCLUDE ||
    type == AST_FOR ||
    type == AST_SET ||
    type == AST_SYMBOL_REF ||
    type == AST_CALL ||
    type == AST_LOOKUP;
}

int ast_is_filter_node(Type type)
{
  return
    type == AST_FILTER_TAG ||
    type == AST_FILTER_ANY ||
    type == AST_FILTER_NOT ||
    type == AST_FILTER_OR ||
    type == AST_FILTER_AND;
}

AstCodeNode ast_to_code_node(ListNode node)
{
  AstCodeNode temp;
  assert(ast_is_code_node(node.type));
  temp.p = node.p;
  temp.type = node.type;
  return temp;
}

AstFilterNode ast_to_filter_node(Dynamic node)
{
  AstFilterNode temp;
  assert(ast_is_filter_node(node.type));
  temp.p = node.p;
  temp.type = node.type;
  return temp;
}

AstCode *ast_to_code(Dynamic node)
{
  assert(node.type == AST_CODE);
  return node.p;
}

AstOutline *ast_to_outline(Dynamic node)
{
  assert(node.type == AST_OUTLINE);
  return node.p;
}

AstOutlineItem *ast_to_outline_item(ListNode node)
{
  assert(node.type == AST_OUTLINE_ITEM);
  return node.p;
}

AstOutlineTag *ast_to_outline_tag(ListNode node)
{
  assert(node.type == AST_OUTLINE_TAG);
  return node.p;
}

AstMapLine *ast_to_map_line(ListNode node)
{
  assert(node.type == AST_MAP_LINE);
  return node.p;
}

AstFilter *ast_to_filter(Dynamic node)
{
  assert(node.type == AST_FILTER);
  return node.p;
}

AstSymbolRef *ast_to_symbol_ref(Dynamic node)
{
  assert(node.type == AST_SYMBOL_REF);
  return node.p;
}

AstCode *ast_code_new(Pool *p, ListNode *nodes)
{
  AstCode *self = pool_alloc(p, sizeof(AstCode));
  if (!self) return 0;
  self->nodes = nodes;

  /* nodes may be NULL */
  return self;
}

AstCodeText *ast_code_text_new(Pool *p, String code)
{
  AstCodeText *self = pool_alloc(p, sizeof(AstCodeText));
  if (!self) return 0;
  self->code = pool_string_copy(p, code);

  if (!string_size(self->code)) return 0;
  return self;
}

AstInclude *ast_include_new(Pool *p, AstCode *code)
{
  AstInclude *self = pool_alloc(p, sizeof(AstInclude));
  if (!self) return 0;
  self->code = code;

  if (!self->code) return 0;
  return self;
}

AstOutline *ast_outline_new(Pool *p, ListNode *items)
{
  AstOutline *self = pool_alloc(p, sizeof(AstOutline));
  if (!self) return 0;
  self->items = items;

  /* items may be NULL */
  return self;
}

AstOutlineItem *ast_outline_item_new(Pool *p, ListNode *tags, String name, AstOutline *children)
{
  AstOutlineItem *self = pool_alloc(p, sizeof(AstOutlineItem));
  if (!self) return 0;
  self->tags = tags;
  self->name = pool_string_copy(p, name);
  self->children = children;

  /* tags may be NULL */
  if (!string_size(self->name)) return 0;
  /* children may be NULL */
  return self;
}

AstOutlineTag *ast_outline_tag_new(Pool *p, String name, AstCode *value)
{
  AstOutlineTag *self = pool_alloc(p, sizeof(AstOutlineTag));
  if (!self) return 0;
  self->name = pool_string_copy(p, name);
  self->value = value;

  if (!string_size(self->name)) return 0;
  /* value may be NULL */
  return self;
}

AstMap *ast_map_new(Pool *p, Symbol *item, ListNode *lines)
{
  AstMap *self = pool_alloc(p, sizeof(AstMap));
  if (!self) return 0;
  self->item = item;
  self->lines = lines;

  if (!self->item) return 0;
  /* lines may be NULL */
  return self;
}

AstMapLine *ast_map_line_new(Pool *p, AstFilter *filter, AstCode *code)
{
  AstMapLine *self = pool_alloc(p, sizeof(AstMapLine));
  if (!self) return 0;
  self->filter = filter;
  self->code = code;

  if (!self->filter) return 0;
  if (!self->code) return 0;
  return self;
}

AstFor *ast_for_new(Pool *p, Symbol *item, Symbol *outline, AstFilter *filter, int reverse, int list, AstCode *code)
{
  AstFor *self = pool_alloc(p, sizeof(AstFor));
  if (!self) return 0;
  self->item = item;
  self->outline = outline;
  self->filter = filter;
  self->reverse = reverse;
  self->list = list;
  self->code = code;

  if (!self->item) return 0;
  if (!self->outline) return 0;
  /* filter may be NULL */
  if (!self->code) return 0;
  return self;
}

AstFilter *ast_filter_new(Pool *p, AstFilterNode test)
{
  AstFilter *self = pool_alloc(p, sizeof(AstFilter));
  if (!self) return 0;
  self->test = test;

  if (!self->test.p) return 0;
  return self;
}

AstFilterTag *ast_filter_tag_new(Pool *p, String tag)
{
  AstFilterTag *self = pool_alloc(p, sizeof(AstFilterTag));
  if (!self) return 0;
  self->tag = pool_string_copy(p, tag);

  if (!string_size(self->tag)) return 0;
  return self;
}

AstFilterAny *ast_filter_any_new(Pool *p)
{
  AstFilterAny *self = pool_alloc(p, sizeof(AstFilterAny));
  if (!self) return 0;

  return self;
}

AstFilterNot *ast_filter_not_new(Pool *p, AstFilterNode test)
{
  AstFilterNot *self = pool_alloc(p, sizeof(AstFilterNot));
  if (!self) return 0;
  self->test = test;

  if (!self->test.p) return 0;
  return self;
}

AstFilterAnd *ast_filter_and_new(Pool *p, AstFilterNode test_a, AstFilterNode test_b)
{
  AstFilterAnd *self = pool_alloc(p, sizeof(AstFilterAnd));
  if (!self) return 0;
  self->test_a = test_a;
  self->test_b = test_b;

  if (!self->test_a.p) return 0;
  if (!self->test_b.p) return 0;
  return self;
}

AstFilterOr *ast_filter_or_new(Pool *p, AstFilterNode test_a, AstFilterNode test_b)
{
  AstFilterOr *self = pool_alloc(p, sizeof(AstFilterOr));
  if (!self) return 0;
  self->test_a = test_a;
  self->test_b = test_b;

  if (!self->test_a.p) return 0;
  if (!self->test_b.p) return 0;
  return self;
}

AstSet *ast_set_new(Pool *p, Symbol *symbol, Dynamic value)
{
  AstSet *self = pool_alloc(p, sizeof(AstSet));
  if (!self) return 0;
  self->symbol = symbol;
  self->value = value;

  if (!self->symbol) return 0;
  if (!self->value.p) return 0;
  return self;
}

AstSymbolRef *ast_symbol_ref_new(Pool *p, Symbol *symbol)
{
  AstSymbolRef *self = pool_alloc(p, sizeof(AstSymbolRef));
  if (!self) return 0;
  self->symbol = symbol;

  if (!self->symbol) return 0;
  return self;
}

AstCall *ast_call_new(Pool *p, Symbol *f, Symbol *data)
{
  AstCall *self = pool_alloc(p, sizeof(AstCall));
  if (!self) return 0;
  self->f = f;
  self->data = data;

  if (!self->f) return 0;
  if (!self->data) return 0;
  return self;
}

AstLookup *ast_lookup_new(Pool *p, Symbol *symbol, String name)
{
  AstLookup *self = pool_alloc(p, sizeof(AstLookup));
  if (!self) return 0;
  self->symbol = symbol;
  self->name = pool_string_copy(p, name);

  if (!self->symbol) return 0;
  if (!string_size(self->name)) return 0;
  return self;
}
