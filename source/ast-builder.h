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

#ifndef AST_BUILDER_H_INCLUDED
#define AST_BUILDER_H_INCLUDED

#include "ast.h"

typedef struct ast_builder AstBuilder;

/**
 * State for assembling an AST item-by-item.
 */
struct ast_builder {
  Pool pool;
  Scope *scope;
};

int ast_builder_init(AstBuilder *b);
void ast_builder_free(AstBuilder *b);

Scope  *ast_builder_scope_new(AstBuilder *b);
Symbol *ast_builder_scope_add(AstBuilder *b, String symbol);
void    ast_builder_scope_pop(AstBuilder *b);
Symbol *ast_builder_scope_find(AstBuilder *b, String symbol);

#endif
