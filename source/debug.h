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
/*
 * Routines to display the contents of the AST for debugging purposes. The
 * indent parameter sets the indentation level for the printout.
 */

#ifndef DEBUG_H_INCLUDED
#define DEBUG_H_INCLUDED

#include "typedefs.h"
#include "ast.h"

void dump_code(AstCode *p, int indent);

void dump_outline(AstOutline *p);
void dump_outline_list(AstOutlineList *p, int indent);
void dump_outline_item(AstOutlineItem *p, int indent);
void dump_outline_node(AstOutlineNode node);
void dump_outline_symbol(AstOutlineSymbol *p);
void dump_outline_string(AstOutlineString *p);
void dump_outline_number(AstOutlineNumber *p);

void dump_filter(AstFilter *p);
void dump_filter_node(AstFilterNode node);
void dump_filter_tag(AstFilterTag *p);
void dump_filter_not(AstFilterNot *p);
void dump_filter_and(AstFilterAnd *p);
void dump_filter_or(AstFilterOr *p);

void dump_match(AstMatch *match, int indent);
void dump_match_line(AstMatchLine *p, int indent);

void dump_pattern(AstPattern *p);
void dump_pattern_node(AstPatternNode node);
void dump_pattern_wild(AstPatternWild *p);
void dump_pattern_symbol(AstPatternSymbol *p);
void dump_pattern_assign(AstPatternAssign *p);

#endif
