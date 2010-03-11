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
      AstOutline *p = node->p;
      dump_outline(p);
    } else if (node->type == AST_FOR_IN) {
      AstForIn *p = node->p;
      dump_for_in(p);
    } else if (node->type == AST_MATCH) {
      AstMatch *p = node->p;
      dump_match(p, indent);
    } else if (node->type == AST_SYMBOL) {
      AstSymbol *p = node->p;
      char *s = string_to_c(p->symbol->symbol);
      printf("<%s>", s);
      free(s);
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
  for (item = p->items; item != p->items_end; ++item) {
    dump_outline_item(*item, indent + 1);
  }
  space(indent);
  printf("}\n");
}

/**
 * Prints a single item within an outline.
 */
void dump_outline_item(AstOutlineItem *p, int indent)
{
  AstOutlineNode *node;

  /* Words: */
  space(indent);
  for (node = p->nodes; node != p->nodes_end; ++node) {
    if (node != p->nodes)
      printf(" ");
    dump_outline_node(*node);
  }

  /* Children: */
  if (p->children && p->children->items != p->children->items_end)
    dump_outline_list(p->children, indent);
  else
    printf(";\n");
}

/**
 * Prints a single word within an outline.
 */
void dump_outline_node(AstOutlineNode node)
{
  switch (node.type) {
  case AST_OUTLINE_SYMBOL:
    dump_outline_symbol((AstOutlineSymbol*)node.p);
    break;
  case AST_OUTLINE_STRING:
    dump_outline_string((AstOutlineString*)node.p);
    break;
  case AST_OUTLINE_NUMBER:
    dump_outline_number((AstOutlineNumber*)node.p);
    break;
  default:
    printf("(Unknown outline node %d)", node.type);
  }
}

void dump_outline_symbol(AstOutlineSymbol *p)
{
  char *temp = string_to_c(p->symbol);
  printf("%s", temp);
  free(temp);
}

void dump_outline_string(AstOutlineString *p)
{
  char *temp = string_to_c(p->string);
  printf("%s", temp);
  free(temp);
}

void dump_outline_number(AstOutlineNumber *p)
{
  char *temp = string_to_c(p->number);
  printf("%s", temp);
  free(temp);
}

/**
 * Prints a for ... in construction.
 */
void dump_for_in(AstForIn *p)
{
  char *name = string_to_c(p->name);
  char *outline = string_to_c(p->outline);
  printf("for %s in %s ", name, outline);
  free(name);
  free(outline);

  if (p->filter) {
    printf("with ");
    dump_filter(p->filter);
    printf(" ");
  }

  printf("{");
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
 * Prints a match statement for debugging purposes.
 */
void dump_match(AstMatch *match, int indent)
{
  printf("@o2c match ");
  if (match->lines_end - match->lines == 1) {
    dump_match_line(*match->lines, indent);
  } else {
    AstMatchLine **line;

    printf("{\n");
    for (line = match->lines; line != match->lines_end; ++line) {
      space(indent + 1);
      dump_match_line(*line, indent + 1);
    }
    space(indent);
    printf("}\n");
  }
}

/**
 * Prints a single pattern line within a match statement.
 */
void dump_match_line(AstMatchLine *p, int indent)
{
  dump_pattern(p->pattern);
  printf("{");
  dump_code(p->code, indent);
  printf("}\n");
}

/*
 * Prints a pattern
 */
void dump_pattern(AstPattern *p)
{
  AstPatternNode *node;

  for (node = p->nodes; node != p->nodes_end; ++node) {
    if (node != p->nodes)
      printf(" ");
    dump_pattern_node(*node);
  }
}

/**
 * Prints a single element within a pattern
 */
void dump_pattern_node(AstPatternNode node)
{
  switch (node.type) {
  case AST_PATTERN_WILD:
    dump_pattern_wild((AstPatternWild*)node.p);
    break;
  case AST_PATTERN_SYMBOL:
    dump_pattern_symbol((AstPatternSymbol*)node.p);
    break;
  case AST_PATTERN_ASSIGN:
    dump_pattern_assign((AstPatternAssign*)node.p);
    break;
  default:
    printf("(Unknown pattern node %d)", node.type);
  }
}

void dump_pattern_wild(AstPatternWild *p)
{
  printf("<>");
}

void dump_pattern_symbol(AstPatternSymbol *p)
{
  char *temp = string_to_c(p->symbol);
  printf("%s", temp);
  free(temp);
}

void dump_pattern_assign(AstPatternAssign *p)
{
  char *temp = string_to_c(p->symbol);
  printf("%s=(", temp);
  free(temp);
  dump_pattern_node(p->pattern);
  printf(")");
}
