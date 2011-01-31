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
/*
 * This is a recursive descent parser which scans through the input, building
 * up an AST representing the file contents.
 *
 * The Source structure holds the current read cursor. Parser functions expect
 * to be called with the read cursor pointing to the first input character to
 * consume. Parser functions return with the cursor pointing one-past the last
 * consumed character.
 */

int parse_macro_call(Pool *pool, Source *in, Scope *scope, OutRoutine or, AstMacro *macro);

/**
 * Parses a block of code in the host language, looking for escape sequences
 * and replacement symbols.
 *
 * When this function is called, the cursor should point to either the start
 * of the file or the opening brace. When the function returns, the cursor
 * will point to either the end of the file or the character after the closing
 * brace.
 *
 * @param scoped If this parameter is non-zero, the parser will stop at the
 * first unbalanced }. Otherwise, the parser will stop at the end of the file.
 */
int parse_code(Pool *pool, Source *in, Scope *scope, OutRoutine or, int scoped)
{
  char const *start_c, *start;
  Token token;
  Dynamic out;
  int indent = 1;

#define WRITE_CODE \
  if (start_c != start) \
    CHECK(or.code(or.data, dynamic(AST_CODE_TEXT, \
      ast_code_text_new(pool, string_init(start_c, start)))));

  start_c = in->cursor;
  start = in->cursor; token = lex(&in->cursor, in->data.end);

code:
  /* We are in a block of host-language code. Select a course of action: */
  if (token == LEX_END) goto done;
  if (token == LEX_PASTE) goto paste;
  if (token == LEX_ESCAPE) goto escape;
  if (token == LEX_IDENTIFIER) {
    if (scope_get(scope, &out, string_init(start, in->cursor))) {
      if (out.type == AST_MACRO) {
        goto macro;
      } else if (out.type == AST_VARIABLE) {
        goto variable;
      }
    }
  } else if (scoped && token == LEX_BRACE_R) {
    if (!--indent)
      goto done;
  } else if (scoped && token == LEX_BRACE_L) {
    ++indent;
  }
  start = in->cursor; token = lex(&in->cursor, in->data.end);
  goto code;

paste:
  WRITE_CODE

  /* Token pasting: */
  start_c = in->cursor;
  start = in->cursor; token = lex(&in->cursor, in->data.end);
  goto code;

escape:
  WRITE_CODE

  /* "\ol" escape sequences: */
  out.type = TYPE_END;
  CHECK(lwl_parse_line(pool, in, scope, or));

  start_c = in->cursor;
  start = in->cursor; token = lex(&in->cursor, in->data.end);
  goto code;

macro:
  WRITE_CODE

  /* Macro invocation: */
  CHECK(parse_macro_call(pool, in, scope, or, out.p));

  start_c = in->cursor;
  start = in->cursor; token = lex(&in->cursor, in->data.end);
  goto code;

variable:
  WRITE_CODE

  /* Is there a lookup modifier? */
  start_c = in->cursor;
  start = in->cursor; token = lex(&in->cursor, in->data.end);
  if (token == LEX_BANG) {
    start = in->cursor; token = lex(&in->cursor, in->data.end);
    if (token == LEX_IDENTIFIER) {
      CHECK(or.code(or.data, dynamic(AST_LOOKUP,
        ast_lookup_new(pool, out.p, string_init(start, in->cursor)))));
      start_c = in->cursor;
      start = in->cursor; token = lex(&in->cursor, in->data.end);
    } else {
      CHECK(or.code(or.data, out));
    }
  } else {
    CHECK(or.code(or.data, out));
  }
  goto code;

done:
  WRITE_CODE

  /* End-of-code: */
  if (scoped && token == LEX_END)
    return source_error(in, "Unexpected end of input in code block.");

  return 1;
}

/**
 * Parses a macro definition.
 */
