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
#include "pool.h"
#include <assert.h>

int ast_is_pattern_item(AstItem item)
{
  return
    item.type == AST_PATTERN_WILD ||
    item.type == AST_PATTERN_ANY_SYMBOL ||
    item.type == AST_PATTERN_ANY_STRING ||
    item.type == AST_PATTERN_ANY_NUMBER ||
    item.type == AST_PATTERN_RULE ||
    item.type == AST_PATTERN_SYMBOL ||
    item.type == AST_PATTERN_STRING ||
    item.type == AST_PATTERN_NUMBER ||
    item.type == AST_PATTERN_ASSIGN;
}

int ast_is_code_item(AstItem item)
{
  return
    item.type == AST_C ||
    item.type == AST_MATCH ||
    item.type == AST_CODE_SYMBOL ||
    item.type == AST_CODE_UPPER ||
    item.type == AST_CODE_LOWER ||
    item.type == AST_CODE_CAMEL ||
    item.type == AST_CODE_MIXED ||
    item.type == AST_CODE_STRING;
}

int ast_is_code_symbol_item(AstItem item)
{
  return
    item.type == AST_CODE_SYMBOL ||
    item.type == AST_CODE_UPPER ||
    item.type == AST_CODE_LOWER ||
    item.type == AST_CODE_CAMEL ||
    item.type == AST_CODE_MIXED ||
    item.type == AST_CODE_STRING;
}

AstPatternItem ast_to_pattern_item(AstItem item)
{
  AstPatternItem temp;
  assert(ast_is_pattern_item(item));
  temp.p = item.p;
  temp.type = item.type;
  return temp;
}

AstCodeItem ast_to_code_item(AstItem item)
{
  AstCodeItem temp;
  assert(ast_is_code_item(item));
  temp.p = item.p;
  temp.type = item.type;
  return temp;
}

AstCodeSymbolItem ast_to_code_symbol_item(AstItem item)
{
  AstCodeSymbolItem temp;
  assert(ast_is_code_symbol_item(item));
  temp.p = item.p;
  temp.type = item.type;
  return temp;
}

AstMatchLine *ast_to_match_line(AstItem item)
{
  assert(item.type == AST_MATCH_LINE);
  return item.p;
}

AstPattern *ast_to_pattern(AstItem item)
{
  assert(item.type == AST_PATTERN);
  return item.p;
}

AstCode *ast_to_code(AstItem item)
{
  assert(item.type == AST_CODE);
  return item.p;
}

AstC *ast_c_new(Pool *p, String code)
{
  AstC *self;
  if (!code.p) return 0;

  self = pool_alloc(p, sizeof(AstC));
  if (!self) return 0;
  self->code = code;
  return self;
}

AstMatch *ast_match_new(Pool *p, AstMatchLine **lines, AstMatchLine **lines_end)
{
  AstMatch *self;
  if (!lines) return 0;

  self = pool_alloc(p, sizeof(AstMatch));
  if (!self) return 0;
  self->lines = lines;
  self->lines_end = lines_end;
  return self;
}

AstMatchLine *ast_match_line_new(Pool *p, AstPattern *pattern, AstCode *code)
{
  AstMatchLine *self;
  if (!pattern) return 0;
  if (!code) return 0;

  self = pool_alloc(p, sizeof(AstMatchLine));
  if (!self) return 0;
  self->pattern = pattern;
  self->code = code;
  return self;
}

AstPattern *ast_pattern_new(Pool *p, AstPatternItem *items, AstPatternItem *items_end)
{
  AstPattern *self;
  if (!items) return 0;

  self = pool_alloc(p, sizeof(AstPattern));
  if (!self) return 0;
  self->items = items;
  self->items_end = items_end;
  return self;
}

AstPatternWild *ast_pattern_wild_new(Pool *p)
{
  AstPatternWild *self = pool_alloc(p, sizeof(AstPatternWild));
  return self;
}

AstPatternAnySymbol *ast_pattern_any_symbol_new(Pool *p)
{
  AstPatternAnySymbol *self = pool_alloc(p, sizeof(AstPatternAnySymbol));
  return self;
}

