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

#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#include "typedefs.h"
#include "ast.h"

void ast_outline_dump(AstOutline *outline, int indent);
void ast_outline_item_dump(AstOutlineItem item);
void ast_outline_symbol_dump(AstOutlineSymbol *p);
void ast_outline_string_dump(AstOutlineString *p);
void ast_outline_number_dump(AstOutlineNumber *p);
void ast_match_dump(AstMatch *match, int indent);
void ast_match_line_dump(AstMatchLine *p, int indent);
void ast_pattern_dump(AstPattern *p);
void ast_pattern_item_dump(AstPatternItem item);
void ast_pattern_wild_dump(AstPatternWild *p);
void ast_pattern_symbol_dump(AstPatternSymbol *p);
void ast_pattern_assign_dump(AstPatternAssign *p);
void ast_code_dump(AstCode *p, int indent);

#endif
