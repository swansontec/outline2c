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

#include "scope.h"

typedef struct ast_file                 AstFile;
typedef struct ast_code                 AstCode;
typedef struct ast_code_text            AstCodeText;
typedef struct ast_include              AstInclude;
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
typedef struct ast_set                  AstSet;
typedef struct ast_symbol_ref           AstSymbolRef;
typedef struct ast_call                 AstCall;
typedef struct ast_lookup               AstLookup;

typedef struct ast_code_node            AstCodeNode;
typedef struct ast_filter_node          AstFilterNode;

/**
 * Points to one of:
 *  AstCodeText
 *  AstInclude
 *  AstFor
 *  AstSet
 *  AstSymbolRef
 *  AstCall
 *  AstLookup
 */
struct ast_code_node {
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
int ast_is_filter_node(Type type);

/* Type-conversion functions */
AstCodeNode         ast_to_code_node(Dynamic node);
AstFilterNode       ast_to_filter_node(Dynamic node);

AstFile            *ast_to_file(Dynamic node);
AstCode            *ast_to_code(Dynamic node);
AstOutline         *ast_to_outline(Dynamic node);
AstOutlineItem     *ast_to_outline_item(Dynamic node);
AstOutlineTag      *ast_to_outline_tag(Dynamic node);
AstMapLine         *ast_to_map_line(Dynamic node);
AstFilter          *ast_to_filter(Dynamic node);
AstSymbolRef       *ast_to_symbol_ref(Dynamic node);

/**
 * A source file. This is the top-level element of the AST.
 */
struct ast_file {
  AstCode *code;
};

/**
 * A block of code in the host language, possibly interspersed with o2c escape
 * sequences and replacement symbols.
 */
struct ast_code {
  AstCodeNode *nodes;
  AstCodeNode *nodes_end;
};

/**
 * A run of text in the host language.
 */
struct ast_code_text {
  String code;
};

/**
 * The include keyword.
 */
struct ast_include {
  AstFile *file;
};

/**
 * An outline.
 */
struct ast_outline {
  AstOutlineItem **items;
  AstOutlineItem **items_end;
};

/**
 * An individual item in an outline.
 */
struct ast_outline_item {
  AstOutlineTag **tags;
  AstOutlineTag **tags_end;
  String name;
  AstOutline *children;
};

/**
 * An individual word in an outline item.
 */
struct ast_outline_tag {
  String name;
  AstCode *value;
};

/**
 * A map statement
 */
struct ast_map
{
  Symbol *item;
  AstMapLine **lines;
  AstMapLine **lines_end;
};

struct ast_map_line
{
  AstFilter *filter;
  AstCode *code;
};

/**
 * A for statement.
 */
struct ast_for {
  Symbol *item;
  Symbol *outline;
  AstFilter *filter;
  int reverse;
  int list;
  AstCode *code;
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
 * An assignment statement.
 */
struct ast_set {
  Symbol *symbol;
  Dynamic value;
};

/**
 * A symbol to be replaced within a block of code.
 */
struct ast_symbol_ref {
  Symbol *symbol;
};

/**
 * A call to a map
 */
struct ast_call {
  Symbol *f;
  Symbol *data;
};

/**
 * A modifier on a symbol.
 */
struct ast_lookup {
  Symbol *symbol;
  String name;
};

AstFile            *ast_file_new                (Pool *p, AstCode *code);
AstCode            *ast_code_new                (Pool *p, AstCodeNode *nodes, AstCodeNode *nodes_end);
AstCodeText        *ast_code_text_new           (Pool *p, String code);
AstInclude         *ast_include_new             (Pool *p, AstFile *file);
AstOutline         *ast_outline_new             (Pool *p, AstOutlineItem **items, AstOutlineItem **items_end);
AstOutlineItem     *ast_outline_item_new        (Pool *p, AstOutlineTag **tags, AstOutlineTag **tags_end, String name, AstOutline *children);
AstOutlineTag      *ast_outline_tag_new         (Pool *p, String name, AstCode *value);
AstMap             *ast_map_new                 (Pool *p, Symbol *item, AstMapLine **lines, AstMapLine **lines_end);
AstMapLine         *ast_map_line_new            (Pool *p, AstFilter *filter, AstCode *code);
AstFor             *ast_for_new                 (Pool *p, Symbol *item, Symbol *outline, AstFilter *filter, int reverse, int list, AstCode *code);
AstFilter          *ast_filter_new              (Pool *p, AstFilterNode test);
AstFilterTag       *ast_filter_tag_new          (Pool *p, String tag);
AstFilterAny       *ast_filter_any_new          (Pool *p);
AstFilterNot       *ast_filter_not_new          (Pool *p, AstFilterNode test);
AstFilterAnd       *ast_filter_and_new          (Pool *p, AstFilterNode test_a, AstFilterNode test_b);
AstFilterOr        *ast_filter_or_new           (Pool *p, AstFilterNode test_a, AstFilterNode test_b);
AstSet             *ast_set_new                 (Pool *p, Symbol *symbol, Dynamic value);
AstSymbolRef       *ast_symbol_ref_new          (Pool *p, Symbol *symbol);
AstCall            *ast_call_new                (Pool *p, Symbol *f, Symbol *data);
AstLookup          *ast_lookup_new              (Pool *p, Symbol *symbol, String name);

#endif