AstPatternAnyString *ast_pattern_any_string_new(Pool *p)
{
  AstPatternAnyString *self = pool_alloc(p, sizeof(AstPatternAnyString));
  return self;
}

AstPatternAnyNumber *ast_pattern_any_number_new(Pool *p)
{
  AstPatternAnyNumber *self = pool_alloc(p, sizeof(AstPatternAnyNumber));
  return self;
}

AstPatternRule *ast_pattern_rule_new(Pool *p, AstRule *rule)
{
  AstPatternRule *self;
  if (!rule) return 0;

  self = pool_alloc(p, sizeof(AstPatternRule));
  if (!self) return 0;
  self->rule = rule;
  return self;
}

AstPatternSymbol *ast_pattern_symbol_new(Pool *p, String symbol)
{
  AstPatternSymbol *self;
  if (!symbol.p) return 0;

  self = pool_alloc(p, sizeof(AstPatternSymbol));
  if (!self) return 0;
  self->symbol = symbol;
  return self;
}

AstPatternString *ast_pattern_string_new(Pool *p, String string)
{
  AstPatternString *self;
  if (!string.p) return 0;

  self = pool_alloc(p, sizeof(AstPatternString));
  if (!self) return 0;
  self->string = string;
  return self;
}

AstPatternNumber *ast_pattern_number_new(Pool *p, String number)
{
  AstPatternNumber *self;
  if (!number.p) return 0;

  self = pool_alloc(p, sizeof(AstPatternNumber));
  if (!self) return 0;
  self->number = number;
  return self;
}

AstPatternAssign *ast_pattern_assign_new(Pool *p, String symbol, AstPatternItem pattern)
{
  AstPatternAssign *self;
  if (!symbol.p) return 0;
  if (!pattern.p) return 0;

  self = pool_alloc(p, sizeof(AstPatternAssign));
  if (!self) return 0;
  self->symbol = symbol;
  self->pattern = pattern;
  return self;
}

AstCode *ast_code_new(Pool *p, AstCodeItem *items, AstCodeItem *items_end)
{
  AstCode *self;
  if (!items) return 0;

  self = pool_alloc(p, sizeof(AstCode));
  if (!self) return 0;
  self->items = items;
  self->items_end = items_end;
  return self;
}

AstCodeSymbol *ast_code_symbol_new(Pool *p, AstPatternAssign *symbol)
{
  AstCodeSymbol *self;
  if (!symbol) return 0;

  self = pool_alloc(p, sizeof(AstCodeSymbol));
  if (!self) return 0;
  self->symbol = symbol;
  return self;
}

AstCodeUpper *ast_code_upper_new(Pool *p, AstCodeSymbolItem symbol)
{
  AstCodeUpper *self;
  if (!symbol.p) return 0;

  self = pool_alloc(p, sizeof(AstCodeUpper));
  if (!self) return 0;
  self->symbol = symbol;
  return self;
}

AstCodeLower *ast_code_lower_new(Pool *p, AstCodeSymbolItem symbol)
{
  AstCodeLower *self;
  if (!symbol.p) return 0;

  self = pool_alloc(p, sizeof(AstCodeLower));
  if (!self) return 0;
  self->symbol = symbol;
  return self;
}

AstCodeCamel *ast_code_camel_new(Pool *p, AstCodeSymbolItem symbol)
{
  AstCodeCamel *self;
  if (!symbol.p) return 0;

  self = pool_alloc(p, sizeof(AstCodeCamel));
  if (!self) return 0;
  self->symbol = symbol;
  return self;
}

AstCodeMixed *ast_code_mixed_new(Pool *p, AstCodeSymbolItem symbol)
{
  AstCodeMixed *self;
  if (!symbol.p) return 0;

  self = pool_alloc(p, sizeof(AstCodeMixed));
  if (!self) return 0;
  self->symbol = symbol;
  return self;
}

AstCodeString *ast_code_string_new(Pool *p, AstCodeSymbolItem symbol)
{
  AstCodeString *self;
  if (!symbol.p) return 0;

  self = pool_alloc(p, sizeof(AstCodeString));
  if (!self) return 0;
  self->symbol = symbol;
  return self;
}
