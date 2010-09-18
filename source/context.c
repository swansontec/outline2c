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

typedef struct Symbol Symbol;
typedef struct Location Location;

/**
 * A symbol definition.
 * TODO: These should be freed when the context pops to an outer scope, which
 * means that they need a different memory management strategy than the AST
 * stuff.
 */
struct Symbol {
  String name;
  Dynamic value;
  Symbol *next;
};

struct Scope {
  Scope *outer;
  Symbol *first;
};

/**
 * Constructs a new scope. The current scope becomes the outer scope.
 * @return 0 for failure.
 */
int context_scope_push(Context *ctx)
{
  Scope *s = pool_alloc(ctx->pool, sizeof(Scope));
  if (!s) return 0;
  s->outer = ctx->scope;
  s->first = 0;
  ctx->scope = s;
  return 1;
}

void context_scope_pop(Context *ctx)
{
  assert(ctx->scope->outer);
  ctx->scope = ctx->scope->outer;
}

/**
 * Adds a symbol to the current scope.
 * @return 0 for failure.
 */
int context_scope_add(Context *ctx, String name, Type type, void *p)
{
  Symbol *sym = pool_alloc(ctx->pool, sizeof(Symbol));
  if (!sym) return 0;
  sym->name = pool_string_copy(ctx->pool, name);
  if (!string_size(sym->name)) return 0;
  sym->value.p = p;
  sym->value.type = type;
  sym->next = ctx->scope->first;
  ctx->scope->first = sym;
  return 1;
}

/**
 * Searches for a symbol to the current scope. Places the symbol's value, if
 * found, into ctx->out.
 * @return 0 if the symbol does not exist.
 */
int context_scope_get(Context *ctx, String name)
{
  Scope *s = ctx->scope;
  while (s) {
    Symbol *sym;
    for (sym = s->first; sym; sym = sym->next)
      if (string_equal(sym->name, name)) {
        ctx->out = sym->value;
        return 1;
      }
    s = s->outer;
  }
  return 0;
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
