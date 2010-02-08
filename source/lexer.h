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

#ifndef LEXER_H_INCLUDED
#define LEXER_H_INCLUDED

typedef struct cursor Cursor;

/**
 * A reference to a location within the input stream.
 */
struct cursor
{
  char const *p;
  unsigned line;
  unsigned column;
};
Cursor cursor_init(char const *start);

/**
 * Possible token types.
 */
enum token {
  LEX_ERROR_END = -100, /* Unexpected end-of-file */
  LEX_ERROR,            /* Unrecognized character sequence */

  LEX_END = 0,          /* Normal end-of-file */

  LEX_WHITESPACE,       /* Newlines, spaces, & tabs */
  LEX_COMMENT,          /* Both types of comment */
  LEX_STRING,           /* C-style double-quoted string */
  LEX_CHAR,             /* C-style single-quoted character */
  LEX_NUMBER,           /* C-style integer */
  LEX_IDENTIFIER,       /* [_a-zA-Z][_a-zA-Z0-9]* */
  LEX_ESCAPE,           /* @[_a-zA-Z0-9]+ */
  LEX_ESCAPE_CO2,       /* @co2 */

  LEX_SLASH,            /* / */
  LEX_SEMICOLON,        /* ; */
  LEX_LESS,             /* < */
  LEX_GREATER,          /* > */
  LEX_PIPE,             /* | */
  LEX_BRACE_OPEN,       /* { */
  LEX_BRACE_CLOSE       /* } */
};
typedef enum token Token;

Token lex(Cursor *cursor, char const *end);

#endif
