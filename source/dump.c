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

void dump_code_node(AstCodeNode node, int indent);
void dump_for_node(AstForNode node);
void dump_filter_node(AstFilterNode node);

#define INDENT 2

static void space(int indent)
{
  int i;
  for (i = 0; i < indent; ++i)
    putchar(' ');
}

void dump_text(String text)
{
  fwrite(text.p, 1, text.end - text.p, stdout);
}

/**
 * Prints a block of code
 */
void dump_code(ListNode *node, int indent)
{
  for (; node; node = node->next)
    dump_code_node(ast_to_code_node(*node), indent);
}

/**
 * Prints a symbol in a debug-friendly manner.
 */
void dump_variable(AstVariable *p)
{
  dump_text(p->name);
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
    Dynamic temp;
    if (call_input != p->inputs)
      printf(", ");

    dump_variable(ast_to_variable(*macro_input));
    printf("=");
    temp.p = call_input->p; temp.type = call_input->type;
    dump_for_node(ast_to_for_node(temp));

    macro_input = macro_input->next;
    call_input = call_input->next;
  }

  printf(") {");
  dump_code(p->macro->code, 0);
  printf("}");
}

void dump_filter_tag(AstFilterTag *p)
{
  dump_text(p->tag);
}

void dump_filter_any(AstFilterAny *p)
{
  printf("*");
}

void dump_filter_not(AstFilterNot *p)
{
  printf("!");
  dump_filter_node(p->test);
}

void dump_filter_and(AstFilterAnd *p)
{
  printf("(");
  dump_filter_node(p->test_a);
  printf(" & ");
  dump_filter_node(p->test_b);
  printf(")");
}

void dump_filter_or(AstFilterOr *p)
{
  printf("(");
  dump_filter_node(p->test_a);
  printf(" | ");
  dump_filter_node(p->test_b);
  printf(")");
}

void dump_outline(AstOutline *p, int indent);

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
    dump_outline_tag(ast_to_outline_tag(*tag), indent);
    printf(" ");
  }

  /* Item name: */
  dump_text(p->name);

  /* Children: */
  if (p->children && p->children->items)
    dump_outline(p->children, indent);
  else
    printf(";");
}

/**
 * Prints a top-level outline statement.
 */
void dump_outline(AstOutline *p, int indent)
{
  ListNode *item;

  printf(" {\n");
  for (item = p->items; item; item = item->next) {
    dump_outline_item(ast_to_outline_item(*item), indent + INDENT);
    printf("\n");
  }
  space(indent);
  printf("}");
}

void dump_map_line(AstMapLine *p)
{
  printf("  ");
  dump_filter_node(p->filter);
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

  printf("map ");
  dump_text(p->item->name);
  printf(" {\n");

  for (line = p->lines; line; line = line->next)
    dump_map_line(ast_to_map_line(*line));

  printf("}");
}

/**
 * Prints a for ... in construction.
 */
void dump_for(AstFor *p)
{
  printf("for ");
  dump_text(p->item->name);
  printf(" in ");

  dump_for_node(p->outline);

  if (p->filter.p) {
    printf(" with ");
    dump_filter_node(p->filter);
  }
  if (p->reverse)
    printf(" reverse");
  if (p->list)
    printf(" list");

  printf(" {");
  dump_code(p->code, 0);
  printf("}");
}

void dump_code_node(AstCodeNode node, int indent)
{
  if (node.type == AST_CODE_TEXT) {
    AstCodeText *p = node.p;
    dump_text(p->code);
  } else if (node.type == AST_FOR) {
    printf("\\ol ");
    dump_for(node.p);
  } else if (node.type == AST_MACRO_CALL) {
    dump_macro_call(node.p);
  } else if (node.type == AST_VARIABLE) {
    dump_variable(node.p);
  } else if (node.type == AST_LOOKUP) {
    dump_lookup(node.p);
  } else {
    printf("(Unknown code node %d)", node.type);
  }
}

void dump_for_node(AstForNode node)
{
  if (node.type == AST_OUTLINE) {
    printf("outline");
    dump_outline((AstOutline*)node.p, 0);
  } else {
    AstVariable *v = node.p;
    dump_text(v->name);
  }
}

/**
 * Prints a single element within a filter expression
 */
void dump_filter_node(AstFilterNode node)
{
  switch (node.type) {
  case AST_FILTER_TAG: dump_filter_tag(node.p); break;
  case AST_FILTER_ANY: dump_filter_any(node.p); break;
  case AST_FILTER_NOT: dump_filter_not(node.p); break;
  case AST_FILTER_AND: dump_filter_and(node.p); break;
  case AST_FILTER_OR:  dump_filter_or(node.p);  break;
  default: printf("(Unknown filter node %d)", node.type);
  }
}
