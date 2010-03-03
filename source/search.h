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
 * Implements the search algorithm for comparing patterns against outlines.
 */
#ifndef SEARCH_H_INCLUDED
#define SEARCH_H_INCLUDED

#include "typedefs.h"
#include "ast.h"

typedef struct outline_list OutlineList;

/**
 * A list of all outlines in-scope. The search algorithm works against this
 * list while generating code. The list can either be extracted from the top-
 * level file, or it can be taken from the children of a matching outline. In
 * the second case, it might make sense to create a parent pointer within this
 * structure, providing the ability to navigate up a level.
 */
struct outline_list {
  AstOutline **p;
  AstOutline **end;
};
void outline_list_free(OutlineList *self);
int outline_list_from_file(OutlineList *self, AstItem *items, AstItem *items_end);
OutlineList outline_list_from_outline(AstOutline *outline);

int ast_match_search(AstMatch *match, OutlineList outlines, FileW *file);
int ast_code_generate(AstCode *code, OutlineList outlines, FileW *file);

int match_pattern(AstPattern *pattern, AstOutline *outline);
int match_pattern_item(AstPatternItem pi, AstOutlineItem oi);
int match_pattern_wild(AstPatternWild *p, AstOutlineItem oi);
int match_pattern_symbol(AstPatternSymbol *p, AstOutlineItem oi);
int match_pattern_assign(AstPatternAssign *p, AstOutlineItem oi);

#endif
