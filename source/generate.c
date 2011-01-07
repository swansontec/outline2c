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

int generate_code_node(FILE *out, AstCodeNode node);
int generate_map(FILE *out, AstMap *p);
int generate_for(FILE *out, AstFor *p);
int generate_macro_call(FILE *out, AstMacroCall *p);
int generate_variable(FILE *out, AstVariable *p);
int generate_lookup(FILE *out, AstLookup *p);
int generate_lookup_tag(FILE *out, AstLookup *p);
int generate_lookup_builtin(FILE *out, AstLookup *p);

/**
 * Processes source code, writing the result to the output file.
 */
int generate_code(FILE *out, ListNode *node)
{
  for (; node; node = node->next)
    CHECK(generate_code_node(out, ast_to_code_node(*node)));
  return 1;
}

/**
 * Processes source code, writing the result to the output file.
 */
int generate_code_node(FILE *out, AstCodeNode node)
{
  if (node.type == AST_CODE_TEXT) {
    AstCodeText *p = node.p;
    CHECK(file_write(out, p->code.p, p->code.end));
  } else if (node.type == AST_MAP) {
    CHECK(generate_map(out, node.p));
  } else if (node.type == AST_FOR) {
    CHECK(generate_for(out, node.p));
  } else if (node.type == AST_MACRO_CALL) {
    CHECK(generate_macro_call(out, node.p));
  } else if (node.type == AST_VARIABLE) {
    CHECK(generate_variable(out, node.p));
  } else if (node.type == AST_LOOKUP) {
    CHECK(generate_lookup(out, node.p));
  } else {
    assert(0);
  }
  return 1;
}

/**
 * Performs code-generation for a map statement.
 */
int generate_map(FILE *out, AstMap *p)
{
  AstOutlineItem *item = p->item->value;
  ListNode *line;
  char *temp;

  /* Match against the map: */
 for (line = p->lines; line; line = line->next) {
   AstMapLine *l = ast_to_map_line(*line);
   if (test_filter(l->filter, item)) {
     CHECK(generate_code(out, l->code));
     return 1;
   }
 }

 /* Nothing matched: */
 temp = string_to_c(p->item->value->name);
 fprintf(stderr, "error: Could not match item \"%s\" against map.\n", temp);
 free(temp);
 return 0;
}

/**
 * Performs code-generation for a for statement node
 */
int generate_for(FILE *out, AstFor *p)
{
  AstOutline *outline;
  int need_comma = 0;

  /* Find the outline list to process: */
  if (p->outline.type == AST_OUTLINE) {
    outline = p->outline.p;
  } else {
    AstVariable *v = p->outline.p;
    outline = v->value->children;
  }
  if (!outline)
    return 1;

  /* Process the list: */
  if (p->reverse) {
    /* Warning: O(n^2) reversing algorithm */
    ListNode *item, *last = 0;
    while (outline->items != last) {
      item = outline->items;
      while (item->next != last)
        item = item->next;
      last = item;

      p->item->value = ast_to_outline_item(*item);
      if (!p->filter || test_filter(p->filter, p->item->value)) {
        if (p->list && need_comma)
          CHECK(file_putc(out, ','));
        CHECK(generate_code(out, p->code));
        need_comma = 1;
      }
    }
  } else {
    ListNode *item;
    for (item = outline->items; item; item = item->next) {
      p->item->value = ast_to_outline_item(*item);
      if (!p->filter || test_filter(p->filter, p->item->value)) {
        if (p->list && need_comma)
          CHECK(file_putc(out, ','));
        CHECK(generate_code(out, p->code));
        need_comma = 1;
      }
    }
  }

  return 1;
}

int generate_macro_call(FILE *out, AstMacroCall *p)
{
  ListNode *call_input;
  ListNode *macro_input;
  Pool pool;
  CHECK_MEM(pool_init(&pool, 0x100));

  /* Assign values to all inputs: */
  macro_input = p->macro->inputs;
  call_input = p->inputs;
  while (macro_input && call_input) {
    AstVariable *input = ast_to_variable(*macro_input);

    if (call_input->type == AST_VARIABLE) {
      AstVariable *value = call_input->p;
      input->value = value->value;
    } else if (call_input->type == AST_OUTLINE) {
      AstOutlineItem *temp = pool_alloc(&pool, sizeof(AstOutlineItem));
      temp->children = call_input->p;
      temp->name = input->name;
      temp->tags = 0;
      input->value = temp;
    } else {
      assert(0);
    }

    macro_input = macro_input->next;
    call_input = call_input->next;
  }

  CHECK(generate_code(out, p->macro->code));
  pool_free(&pool);
  return 1;
}

/**
 * Performs code-generation for a variable lookup
 */
int generate_variable(FILE *out, AstVariable *p)
{
  AstOutlineItem *item = p->value;

  CHECK(file_write(out, item->name.p, item->name.end));
  return 1;
}

/**
 * Performs code-generation for a lookup node.
 */
int generate_lookup(FILE *out, AstLookup *p)
{
  int rv;

  /* If a tag satisfies the lookup, generate that: */
  rv = generate_lookup_tag(out, p);
  if (rv == 1) return 1;
  if (rv == 0) return 0;

  /* If that didn't work, try the built-in transforms: */
  if (generate_lookup_builtin(out, p))
    return 1;

  {
    char *temp = string_to_c(p->name);
    fprintf(stderr, "Could not find a transform named %s.\n", temp);
    free(temp);
  }
  return 0;
}

/**
 * Searches for a tag with the specified name in an outline item. If the tag
 * exists and has a value, the function emits the value and returns 1.
 * Otherwise, the function returns -1. Returns 0 for errors.
 */
int generate_lookup_tag(FILE *out, AstLookup *p)
{
  AstOutlineItem *item = p->item->value;
  ListNode *tag;

  for (tag = item->tags; tag; tag = tag->next) {
    AstOutlineTag *t = ast_to_outline_tag(*tag);
    if (t->value && string_equal(t->name, p->name)) {
      CHECK(generate_code(out, t->value));
      return 1;
    }
  }

  return -1;
}

/**
 * If the lookup name matches one of the built-in transformations, generate
 * that and return 1. Otherwise, return 0.
 */
int generate_lookup_builtin(FILE *out, AstLookup *p)
{
  AstOutlineItem *item = p->item->value;

  if (string_equal(p->name, string_init_k("quote"))) {
    CHECK(file_putc(out, '"'));
    CHECK(file_write(out, item->name.p, item->name.end));
    CHECK(file_putc(out, '"'));
    return 1;
  } else if (string_equal(p->name, string_init_k("lower"))) {
    return generate_lower(out, item->name);
  } else if (string_equal(p->name, string_init_k("upper"))) {
    return generate_upper(out, item->name);
  } else if (string_equal(p->name, string_init_k("camel"))) {
    return generate_camel(out, item->name);
  } else if (string_equal(p->name, string_init_k("mixed"))) {
    return generate_mixed(out, item->name);
  }

  return 0;
}
