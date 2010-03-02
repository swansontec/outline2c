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
#include "outline.h"
#include "match.h"
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
void outline_dump(Outline *node, int indent)
{
  OutlineItem *word = node->words;
  char *temp;
  space(indent);
  while (word) {
    temp = string_to_c(string_init(word->p, word->end));
    printf("%s ", temp);
    free(temp);
    word = word->next;
  }
  if (node->children) {
    printf("{\n");
    node = node->children;
    while (node) {
      outline_dump(node, indent + 1);
      node = node->next;
    }
    space(indent);
    printf("}\n");
  } else {
    printf(";\n");
  }
}

void match_dump_line(Match *match, int indent);

/**
 * Dumps a match for debugging purposes.
 */
void match_dump(Match *match, int indent)
{
  if (match) {
    printf("@o2c match ");
    if (match->next) {
      printf("{\n");
      while (match) {
        space(indent + 1);
        match_dump_line(match, indent + 1);
        match = match->next;
      }
      space(indent);
      printf("}\n");
    } else {
      match_dump_line(match, indent);
    }
  }
}

/**
 * Dumps a single pattern line within a match.
 */
void match_dump_line(Match *match, int indent)
{
  Code code;

  /* Pattern: */
  ast_pattern_dump(match->pattern);

  /* Code block (the formatting is all wrong, but the info is there): */
  printf("{");
  code = match->code;
  while (code.p) {
    if (code.type == CODE_CODE) {
      CodeCode *p = (CodeCode *)code.p;
      char *temp = string_to_c(string_init(p->p, p->end));
      printf("%s", temp);
      free(temp);
    } else if (code.type == CODE_REPLACE) {
      CodeReplace *p = (CodeReplace *)code.p;
      char *s = string_to_c(*p->symbol);
      printf("<%s>", s);
      free(s);
    } else if (code.type == CODE_MATCH) {
      CodeMatch *p = (CodeMatch *)code.p;
      match_dump(p->match, indent);
    }
    code = code_get_next(code);
  }
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
