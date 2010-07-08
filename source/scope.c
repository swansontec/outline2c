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

#include "scope.h"

Scope *scope_new(Pool *p, Scope *outer)
{
  Scope *self;
  self = pool_alloc(p, sizeof(Scope));
  if (!self) return 0;
  self->outer = outer;
  self->first = 0;
  return self;
}

Symbol *scope_add(Scope *s, Pool *p, String symbol)
{
  Symbol *sym;
  if (!string_size(symbol)) return 0;

  sym = pool_alloc(p, sizeof(Symbol));
  sym->symbol = symbol;
  sym->type = AST_END;
  sym->value = 0;
  sym->next = s->first;
  s->first = sym;
  return sym;
}

Symbol *scope_find(Scope *s, String symbol)
{
  while (s) {
    Symbol *sym;
    for (sym = s->first; sym; sym = sym->next)
      if (string_equal(sym->symbol, symbol))
        return sym;
    s = s->outer;
  }
  return 0;
}
