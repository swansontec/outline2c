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

#ifndef PATTERN_H_INCLUDED
#define PATTERN_H_INCLUDED

#include "typedefs.h"
#include "string.h"

typedef struct symbol Symbol;
/**
 * A symbol is a word which will be replaced with something else.
 */
struct symbol {
  char const *p;        /* First letter of the symbol */
  char const *end;      /* One-past the last character */
  Symbol *next;         /* The next symbol in the current scope. */
};

/**
 * A typed pointer to an item in a pattern.
 */
struct pattern {
  void *p;
  enum {
    PATTERN_WORD,
    PATTERN_REPLACE,
    PATTERN_WILD
  } type;
};
Pattern pattern_get_next(Pattern pattern);
void pattern_set_next(Pattern pattern, Pattern next);
int pattern_compare(Pattern pattern, Outline *outline);

/**
 * Matches anything
 */
struct pattern_wild {
  Pattern next;
};
Pattern pattern_wild_new();
int pattern_wild_match(PatternWild *p, OutlineItem *o);

/**
 * A word within a pattern to match exactly.
 */
struct pattern_word {
  String symbol;        /* The symbol to match */
  Pattern next;         /* The next pattern in the chain */
};
Pattern pattern_word_new(char const *p, char const *end);
int pattern_word_match(PatternWord *p, OutlineItem *o);

/**
 * A word within a pattern to replace.
 */
struct pattern_replace {
  String symbol;        /* The name of the symbol */
  Pattern pattern;      /* The pattern that will replace the symbol */
  Pattern next;         /* The next pattern in the chain */
};
Pattern pattern_replace_new(String symbol, Pattern pattern);
int pattern_replace_match(PatternReplace *p, OutlineItem *o);

/**
 * Links pattern items together into a list.
 */
struct pattern_builder {
  Pattern first;
  Pattern last;
};
typedef struct pattern_builder PatternBuilder;
void pattern_builder_init(PatternBuilder *b);
int pattern_builder_add_wild(PatternBuilder *b);
int pattern_builder_add_word(PatternBuilder *b, char const *p, char const *end);
int pattern_builder_add_replace(PatternBuilder *b, String symbol, Pattern pattern);

#endif
