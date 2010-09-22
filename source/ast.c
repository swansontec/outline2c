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
#include <stdio.h>

int ast_is_code_node(Type type)
{
  return
    type == AST_CODE_TEXT ||
    type == AST_FOR ||
    type == AST_VARIABLE ||
    type == AST_CALL ||
    type == AST_LOOKUP;
}

int ast_is_for_node(Type type)
{
  return
    type == AST_OUTLINE ||
    type == AST_VARIABLE;
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

AstForNode ast_to_for_node(Dynamic node)
{
  AstForNode temp;
  assert(ast_is_for_node(node.type));
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

AstMap *ast_to_map(Dynamic node)
{
  assert(node.type == AST_MAP);
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

AstVariable *ast_to_variable(Dynamic node)
{
  assert(node.type == AST_VARIABLE);
  return node.p;
}

AstCode *ast_code_new(Pool *p, ListNode *nodes)
{
  AstCode *self = pool_alloc(p, sizeof(AstCode));
  CHECK_MEM(self);
  self->nodes = nodes;

  /* nodes may be NULL */
  return self;
}

AstCodeText *ast_code_text_new(Pool *p, String code)
{
  AstCodeText *self = pool_alloc(p, sizeof(AstCodeText));
  CHECK_MEM(self);
  self->code = pool_string_copy(p, code);

  CHECK_MEM(string_size(self->code));
  return self;
}

AstOutline *ast_outline_new(Pool *p, ListNode *items)
{
  AstOutline *self = pool_alloc(p, sizeof(AstOutline));
  CHECK_MEM(self);
  self->items = items;

  /* items may be NULL */
  return self;
}

AstOutlineItem *ast_outline_item_new(Pool *p, ListNode *tags, String name, AstOutline *children)
{
  AstOutlineItem *self = pool_alloc(p, sizeof(AstOutlineItem));
  CHECK_MEM(self);
  self->tags = tags;
  self->name = pool_string_copy(p, name);
  self->children = children;

  /* tags may be NULL */
  CHECK_MEM(string_size(self->name));
  /* children may be NULL */
  return self;
}

AstOutlineTag *ast_outline_tag_new(Pool *p, String name, AstCode *value)
{
  AstOutlineTag *self = pool_alloc(p, sizeof(AstOutlineTag));
  CHECK_MEM(self);
  self->name = pool_string_copy(p, name);
  self->value = value;

  CHECK_MEM(string_size(self->name));
  /* value may be NULL */
  return self;
}

AstMap *ast_map_new(Pool *p, AstVariable *item, ListNode *lines)
{
  AstMap *self = pool_alloc(p, sizeof(AstMap));
  CHECK_MEM(self);
  self->item = item;
  self->lines = lines;

  assert(self->item);
  /* lines may be NULL */
  return self;
}

AstMapLine *ast_map_line_new(Pool *p, AstFilter *filter, AstCode *code)
{
  AstMapLine *self = pool_alloc(p, sizeof(AstMapLine));
  CHECK_MEM(self);
  self->filter = filter;
  self->code = code;

  assert(self->filter);
  assert(self->code);
  return self;
}

AstFor *ast_for_new(Pool *p, AstVariable *item, AstForNode outline, AstFilter *filter, int reverse, int list, AstCode *code)
{
  AstFor *self = pool_alloc(p, sizeof(AstFor));
  CHECK_MEM(self);
  self->item = item;
  self->outline = outline;
  self->filter = filter;
  self->reverse = reverse;
  self->list = list;
  self->code = code;

  assert(self->item);
  assert(self->outline.p);
  /* filter may be NULL */
  assert(self->code);
  return self;
}

AstFilter *ast_filter_new(Pool *p, AstFilterNode test)
{
  AstFilter *self = pool_alloc(p, sizeof(AstFilter));
  CHECK_MEM(self);
  self->test = test;

  assert(self->test.p);
  return self;
}

AstFilterTag *ast_filter_tag_new(Pool *p, String tag)
{
  AstFilterTag *self = pool_alloc(p, sizeof(AstFilterTag));
  CHECK_MEM(self);
  self->tag = pool_string_copy(p, tag);

  CHECK_MEM(string_size(self->tag));
  return self;
}

AstFilterAny *ast_filter_any_new(Pool *p)
{
  AstFilterAny *self = pool_alloc(p, sizeof(AstFilterAny));
  CHECK_MEM(self);

  return self;
}

AstFilterNot *ast_filter_not_new(Pool *p, AstFilterNode test)
{
  AstFilterNot *self = pool_alloc(p, sizeof(AstFilterNot));
  CHECK_MEM(self);
  self->test = test;

  assert(self->test.p);
  return self;
}

AstFilterAnd *ast_filter_and_new(Pool *p, AstFilterNode test_a, AstFilterNode test_b)
{
  AstFilterAnd *self = pool_alloc(p, sizeof(AstFilterAnd));
  CHECK_MEM(self);
  self->test_a = test_a;
  self->test_b = test_b;

  assert(self->test_a.p);
  assert(self->test_b.p);
  return self;
}

AstFilterOr *ast_filter_or_new(Pool *p, AstFilterNode test_a, AstFilterNode test_b)
{
  AstFilterOr *self = pool_alloc(p, sizeof(AstFilterOr));
  CHECK_MEM(self);
  self->test_a = test_a;
  self->test_b = test_b;

  assert(self->test_a.p);
  assert(self->test_b.p);
  return self;
}

AstVariable *ast_variable_new(Pool *p, String name)
{
  AstVariable *self = pool_alloc(p, sizeof(AstVariable));
  CHECK_MEM(self);
  self->name = pool_string_copy(p, name);
  self->value = 0;

  CHECK_MEM(string_size(self->name));
  return self;
}

AstCall *ast_call_new(Pool *p, AstVariable *item, AstMap *map)
{
  AstCall *self = pool_alloc(p, sizeof(AstCall));
  CHECK_MEM(self);
  self->item = item;
  self->map = map;

  assert(self->item);
  assert(self->map);
  return self;
}

AstLookup *ast_lookup_new(Pool *p, AstVariable *item, String name)
{
  AstLookup *self = pool_alloc(p, sizeof(AstLookup));
  CHECK_MEM(self);
  self->item = item;
  self->name = pool_string_copy(p, name);

  assert(self->item);
  CHECK_MEM(string_size(self->name));
  return self;
}
