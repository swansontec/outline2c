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

#ifndef SEARCH_H_INCLUDED
#define SEARCH_H_INCLUDED

#include "ast.h"

typedef struct scope Scope;

int test_filter(AstFilter *test, AstOutlineItem *item);

/**
 * Represents a collection of symbols that are in scope. For now, this struct
 * simply contains the information needed to search for a particular symbol.
 * At some point in the future, this structure will gain some sort of caching
 * ability to accelerate the search.
 */
struct scope {
  /* The current code block being processed: */
  AstCode *code;
  /* The next-outer scope, if any: */
  Scope *outer;
  /* The current outline item being processed, if any: */
  AstOutlineItem *item;
};

Scope scope_init(AstCode *code, Scope *outer, AstOutlineItem *item);
AstOutline *scope_find_outline(Scope *s, String name);
AstMap *scope_find_map(Scope *s, String name);

#endif
