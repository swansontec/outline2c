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

#include "debug.h"
#include <stdio.h>

void dump_code_node(AstCodeNode node, int indent);

void dump_outline(AstOutline *p);
void dump_outline_list(AstOutlineList *p, int indent);
void dump_outline_item(AstOutlineItem *p, int indent);
void dump_outline_tag(AstOutlineTag *p, int indent);

void dump_map(AstMap *p);
void dump_map_line(AstMapLine *p);

void dump_for(AstFor *p);

void dump_filter(AstFilter *p);
void dump_filter_node(AstFilterNode node);
void dump_filter_tag(AstFilterTag *p);
void dump_filter_any(AstFilterAny *p);
void dump_filter_not(AstFilterNot *p);
void dump_filter_and(AstFilterAnd *p);
void dump_filter_or(AstFilterOr *p);

void dump_symbol_new(AstSymbolNew *p);
void dump_symbol_ref(AstSymbolRef *p);
void dump_lookup(AstLookup *p);

#define INDENT 2

static void space(int indent)
{
  int i;
  for (i = 0; i < indent; ++i)
    putchar(' ');
}

/**
 * Prints a block of code
 */
void dump_code(AstCode *p, int indent)
{
  AstCodeNode *node;

  for (node = p->nodes; node != p->nodes_end; ++node)
    dump_code_node(*node, indent);
}

void dump_code_node(AstCodeNode node, int indent)
{
  if (node.type == AST_CODE_TEXT) {
    AstCodeText *p = node.p;
    char *temp = string_to_c(p->code);
    printf("%s", temp);
    free(temp);
  } else if (node.type == AST_INCLUDE) {
    AstInclude *p = node.p;
    printf("\\ol include {\n");
    dump_code(p->file->code, indent+1);
    printf("} /* end include */\n");
  } else if (node.type == AST_OUTLINE) {
    dump_outline(node.p);
  } else if (node.type == AST_MAP) {
    dump_map(node.p);
  } else if (node.type == AST_FOR) {
    dump_for(node.p);
  } else if (node.type == AST_SYMBOL_REF) {
    dump_symbol_ref(node.p);
  } else if (node.type == AST_LOOKUP) {
    dump_lookup(node.p);
  } else {
    printf("(Unknown code node %d)", node.type);
  }
}

/**
 * Prints a top-level outline statement.
 */
void dump_outline(AstOutline *p)
{
  char *temp = string_to_c(p->name);
  printf("\\ol outline %s", temp);
  free(temp);
  dump_outline_list(p->children, 0);
}

/**
 * Prints a list of outline items
 */
void dump_outline_list(AstOutlineList *p, int indent)
{
  AstOutlineItem **item;

  printf(" {\n");
  for (item = p->items; item != p->items_end; ++item) {
    dump_outline_item(*item, indent + INDENT);
    printf("\n");
  }
  space(indent);
  printf("}");
}

/**
 * Prints a single item within an outline.
 */
void dump_outline_item(AstOutlineItem *p, int indent)
{
  char *temp;
  AstOutlineTag **tag;

  /* Tags: */
  space(indent);
  for (tag = p->tags; tag != p->tags_end; ++tag) {
    dump_outline_tag(*tag, indent);
    printf(" ");
  }

  /* Item name: */
  temp = string_to_c(p->name);
  printf("%s", temp);
  free(temp);

  /* Children: */
  if (p->children && p->children->items != p->children->items_end)
    dump_outline_list(p->children, indent);
  else
    printf(";");
}

void dump_outline_tag(AstOutlineTag *p, int indent)
{
  char *temp = string_to_c(p->name);
  printf("%s", temp);
  free(temp);
  if (p->value) {
    printf("={");
    dump_code(p->value, indent);
    printf("}");
  }
}

/**
 * Prints a map statement for debugging purposes.
 */
void dump_map(AstMap *p)
{
  char *temp;
  AstMapLine **line;

  temp = string_to_c(p->name);
  printf("\\ol map %s {\n", temp);
  free(temp);

  for (line = p->lines; line != p->lines_end; ++line)
    dump_map_line(*line);

  printf("}");
}

void dump_map_line(AstMapLine *p)
{
  printf("  ");
  dump_filter(p->filter);
  printf(" {");
  dump_code(p->code, 1);
  printf("}\n");
}

/**
 * Prints a for ... in construction.
 */
void dump_for(AstFor *p)
{
  printf("\\ol for ");
  dump_symbol_new(p->symbol);

  if (string_size(p->outline)) {
    char *outline = string_to_c(p->outline);
    printf(" in %s", outline);
    free(outline);
  } else {
    printf(" in .");
  }
  if (p->filter) {
    printf(" with ");
    dump_filter(p->filter);
  }
  if (p->reverse)
    printf(" reverse");
  if (p->list)
    printf(" list");

  printf(" {");
  dump_code(p->code, 0);
  printf("}");
}

/**
 * Prints a filter expression
 */
void dump_filter(AstFilter *p)
{
  dump_filter_node(p->test);
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

void dump_filter_tag(AstFilterTag *p)
{
  char *temp = string_to_c(p->tag);
  printf("%s", temp);
  free(temp);
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

/**
 * Prints an new symbol name definition.
 */
void dump_symbol_new(AstSymbolNew *p)
{
  char *symbol = string_to_c(p->symbol);
  printf("%s", symbol);
  free(symbol);
}

/**
 * Prints a symbol in a debug-friendly manner.
 */
void dump_symbol_ref(AstSymbolRef *p)
{
  char *symbol = string_to_c(p->symbol->symbol);
  printf("<%s>", symbol);
  free(symbol);
}

/**
 * Prints a lookup symbol.
 */
void dump_lookup(AstLookup *p)
{
  char *temp = string_to_c(p->name);
  dump_symbol_ref(p->symbol);
  printf("!%s", temp);
  free(temp);
}
