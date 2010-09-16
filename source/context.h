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

#ifndef CONTEXT_H_INCLUDED
#define CONTEXT_H_INCLUDED

#include "scope.h"

typedef struct Context Context;

/**
 * Different parts of the compiler communicate in several ways:
 *
 * - Allocating data from a common memory pool
 * - Storing variables in a common namespace
 * - Reading from a common input stream
 * - Sending errors to a common output
 *
 * The Context object provides these common facilities.
 */
struct Context {
  /* Memory allocation: */
  Pool *pool;

  /* Current scope: */
  Scope *scope;

  /* Input stream: */
  String file;
  String filename;
  char const *cursor;

  /* Return value: */
  Dynamic out;
};

Scope  *context_scope_push(Context *ctx);
void    context_scope_pop(Context *ctx);
Symbol *context_scope_add(Context *ctx, String symbol);
Symbol *context_scope_get(Context *ctx, String symbol);

int context_error(Context *ctx, char const *message);

#endif