int parse_macro(Pool *pool, Source *in, Scope *scope, OutRoutine or)
{
  char const *start;
  Token token;
  Scope inner = scope_init(scope);
  ListBuilder inputs = list_builder_init(pool);
  ListBuilder code = list_builder_init(pool);
  AstMacro *self = pool_new(pool, AstMacro);
  CHECK_MEM(self);

  /* Opening parenthesis: */
  token = lex_next(&start, &in->cursor, in->data.end);
  if (token != LEX_PAREN_L)
    return source_error(in, "A macro definition must begin with an argument list.");

input:
  /* Argument? */
  token = lex_next(&start, &in->cursor, in->data.end);
  if (token == LEX_IDENTIFIER) {
    AstVariable *v;
    CHECK(v = ast_variable_new(pool, string_init(start, in->cursor)));
    CHECK(list_builder_add(&inputs, dynamic(AST_VARIABLE, v)));
    CHECK(scope_add(&inner, pool, v->name, dynamic(AST_VARIABLE, v)));

    /* Comma or closing parenthesis: */
    token = lex_next(&start, &in->cursor, in->data.end);
    if (token == LEX_COMMA)
      goto input;
    else if (token != LEX_PAREN_R)
      return source_error(in, "Expecting a closing ) or another argument.");
  }
  self->inputs = inputs.first;

  /* Opening brace: */
  token = lex_next(&start, &in->cursor, in->data.end);
  if (token != LEX_BRACE_L)
    return source_error(in, "A macro definition must end with a code block.");

  /* Code: */
  CHECK(parse_code(pool, in, &inner, out_list_builder(&code), 1));
  self->code = code.first;

  CHECK(or.code(or.data, dynamic(AST_MACRO, self)));
  return 1;
}

/**
 * Parses a macro invocation.
 */
int parse_macro_call(Pool *pool, Source *in, Scope *scope, OutRoutine or, AstMacro *macro)
{
  char const *start;
  Token token;
  Dynamic out;
  ListBuilder inputs = list_builder_init(pool);
  AstMacroCall *self = pool_new(pool, AstMacroCall);
  CHECK_MEM(self);

  self->macro = macro;

  /* Opening parenthesis: */
  token = lex_next(&start, &in->cursor, in->data.end);
  if (token != LEX_PAREN_L)
    return source_error(in, "A macro invocation must have an argument list.");

input:
  /* Argument? */
  token = lex_next(&start, &in->cursor, in->data.end);
  if (token == LEX_IDENTIFIER) {
    in->cursor = start;
    CHECK(lwl_parse_value(pool, in, scope, out_dynamic(&out)));
    if (!ast_is_for_node(out.type))
      return source_error(in, "Wrong type - macro parameters must be outlines.\n");
    CHECK(list_builder_add(&inputs, out));

    /* Comma or closing parenthesis: */
    token = lex_next(&start, &in->cursor, in->data.end);
    if (token == LEX_COMMA)
      goto input;
    else if (token != LEX_PAREN_R)
      return source_error(in, "Expecting a closing ) or another argument.");
  }
  self->inputs = inputs.first;

  if (list_length(self->inputs) != list_length(macro->inputs))
    return source_error(in, "Wrong number of arguments.");

  CHECK(or.code(or.data, dynamic(AST_MACRO_CALL, self)));
  return 1;
}

/**
 * Parses a filter definition. Uses the standard shunting-yard algorithm with
 * the following order of precedence: () ! & |
 */
