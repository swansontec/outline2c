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
typedef struct ast_c                    AstC;
typedef struct ast_include              AstInclude;
typedef struct ast_outline              AstOutline;
typedef struct ast_rule                 AstRule;
typedef struct ast_match                AstMatch;
typedef struct ast_rule_line            AstRuleLine;
typedef struct ast_match_line           AstMatchLine;
typedef struct ast_outline_symbol       AstOutlineSymbol;
typedef struct ast_outline_string       AstOutlineString;
typedef struct ast_outline_number       AstOutlineNumber;
typedef struct ast_pattern              AstPattern;
typedef struct ast_pattern_wild         AstPatternWild;
typedef struct ast_pattern_any_symbol   AstPatternAnySymbol;
typedef struct ast_pattern_any_string   AstPatternAnyString;
typedef struct ast_pattern_any_number   AstPatternAnyNumber;
typedef struct ast_pattern_rule         AstPatternRule;
typedef struct ast_pattern_symbol       AstPatternSymbol;
typedef struct ast_pattern_string       AstPatternString;
typedef struct ast_pattern_number       AstPatternNumber;
typedef struct ast_pattern_assign       AstPatternAssign;
typedef struct ast_code                 AstCode;
typedef struct ast_code_c               AstCodeC;
typedef struct ast_code_symbol          AstCodeSymbol;
typedef struct ast_code_upper           AstCodeUpper;
typedef struct ast_code_lower           AstCodeLower;
typedef struct ast_code_camel           AstCodeCamel;
typedef struct ast_code_mixed           AstCodeMixed;
typedef struct ast_code_string          AstCodeString;

typedef struct ast_item                 AstItem;
typedef struct ast_file_item            AstFileItem;
typedef struct ast_outline_item         AstOutlineItem;
typedef struct ast_pattern_item         AstPatternItem;
typedef struct ast_code_item            AstCodeItem;
typedef struct ast_code_symbol_item     AstCodeSymbolItem;

/**
 * Types
 */
enum ast_type {
  AST_FILE,
  AST_C,
  AST_INCLUDE,
  AST_OUTLINE,
  AST_RULE,
  AST_MATCH,
  AST_RULE_LINE,
  AST_MATCH_LINE,
  AST_OUTLINE_SYMBOL,
  AST_OUTLINE_STRING,
  AST_OUTLINE_NUMBER,
  AST_PATTERN,
  AST_PATTERN_WILD,
  AST_PATTERN_ANY_SYMBOL,
  AST_PATTERN_ANY_STRING,
  AST_PATTERN_ANY_NUMBER,
  AST_PATTERN_RULE,
  AST_PATTERN_SYMBOL,
  AST_PATTERN_STRING,
  AST_PATTERN_NUMBER,
  AST_PATTERN_ASSIGN,
  AST_CODE,
  AST_CODE_C,
  AST_CODE_SYMBOL,
  AST_CODE_UPPER,
  AST_CODE_LOWER,
  AST_CODE_CAMEL,
  AST_CODE_MIXED,
  AST_CODE_STRING
};
typedef enum ast_type AstType;

/**
 * Points to any AST node.
 */
struct ast_item {
  void *p;
  AstType type;
};

/**
 * Points to one of:
 *  AstC
 *  AstInclude
 *  AstOutline
 *  AstRule
 *  AstMatch
 */
struct ast_file_item {
  void *p;
  AstType type;
};

/**
 * Points to one of:
 *  AstOutlineSymbol
 *  AstOutlineString
 *  AstOutlineNumber
 */
struct ast_outline_item {
  void *p;
  AstType type;
};

/**
 * Points to one of:
 *  AstPatternWild
 *  AstPatternAnySymbol
 *  AstPatternAnyString
 *  AstPatternAnyNumber
 *  AstPatternRule
 *  AstPatternSymbol
 *  AstPatternString
 *  AstPatternNumber
 *  AstPatternAssign
 */
struct ast_pattern_item {
  void *p;
  AstType type;
};

/**
 * Points to one of:
 *  AstCodeC
 *  AstCodeSymbol
 *  AstCodeUpper
 *  AstCodeLower
 *  AstCodeCamel
 *  AstCodeMixed
 *  AstCodeString
 */
struct ast_code_item {
  void *p;
  AstType type;
};

/**
 * Points to one of:
 *  AstCodeSymbol
 *  AstCodeUpper
 *  AstCodeLower
 *  AstCodeCamel
 *  AstCodeMixed
 */
struct ast_code_symbol_item {
  void *p;
  AstType type;
};

/*
 * Type-checking functions
 */
int ast_is_pattern_item(AstItem item);

