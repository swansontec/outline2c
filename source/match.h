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
#include "ast.h"

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
 * Links code items together into a list.
 */
struct code_builder {
  Code first;
  Code last;
};
typedef struct code_builder CodeBuilder;
void code_builder_init(CodeBuilder *b);
int code_builder_add_code(CodeBuilder *b, char const *p, char const *end);
int code_builder_add_replace(CodeBuilder *b, String *symbol);
int code_builder_add_match(CodeBuilder *b, Match *match);

/**
 * A code-generation pattern
 */
struct match {
  AstPattern *pattern;  /* The search pattern */
  Code     code;        /* The first item in the code-generation template */
  Match   *outer;       /* Enclosing match block, if this block is nested. */
  Match   *next;        /* The next pattern in this group, if any */
};
Match *match_new(Match *outer);
String *match_find_symbol(Match *match, String symbol);
int match_search(Match *match, Outline *outline, FileW *file);
int match_generate(Match *match, Outline *outline, FileW *file);

#endif
