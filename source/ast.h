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

#ifndef AST_H_INCLUDED
#define AST_H_INCLUDED

#include "list.h"

typedef struct ast_code_text            AstCodeText;
typedef struct ast_outline              AstOutline;
typedef struct ast_outline_item         AstOutlineItem;
typedef struct ast_outline_tag          AstOutlineTag;
typedef struct ast_map                  AstMap;
typedef struct ast_map_line             AstMapLine;
typedef struct ast_for                  AstFor;
typedef struct ast_filter               AstFilter;
typedef struct ast_filter_tag           AstFilterTag;
typedef struct ast_filter_any           AstFilterAny;
typedef struct ast_filter_not           AstFilterNot;
typedef struct ast_filter_and           AstFilterAnd;
typedef struct ast_filter_or            AstFilterOr;
typedef struct ast_macro                AstMacro;
typedef struct ast_macro_call           AstMacroCall;
typedef struct ast_variable             AstVariable;
typedef struct ast_map_call             AstMapCall;
typedef struct ast_lookup               AstLookup;

typedef struct ast_code_node            AstCodeNode;
typedef struct ast_for_node             AstForNode;
typedef struct ast_filter_node          AstFilterNode;

/**
 * Points to one of:
 *  AstCodeText
 *  AstFor
 *  AstMacroCall
 *  AstVariable
 *  AstMapCall
 *  AstLookup
 */
struct ast_code_node {
  void *p;
  Type type;
};

/**
 * Points to one of:
 *  AstOutline
 *  AstVariable
 */
struct ast_for_node {
  void *p;
  Type type;
};

/**
 * Points to one of:
 *  AstFilterTag
 *  AstFilterAny
 *  AstFilterNot
 *  AstFilterAnd
 *  AstFilterOr
 */
struct ast_filter_node {
  void *p;
  Type type;
};

/* Type-checking functions */
int ast_is_code_node(Type type);
int ast_is_for_node(Type type);
int ast_is_filter_node(Type type);

/* Type-conversion functions */
AstCodeNode         ast_to_code_node(ListNode node);
AstForNode          ast_to_for_node(Dynamic node);
AstFilterNode       ast_to_filter_node(Dynamic node);

AstOutline         *ast_to_outline(Dynamic node);
AstOutlineItem     *ast_to_outline_item(ListNode node);
AstOutlineTag      *ast_to_outline_tag(ListNode node);
AstMap             *ast_to_map(Dynamic node);
AstMapLine         *ast_to_map_line(ListNode node);
AstFilter          *ast_to_filter(Dynamic node);
AstVariable        *ast_to_variable(ListNode node);

/**
 * A run of text in the host language.
 */
struct ast_code_text {
  String code;
};

/**
 * An outline.
 */
struct ast_outline {
  ListNode *items;
};

/**
 * An individual item in an outline.
 */
struct ast_outline_item {
  ListNode *tags;
  String name;
  AstOutline *children;
};

/**
 * An individual word in an outline item.
 */
struct ast_outline_tag {
  String name;
  ListNode *value;
};

/**
 * A map statement
 */
struct ast_map
{
  AstVariable *item;
  ListNode *lines;
};

struct ast_map_line
{
  AstFilter *filter;
  ListNode *code;
};

/**
 * A for statement.
 */
struct ast_for {
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
struct ast_filter {
  AstFilterNode test;
};

/**
 * Accepts an outline item if the given tag is present.
 */
struct ast_filter_tag {
  String tag;
};

/**
 * Always returns true.
 */
struct ast_filter_any {
  int dummy;
};

/**
 * Accepts an outline item if the sub-conditions is false.
 */
struct ast_filter_not {
  AstFilterNode test;
};

/**
 * Accepts an outline item if both sub-conditions are true.
 */
struct ast_filter_and {
  AstFilterNode test_a;
  AstFilterNode test_b;
};

/**
 * Accepts an outline item if either sub-conditions are true.
 */
struct ast_filter_or {
  AstFilterNode test_a;
  AstFilterNode test_b;
};

/**
 * A macro definition
 */
struct ast_macro {
  ListNode *inputs; /* Real type is AstVariable */
  ListNode *code;
};

/**
 * A macro invocation
 */
struct ast_macro_call {
  AstMacro *macro;
  ListNode *inputs; /* Real type is AstForNode */
};

/**
 * A changing value
 */
struct ast_variable {
  String name;
  AstOutlineItem *value;
};

/**
 * A call to a map
 */
struct ast_map_call {
  AstVariable *item;
  AstMap *map;
};

/**
 * A modifier on a symbol.
 */
struct ast_lookup {
  AstVariable *item;
  String name;
};

AstCodeText        *ast_code_text_new           (Pool *p, String code);
AstOutlineTag      *ast_outline_tag_new         (Pool *p, String name, ListNode *value);
AstVariable        *ast_variable_new            (Pool *p, String name);
AstMapCall         *ast_map_call_new            (Pool *p, AstVariable *item, AstMap *map);
AstLookup          *ast_lookup_new              (Pool *p, AstVariable *item, String name);

#endif
