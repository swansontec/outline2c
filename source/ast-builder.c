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

#include "ast-builder.h"
#include <assert.h>

/**
 * Initializes the AST-building stack.
 * @return 0 for failure
 */
int ast_builder_init(AstBuilder *b)
{
  if (!pool_init(&b->pool, 0x10000)) /* 64K block size */
    return 0;
  b->scope = scope_new(&b->pool, 0);
  if (!b->scope) return 0;
  return 1;
}

void ast_builder_free(AstBuilder *b)
{
  pool_free(&b->pool);
}

/**
 * Constructs a new scope. The current scope becomes the outer scope.
 * @return the new scope, or 0 for failure.
 */
Scope *ast_builder_scope_new(AstBuilder *b)
{
  Scope *s = scope_new(&b->pool, b->scope);
  if (!s) return 0;
  b->scope = s;
  return s;
}

/**
 * Adds a symbol to the current scope.
 * @return the new symbol, or 0 for failure.
 */
Symbol *ast_builder_scope_add(AstBuilder *b, String symbol)
{
  return scope_add(b->scope, &b->pool, pool_string_copy(&b->pool,symbol));
}

void ast_builder_scope_pop(AstBuilder *b)
{
  assert(b->scope->outer);
  b->scope = b->scope->outer;
}

Symbol *ast_builder_scope_find(AstBuilder *b, String symbol)
{
  return scope_find(b->scope, symbol);
}
