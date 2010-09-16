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

#include "context.h"
#include <stdio.h>
#include <assert.h>

typedef struct Location Location;

/**
 * Constructs a new scope. The current scope becomes the outer scope.
 * @return the new scope, or 0 for failure.
 */
Scope *context_scope_push(Context *ctx)
{
  Scope *s = scope_new(ctx->pool, ctx->scope);
  if (!s) return 0;
  ctx->scope = s;
  return s;
}

void context_scope_pop(Context *ctx)
{
  assert(ctx->scope->outer);
  ctx->scope = ctx->scope->outer;
}

/**
 * Adds a symbol to the current scope.
 * @return the new symbol, or 0 for failure.
 */
Symbol *context_scope_add(Context *ctx, String symbol)
{
  return scope_add(ctx->scope, ctx->pool, pool_string_copy(ctx->pool, symbol));
}

Symbol *context_scope_get(Context *ctx, String symbol)
{
  return scope_find(ctx->scope, symbol);
}

/**
 * Holds line and column information
 */
struct Location {
  unsigned line;
  unsigned column;
};

/**
 * Obtains line and column numbers given a pointer into a file. This is not
 * fast, but printing errors is hardly a bottleneck. This routine counts from
 * 0, but most text editors start from 1. It might make sense to add 1 to the
 * returned values.
 */
Location location_init(String file, char const *position)
{
  Location self;
  char const *p;

  self.line = 0;
  self.column = 0;
  for (p = file.p; p < position; ++p) {
    if (*p == '\n') {
      ++self.line;
      self.column = 0;
    } else if (*p == '\t') {
      self.column += 8;
      self.column -= self.column % 8;
    } else {
      ++self.column;
    }
  }
  return self;
}

/**
 * Prints an error message.
 */
int context_error(Context *ctx, char const *message)
{
  char *name = string_to_c(ctx->filename);
  Location l = location_init(ctx->file, ctx->cursor);
  fprintf(stderr, "%s:%d:%d: error: %s\n", name, l.line + 1, l.column + 1, message);
  free(name);
  return 0;
}
