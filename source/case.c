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

/* Writes bytes to a file, and returns 0 for failure. */
#define file_write(file, p, end) (fwrite(p, 1, end - p, file) == end - p)
#define file_putc(file, c) (fputc(c, file) != EOF)

/**
 * Removes the leading and trailing underscores from an identifier.
 */
String strip_symbol(String s)
{
  while (s.p < s.end && *s.p == '_')
    ++s.p;
  while (s.p < s.end && s.end[-1] == '_')
    --s.end;
  return s;
}

/**
 * Locates individual words within an indentifier. The identifier must have its
 * leading and trailing underscores stripped off before being passed to this
 * function. As always, the only valid symbols within an indentifier are
 * [_a-zA-Z0-9]
 * @param s the input string to break into words
 * @param p a pointer into string s, which marks the first character to begin
 * scanning at.
 * @return the next located word, or a null string upon reaching the end of the
 * input
 */
String scan_symbol(String s, char const *p)
{
  char const *start;

  /* Trim underscores between words: */
  while (p < s.end && *p == '_')
    ++p;
  if (p == s.end)
    return string_null();
  start = p;

  /* Numbers? */
  if ('0' <= *p && *p <= '9') {
    do {
      ++p;
    } while (p < s.end && '0' <= *p && *p <= '9');
    return string_init(start, p);
  }

  /* Lower-case letters? */
  if ('a' <= *p && *p <= 'z') {
    do {
      ++p;
    } while (p < s.end && 'a' <= *p && *p <= 'z');
    return string_init(start, p);
  }

  /* Upper-case letters? */
  if ('A' <= *p && *p <= 'Z') {
    do {
      ++p;
    } while (p < s.end && 'A' <= *p && *p <= 'Z');
    /* Did the last upper-case letter start a lower-case word? */
    if (p < s.end && 'a' <= *p && *p <= 'z') {
      --p;
      if (p == start) {
        do {
          ++p;
        } while (p < s.end && 'a' <= *p && *p <= 'z');
      }
    }
    return string_init(start, p);
  }

  /* Anything else is a bug */
  assert(0);
  return string_null();
}

/**
 * Writes leading underscores to a file, if any.
 * @param s the entire string, including leading and trailing underscores.
 * @param inner the inner portion of the string after underscores have been
 * stripped.
 * @return 0 for failure
 */
int write_leading(FILE *out, String s, String inner)
{
  if (s.p != inner.p)
    return file_write(out, s.p, inner.p);
  return 1;
}

int write_trailing(FILE *out, String s, String inner)
{
  if (inner.end != s.end)
    return file_write(out, inner.end, s.end);
  return 1;
}

/**
 * Writes a word to a file in lower case.
 */
int write_lower(FILE *out, String s)
{
  char const *p;
  for (p = s.p; p != s.end; ++p) {
    char c = 'A' <= *p && *p <= 'Z' ? *p - 'A' + 'a' : *p;
    CHECK(file_putc(out, c));
  }
  return 1;
}

/**
 * Writes a word to a file in UPPER case.
 */
int write_upper(FILE *out, String s)
{
  char const *p;
  for (p = s.p; p != s.end; ++p) {
    char c = 'a' <= *p && *p <= 'z' ? *p - 'a' + 'A' : *p;
    CHECK(file_putc(out, c));
  }
  return 1;
}

/**
 * Writes a word to a file in Capitalized case.
 */
int write_cap(FILE *out, String s)
{
  char const *p;
  for (p = s.p; p != s.end; ++p) {
    char c = (p == s.p) ?
      ('a' <= *p && *p <= 'z' ? *p - 'a' + 'A' : *p) :
      ('A' <= *p && *p <= 'Z' ? *p - 'A' + 'a' : *p) ;
    CHECK(file_putc(out, c));
  }
  return 1;
}

/**
 * Writes a string to the output file, converting it to lower_case
 */
int generate_lower(FILE *out, String s)
{
  String inner = strip_symbol(s);
  String word = scan_symbol(inner, inner.p);

  write_leading(out, s, inner);
  while (string_size(word)) {
    write_lower(out, word);
    word = scan_symbol(inner, word.end);
    if (string_size(word))
      CHECK(file_putc(out, '_'));
  }
  write_trailing(out, s, inner);

  return 1;
}

/**
 * Writes a string to the output file, converting it to UPPER_CASE
 */
int generate_upper(FILE *out, String s)
{
  String inner = strip_symbol(s);
  String word = scan_symbol(inner, inner.p);

  write_leading(out, s, inner);
  while (string_size(word)) {
    write_upper(out, word);
    word = scan_symbol(inner, word.end);
    if (string_size(word))
      CHECK(file_putc(out, '_'));
  }
  write_trailing(out, s, inner);

  return 1;
}

/**
 * Writes a string to the output file, converting it to CamelCase
 */
int generate_camel(FILE *out, String s)
{
  String inner = strip_symbol(s);
  String word = scan_symbol(inner, inner.p);

  write_leading(out, s, inner);
  while (string_size(word)) {
    write_cap(out, word);
    word = scan_symbol(inner, word.end);
  }
  write_trailing(out, s, inner);

  return 1;
}

/**
 * Writes a string to the output file, converting it to mixedCase
 */
int generate_mixed(FILE *out, String s)
{
  String inner = strip_symbol(s);
  String word = scan_symbol(inner, inner.p);

  write_leading(out, s, inner);
  if (string_size(word)) {
    write_lower(out, word);
    word = scan_symbol(inner, word.end);
  }
  while (string_size(word)) {
    write_cap(out, word);
    word = scan_symbol(inner, word.end);
  }
  write_trailing(out, s, inner);

  return 1;
}
