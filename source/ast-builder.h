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
#include "pool.h"
#include <stdlib.h>

/**
 * State for assembling an AST item-by-item.
 */
struct ast_builder {
  Pool pool;
  AstNode *stack;
  size_t stack_size;
  size_t stack_top;
};

int ast_builder_init(AstBuilder *b);
void ast_builder_free(AstBuilder *b);

int ast_builder_push(AstBuilder *b, AstType type, void *p);
int ast_builder_push_start(AstBuilder *b);
AstNode ast_builder_pop(AstBuilder *b);
AstNode ast_builder_peek(AstBuilder *b);

int ast_builder_find_symbol(AstBuilder *b, String symbol);

/*
 * Functions for assembling an AST. All functions return 0 on success.
 */
int ast_build_file(AstBuilder *b);
int ast_build_code(AstBuilder *b);
int ast_build_code_text(AstBuilder *b, String code);
int ast_build_include(AstBuilder *b);
int ast_build_outline(AstBuilder *b, String name);
int ast_build_outline_list(AstBuilder *b);
int ast_build_outline_item(AstBuilder *b, String name);
int ast_build_outline_tag(AstBuilder *b, String symbol);
int ast_build_map(AstBuilder *b, String name);
int ast_build_map_line(AstBuilder *b);
int ast_build_for(AstBuilder *b);
int ast_build_in(AstBuilder *b, String symbol, String name, int reverse, int list);
int ast_build_filter(AstBuilder *b);
int ast_build_filter_tag(AstBuilder *b, String tag);
int ast_build_filter_any(AstBuilder *b);
int ast_build_filter_not(AstBuilder *b);
int ast_build_filter_and(AstBuilder *b);
int ast_build_filter_or(AstBuilder *b);
int ast_build_symbol(AstBuilder *b, int level);
int ast_build_lookup(AstBuilder *b, String name);

#endif
