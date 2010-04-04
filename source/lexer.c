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
/*
 * This is a hand-written lexer. The lexer operates on a cursor structure,
 * which includes a pointer into the input as well as line number and column
 * information for the current location. The lexer keeps this information up-
 * to-date as it scans through the input.
 */

#include "lexer.h"
#include "string.h"

/**
 * Prepares a fresh cursor, starting at the beginning of a file.
 */
Cursor cursor_init(char const *start)
{
  Cursor cursor;
  cursor.p = start;
  cursor.line = 0;
  cursor.column = 0;
  return cursor;
}

/**
 * Advances the cursor to the next character, updating the line and column
 * information. Returns 0 if the cursor reaches the end marker.
 */
static int advance(Cursor *cursor, char const *end)
{
  ++cursor->p;
  if (end <= cursor->p)
    return 0;
  if (*cursor->p == '\n') {
    cursor->column = 0;
    ++cursor->line;
  } else {
    ++cursor->column;
  }
  return 1;
}

#define IS_SPACE(c) \
  c == ' '  || c == '\t' || \
  c == '\r' || c == '\n'

#define IS_ALPHA(c) \
  'a' <= c && c <= 'z' || \
  'A' <= c && c <= 'Z' || c == '_'

#define IS_ALPHANUM(c) \
  '0' <= c && c <= '9' || \
  'a' <= c && c <= 'z' || \
  'A' <= c && c <= 'Z' || c == '_'

/**
 * Identifies the next token in the input stream. When the fuction starts, the
 * cursor points to the beginning of the token to identify. When the function
 * returns, the cursor points to the beginning of the next token, one character
 * past the end of the current token. The return value is an enum indicating
 * the identified token's type. Since this function also identifies non-syntax
 * elements such as whitespace and comments, additional filtering may be useful.
 */
Token lex(Cursor *cursor, char const *end)
{
  if (end <= cursor->p) {
    return LEX_END;
  /* Whitespace: */
  } else if (IS_SPACE(*cursor->p)) {
    do {
      if (!advance(cursor, end)) break;
    } while (IS_SPACE(*cursor->p));
    return LEX_WHITESPACE;
  /* Comments: */
  } else if (*cursor->p == '/') {
    if (!advance(cursor, end)) return LEX_SLASH;
    /* C++ style comments: */
    if (*cursor->p == '/') {
      do {
        if (!advance(cursor, end)) return LEX_COMMENT;
      } while (*cursor->p != '\n');
      advance(cursor, end);
      return LEX_COMMENT;
    /* C style comments: */
    } else if (*cursor->p == '*') {
      do {
        do {
          if (!advance(cursor, end)) return LEX_ERROR_END;
        } while (*cursor->p != '*');
        do {
          if (!advance(cursor, end)) return LEX_ERROR_END;
        } while (*cursor->p == '*');
      } while (*cursor->p != '/');
      advance(cursor, end);
      return LEX_COMMENT;
    /* Lone slash: */
    } else {
      return LEX_SLASH;
    }
  /* Double-quoted string literal: */
  } else if (*cursor->p == '\"') {
    do {
      if (!advance(cursor, end)) return LEX_ERROR_END;
      if (*cursor->p == '\\') {
        if (!advance(cursor, end)) return LEX_ERROR_END;
        if (!advance(cursor, end)) return LEX_ERROR_END;
      }
    } while (*cursor->p != '\"');
    advance(cursor, end);
    return LEX_STRING;
  /* Single-quoted character literal: */
  } else if (*cursor->p == '\'') {
    do {
      if (!advance(cursor, end)) return LEX_ERROR_END;
      if (*cursor->p == '\\') {
        if (!advance(cursor, end)) return LEX_ERROR_END;
        if (!advance(cursor, end)) return LEX_ERROR_END;
      }
    } while (*cursor->p != '\'');
    advance(cursor, end);
    return LEX_CHAR;
  /* Numeric literals (including malformed ones, which are ignored): */
  } else if ('0' <= *cursor->p && *cursor->p <= '9') {
    do {
      if (!advance(cursor, end)) break;
    } while (IS_ALPHANUM(*cursor->p));
    return LEX_NUMBER;
  /* Identifiers: */
  } else if (IS_ALPHA(*cursor->p)) {
    do {
      if (!advance(cursor, end)) break;
    } while (IS_ALPHANUM(*cursor->p));
    return LEX_IDENTIFIER;
  /* Escape codes: */
  } else if (*cursor->p == '@') {
    String token;
    token.p = cursor->p;
    do {
      if (!advance(cursor, end)) break;
    } while (IS_ALPHANUM(*cursor->p));
    token.end = cursor->p;
    if (string_equal(token, string_init_l("@o2c", 4)))
      return LEX_ESCAPE_O2C;
    return LEX_ESCAPE;
  /* Token-pasting: */
  } else if (*cursor->p == '\\') {
    if (!advance(cursor, end)) return LEX_BACKSLASH;
    if (*cursor->p == '\\') {
      advance(cursor, end);
      return LEX_PASTE;
    } else if (*cursor->p == 'o') {
      if (!advance(cursor, end)) return LEX_ESCAPE;
      if (*cursor->p == 'l') {
        advance(cursor, end);
        return LEX_ESCAPE_O2C;
      }
      return LEX_ESCAPE;
    }
    return LEX_BACKSLASH;
  /* Symbols: */
  } else if (*cursor->p == '!') { advance(cursor, end); return LEX_BANG;
  } else if (*cursor->p == '&') { advance(cursor, end); return LEX_AMP;
  } else if (*cursor->p == '(') { advance(cursor, end); return LEX_PAREN_L;
  } else if (*cursor->p == ')') { advance(cursor, end); return LEX_PAREN_R;
  } else if (*cursor->p == '*') { advance(cursor, end); return LEX_STAR;
  } else if (*cursor->p == '.') { advance(cursor, end); return LEX_DOT;
  /* LEX_SLASH was recognized earlier. */
  } else if (*cursor->p == ';') { advance(cursor, end); return LEX_SEMICOLON;
  } else if (*cursor->p == '<') { advance(cursor, end); return LEX_LT;
  } else if (*cursor->p == '=') { advance(cursor, end); return LEX_EQUALS;
  } else if (*cursor->p == '>') { advance(cursor, end); return LEX_GT;
  /* LEX_BACKSLASH was recognized earlier. */
  } else if (*cursor->p == '{') { advance(cursor, end); return LEX_BRACE_L;
  } else if (*cursor->p == '|') { advance(cursor, end); return LEX_PIPE;
  } else if (*cursor->p == '}') { advance(cursor, end); return LEX_BRACE_R;
  /* Any other character: */
  } else {
    advance(cursor, end);
    return LEX_ERROR;
  }
}
