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
#include "string.h"
#include <stdio.h>

static void space(int indent)
{
  int i;
  for (i = 0; i < indent; ++i)
    printf("  ");
}

/**
 * Prints a block of code
 */
void dump_code(AstCode *p, int indent)
{
  AstCodeNode *node;

  for (node = p->nodes; node != p->nodes_end; ++node) {
    if (node->type == AST_CODE_TEXT) {
      AstCodeText *p = node->p;
      char *temp = string_to_c(string_init(p->code.p, p->code.end));
      printf("%s", temp);
      free(temp);
    } else if (node->type == AST_INCLUDE) {
      AstInclude *p = node->p;
      printf("@o2c include {{{\n");
      dump_code(p->file->code, indent+1);
      printf("}}}\n");
    } else if (node->type == AST_OUTLINE) {
      dump_outline(node->p);
    } else if (node->type == AST_MAP) {
      dump_map(node->p);
    } else if (node->type == AST_FOR) {
      dump_for(node->p);
    } else if (node->type == AST_SYMBOL) {
      dump_symbol(node->p);
    } else if (node->type == AST_LOOKUP) {
      dump_lookup(node->p);
    } else {
      printf("(Unknown code node %d)", node->type);
    }
  }
}

/**
 * Prints a top-level outline statement.
 */
void dump_outline(AstOutline *p)
{
  char *temp = string_to_c(p->name);
  printf("@o2c outline %s", temp);
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
  for (item = p->items; item != p->items_end; ++item)
    dump_outline_item(*item, indent + 1);
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
    if (tag != p->tags)
      printf(" ");
    dump_outline_tag(*tag, indent);
  }

  /* Item name: */
  temp = string_to_c(p->name);
  printf(" %s", temp);
  free(temp);

  /* Children: */
  if (p->children && p->children->items != p->children->items_end)
    dump_outline_list(p->children, indent);
  else
    printf(";\n");
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
  printf("@o2c map %s {\n", temp);
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
  printf("@o2c for ");
  dump_in(p->in);

  if (p->filter) {
    printf(" with ");
    dump_filter(p->filter);
  }

  printf(" {");
  dump_code(p->code, 0);
  printf("}");
}

/**
 * Prints an "x in y" portion of a for statement.
 */
void dump_in(AstIn *p)
{
  char *symbol = string_to_c(p->symbol);
  if (p->name.p) {
    char *name = string_to_c(p->name);
    printf("%s in %s", symbol, name);
    free(name);
  } else {
    printf("%s in .", symbol);
  }
  free(symbol);
  if (p->reverse)
    printf(" reverse");
  if (p->list)
    printf(" list");
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
 * Prints a symbol in a debug-friendly manner.
 */
void dump_symbol(AstSymbol *p)
{
  printf("<%d>", p->level);
}

/**
 * Prints a lookup symbol.
 */
void dump_lookup(AstLookup *p)
{
  char *temp = string_to_c(p->name);
  dump_symbol(p->symbol);
  printf("\\%s", temp);
  free(temp);
}
