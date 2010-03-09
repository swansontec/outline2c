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
AstNode ast_builder_pop(AstBuilder *b);
AstNode ast_builder_peek(AstBuilder *b);

AstPatternAssign *ast_builder_find_assign(AstBuilder *b, String symbol);
AstPatternAssign *ast_pattern_find_assign(AstPattern *pattern, String symbol);

/*
 * Functions for assembling an AST. All functions return 0 on success.
 */
int ast_build_file(AstBuilder *b);
int ast_build_code(AstBuilder *b, size_t node_n);
int ast_build_code_text(AstBuilder *b, String code);
int ast_build_include(AstBuilder *b);
int ast_build_outline(AstBuilder *b, String name);
int ast_build_outline_list(AstBuilder *b, size_t item_n);
int ast_build_outline_item(AstBuilder *b, size_t node_n);
int ast_build_outline_symbol(AstBuilder *b, String symbol);
int ast_build_outline_string(AstBuilder *b, String string);
int ast_build_outline_number(AstBuilder *b, String number);
int ast_build_symbol(AstBuilder *b, AstPatternAssign *symbol);

int ast_build_match(AstBuilder *b, size_t line_n);
int ast_build_match_line(AstBuilder *b);
int ast_build_pattern(AstBuilder *b, size_t node_n);
int ast_build_pattern_wild(AstBuilder *b);
int ast_build_pattern_symbol(AstBuilder *b, String symbol);
int ast_build_pattern_assign(AstBuilder *b, String symbol);

#endif
