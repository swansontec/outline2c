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

#ifndef MATCH_H_INCLUDED
#define MATCH_H_INCLUDED

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
 * Pattern structure types.
 */
enum pattern_type {
  PATTERN_WORD,
  PATTERN_REPLACE
};

/**
 * A pointer to an item in a pattern.
 */
struct pattern {
  void *p;
  enum pattern_type type;
};
Pattern pattern_get_next(Pattern pattern);
void pattern_set_next(Pattern pattern, Pattern next);

/**
 * A word within a pattern to match exactly.
 */
struct pattern_word {
  char const *p;        /* First character */
  char const *end;      /* One-past the last character */
  Pattern next;         /* The next pattern in the chain */
};
Pattern pattern_word_new(char const *p, char const *end);

/**
 * A word within a pattern to replace.
 */
struct pattern_replace {
  char const *p;        /* First character */
  char const *end;      /* One-past the last character */
  Pattern next;         /* The next pattern in the chain */
};
Pattern pattern_replace_new(char const *p, char const *end);

/**
 * Code-generation template structure types.
 */
enum code_type {
  CODE_CODE,
  CODE_REPLACE,
  CODE_MATCH
};

/**
 * A pointer to an item in a code-generation template.
 */
struct code {
  void *p;
  enum code_type type;
};
Code code_get_next(Code code);
void code_set_next(Code code, Code next);

/**
 * A block of code within a code-generation pattern. This will be written out
 * in an unmodified form.
 */
struct code_code {
  char const *p;        /* The beginning of the code block */
  char const *end;      /* One-past the end of the code block */
  Code next;            /* The next item in the code-generation template, if any */
};
Code code_code_new(char const *p, char const *end);

/**
 * A word within a code-generation pattern. This will be replaced with the
 * the word's substitution in the output file.
 */
struct code_replace {
  String *symbol;       /* Which symbol the item should be replaced with */
  Code next;            /* The next item in the code-generation template, if any */
};
Code code_replace_new(String *symbol);

/**
 * A sub-pattern within a code-generation template. The system will process the
 * pattern recursively and write its output at the current location.
 */
struct code_match {
  Match *match;         /* The first element in the sub-pattern. */
  Code next;            /* The next item in the code-generation template, if any */
};
Code code_match_new(Match *match);

/**
 * A code-generation pattern
 */
struct match {
  Pattern  pattern;     /* The first item in the pattern */
  int      pattern_n;   /* Number of elements in the pattern list */
  Code     code;        /* The first item in the code-generation template */
  Match   *outer;       /* Enclosing match block, if this block is nested. */
  Match   *next;        /* The next pattern in this group, if any */
};
Match *match_new(Match *outer);
String *match_find_symbol(Match *match, String symbol);
int match_search(Match *match, Outline *outline, FileW *file);
int match_compare(Match *match, Outline *outline);
int match_generate(Match *match, Outline *outline, FileW *file);

/**
 * Builds a Match structure element-by-element.
 */
struct match_builder {
  Match   *match;       /* The match structure being built */
  Pattern pattern_last; /* The last pattern node in the linked list */
  Code    code_last;    /* The last code node in the linked list. */
};
int match_builder_init(MatchBuilder *b, Match *outer);
int match_builder_add_pattern_word(MatchBuilder *b, char const *p, char const *end);
int match_builder_add_pattern_replace(MatchBuilder *b, char const *p, char const *end);
int match_builder_add_code_code(MatchBuilder *b, char const *p, char const *end);
int match_builder_add_code_replace(MatchBuilder *b, String *symbol);
int match_builder_add_code_match(MatchBuilder *b, Match *match);

#endif
