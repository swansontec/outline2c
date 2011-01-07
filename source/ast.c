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

/**
 * A source-level statement
 */
typedef struct {
  void *p;
  Type type;
} AstCodeNode;

int ast_is_code_node(Type type)
{
  return
    type == AST_CODE_TEXT ||
    type == AST_MAP ||
    type == AST_FOR ||
    type == AST_MACRO_CALL ||
    type == AST_VARIABLE ||
    type == AST_LOOKUP;
}

AstCodeNode ast_to_code_node(ListNode node)
{
  AstCodeNode temp;
  assert(ast_is_code_node(node.type));
  temp.p = node.p;
  temp.type = node.type;
  return temp;
}

/**
 * Possible items the for statement can loop over
 */
typedef struct {
  void *p;
  Type type;
} AstForNode;

int ast_is_for_node(Type type)
{
  return
    type == AST_OUTLINE ||
    type == AST_VARIABLE;
}

AstForNode ast_to_for_node(Dynamic node)
{
  AstForNode temp;
  assert(ast_is_for_node(node.type));
  temp.p = node.p;
  temp.type = node.type;
  return temp;
}

/**
 * A filter element
 */
typedef struct {
  void *p;
  Type type;
} AstFilterNode;

int ast_is_filter_node(Type type)
{
  return
    type == AST_FILTER_TAG ||
    type == AST_FILTER_ANY ||
    type == AST_FILTER_NOT ||
    type == AST_FILTER_OR ||
    type == AST_FILTER_AND;
}

AstFilterNode ast_to_filter_node(Dynamic node)
{
  AstFilterNode temp;
  assert(ast_is_filter_node(node.type));
  temp.p = node.p;
  temp.type = node.type;
  return temp;
}

typedef struct AstCodeText      AstCodeText;
typedef struct AstOutline       AstOutline;
typedef struct AstOutlineItem   AstOutlineItem;
typedef struct AstOutlineTag    AstOutlineTag;
typedef struct AstMap           AstMap;
typedef struct AstMapLine       AstMapLine;
typedef struct AstFor           AstFor;
typedef struct AstFilter        AstFilter;
typedef struct AstFilterTag     AstFilterTag;
typedef struct AstFilterAny     AstFilterAny;
typedef struct AstFilterNot     AstFilterNot;
typedef struct AstFilterAnd     AstFilterAnd;
typedef struct AstFilterOr      AstFilterOr;
typedef struct AstMacro         AstMacro;
typedef struct AstMacroCall     AstMacroCall;
typedef struct AstVariable      AstVariable;
typedef struct AstLookup        AstLookup;

/**
 * A run of text in the host language.
 */
struct AstCodeText {
  String code;
};

/**
 * An outline.
 */
struct AstOutline {
  ListNode *items;
};

/**
 * An individual item in an outline.
 */
struct AstOutlineItem {
  ListNode *tags;
  String name;
  AstOutline *children;
};

/**
 * An individual word in an outline item.
 */
struct AstOutlineTag {
  String name;
  ListNode *value;
};

/**
 * A map statement
 */
struct AstMap
{
  AstVariable *item;
  ListNode *lines;
};

struct AstMapLine
{
  AstFilter *filter;
  ListNode *code;
};

/**
 * A for statement.
 */
struct AstFor {
  AstVariable *item;
  AstForNode outline;
  AstFilter *filter;
  int reverse;
  int list;
  ListNode *code;
};

/**
 * Filters outline items based on the presence or absence of tags.
 */
struct AstFilter {
  AstFilterNode test;
};

/**
 * Accepts an outline item if the given tag is present.
 */
struct AstFilterTag {
  String tag;
};

/**
 * Always returns true.
 */
struct AstFilterAny {
  int dummy;
};

/**
 * Accepts an outline item if the sub-conditions is false.
 */
struct AstFilterNot {
  AstFilterNode test;
};

/**
 * Accepts an outline item if both sub-conditions are true.
 */
struct AstFilterAnd {
  AstFilterNode test_a;
  AstFilterNode test_b;
};

/**
 * Accepts an outline item if either sub-conditions are true.
 */
struct AstFilterOr {
  AstFilterNode test_a;
  AstFilterNode test_b;
};

/**
 * A macro definition
 */
struct AstMacro {
  ListNode *inputs; /* Real type is AstVariable */
  ListNode *code;
};

/**
 * A macro invocation
 */
struct AstMacroCall {
  AstMacro *macro;
  ListNode *inputs; /* Real type is AstForNode */
};

/**
 * A changing value
 */
struct AstVariable {
  String name;
  AstOutlineItem *value;
};

/**
 * A modifier on a symbol.
 */
struct AstLookup {
  AstVariable *item;
  String name;
};

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

AstVariable *ast_to_variable(ListNode node)
{
  assert(node.type == AST_VARIABLE);
  return node.p;
}

AstCodeText *ast_code_text_new(Pool *p, String code)
{
  AstCodeText *self = pool_new(p, AstCodeText);
  CHECK_MEM(self);
  self->code = string_copy(p, code);

  CHECK_MEM(string_size(self->code));
  return self;
}

AstOutlineTag *ast_outline_tag_new(Pool *p, String name, ListNode *value)
{
  AstOutlineTag *self = pool_new(p, AstOutlineTag);
  CHECK_MEM(self);
  self->name = string_copy(p, name);
  self->value = value;

  CHECK_MEM(string_size(self->name));
  /* value may be NULL */
  return self;
}

AstVariable *ast_variable_new(Pool *p, String name)
{
  AstVariable *self = pool_new(p, AstVariable);
  CHECK_MEM(self);
  self->name = string_copy(p, name);
  self->value = 0;

  CHECK_MEM(string_size(self->name));
  return self;
}

AstLookup *ast_lookup_new(Pool *p, AstVariable *item, String name)
{
  AstLookup *self = pool_new(p, AstLookup);
  CHECK_MEM(self);
  self->item = item;
  self->name = string_copy(p, name);

  assert(self->item);
  CHECK_MEM(string_size(self->name));
  return self;
}
