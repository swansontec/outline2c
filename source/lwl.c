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

typedef int (*KeywordFn)(Pool *pool, Source *in, Scope *scope, OutRoutine or);

/**
 * Represents an LWL keyword implemented in C
 */
typedef struct {
  KeywordFn code;
} Keyword;

Keyword *keyword_new(Pool *p, KeywordFn code)
{
  Keyword *self = pool_new(p, Keyword);
  CHECK_MEM(self);
  self->code = code;

  if (!self->code) return 0;
  return self;
}

int lwl_parse_statement(Pool *pool, Source *in, Scope *scope, OutRoutine or, int allow_assign);

/**
 * Parses an LWL value. Assignment statements are not permitted.
 */
int lwl_parse_value(Pool *pool, Source *in, Scope *scope, OutRoutine or)
{
  return lwl_parse_statement(pool, in, scope, or, 0);
}

/**
 * Parses a line of LWL code, which could be either an assignment or a value.
 */
int lwl_parse_line(Pool *pool, Source *in, Scope *scope, OutRoutine or)
{
  return lwl_parse_statement(pool, in, scope, or, 1);
}

int lwl_parse_statement(Pool *pool, Source *in, Scope *scope, OutRoutine or, int allow_assign)
{
  char const *start;
  Token token;
  Dynamic out;
  String name;

  /* Symbol: */
  token = lex_next(&start, &in->cursor, in->data.end);
  if (token != LEX_IDENTIFIER)
    return source_error(in, "Expecting a keyword or variable name here.");
  name = string_init(start, in->cursor);

  /* Equals sign? */
  if (allow_assign) {
    token = lex_next(&start, &in->cursor, in->data.end);
    if (token == LEX_EQUALS) {
      CHECK(lwl_parse_value(pool, in, scope, out_dynamic(&out)));
      if (out.type == TYPE_END)
        return source_error(in, "Wrong type - this must be a value.");
      CHECK(scope_add(scope, pool, name, out.type, out.p));
      return 1;
    }
    in->cursor = start;
  }

  if (!scope_get(scope, &out, name))
    return source_error(in, "Unknown variable or keyword.");
  if (out.type == TYPE_KEYWORD)
    CHECK(((Keyword*)out.p)->code(pool, in, scope, or));
  else
    CHECK(or.code(or.data, out.type, out.p));
  return 1;
}
