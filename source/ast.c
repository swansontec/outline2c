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

typedef struct AstOutlineItem AstOutlineItem;

/**
 * A modifier on a symbol.
 */
typedef struct {
  AstOutlineItem *item;
  String name;
} AstLookup;

/**
 * A macro definition
 */
typedef struct {
  ListNode *inputs; /* Real type is String packed into AstCodeText */
  Scope *scope;
  Source code;
} AstMacro;

/**
 * A macro invocation
 */
typedef struct {
  AstMacro *macro;
  ListNode *inputs;
} AstMacroCall;

/**
 * Accepts an outline item if the given tag is present.
 */
typedef struct {
  String tag;
} AstFilterTag;

/**
 * Always returns true.
 */
typedef struct {
  int dummy;
} AstFilterAny;

/**
 * Accepts an outline item if the sub-conditions is false.
 */
typedef struct {
  Dynamic test;
} AstFilterNot;

/**
 * Accepts an outline item if both sub-conditions are true.
 */
typedef struct {
  Dynamic test_a;
  Dynamic test_b;
} AstFilterAnd;

/**
 * Accepts an outline item if either sub-conditions are true.
 */
typedef struct {
  Dynamic test_a;
  Dynamic test_b;
} AstFilterOr;

typedef struct AstOutline AstOutline;

/**
 * An individual word in an outline item.
 */
typedef struct {
  String name;
  ListNode *value;
} AstOutlineTag;

/**
 * An individual item in an outline.
 */
struct AstOutlineItem {
  ListNode *tags; /* Real type is AstOutlineTag */
  String name;
  AstOutline *children;
};

/**
 * An outline.
 */
struct AstOutline {
  ListNode *items; /* Real type is AstOutlineItem */
};

typedef struct {
  Dynamic filter;
  ListNode *code;
} AstMapLine;

/**
 * A map statement
 */
typedef struct {
  AstOutlineItem *item;
  ListNode *lines; /* Real type is AstMapLine */
} AstMap;

/**
 * A for statement.
 */
typedef struct {
  String item;
  Dynamic outline;
  Dynamic filter;
  int reverse;
  int list;
  Scope *scope;
  Source code;
} AstFor;

/**
 * A run of text in the host language.
 */
typedef struct {
  String code;
} AstCodeText;

AstOutlineTag *ast_to_outline_tag(Dynamic node)
{
  assert(node.type == AST_OUTLINE_TAG);
  return node.p;
}

AstOutlineItem *ast_to_outline_item(Dynamic node)
{
  assert(node.type == AST_OUTLINE_ITEM);
  return node.p;
}

AstMapLine *ast_to_map_line(Dynamic node)
{
  assert(node.type == AST_MAP_LINE);
  return node.p;
}

AstLookup *ast_lookup_new(Pool *p, AstOutlineItem *item, String name)
{
  AstLookup *self = pool_new(p, AstLookup);
  CHECK_MEM(self);
  self->item = item;
  self->name = string_copy(p, name);

  assert(self->item);
  CHECK_MEM(string_size(self->name));
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

AstCodeText *ast_code_text_new(Pool *p, String code)
{
  AstCodeText *self = pool_new(p, AstCodeText);
  CHECK_MEM(self);
  self->code = string_copy(p, code);

  CHECK_MEM(string_size(self->code));
  return self;
}

/**
 * The ability to appear in debug dumps
 */
void dump(Dynamic node, int indent);

/**
 * The ability to test against an outline item
 */
int test_filter(Dynamic test, AstOutlineItem *item);
int can_test_filter(Dynamic value)
{
  return
    value.type == AST_FILTER_TAG ||
    value.type == AST_FILTER_ANY ||
    value.type == AST_FILTER_NOT ||
    value.type == AST_FILTER_OR ||
    value.type == AST_FILTER_AND;
}

/**
 * The ability to behave as an outline
 */
ListNode *get_items(Dynamic node);
int can_get_items(Dynamic value)
{
  return
    value.type == AST_OUTLINE_ITEM ||
    value.type == AST_OUTLINE;
}

/**
 * The ability to generate output text
 */
int generate(Pool *pool, FILE *out, Dynamic node);
int can_generate(Dynamic value)
{
  return
    value.type == AST_LOOKUP ||
    value.type == AST_MACRO_CALL ||
    value.type == AST_OUTLINE_ITEM ||
    value.type == AST_MAP ||
    value.type == AST_FOR ||
    value.type == AST_CODE_TEXT;
}
