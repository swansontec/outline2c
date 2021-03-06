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

typedef int (*KeywordFn)(Pool *pool, Source *in, Scope *scope, OutRoutine or);

/**
 * An outline2c keyword.
 */
typedef struct {
  KeywordFn code;
} Keyword;

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

/* AstFilterAny has no data */

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
  assert(node.type == type_outline_tag);
  return node.p;
}

AstOutlineItem *ast_to_outline_item(Dynamic node)
{
  assert(node.type == type_outline_item);
  return node.p;
}

AstMapLine *ast_to_map_line(Dynamic node)
{
  assert(node.type == type_map_line);
  return node.p;
}

Keyword *keyword_new(Pool *p, KeywordFn code)
{
  Keyword *self = pool_new(p, Keyword);
  self->code = code;

  if (!self->code) return 0;
  return self;
}

AstLookup *ast_lookup_new(Pool *p, AstOutlineItem *item, String name)
{
  AstLookup *self = pool_new(p, AstLookup);
  self->item = item;
  self->name = string_copy(p, name);

  assert(self->item);
  return self;
}

AstOutlineTag *ast_outline_tag_new(Pool *p, String name, ListNode *value)
{
  AstOutlineTag *self = pool_new(p, AstOutlineTag);
  self->name = string_copy(p, name);
  self->value = value; /* value may be NULL */
  return self;
}

AstCodeText *ast_code_text_new(Pool *p, String code)
{
  AstCodeText *self = pool_new(p, AstCodeText);
  self->code = string_copy(p, code);
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
    value.type == type_filter_tag ||
    value.type == type_filter_any ||
    value.type == type_filter_not ||
    value.type == type_filter_or ||
    value.type == type_filter_and;
}

/**
 * The ability to behave as an outline
 */
ListNode *get_items(Dynamic node);
int can_get_items(Dynamic value)
{
  return
    value.type == type_outline_item ||
    value.type == type_outline;
}

/**
 * The ability to generate output text
 */
int generate(Pool *pool, FILE *out, Dynamic node);
int can_generate(Dynamic value)
{
  return
    value.type == type_lookup ||
    value.type == type_macro_call ||
    value.type == type_outline_item ||
    value.type == type_map ||
    value.type == type_for ||
    value.type == type_code_text;
}