int parse_filter(Pool *pool, Source *in, Scope *scope, OutRoutine or)
{
  char const *start;
  Token token;
  FilterBuilder fb;
  enum operators { NOT, AND, OR, LPAREN } stack[32];
  int top = 0;

  CHECK(filter_builder_init(&fb));

want_term:
  token = lex_next(&start, &in->cursor, in->data.end);
  if (token == LEX_IDENTIFIER) {
    CHECK(filter_build_tag(&fb, pool, string_init(start, in->cursor)));
    goto want_operator;

  } else if (token == LEX_STAR) {
    CHECK(filter_build_any(&fb, pool));
    goto want_operator;

  } else if (token == LEX_BANG) {
    stack[top++] = NOT;
    goto want_term;

  } else if (token == LEX_PAREN_L) {
    stack[top++] = LPAREN;
    goto want_term;

  } else {
    return source_error(in, "There seems to be a missing term here.");
  }

want_operator:
  token = lex_next(&start, &in->cursor, in->data.end);
  if (token == LEX_AMP) {
    for (; top && stack[top-1] <= AND; --top) {
      if (stack[top-1] == NOT) {
        CHECK(filter_build_not(&fb, pool));
      } else if (stack[top-1] == AND) {
        CHECK(filter_build_and(&fb, pool));
      }
    }
    stack[top++] = AND;
    goto want_term;

  } else if (token == LEX_PIPE) {
    for (; top && stack[top-1] <= OR; --top) {
      if (stack[top-1] == NOT) {
        CHECK(filter_build_not(&fb, pool));
      } else if (stack[top-1] == AND) {
        CHECK(filter_build_and(&fb, pool));
      } else if (stack[top-1] == OR) {
        CHECK(filter_build_or(&fb, pool));
      }
    }
    stack[top++] = OR;
    goto want_term;

  } else if (token == LEX_PAREN_R) {
    for (; top && stack[top-1] < LPAREN; --top) {
      if (stack[top-1] == NOT) {
        CHECK(filter_build_not(&fb, pool));
      } else if (stack[top-1] == AND) {
        CHECK(filter_build_and(&fb, pool));
      } else if (stack[top-1] == OR) {
        CHECK(filter_build_or(&fb, pool));
      }
    }
    if (!top)
      return source_error(in, "No maching opening parenthesis.");
    --top;
    goto want_operator;

  } else if (token == LEX_BANG || token == LEX_PAREN_L) {
    return source_error(in, "There seems to be a missing operator here.");

  } else {
    in->cursor = start;
    goto done;
  }

done:
  for (; top; --top) {
    if (stack[top-1] == NOT) {
      CHECK(filter_build_not(&fb, pool));
    } else if (stack[top-1] == AND) {
      CHECK(filter_build_and(&fb, pool));
    } else if (stack[top-1] == OR) {
      CHECK(filter_build_or(&fb, pool));
    } else if (stack[top-1] == LPAREN) {
      return source_error(in, "No maching closing parenthesis.");
    }
  }

  CHECK(or.code(or.data, filter_builder_pop(&fb)));
  filter_builder_free(&fb);
  return 1;
}

int parse_outline(Pool *pool, Source *in, Scope *scope, OutRoutine or);

/**
 * Parses an individual item within an outline, including its tags and
 * children.
 */
int parse_outline_item(Pool *pool, Source *in, Scope *scope, OutRoutine or)
{
  char const *start;
  Token token;
  Dynamic out;
  String last = string_null();
  ListBuilder tags = list_builder_init(pool);
  AstOutlineItem *self = pool_new(pool, AstOutlineItem);
  CHECK_MEM(self);

  /* Handle the words making up the item: */
  token = lex_next(&start, &in->cursor, in->data.end);
  while (token == LEX_IDENTIFIER) {
    if (string_size(last)) {
      CHECK(list_builder_add(&tags, dynamic(AST_OUTLINE_TAG,
        ast_outline_tag_new(pool, last, 0))));
    }
    last = string_init(start, in->cursor);
    token = lex_next(&start, &in->cursor, in->data.end);
    if (token == LEX_EQUALS) {
      ListBuilder code = list_builder_init(pool);

      /* Opening brace: */
      token = lex_next(&start, &in->cursor, in->data.end);
      if (token != LEX_BRACE_L)
        return source_error(in, "A tag's value must be a code block.");

      /* Value: */
      CHECK(parse_code(pool, in, scope, out_list_builder(&code), 1));

      CHECK(list_builder_add(&tags, dynamic(AST_OUTLINE_TAG,
        ast_outline_tag_new(pool, last, code.first))));

      last = string_null();
      token = lex_next(&start, &in->cursor, in->data.end);
    }
  }
  if (!string_size(last))
    return source_error(in, "An outline item must have a name.");
  self->tags = tags.first;
  self->name = string_copy(pool, last);
  CHECK_MEM(string_size(self->name));

  /* Is there a sub-outline? */
  self->children = 0;
  if (token == LEX_BRACE_L) {
    in->cursor = start;
    CHECK(parse_outline(pool, in, scope, out_dynamic(&out)));
    self->children = ast_to_outline(out);
  } else if (token != LEX_SEMICOLON) {
    return source_error(in, "An outline can only end with a semicolon or an opening brace.");
  }

  CHECK(or.code(or.data, dynamic(AST_OUTLINE_ITEM, self)));
  return 1;
}

/**
 * Parses a list of outline items.
 */
