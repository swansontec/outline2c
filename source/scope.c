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

/**
 * A symbol definition.
 * TODO: These should be freed when the context pops to an outer scope, which
 * means that they need a different memory management strategy than the AST
 * stuff.
 */
typedef struct Symbol Symbol;
struct Symbol {
  String name;
  Dynamic value;
  Symbol *next;
};

/**
 * One level in the symbol table. This implementation uses a linked list for
 * now, which is simple but not too efficient.
 */
typedef struct Scope Scope;
struct Scope {
  Scope *outer;
  Symbol *first;
};

Scope scope_init(Scope *outer)
{
  Scope self;
  self.outer = outer;
  self.first = 0;
  return self;
}

/**
 * Adds a symbol to the current scope.
 */
void scope_add(Scope *scope, Pool *pool, String name, Dynamic value)
{
  Symbol *sym = pool_new(pool, Symbol);
  sym->name = string_copy(pool, name);
  sym->value = value;
  sym->next = scope->first;
  scope->first = sym;
}

/**
 * Searches for a symbol to the current scope. Places the symbol's value, if
 * found, into *out.
 * @return 0 if the symbol does not exist.
 */
int scope_get(Scope *s, Dynamic *out, String name)
{
  while (s) {
    Symbol *sym;
    for (sym = s->first; sym; sym = sym->next)
      if (string_equal(sym->name, name)) {
        *out = sym->value;
        return 1;
      }
    s = s->outer;
  }
  return 0;
}
