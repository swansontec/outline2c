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

#include "lwl.h"
#include "lexer.h"
#include <stdio.h>

Keyword *keyword_new(Pool *p, int (*code)(Context *ctx, OutRoutine or))
{
  Keyword *self = pool_alloc(p, sizeof(Keyword));
  CHECK_MEM(self);
  self->code = code;

  if (!self->code) return 0;
  return self;
}

static int parse_statement(Context *ctx, OutRoutine or, int allow_assign)
{
  char const *start;
  Token token;
  Dynamic out;
  String name;

  /* Symbol: */
  token = lex_next(&start, &ctx->cursor, ctx->file.end);
  if (token != LEX_IDENTIFIER)
    return context_error(ctx, "Expecting a keyword or variable name here.");
  name = string_init(start, ctx->cursor);

  /* Equals sign? */
  if (allow_assign) {
    token = lex_next(&start, &ctx->cursor, ctx->file.end);
    if (token == LEX_EQUALS) {
      CHECK(lwl_parse_value(ctx, dynamic_out(&out)));
      if (out.type == TYPE_END)
        return context_error(ctx, "Wrong type - this must be a value.");
      CHECK(context_scope_add(ctx, name, out.type, out.p));
      return 1;
    }
    ctx->cursor = start;
  }

  if (!context_scope_get(ctx, &out, name))
    return context_error(ctx, "Unknown variable or keyword.");
  if (out.type == TYPE_KEYWORD)
    CHECK(((Keyword*)out.p)->code(ctx, or));
  else
    CHECK(or.code(or.data, out.type, out.p));
  return 1;
}

/**
 * Parses an LWL value. Assignment statements are not permitted.
 */
int lwl_parse_value(Context *ctx, OutRoutine or)
{
  return parse_statement(ctx, or, 0);
}

/**
 * Parses a line of LWL code, which could be either an assignment or a value.
 */
int lwl_parse_line(Context *ctx, OutRoutine or)
{
  return parse_statement(ctx, or, 1);
}
