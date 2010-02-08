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

#ifndef PARSER_H_INCLUDED
#define PARSER_H_INCLUDED

#include "typedefs.h"
typedef struct context Context;

#include "lexer.h"
#include "outline.h"
#include "string.h"
struct context
{
  /* Current files: */
  String file;
  String filename;
  FileW *out;

  /* Current scanner state: */
  Cursor cursor;
  Cursor marker;
  Token token;

  /* Outline: */
  OutlineBuilder root;
};
Context context_init(String file, String filename, FileW *out);

int parser_start(String aIn, String aFilename, FileW *aOut);

int parse_source_file(Context *ctx);
int parse_header_file(Context *ctx);
int parse_co2(Context *ctx);

int parse_outline(Context *ctx);
int parse_outline_node(Context *ctx, OutlineBuilder *b);

int parse_match_top(Context *ctx);
int parse_match(Context *ctx, Match **match, Match *outer);
int parse_match_entry(Context *ctx, MatchBuilder *b);

#endif
