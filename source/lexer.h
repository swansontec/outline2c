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

/**
 * Possible token types.
 */
enum token {
  LEX_ERROR_END = -100, /* Unexpected end-of-file */
  LEX_ERROR,            /* Unrecognized character sequence */

  LEX_START,            /* Start of file */
  LEX_END = 0,          /* Normal end-of-file */

  LEX_WHITESPACE,       /* Newlines, spaces, & tabs */
  LEX_COMMENT,          /* Both types of comment */
  LEX_STRING,           /* C-style double-quoted string */
  LEX_CHAR,             /* C-style single-quoted character */
  LEX_NUMBER,           /* C-style integer */
  LEX_IDENTIFIER,       /* [_a-zA-Z][_a-zA-Z0-9]* */
  LEX_ESCAPE,           /* @[_a-zA-Z0-9]+ */
  LEX_ESCAPE_O2C,       /* @o2c */
  LEX_PASTE,            /* \\ */

  LEX_BANG,             /* ! */
  LEX_AMP,              /* & */
  LEX_PAREN_L,          /* ( */
  LEX_PAREN_R,          /* ) */
  LEX_STAR,             /* * */
  LEX_DOT,              /* . */
  LEX_SLASH,            /* / */
  LEX_SEMICOLON,        /* ; */
  LEX_LT,               /* < */
  LEX_EQUALS,           /* = */
  LEX_GT,               /* > */
  LEX_BACKSLASH,        /* \ */
  LEX_BRACE_L,          /* { */
  LEX_PIPE,             /* | */
  LEX_BRACE_R           /* } */
};
typedef enum token Token;

Token lex(char const **p, char const *end);

#endif
