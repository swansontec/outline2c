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

#ifndef OUTLINE_H_INCLUDED
#define OUTLINE_H_INCLUDED

#include "typedefs.h"

/**
 * Represents a single word in an outline.
 */
struct outline_word {
  char const *p;        /* First character */
  char const *end;      /* One-past the last character */
  OutlineWord *next;    /* The next word in the outline, if any */
};
OutlineWord *outline_word_new(char const *p, char const *end);

/**
 * Represents a node in an outline. A node consists of several words an an
 * optional list of sub-nodes.
 */
struct outline {
  OutlineWord *words;   /* The first word */
  Outline *children;    /* The first child node, if any */
  Outline *next;        /* The next node at this level, if any */
  int word_n;           /* Number of words in the linked list */
};
Outline *outline_new();

/**
 * Builds an Outline structure element-by-element.
 */
struct outline_builder {
  Outline *outline;
  OutlineWord *word_last;
  Outline *outline_last;
};
typedef struct outline_builder OutlineBuilder;
void outline_builder_init(OutlineBuilder *b, Outline *root);
int outline_builder_init_new(OutlineBuilder *b);
int outline_builder_add_word(OutlineBuilder *b, char const *p, char const *end);
void outline_builder_insert_outline(OutlineBuilder *b, Outline *outline);

#endif
