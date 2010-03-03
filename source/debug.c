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
 * Dumps a node for debugging purposes. Indent sets the indentation level for
 * the printout.
 */
void ast_outline_dump(AstOutline *outline, int indent)
{
  AstOutlineItem *item;
  AstOutline **child;

  /* Words: */
  space(indent);
  item = outline->items;
  while (item < outline->items_end) {
    if (item != outline->items)
      printf(" ");
    ast_outline_item_dump(*item);
    ++item;
  }

  /* Children: */
  child = outline->children;
  if (child != outline->children_end) {
    printf(" {\n");
    while (child != outline->children_end) {
      ast_outline_dump(*child, indent + 1);
      ++child;
    }
    space(indent);
    printf("}\n");
  } else {
    printf(";\n");
  }
}

/**
 * Dumps a single item within an outline.
 */
void ast_outline_item_dump(AstOutlineItem item)
{
  switch (item.type) {
  case AST_OUTLINE_SYMBOL:
    ast_outline_symbol_dump((AstOutlineSymbol*)item.p);
    break;
  case AST_OUTLINE_STRING:
    ast_outline_string_dump((AstOutlineString*)item.p);
    break;
  case AST_OUTLINE_NUMBER:
    ast_outline_number_dump((AstOutlineNumber*)item.p);
    break;
  default:
    printf("(Unknown outline item %d)", item.type);
  }
}

void ast_outline_symbol_dump(AstOutlineSymbol *p)
{
  char *temp = string_to_c(p->symbol);
  printf("%s", temp);
  free(temp);
}

void ast_outline_string_dump(AstOutlineString *p)
{
  char *temp = string_to_c(p->string);
  printf("%s", temp);
  free(temp);
}

void ast_outline_number_dump(AstOutlineNumber *p)
{
  char *temp = string_to_c(p->number);
  printf("%s", temp);
  free(temp);
}

/**
 * Dumps a match for debugging purposes.
 */
void ast_match_dump(AstMatch *match, int indent)
{
  printf("@o2c match ");
  if (match->lines_end - match->lines == 1) {
    ast_match_line_dump(*match->lines, indent);
  } else {
    AstMatchLine **line;

    printf("{\n");
    line = match->lines;
    while (line < match->lines_end) {
      space(indent + 1);
      ast_match_line_dump(*line, indent + 1);
      ++line;
    }
    space(indent);
    printf("}\n");
  }
}

/**
 * Dumps a single pattern line within a match.
 */
void ast_match_line_dump(AstMatchLine *p, int indent)
{
  ast_pattern_dump(p->pattern);
  printf("{");
  ast_code_dump(p->code, indent);
  printf("}\n");
}

/*
 * Formats and displays a pattern
 */
void ast_pattern_dump(AstPattern *p)
{
  AstPatternItem *item;

  item = p->items;
  while (item < p->items_end) {
    if (item != p->items)
      printf(" ");
    ast_pattern_item_dump(*item);
    ++item;
  }
}

/**
 * Formats and displays a single element within a pattern
 */
void ast_pattern_item_dump(AstPatternItem item)
{
  switch (item.type) {
  case AST_PATTERN_WILD:
    ast_pattern_wild_dump((AstPatternWild*)item.p);
    break;
  case AST_PATTERN_SYMBOL:
    ast_pattern_symbol_dump((AstPatternSymbol*)item.p);
    break;
  case AST_PATTERN_ASSIGN:
    ast_pattern_assign_dump((AstPatternAssign*)item.p);
    break;
  default:
    printf("(Unknown pattern item %d)", item.type);
  }
}

void ast_pattern_wild_dump(AstPatternWild *p)
{
  printf("<>");
}

void ast_pattern_symbol_dump(AstPatternSymbol *p)
{
  char *temp = string_to_c(p->symbol);
  printf("%s", temp);
  free(temp);
}

void ast_pattern_assign_dump(AstPatternAssign *p)
{
  char *temp = string_to_c(p->symbol);
  printf("%s=(", temp);
  free(temp);
  ast_pattern_item_dump(p->pattern);
  printf(")");
}

void ast_code_dump(AstCode *p, int indent)
{
  AstCodeItem *item;

  item = p->items;
  while (item < p->items_end) {
    if (item->type == AST_C) {
      AstC *p = item->p;
      char *temp = string_to_c(string_init(p->code.p, p->code.end));
      printf("%s", temp);
      free(temp);
    } else if (item->type == AST_MATCH) {
      AstMatch *p = item->p;
      ast_match_dump(p, indent);
    } else if (item->type == AST_CODE_SYMBOL) {
      AstCodeSymbol *p = item->p;
      char *s = string_to_c(p->symbol->symbol);
      printf("<%s>", s);
      free(s);
    } else {
      printf("(Unknown code item %d)", item->type);
    }
    ++item;
  }
}