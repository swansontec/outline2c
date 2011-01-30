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
 * Holds line and column information
 */
typedef struct {
  unsigned line;
  unsigned column;
} Location;

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
 * A stream of input text feeding the parser
 */
typedef struct {
  String filename;
  String data;
  char const *cursor;
} Source;

/**
 * Prints an error message.
 */
int source_error(Source *in, char const *message)
{
  char *name = string_to_c(in->filename);
  Location l = location_init(in->data, in->cursor);
  fprintf(stderr, "%s:%d:%d: error: %s\n", name, l.line + 1, l.column + 1, message);
  free(name);
  return 0;
}

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
 * One level in the symbol table. This implmentation uses a linked list for
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
 * @return 0 for failure.
 */
int scope_add(Scope *scope, Pool *pool, String name, Type type, void *p)
{
  Symbol *sym = pool_alloc(pool, sizeof(Symbol));
  CHECK_MEM(sym);
  sym->name = string_copy(pool, name);
  CHECK_MEM(string_size(sym->name));
  sym->value.p = p;
  sym->value.type = type;
  sym->next = scope->first;
  scope->first = sym;
  return 1;
}

/**
 * Searches for a symbol to the current scope. Places the symbol's value, if
 * found, into ctx->out.
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

/**
 * Accepts an output value. Parser functions only use their return value to
 * indicate success or failure. To output data, such as AST nodes, they call
 * the current output routine. Calling the routine multiple times allows a
 * parser function to produce multiple output items, which is not possible
 * with ordinary return values.
 */
typedef struct {
  int (*code)(void *data, Type type, void *p);
  void *data;
} OutRoutine;

static int dynamic_out_fn(void *data, Type type, void *p)
{
  Dynamic *out = data;
  if (out->type != TYPE_END) return 0;
  out->p = p;
  out->type = type;
  return 1;
}

/**
 * Produces an output routine which captures a single return value in a
 * Dynamic variable. Attempting to return more than one value will produce
 * an error in the output routine.
 */
OutRoutine dynamic_out(Dynamic *out)
{
  OutRoutine self;
  self.code = dynamic_out_fn;
  self.data = out;
  out->type = TYPE_END;
  return self;
}