int parse_outline(Pool *pool, Source *in, Scope *scope, OutRoutine or)
{
  char const *start;
  Token token;
  ListBuilder items = list_builder_init(pool);
  AstOutline *self = pool_new(pool, AstOutline);
  CHECK_MEM(self);

  /* Opening brace: */
  token = lex_next(&start, &in->cursor, in->data.end);
  if (token != LEX_BRACE_L)
    return source_error(in, "An outline must start with an opening {.");

  /* Items: */
  token = lex_next(&start, &in->cursor, in->data.end);
  while (token != LEX_BRACE_R) {
    in->cursor = start;
    CHECK(parse_outline_item(pool, in, scope, out_list_builder(&items)));
    token = lex_next(&start, &in->cursor, in->data.end);
  }
  self->items = items.first;

  CHECK(or.code(or.data, dynamic(AST_OUTLINE, self)));
  return 1;
}

/**
 * Parses a union of outlines
 */
int parse_union(Pool *pool, Source *in, Scope *scope, OutRoutine or)
{
  char const *start;
  Token token;
  Dynamic out;
  AstOutline *outline;
  AstFilterNode filter;
  ListBuilder items = list_builder_init(pool);
  ListNode *item;
  AstOutline *self = pool_new(pool, AstOutline);
  CHECK_MEM(self);

  /* Opening brace: */
  token = lex_next(&start, &in->cursor, in->data.end);
  if (token != LEX_BRACE_L)
    return source_error(in, "Expecting an opening {.");

outline:
  /* Outline: */
  CHECK(lwl_parse_value(pool, in, scope, out_dynamic(&out)));
  if (out.type != AST_OUTLINE)
    return source_error(in, "Wrong type - the union statement expects an outline.\n");
  outline = ast_to_outline(out);

  /* Map? */
  token = lex_next(&start, &in->cursor, in->data.end);
  if (token == LEX_IDENTIFIER) {
    if (!string_equal(string_init(start, in->cursor), string_init_k("with")))
      return source_error(in, "Only the \"with\" modifier is allowed here.");

    /* Filter: */
    CHECK(parse_filter(pool, in, scope, out_dynamic(&out)));
    filter = ast_to_filter_node(out);
    token = lex_next(&start, &in->cursor, in->data.end);
  } else {
    filter.p = 0;
  }

  /* Process items: */
  for (item = outline->items; item; item = item->next)
    if (!filter.p || test_filter_node(filter, ast_to_outline_item(item->d)))
      CHECK(list_builder_add(&items, item->d));

  /* Another outline? */
  if (token == LEX_COMMA) {
    goto outline;
  } else if (token != LEX_BRACE_R) {
    return source_error(in, "The list of outlines must end with a closing }.");
  }

  self->items = items.first;
  CHECK(or.code(or.data, dynamic(AST_OUTLINE, self)));
  return 1;
}

/**
 * Parses an individual line within a map statement.
 */
int parse_map_line(Pool *pool, Source *in, Scope *scope, OutRoutine or)
{
  char const *start;
  Token token;
  Dynamic out;
  ListBuilder code = list_builder_init(pool);
  AstMapLine *self = pool_new(pool, AstMapLine);
  CHECK_MEM(self);

  /* Filter: */
  CHECK(parse_filter(pool, in, scope, out_dynamic(&out)));
  self->filter = ast_to_filter_node(out);

  /* Opening brace: */
  token = lex_next(&start, &in->cursor, in->data.end);
  if (token != LEX_BRACE_L)
    return source_error(in, "A line within a \"map\" statement must end with a code block.");

  /* Code: */
  CHECK(parse_code(pool, in, scope, out_list_builder(&code), 1));
  self->code = code.first;

  CHECK(or.code(or.data, dynamic(AST_MAP_LINE, self)));
  return 1;
}

/**
 * Parses a map statement.
 */
int parse_map(Pool *pool, Source *in, Scope *scope, OutRoutine or)
{
  char const *start;
  Token token;
  Dynamic out;
  ListBuilder lines = list_builder_init(pool);
  AstMap *self = pool_new(pool, AstMap);
  CHECK_MEM(self);

  /* Item to look up: */
  CHECK(lwl_parse_value(pool, in, scope, out_dynamic(&out)));
  if (out.type != AST_VARIABLE)
    return source_error(in, "Wrong type - expecting an outline item as a map parameter.");
  self->item = out.p;

  /* Opening brace: */
  token = lex_next(&start, &in->cursor, in->data.end);
  if (token != LEX_BRACE_L)
    return source_error(in, "An opening { must come after the name of a map.");

  /* Lines: */
  token = lex_next(&start, &in->cursor, in->data.end);
  while (token != LEX_BRACE_R) {
    in->cursor = start;
    CHECK(parse_map_line(pool, in, scope, out_list_builder(&lines)));
    token = lex_next(&start, &in->cursor, in->data.end);
  }
  self->lines = lines.first;

  CHECK(or.code(or.data, dynamic(AST_MAP, self)));
  return 1;
}

