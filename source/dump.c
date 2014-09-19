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

#define INDENT 2

static void space(int indent)
{
  int i;
  for (i = 0; i < indent; ++i)
    putchar(' ');
}

void dump_text(String text)
{
  fwrite(text.p, string_size(text), 1, stdout);
}

/**
 * Prints a block of code
 */
void dump_code(ListNode *node, int indent)
{
  for (; node; node = node->next)
    dump(node->d, indent);
}

/**
 * Prints a lookup symbol.
 */
void dump_lookup(AstLookup *p)
{
  dump_text(p->item->name);
  printf("!");
  dump_text(p->name);
}

void dump_macro_call(AstMacroCall *p)
{
  ListNode *macro_input;
  ListNode *call_input;

  printf("macro(");

  macro_input = p->macro->inputs;
  call_input = p->inputs;
  while (macro_input && call_input) {
    AstCodeText *name = macro_input->d.p;
    if (call_input != p->inputs)
      printf(", ");

    dump_text(name->code);
    printf("=");
    dump(call_input->d, 0);

    macro_input = macro_input->next;
    call_input = call_input->next;
  }

  printf(") {");
  dump_text(string(p->macro->code.cursor, p->macro->code.data.end));
  printf("}");
}

void dump_filter_tag(AstFilterTag *p)
{
  dump_text(p->tag);
}

void dump_filter_any(void *data)
{
  printf("*");
}

void dump_filter_not(AstFilterNot *p)
{
  printf("!");
  dump(p->test, 0);
}

void dump_filter_and(AstFilterAnd *p)
{
  printf("(");
  dump(p->test_a, 0);
  printf(" & ");
  dump(p->test_b, 0);
  printf(")");
}

void dump_filter_or(AstFilterOr *p)
{
  printf("(");
  dump(p->test_a, 0);
  printf(" | ");
  dump(p->test_b, 0);
  printf(")");
}

void dump_outline_items(AstOutline *p, int indent);

void dump_outline_tag(AstOutlineTag *p, int indent)
{
  dump_text(p->name);
  if (p->value) {
    printf("={");
    dump_code(p->value, indent);
    printf("}");
  }
}

/**
 * Prints a single item within an outline.
 */
void dump_outline_item(AstOutlineItem *p, int indent)
{
  ListNode *tag;

  /* Tags: */
  space(indent);
  for (tag = p->tags; tag; tag = tag->next) {
    dump_outline_tag(ast_to_outline_tag(tag->d), indent);
    printf(" ");
  }

  /* Item name: */
  dump_text(p->name);

  /* Children: */
  if (p->children && p->children->items)
    dump_outline_items(p->children, indent);
  else
    printf(";");
}

void dump_outline_items(AstOutline *p, int indent)
{
  ListNode *item;

  printf(" {\n");
  for (item = p->items; item; item = item->next) {
    dump_outline_item(ast_to_outline_item(item->d), indent + INDENT);
    printf("\n");
  }
  space(indent);
  printf("}");
}

/**
 * Prints a top-level outline statement.
 */
void dump_outline(AstOutline *p, int indent)
{
  printf("outline");
  dump_outline_items(p, indent);
}

void dump_map_line(AstMapLine *p)
{
  printf("  ");
  dump(p->filter, 0);
  printf(" {");
  dump_code(p->code, 1);
  printf("}\n");
}

/**
 * Prints a map statement for debugging purposes.
 */
void dump_map(AstMap *p)
{
  ListNode *line;

  printf("\\ol map ");
  dump_text(p->item->name);
  printf(" {\n");

  for (line = p->lines; line; line = line->next)
    dump_map_line(ast_to_map_line(line->d));

  printf("}");
}

/**
 * Prints a for ... in construction.
 */
void dump_for(AstFor *p)
{
  printf("\\ol for ");
  dump_text(p->item);
  printf(" in ");

  dump(p->outline, 0);

  if (dynamic_ok(p->filter)) {
    printf(" with ");
    dump(p->filter, 0);
  }
  if (p->reverse)
    printf(" reverse");
  if (p->list)
    printf(" list");

  printf(" {");
  dump_text(string(p->code.cursor, p->code.data.end));
  printf("}");
}

void dump_code_text(AstCodeText *p)
{
  dump_text(p->code);
}

void dump(Dynamic node, int indent)
{
  if (node.type == type_lookup)      dump_lookup(node.p); return;
  if (node.type == type_macro_call)  dump_macro_call(node.p); return;
  if (node.type == type_outline_item)dump_outline_item(node.p, indent); return;
  if (node.type == type_filter_tag)  dump_filter_tag(node.p); return;
  if (node.type == type_filter_any)  dump_filter_any(node.p); return;
  if (node.type == type_filter_not)  dump_filter_not(node.p); return;
  if (node.type == type_filter_and)  dump_filter_and(node.p); return;
  if (node.type == type_filter_or)   dump_filter_or(node.p); return;
  if (node.type == type_outline)     dump_outline(node.p, indent); return;
  if (node.type == type_map)         dump_map(node.p); return;
  if (node.type == type_for)         dump_for(node.p); return;
  if (node.type == type_code_text)   dump_code_text(node.p); return;
  printf("<Unknown node>");
}
