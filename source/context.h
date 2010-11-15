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

#include "type.h"
#include "pool.h"

typedef struct Source Source;
typedef struct Scope Scope;
typedef struct Symbol Symbol;
typedef struct OutRoutine OutRoutine;

/**
 * A stream of input text feeding the parser
 */
struct Source {
  String filename;
  String data;
  char const *cursor;
};

int source_error(Source *in, char const *message);

/**
 * One level in the symbol table. This implmentation uses a linked list for
 * now, which is simple but not too efficient.
 */
struct Scope {
  Scope *outer;
  Symbol *first;
};

Scope scope_init(Scope *outer);
int scope_add(Scope *scope, Pool *pool, String name, Type type, void *p);
int scope_get(Scope *scope, Dynamic *out, String name);

/**
 * Accepts an output value. Parser functions only use their return value to
 * indicate success or failure. To output data, such as AST nodes, they call
 * the current output routine. Calling the routine multiple times allows a
 * parser function to produce multiple output items, which is not possible
 * with ordinary return values.
 */
struct OutRoutine {
  int (*code)(void *data, Type type, void *p);
  void *data;
};

OutRoutine dynamic_out(Dynamic *out);

#endif
