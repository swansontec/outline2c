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

#ifndef SCOPE_H_INCLUDED
#define SCOPE_H_INCLUDED

#include "type.h"
#include "pool.h"

typedef struct symbol Symbol;
typedef struct scope Scope;

/**
 * A symbol definition.
 */
struct symbol {
  String symbol;
  Type type;
  void *value;

  /* The linked-list structure should not be embedded within this struct,
   * but this is the simplest way to do things for now. Eventually, the scope
   * structure should be allocated from a different pool than the symbol
   * structure, since it has a different lifetime. */
  Symbol *next;
};

struct scope {
  Scope *outer;
  Symbol *first;
};

Scope *scope_new(Pool *p, Scope *outer);
Symbol *scope_add(Scope *s, Pool *p, String symbol);
Symbol *scope_find(Scope *s, String symbol);

#endif
