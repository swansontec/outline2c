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
 * Extracts an AstOutlineItem list from an AST node
 */
ListNode *get_items(Dynamic node)
{
  if (node.type == AST_VARIABLE) {
    AstVariable *v = node.p;
    assert(v->value);
    return v->value->children ? v->value->children->items : 0;
  } else if (node.type == AST_OUTLINE_ITEM) {
    AstOutlineItem *item = node.p;
    return item->children ? item->children->items : 0;
  } else if (node.type == AST_OUTLINE) {
    AstOutline *outline = node.p;
    return outline->items;
  } else {
    assert(0);
    return 0;
  }
}

/**
 * Processes source code, writing the result to the output file.
 */
int generate_code(Pool *pool, FILE *out, ListNode *node)
{
  for (; node; node = node->next)
    CHECK(generate(pool, out, node->d));
  return 1;
}

/**
 * Performs code-generation for a variable lookup
 */
int generate_variable(Pool *pool, FILE *out, AstVariable *p)
{
  AstOutlineItem *item = p->value;

  CHECK(file_write(out, item->name.p, item->name.end));
  return 1;
}

/**
 * Searches for a tag with the specified name in an outline item. If the tag
 * exists and has a value, the function emits the value and returns 1.
 * Otherwise, the function returns -1. Returns 0 for errors.
 */
int generate_lookup_tag(Pool *pool, FILE *out, AstLookup *p)
{
  AstOutlineItem *item;
  ListNode *tag;

  if (p->item.type == AST_VARIABLE) {
    AstVariable *v = p->item.p;
    item = v->value;
  } else if (p->item.type == AST_OUTLINE_ITEM) {
    item = p->item.p;
  }

  for (tag = item->tags; tag; tag = tag->next) {
    AstOutlineTag *t = ast_to_outline_tag(tag->d);
    if (t->value && string_equal(t->name, p->name)) {
      CHECK(generate_code(pool, out, t->value));
      return 1;
    }
  }

  return -1;
}

/**
 * If the lookup name matches one of the built-in transformations, generate
 * that and return 1. Otherwise, return 0.
 */
int generate_lookup_builtin(Pool *pool, FILE *out, AstLookup *p)
{
  AstOutlineItem *item;

  if (p->item.type == AST_VARIABLE) {
    AstVariable *v = p->item.p;
    item = v->value;
  } else if (p->item.type == AST_OUTLINE_ITEM) {
    item = p->item.p;
  }

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

/**
 * Performs code-generation for a lookup node.
 */
int generate_lookup(Pool *pool, FILE *out, AstLookup *p)
{
  int rv;

  /* If a tag satisfies the lookup, generate that: */
  rv = generate_lookup_tag(pool, out, p);
  if (rv == 1) return 1;
  if (rv == 0) return 0;

  /* If that didn't work, try the built-in transforms: */
  if (generate_lookup_builtin(pool, out, p))
    return 1;

  {
    char *temp = string_to_c(p->name);
    fprintf(stderr, "Could not find a transform named %s.\n", temp);
    free(temp);
  }
  return 0;
}

int generate_macro_call(Pool *pool, FILE *out, AstMacroCall *p)
{
  ListNode *call_input;
  ListNode *macro_input;
  Scope scope = scope_init(p->macro->scope);
  ListBuilder code = list_builder_init(pool);

  /* Assign values to all inputs: */
  macro_input = p->macro->inputs;
  call_input = p->inputs;
  while (macro_input && call_input) {
    AstCodeText *name = macro_input->d.p;
    CHECK(scope_add(&scope, pool, name->code, call_input->d));

    macro_input = macro_input->next;
    call_input = call_input->next;
  }

  CHECK(parse_code(pool, &p->macro->code, &scope, out_list_builder(&code)));
  CHECK(generate_code(pool, out, code.first));
  return 1;
}

int generate_outline_item(Pool *pool, FILE *out, AstOutlineItem *p)
{
  CHECK(file_write(out, p->name.p, p->name.end));
  return 1;
}

/**
 * Performs code-generation for a map statement.
 */
int generate_map(Pool *pool, FILE *out, AstMap *p)
{
  AstOutlineItem *item;
  ListNode *line;
  char *temp;

  if (p->item.type == AST_VARIABLE) {
    AstVariable *v = p->item.p;
    item = v->value;
  } else if (p->item.type == AST_OUTLINE_ITEM) {
    item = p->item.p;
  }

  /* Match against the map: */
  for (line = p->lines; line; line = line->next) {
    AstMapLine *l = ast_to_map_line(line->d);
    if (test_filter(l->filter, item)) {
      CHECK(generate_code(pool, out, l->code));
      return 1;
    }
  }

  /* Nothing matched: */
  temp = string_to_c(item->name);
  fprintf(stderr, "error: Could not match item \"%s\" against map.\n", temp);
  free(temp);
  return 0;
}

int generate_for_item(Pool *pool, FILE *out, AstFor *p, ListNode *item, int *need_comma)
{
  Scope scope = scope_init(p->scope);
  ListBuilder code = list_builder_init(pool);

  if (p->filter.p && !test_filter(p->filter, ast_to_outline_item(item->d)))
    return 1;

  if (p->list && *need_comma)
    CHECK(file_putc(out, ','));
  *need_comma = 1;

  CHECK(scope_add(&scope, pool, p->item, item->d));
  CHECK(parse_code(pool, &p->code, &scope, out_list_builder(&code)));
  CHECK(generate_code(pool, out, code.first));
  return 1;
}

/**
 * Performs code-generation for a for statement node
 */
int generate_for(Pool *pool, FILE *out, AstFor *p)
{
  ListNode *items = get_items(p->outline);
  int need_comma = 0;

  /* Process the list: */
  if (p->reverse) {
    /* Warning: O(n^2) reversing algorithm */
    ListNode *item, *last = 0;
    while (items != last) {
      item = items;
      while (item->next != last)
        item = item->next;
      last = item;

      CHECK(generate_for_item(pool, out, p, item, &need_comma));
    }
  } else {
    ListNode *item;
    for (item = items; item; item = item->next)
      CHECK(generate_for_item(pool, out, p, item, &need_comma));
  }

  return 1;
}

int generate_code_text(Pool *pool, FILE *out, AstCodeText *p)
{
  CHECK(file_write(out, p->code.p, p->code.end));
  return 1;
}

/**
 * Processes source code, writing the result to the output file.
 */
int generate(Pool *pool, FILE *out, Dynamic node)
{
  switch (node.type) {
  case AST_VARIABLE:   return generate_variable(pool, out, node.p);
  case AST_LOOKUP:     return generate_lookup(pool, out, node.p);
  case AST_MACRO_CALL: return generate_macro_call(pool, out, node.p);
  case AST_OUTLINE_ITEM: return generate_outline_item(pool, out, node.p);
  case AST_MAP:        return generate_map(pool, out, node.p);
  case AST_FOR:        return generate_for(pool, out, node.p);
  case AST_CODE_TEXT:  return generate_code_text(pool, out, node.p);
  default: assert(0);  return 0;
  }
}
