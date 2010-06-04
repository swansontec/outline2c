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

int ast_is_code_node(AstNode node)
{
  return
    node.type == AST_CODE_TEXT ||
    node.type == AST_INCLUDE ||
    node.type == AST_OUTLINE ||
    node.type == AST_MAP ||
    node.type == AST_FOR ||
    node.type == AST_SET ||
    node.type == AST_SYMBOL_REF ||
    node.type == AST_CALL ||
    node.type == AST_LOOKUP;
}

int ast_is_filter_node(AstNode node)
{
  return
    node.type == AST_FILTER_TAG ||
    node.type == AST_FILTER_ANY ||
    node.type == AST_FILTER_NOT ||
    node.type == AST_FILTER_OR ||
    node.type == AST_FILTER_AND;
}

AstCodeNode ast_to_code_node(AstNode node)
{
  AstCodeNode temp;
  assert(ast_is_code_node(node));
  temp.p = node.p;
  temp.type = node.type;
  return temp;
}

AstFilterNode ast_to_filter_node(AstNode node)
{
  AstFilterNode temp;
  assert(ast_is_filter_node(node));
  temp.p = node.p;
  temp.type = node.type;
  return temp;
}

AstFile *ast_to_file(AstNode node)
{
  assert(node.type == AST_FILE);
  return node.p;
}

AstCode *ast_to_code(AstNode node)
{
  assert(node.type == AST_CODE);
  return node.p;
}

AstOutline *ast_to_outline(AstNode node)
{
  assert(node.type == AST_OUTLINE);
  return node.p;
}

AstOutlineItem *ast_to_outline_item(AstNode node)
{
  assert(node.type == AST_OUTLINE_ITEM);
  return node.p;
}

AstOutlineTag *ast_to_outline_tag(AstNode node)
{
  assert(node.type == AST_OUTLINE_TAG);
  return node.p;
}

AstMapLine *ast_to_map_line(AstNode node)
{
  assert(node.type == AST_MAP_LINE);
  return node.p;
}

AstFilter *ast_to_filter(AstNode node)
{
  assert(node.type == AST_FILTER);
  return node.p;
}

AstSymbolNew *ast_to_symbol_new(AstNode node)
{
  assert(node.type == AST_SYMBOL_NEW);
  return node.p;
}

AstSymbolRef *ast_to_symbol_ref(AstNode node)
{
  assert(node.type == AST_SYMBOL_REF);
  return node.p;
}

AstFile *ast_file_new(Pool *p, AstCode *code)
{
  AstFile *self;
  if (!code) return 0;

  self = pool_alloc(p, sizeof(AstFile));
  if (!self) return 0;
  self->code = code;
  return self;
}

AstCode *ast_code_new(Pool *p, AstCodeNode *nodes, AstCodeNode *nodes_end)
{
  AstCode *self;
  if (!nodes) return 0;

  self = pool_alloc(p, sizeof(AstCode));
  if (!self) return 0;
  self->nodes = nodes;
  self->nodes_end = nodes_end;
  return self;
}

AstCodeText *ast_code_text_new(Pool *p, String code)
{
  AstCodeText *self;
  if (!string_size(code)) return 0;

  self = pool_alloc(p, sizeof(AstCodeText));
  if (!self) return 0;
  self->code = code;
  return self;
}

AstInclude *ast_include_new(Pool *p, AstFile *file)
{
  AstInclude *self;
  if (!file) return 0;

  self = pool_alloc(p, sizeof(AstInclude));
  if (!self) return 0;
  self->file = file;
  return self;
}

AstOutline *ast_outline_new(Pool *p, AstOutlineItem **items, AstOutlineItem **items_end)
{
  AstOutline *self;
  if (!items) return 0;

  self = pool_alloc(p, sizeof(AstOutline));
  if (!self) return 0;
  self->items = items;
  self->items_end = items_end;
  return self;
}

AstOutlineItem *ast_outline_item_new(Pool *p, AstOutlineTag **tags, AstOutlineTag **tags_end, String name, AstOutline *children)
{
  AstOutlineItem *self;
  if (!tags) return 0;
  if (!string_size(name)) return 0;
  /* children may be NULL */

  self = pool_alloc(p, sizeof(AstOutlineItem));
  if (!self) return 0;
  self->tags = tags;
  self->tags_end = tags_end;
  self->name = name;
  self->children = children;
  return self;
}

AstOutlineTag *ast_outline_tag_new(Pool *p, String name, AstCode *value)
{
  AstOutlineTag *self;
  if (!string_size(name)) return 0;
  /* value may be NULL */

  self = pool_alloc(p, sizeof(AstOutlineTag));
  if (!self) return 0;
  self->name = name;
  self->value = value;
  return self;
}

