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

/**
 * Possible token types
 */
typedef enum {
  LEX_ERROR_END = -100, /* Unexpected end-of-file */
  LEX_ERROR,            /* Unrecognized character sequence */

  LEX_END = 0,          /* Normal end-of-file */

  LEX_WHITESPACE,       /* Spaces, & tabs */
  LEX_NEWLINE,          /* Newlines */
  LEX_COMMENT,          /* Both types of comment */
  LEX_STRING,           /* C-style double-quoted string */
  LEX_CHAR,             /* C-style single-quoted character */
  LEX_NUMBER,           /* C-style integer */
  LEX_IDENTIFIER,       /* [_a-zA-Z][_a-zA-Z0-9]* */
  LEX_ESCAPE,           /* \ol */
  LEX_PASTE,            /* \\ */

  LEX_BANG,             /* ! */
  LEX_AMP,              /* & */
  LEX_PAREN_L,          /* ( */
  LEX_PAREN_R,          /* ) */
  LEX_STAR,             /* * */
  LEX_COMMA,            /* , */
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
} Token;

#define IS_SPACE(c) ( \
  c == ' ' || c == '\t')

#define IS_NEWLINE(c) ( \
  c == '\n' || c == '\f' || c == '\r')

#define IS_ALPHA(c) ( \
  ('a' <= c && c <= 'z') || \
  ('A' <= c && c <= 'Z') || c == '_')

#define IS_ALPHANUM(c) ( \
  ('0' <= c && c <= '9') || \
  ('a' <= c && c <= 'z') || \
  ('A' <= c && c <= 'Z') || c == '_')

/**
 * Identifies the next token in the input stream. When the function starts, the
 * cursor points to the beginning of the token to identify. When the function
 * returns, the cursor points to the beginning of the next token, one character
 * past the end of the current token. The return value is an enum indicating
 * the identified token's type. Since this function also identifies non-syntax
 * elements such as whitespace and comments, additional filtering may be useful.
 */
Token lex(char const **p, char const *end)
{
  if (end <= *p) {
    return LEX_END;
  /* Whitespace: */
  } else if (IS_SPACE(**p)) {
    do {
      ++*p;
    } while (*p < end && IS_SPACE(**p));
    return LEX_WHITESPACE;
  /* Newline: */
  } else if (**p == '\n' || **p == '\f') {
    ++*p;
    return LEX_NEWLINE;
  } else if (**p == '\r') {
    ++*p;
    if (*p < end && **p == '\n')
      ++*p;
    return LEX_NEWLINE;
  /* Comments: */
  } else if (**p == '/') {
    ++*p;
    if (end <= *p) return LEX_SLASH;
    /* C++ style comments: */
    if (**p == '/') {
      do {
        ++*p;
      } while (*p < end && !IS_NEWLINE(**p));
      return LEX_COMMENT;
    /* C style comments: */
    } else if (**p == '*') {
      do {
        do {
          ++*p;
          if (end <= *p) return LEX_ERROR_END;
        } while (**p != '*');
        do {
          ++*p;
          if (end <= *p) return LEX_ERROR_END;
        } while (**p == '*');
      } while (**p != '/');
      ++*p;
      return LEX_COMMENT;
    /* Lone slash: */
    } else {
      return LEX_SLASH;
    }
  /* Double-quoted string literal: */
  } else if (**p == '\"') {
    do {
      ++*p;
      if (end <= *p) return LEX_ERROR_END;
      if (**p == '\\') {
        ++*p;
        if (end <= *p) return LEX_ERROR_END;
        ++*p;
        if (end <= *p) return LEX_ERROR_END;
      }
    } while (**p != '\"');
    ++*p;
    return LEX_STRING;
  /* Single-quoted character literal: */
  } else if (**p == '\'') {
    do {
      ++*p;
      if (end <= *p) return LEX_ERROR_END;
      if (**p == '\\') {
        ++*p;
        if (end <= *p) return LEX_ERROR_END;
        ++*p;
        if (end <= *p) return LEX_ERROR_END;
      }
    } while (**p != '\'');
    ++*p;
    return LEX_CHAR;
  /* Numeric literals (including malformed ones, which are ignored): */
  } else if ('0' <= **p && **p <= '9') {
    do {
      ++*p;
    } while (*p < end && IS_ALPHANUM(**p));
    return LEX_NUMBER;
  /* Identifiers: */
  } else if (IS_ALPHA(**p)) {
    do {
      ++*p;
    } while (*p < end && IS_ALPHANUM(**p));
    return LEX_IDENTIFIER;
  /* Token-pasting: */
  } else if (**p == '\\') {
    ++*p;
    if (end <= *p) return LEX_BACKSLASH;
    if (**p == '\\') {
      ++*p;
      return LEX_PASTE;
    } else if (**p == 'o') {
      ++*p;
      if (*p < end && **p == 'l') {
        ++*p;
        return LEX_ESCAPE;
      } else {
        --*p;
        return LEX_BACKSLASH;
      }
    }
    return LEX_BACKSLASH;
  /* Symbols: */
  } else if (**p == '!') { ++*p; return LEX_BANG;
  } else if (**p == '&') { ++*p; return LEX_AMP;
  } else if (**p == '(') { ++*p; return LEX_PAREN_L;
  } else if (**p == ')') { ++*p; return LEX_PAREN_R;
  } else if (**p == '*') { ++*p; return LEX_STAR;
  } else if (**p == ',') { ++*p; return LEX_COMMA;
  } else if (**p == '.') { ++*p; return LEX_DOT;
  /* LEX_SLASH was recognized earlier. */
  } else if (**p == ';') { ++*p; return LEX_SEMICOLON;
  } else if (**p == '<') { ++*p; return LEX_LT;
  } else if (**p == '=') { ++*p; return LEX_EQUALS;
  } else if (**p == '>') { ++*p; return LEX_GT;
  /* LEX_BACKSLASH was recognized earlier. */
  } else if (**p == '{') { ++*p; return LEX_BRACE_L;
  } else if (**p == '|') { ++*p; return LEX_PIPE;
  } else if (**p == '}') { ++*p; return LEX_BRACE_R;
  /* Any other character: */
  } else {
    ++*p;
    return LEX_ERROR;
  }
}

/**
 * Identifies the next token, filtering out whitespace & comments.
 */
Token lex_next(char const **start, char const **p, char const *end)
{
  Token token;
  do {
    *start = *p; token = lex(p, end);
  } while (
    token == LEX_WHITESPACE ||
    token == LEX_NEWLINE ||
    token == LEX_COMMENT);
  return token;
}

/**
 * Determines the extent of a block. This function looks for matching braces,
 * ignoring braces inside quotes, comments and so forth. Returns an all-null
 * Source structure response to an error.
 */
Source lex_block(Source *in)
{
  char const *start;
  Token token;
  Source out = *in;
  Source null = {{0}};
  int depth = 1;

  /* Find the opening brace: */
  token = lex_next(&start, &in->cursor, in->data.end);
  if (token != LEX_BRACE_L)
    return null;
  out.cursor = in->cursor;

  /* Find the ending brace: */
  do {
    start = in->cursor; token = lex(&in->cursor, in->data.end);
    if (token == LEX_BRACE_L) ++depth;
    if (token == LEX_BRACE_R) --depth;
  } while (token != LEX_END && depth);
  if (token == LEX_END)
    return null;
  out.data.end = start;
  return out;
}
