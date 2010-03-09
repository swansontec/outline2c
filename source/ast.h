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
typedef struct ast_outline_symbol       AstOutlineSymbol;
typedef struct ast_outline_string       AstOutlineString;
typedef struct ast_outline_number       AstOutlineNumber;
typedef struct ast_symbol               AstSymbol;

typedef struct ast_match                AstMatch;
typedef struct ast_match_line           AstMatchLine;
typedef struct ast_pattern              AstPattern;
typedef struct ast_pattern_wild         AstPatternWild;
typedef struct ast_pattern_symbol       AstPatternSymbol;
typedef struct ast_pattern_assign       AstPatternAssign;

typedef struct ast_node                 AstNode;
typedef struct ast_code_node            AstCodeNode;
typedef struct ast_outline_node         AstOutlineNode;
typedef struct ast_pattern_node         AstPatternNode;

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
  AST_OUTLINE_SYMBOL,
  AST_OUTLINE_STRING,
  AST_OUTLINE_NUMBER,
  AST_SYMBOL,

  AST_MATCH,
  AST_MATCH_LINE,
  AST_PATTERN,
  AST_PATTERN_WILD,
  AST_PATTERN_SYMBOL,
  AST_PATTERN_ASSIGN,
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
 *  AstSymbol
 *
 *  AstRule
 *  AstMatch
 */
struct ast_code_node {
  void *p;
  AstType type;
};

/**
 * Points to one of:
 *  AstOutlineSymbol
 *  AstOutlineString
 *  AstOutlineNumber
 */
struct ast_outline_node {
  void *p;
  AstType type;
};

/**
 * Points to one of:
 *  AstPatternWild
 *  AstPatternSymbol
 *  AstPatternAssign
 */
struct ast_pattern_node {
  void *p;
  AstType type;
};

/* Type-checking functions */
int ast_is_code_node(AstNode node);
int ast_is_outline_node(AstNode node);
int ast_is_pattern_node(AstNode node);

/* Type-conversion functions */
AstCodeNode         ast_to_code_node(AstNode node);
AstOutlineNode      ast_to_outline_node(AstNode node);
AstPatternNode      ast_to_pattern_node(AstNode node);

AstFile            *ast_to_file(AstNode node);
AstCode            *ast_to_code(AstNode node);
AstOutlineList     *ast_to_outline_list(AstNode node);
AstOutlineItem     *ast_to_outline_item(AstNode node);
AstMatchLine       *ast_to_match_line(AstNode node);
AstPattern         *ast_to_pattern(AstNode node);

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
  AstOutlineNode *nodes;
  AstOutlineNode *nodes_end;
  AstOutlineList *children;
};

/**
 * An individual word in an outline item.
 */
struct ast_outline_symbol {
  String symbol;
};

struct ast_outline_string {
  String string;
};

struct ast_outline_number {
  String number;
};

/**
 * A symbol to be replaced within a block of code.
 */
struct ast_symbol {
  AstPatternAssign *symbol;
};

/* Match elements */
struct ast_match {
  AstMatchLine **lines;
  AstMatchLine **lines_end;
};

struct ast_match_line {
  AstPattern *pattern;
  AstCode *code;
};

/* Pattern elements */
struct ast_pattern {
  AstPatternNode *nodes;
  AstPatternNode *nodes_end;
};

struct ast_pattern_wild {
  int dummy;
};

struct ast_pattern_symbol {
  String symbol;
};

struct ast_pattern_assign {
  String symbol;
  AstPatternNode pattern;
};

AstFile            *ast_file_new                (Pool *p, AstCode *code);
AstCode            *ast_code_new                (Pool *p, AstCodeNode *nodes, AstCodeNode *nodes_end);
AstCodeText        *ast_code_text_new           (Pool *p, String code);
AstInclude         *ast_include_new             (Pool *p, AstFile *file);
AstOutline         *ast_outline_new             (Pool *p, String name, AstOutlineList *children);
AstOutlineList     *ast_outline_list_new        (Pool *p, AstOutlineItem **items, AstOutlineItem **items_end);
AstOutlineItem     *ast_outline_item_new        (Pool *p, AstOutlineNode *nodes, AstOutlineNode *nodes_end, AstOutlineList *children);
AstOutlineSymbol   *ast_outline_symbol_new      (Pool *p, String symbol);
AstOutlineString   *ast_outline_string_new      (Pool *p, String string);
AstOutlineNumber   *ast_outline_number_new      (Pool *p, String number);
AstSymbol          *ast_symbol_new              (Pool *p, AstPatternAssign *symbol);

AstMatch           *ast_match_new               (Pool *p, AstMatchLine **lines, AstMatchLine **lines_end);
AstMatchLine       *ast_match_line_new          (Pool *p, AstPattern *pattern, AstCode *code);
AstPattern         *ast_pattern_new             (Pool *p, AstPatternNode *nodes, AstPatternNode *nodes_end);
AstPatternWild     *ast_pattern_wild_new        (Pool *p);
AstPatternSymbol   *ast_pattern_symbol_new      (Pool *p, String symbol);
AstPatternAssign   *ast_pattern_assign_new      (Pool *p, String symbol, AstPatternNode pattern);

#endif