AstMap *ast_map_new(Pool *p, String name, AstMapLine **lines, AstMapLine **lines_end)
{
  AstMap *self;
  if (!string_size(name)) return 0;
  if (!lines) return 0;

  self = pool_alloc(p, sizeof(AstMap));
  if (!self) return 0;
  self->name = name;
  self->lines = lines;
  self->lines_end = lines_end;
  return self;
}

AstMapLine *ast_map_line_new(Pool *p, AstFilter *filter, AstCode *code)
{
  AstMapLine *self;
  if (!filter) return 0;
  if (!code) return 0;

  self = pool_alloc(p, sizeof(AstMapLine));
  if (!self) return 0;
  self->filter = filter;
  self->code = code;
  return self;
}

AstFor *ast_for_new(Pool *p, AstSymbolNew *symbol, AstSymbolRef *outline, AstFilter *filter, int reverse, int list, AstCode *code)
{
  AstFor *self;
  if (!symbol) return 0;
  if (!outline) return 0;
  /* filter may be NULL */
  if (!code) return 0;

  self = pool_alloc(p, sizeof(AstFor));
  if (!self) return 0;
  self->symbol = symbol;
  self->outline = outline;
  self->filter = filter;
  self->reverse = reverse;
  self->list = list;
  self->code = code;
  return self;
}

AstFilter *ast_filter_new(Pool *p, AstFilterNode test)
{
  AstFilter *self;
  if (!test.p) return 0;

  self = pool_alloc(p, sizeof(AstFilter));
  if (!self) return 0;
  self->test = test;
  return self;
}

AstFilterTag *ast_filter_tag_new(Pool *p, String tag)
{
  AstFilterTag *self;
  if (!string_size(tag)) return 0;

  self = pool_alloc(p, sizeof(AstFilterTag));
  if (!self) return 0;
  self->tag = tag;
  return self;
}

AstFilterAny *ast_filter_any_new(Pool *p)
{
  AstFilterAny *self;

  self = pool_alloc(p, sizeof(AstFilterAny));
  if (!self) return 0;
  return self;
}

AstFilterNot *ast_filter_not_new(Pool *p, AstFilterNode test)
{
  AstFilterNot *self;
  if (!test.p) return 0;

  self = pool_alloc(p, sizeof(AstFilterNot));
  if (!self) return 0;
  self->test = test;
  return self;
}

AstFilterAnd *ast_filter_and_new(Pool *p, AstFilterNode test_a, AstFilterNode test_b)
{
  AstFilterAnd *self;
  if (!test_a.p) return 0;
  if (!test_b.p) return 0;

  self = pool_alloc(p, sizeof(AstFilterAnd));
  if (!self) return 0;
  self->test_a = test_a;
  self->test_b = test_b;
  return self;
}

AstFilterOr *ast_filter_or_new(Pool *p, AstFilterNode test_a, AstFilterNode test_b)
{
  AstFilterOr *self;
  if (!test_a.p) return 0;
  if (!test_b.p) return 0;

  self = pool_alloc(p, sizeof(AstFilterOr));
  if (!self) return 0;
  self->test_a = test_a;
  self->test_b = test_b;
  return self;
}

AstSet *ast_set_new(Pool *p, AstSymbolNew *symbol, AstCodeNode value)
{
  AstSet *self;
  if (!symbol) return 0;
  if (!value.p) return 0;

  self = pool_alloc(p, sizeof(AstSet));
  if (!self) return 0;
  self->symbol = symbol;
  self->value = value;
  return self;
}

AstSymbolNew *ast_symbol_new_new(Pool *p, String symbol)
{
  AstSymbolNew *self;
  if (!string_size(symbol)) return 0;

  self = pool_alloc(p, sizeof(AstSymbolNew));
  if (!self) return 0;
  self->symbol = symbol;
  self->value = 0;
  self->type = AST_END;
  return self;
}

AstSymbolRef *ast_symbol_ref_new(Pool *p, AstSymbolNew *symbol)
{
  AstSymbolRef *self;
  if (!symbol) return 0;

  self = pool_alloc(p, sizeof(AstSymbolRef));
  if (!self) return 0;
  self->symbol = symbol;
  return self;
}

AstCall *ast_call_new(Pool *p, AstSymbolRef *f, AstSymbolRef *data)
{
  AstCall *self;
  if (!f) return 0;
  if (!data) return 0;

  self = pool_alloc(p, sizeof(AstCall));
  if (!self) return 0;
  self->f = f;
  self->data = data;
  return self;
}

AstLookup *ast_lookup_new(Pool *p, AstSymbolRef *symbol, String name)
{
  AstLookup *self;
  if (!symbol) return 0;
  if (!string_size(name)) return 0;

  self = pool_alloc(p, sizeof(AstLookup));
  if (!self) return 0;
  self->symbol = symbol;
  self->name = name;
  return self;
}
