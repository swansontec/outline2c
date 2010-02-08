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

#include "outline.h"
#include <stdlib.h>

/**
 * Allocates and initializes an OutlineWord structure.
 */
OutlineWord *outline_word_new(char const *p, char const *end)
{
  OutlineWord *temp = malloc(sizeof(OutlineWord));
  if (!temp) return 0;
  temp->p = p;
  temp->end = end;
  temp->next = 0;
  return temp;
}

/**
 * Allocates and initializes an Outline structure.
 */
Outline *outline_new()
{
  Outline *temp = malloc(sizeof(Outline));
  if (!temp) return 0;
  temp->words = 0;
  temp->children = 0;
  temp->next = 0;
  temp->word_n = 0;
  return temp;
}

/**
 * Initializes an outline builder, setting the initial Outline pointer to
 * the passed-in outline.
 */
void outline_builder_init(OutlineBuilder *b, Outline *outline)
{
  b->outline = outline;
  b->word_last = 0;
  b->outline_last = 0;
}

/**
 * Initializes an outline builder, creating a new initial Outline structure.
 * @return 0 for success.
 */
int outline_builder_init_new(OutlineBuilder *b)
{
  b->outline = outline_new();
  if (!b->outline) return 1;
  b->word_last = 0;
  b->outline_last = 0;
  return 0;
}

/**
 * Appends a new word to the end of the Outline structure's word list.
 */
int outline_builder_add_word(OutlineBuilder *b, char const *p, char const *end)
{
  OutlineWord *temp = outline_word_new(p, end);
  if (!temp)
    return 1;

  if (b->word_last)
    b->word_last->next = temp;
  else
    b->outline->words = temp;
  b->word_last = temp;
  ++b->outline->word_n;
  return 0;
}

/**
 * Appends an item to the end of the Outline structure's child list.
 */
void outline_builder_insert_outline(OutlineBuilder *b, Outline *outline)
{
  if (b->outline_last)
    b->outline_last->next = outline;
  else
    b->outline->children = outline;
  b->outline_last = outline;
}