/**
 * Parses a for ... in ... construction.
 */
int parse_for(Pool *pool, Source *in, Scope *scope, OutRoutine or)
{
  char const *start;
  Token token;
  Dynamic out;
  ListBuilder code = list_builder_init(pool);
  Scope inner = scope_init(scope);
  AstFor *self = pool_new(pool, AstFor);
  CHECK_MEM(self);

  /* Variable name: */
  token = lex_next(&start, &in->cursor, in->data.end);
  if (token != LEX_IDENTIFIER)
    return source_error(in, "Expecting a new symbol name here.");
  CHECK(self->item = ast_variable_new(pool, string_init(start, in->cursor)));
  CHECK(scope_add(&inner, pool, self->item->name, dynamic(AST_VARIABLE, self->item)));
  assert(self->item);

  /* "in" keyword: */
  token = lex_next(&start, &in->cursor, in->data.end);
  if (token != LEX_IDENTIFIER ||
    !string_equal(string_init(start, in->cursor), string_init_k("in")))
    return source_error(in, "Expecting the \"in\" keyword here.");

  /* Outline name: */
  CHECK(lwl_parse_value(pool, in, scope, out_dynamic(&out)));
  if (!ast_is_for_node(out.type))
    return source_error(in, "Wrong type - the for statement expects an outline.\n");
  self->outline = ast_to_for_node(out);
  assert(self->outline.p);

  /* Behavior modification keywords: */
  self->filter.p = 0;
  self->reverse = 0;
  self->list = 0;
modifier:
  token = lex_next(&start, &in->cursor, in->data.end);
  if (token == LEX_IDENTIFIER) {
    String s = string_init(start, in->cursor);

    /* "with" modifier: */
    if (string_equal(s, string_init_k("with"))) {
      CHECK(parse_filter(pool, in, scope, out_dynamic(&out)));
      self->filter = ast_to_filter_node(out);
      goto modifier;

    /* "reverse" modifier: */
    } else if (string_equal(s, string_init_k("reverse"))) {
      self->reverse = 1;
      goto modifier;

    /* "list" modifier: */
    } else if (string_equal(s, string_init_k("list"))) {
      self->list = 1;
      goto modifier;
    } else {
      return source_error(in, "Invalid \"for\" statement modifier.");
    }
  } else if (token != LEX_BRACE_L) {
    return source_error(in, "A \"for\" statement must end with a code block.");
  }

  /* Code: */
  CHECK(parse_code(pool, in, &inner, out_list_builder(&code), 1));
  self->code = code.first;

  CHECK(or.code(or.data, dynamic(AST_FOR, self)));
  return 1;
}

/**
 * Parses the "include" directive
 */
int parse_include(Pool *pool, Source *in, Scope *scope, OutRoutine or)
{
  char const *start;
  Token token;
  char const *p, *base_end;
  String filename;
  Source source;
  ListBuilder code = list_builder_init(pool);

  /* File name: */
  token = lex_next(&start, &in->cursor, in->data.end);
  if (token != LEX_STRING)
    return source_error(in, "An include statment expects a quoted filename.");

  /* Resolve relative paths: */
  base_end = in->filename.p;
  for (p = in->filename.p; p < in->filename.end; ++p)
    if (*p == '\\' || *p == '/')
      base_end = p + 1;
  filename = string_merge(pool,
    string_init(in->filename.p, base_end),
    string_init(start + 1, in->cursor - 1));

  /* Process the file's contents: */
  if (!source_load(&source, pool, filename))
    return source_error(in, "Could not open the included file.");
  CHECK(parse_code(pool, &source, scope, out_list_builder(&code), 0));

  /* Closing semicolon: */
  token = lex_next(&start, &in->cursor, in->data.end);
  if (token != LEX_SEMICOLON)
    return source_error(in, "An include stament must end with a semicolon.");

  return 1;
}