/*
 * Type-conversion functions
 */
AstPatternItem ast_to_pattern_item(AstItem item);

/* Top-level elements */
struct ast_file {
  AstFileItem *items;
  AstFileItem *item_end;
};

struct ast_c {
  String code;
};

struct ast_include {
  AstFile *file;
};

struct ast_outline {
  AstOutlineItem *items;
  AstOutlineItem *items_end;
  AstOutline *children;
  AstOutline *children_end;
};

struct ast_rule {
  AstRuleLine *lines;
  AstRuleLine *lines_end;
};

struct ast_match {
  AstMatchLine **lines;
  int line_n;
};

/* Elements within keywords */
struct ast_rule_line {
  AstPattern *pattern;
  AstCode *code;
};

struct ast_match_line {
  AstPattern *pattern;
  AstCode *code;
};

/* Outline elements */
struct ast_outline_symbol {
  String symbol;
};

struct ast_outline_string {
  String string;
};

struct ast_outline_number {
  String number;
};

/* Pattern-matching elements  */
struct ast_pattern {
  AstPatternItem *items;
  AstPatternItem *items_end;
};

struct ast_pattern_wild {
  int dummy;
};

struct ast_pattern_any_symbol {
  int dummy;
};

struct ast_pattern_any_string {
  int dummy;
};

struct ast_pattern_any_number {
  int dummy;
};

struct ast_pattern_rule {
  AstRule *rule;
};

struct ast_pattern_symbol {
  String symbol;
};

struct ast_pattern_string {
  String string;
};

struct ast_pattern_number {
  String number;
};

struct ast_pattern_assign {
  String symbol;
  AstPatternItem pattern;
};

/* Code-generation elements */
struct ast_code {
  AstCodeItem *items;
  AstCodeItem *items_end;
};

struct ast_code_c { /* Untransformed code */
  String code;
};

struct ast_code_symbol { /* Replacement symbol from pattern */
  AstPatternAssign *symbol;
};

struct ast_code_upper { /* UPPER_CASE */
  AstCodeSymbolItem symbol;
};

struct ast_code_lower { /* lower_case */
  AstCodeSymbolItem symbol;
};

struct ast_code_camel { /* CamelCase */
  AstCodeSymbolItem symbol;
};

struct ast_code_mixed { /* mixedCase */
  AstCodeSymbolItem symbol;
};

struct ast_code_string { /* Stringification */
  AstCodeSymbolItem symbol;
};

/*int ast_file_new(Pool *p);
int ast_c_new(Pool *p, String code);
int ast_include_new(Pool *p);
int ast_outline_new(Pool *p);
int ast_rule_new(Pool *p);
int ast_match_new(Pool *p);
int ast_rule_line_new(Pool *p);
int ast_match_line_new(Pool *p);
int ast_outline_symbol_new(Pool *p, String symbol);
int ast_outline_string_new(Pool *p, String string);
int ast_outline_number_new(Pool *p, String number);*/
AstPattern         *ast_pattern_new             (Pool *p, AstPatternItem *items, AstPatternItem *items_end);
AstPatternWild     *ast_pattern_wild_new        (Pool *p);
AstPatternAnySymbol*ast_pattern_any_symbol_new  (Pool *p);
AstPatternAnyString*ast_pattern_any_string_new  (Pool *p);
AstPatternAnyNumber*ast_pattern_any_number_new  (Pool *p);
AstPatternRule     *ast_pattern_rule_new        (Pool *p, AstRule *rule);
AstPatternSymbol   *ast_pattern_symbol_new      (Pool *p, String symbol);
AstPatternString   *ast_pattern_string_new      (Pool *p, String string);
AstPatternNumber   *ast_pattern_number_new      (Pool *p, String number);
AstPatternAssign   *ast_pattern_assign_new      (Pool *p, String symbol, AstPatternItem pattern);
/*AstCode            *ast_code_new                (Pool *p, AstCodeItem *items, AstCodeItem *items_end);
AstCodeC           *ast_code_c_new              (Pool *p, String code);
AstCodeSymbol      *ast_code_symbol_new         (Pool *p, String symbol);
AstCodeUpper       *ast_code_upper_new          (Pool *p, AstCodeSymbolItem symbol);
AstCodeLower       *ast_code_lower_new          (Pool *p, AstCodeSymbolItem symbol);
AstCodeCamel       *ast_code_camel_new          (Pool *p, AstCodeSymbolItem symbol);
AstCodeMixed       *ast_code_mixed_new          (Pool *p, AstCodeSymbolItem symbol);
AstCodeString      *ast_code_string_new         (Pool *p, AstCodeSymbolItem symbol);*/

#endif
