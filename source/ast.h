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

#include "typedefs.h"
#include "string.h"

typedef struct ast_file                 AstFile;
typedef struct ast_code                 AstCode;
typedef struct ast_code_text            AstCodeText;
typedef struct ast_include              AstInclude;
typedef struct ast_outline              AstOutline;
typedef struct ast_outline_list         AstOutlineList;
typedef struct ast_outline_item         AstOutlineItem;
typedef struct ast_outline_tag          AstOutlineTag;
typedef struct ast_map                  AstMap;
typedef struct ast_map_line             AstMapLine;
typedef struct ast_for                  AstFor;
typedef struct ast_in                   AstIn;
typedef struct ast_filter               AstFilter;
typedef struct ast_filter_tag           AstFilterTag;
typedef struct ast_filter_not           AstFilterNot;
typedef struct ast_filter_and           AstFilterAnd;
typedef struct ast_filter_or            AstFilterOr;
typedef struct ast_symbol               AstSymbol;
typedef struct ast_lookup               AstLookup;

typedef struct ast_node                 AstNode;
typedef struct ast_code_node            AstCodeNode;
typedef struct ast_filter_node          AstFilterNode;

/**
 * Types
 */
enum ast_type {
  AST_FILE,
  AST_CODE,
  AST_CODE_TEXT,
  AST_INCLUDE,
  AST_OUTLINE,
  AST_OUTLINE_LIST,
  AST_OUTLINE_ITEM,
  AST_OUTLINE_TAG,
  AST_MAP,
  AST_MAP_LINE,
  AST_FOR,
  AST_IN,
  AST_FILTER,
  AST_FILTER_TAG,
  AST_FILTER_NOT,
  AST_FILTER_AND,
  AST_FILTER_OR,
  AST_SYMBOL,
  AST_LOOKUP
};
typedef enum ast_type AstType;

/**
 * Points to any AST node.
 */
struct ast_node {
  void *p;
  AstType type;
};

/**
 * Points to one of:
 *  AstCodeText
 *  AstInclude
 *  AstOutline
 *  AstMap
 *  AstFor
 *  AstSymbol
 *  AstLookup
 */
struct ast_code_node {
  void *p;
  AstType type;
};

/**
 * Points to one of:
 *  AstFilterTag
 *  AstFilterNot
 *  AstFilterAnd
 *  AstFilterOr
 */
struct ast_filter_node {
  void *p;
  AstType type;
};

/* Type-checking functions */
int ast_is_code_node(AstNode node);
int ast_is_filter_node(AstNode node);

/* Type-conversion functions */
AstCodeNode         ast_to_code_node(AstNode node);
AstFilterNode       ast_to_filter_node(AstNode node);

AstFile            *ast_to_file(AstNode node);
AstCode            *ast_to_code(AstNode node);
AstOutlineList     *ast_to_outline_list(AstNode node);
AstOutlineItem     *ast_to_outline_item(AstNode node);
AstOutlineTag      *ast_to_outline_tag(AstNode node);
AstMapLine         *ast_to_map_line(AstNode node);
AstIn              *ast_to_in(AstNode node);
AstFilter          *ast_to_filter(AstNode node);
AstSymbol          *ast_to_symbol(AstNode node);

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
 * The outline keyword
 */
struct ast_outline {
  String name;
  AstOutlineList *children;
};

/**
 * A list of items in an outline.
 */
struct ast_outline_list {
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
  AstOutlineList *children;
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
  String name;
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
  AstIn *in;
  AstFilter *filter;
  AstCode *code;
};

/**
 * An "x in y" portion of a for statement.
 */
struct ast_in {
  String symbol;
  String name;
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
 * A symbol to be replaced within a block of code.
 */
struct ast_symbol {
  int level;
};

/**
 * A modifier on a symbol.
 */
struct ast_lookup {
  AstSymbol *symbol;
  String name;
};

AstFile            *ast_file_new                (Pool *p, AstCode *code);
AstCode            *ast_code_new                (Pool *p, AstCodeNode *nodes, AstCodeNode *nodes_end);
AstCodeText        *ast_code_text_new           (Pool *p, String code);
AstInclude         *ast_include_new             (Pool *p, AstFile *file);
AstOutline         *ast_outline_new             (Pool *p, String name, AstOutlineList *children);
AstOutlineList     *ast_outline_list_new        (Pool *p, AstOutlineItem **items, AstOutlineItem **items_end);
AstOutlineItem     *ast_outline_item_new        (Pool *p, AstOutlineTag **tags, AstOutlineTag **tags_end, String name, AstOutlineList *children);
AstOutlineTag      *ast_outline_tag_new         (Pool *p, String name, AstCode *value);
AstMap             *ast_map_new                 (Pool *p, String name, AstMapLine **lines, AstMapLine **lines_end);
AstMapLine         *ast_map_line_new            (Pool *p, AstFilter *filter, AstCode *code);
AstFor             *ast_for_new                 (Pool *p, AstIn *in, AstFilter *filter, AstCode *code);
AstIn              *ast_in_new                  (Pool *p, String symbol, String name);
AstFilter          *ast_filter_new              (Pool *p, AstFilterNode test);
AstFilterTag       *ast_filter_tag_new          (Pool *p, String tag);
AstFilterNot       *ast_filter_not_new          (Pool *p, AstFilterNode test);
AstFilterAnd       *ast_filter_and_new          (Pool *p, AstFilterNode test_a, AstFilterNode test_b);
AstFilterOr        *ast_filter_or_new           (Pool *p, AstFilterNode test_a, AstFilterNode test_b);
AstSymbol          *ast_symbol_new              (Pool *p, int level);
AstLookup          *ast_lookup_new              (Pool *p, AstSymbol *symbol, String name);

#endif
