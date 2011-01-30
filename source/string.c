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
 * A string without a null terminator. The structure consists of a pointer to
 * the beginning of the string, p, and a pointer one-past the end of the
 * string, end. This scheme makes it possible divide a longer string into
 * smaller pieces without making copies. It also makes it trivial to find a
 * string's length.
 */
typedef struct {
  char const *p;
  char const *end;
} String;

#define string_size(s) ((s).end - (s).p)

String string_init(char const *p, char const *end)
{
  String s;
  s.p = p;
  s.end = end;
  return s;
}

String string_init_l(char const *p, size_t size)
{
  String s;
  s.p = p;
  s.end = p + size;
  return s;
}

String string_null()
{
  String s = {0, 0};
  return s;
}

String string_from_c(char const *p)
{
  String s;
  s.p = p;
  s.end = p + strlen(p);
  return s;
}

/**
 * Copies the string and null-terminates it, forming a C-style string. The
 * caller must free() the memory when it is done using it.
 */
char *string_to_c(String s)
{
  char *p = malloc(string_size(s) + 1);
  memcpy(p, s.p, s.end - s.p);
  p[s.end - s.p] = 0;
  return p;
}

/**
 * Tests two strings for equality. Returns a non-zero value if the strings
 * are the same.
 */
int string_equal(String s1, String s2)
{
  if (string_size(s1) != string_size(s2))
    return 0;
  return !memcmp(s1.p, s2.p, string_size(s1));
}

/**
 * Compares two strings starting from the beginning. Returns the number of
 * matching characters.
 */
size_t string_match(String s1, String s2)
{
  char const *p1 = s1.p;
  char const *p2 = s2.p;
  while (p1 < s1.end && p2 < s2.end && *p1 == *p2) {
    ++p1; ++p2;
  }
  return p1 - s1.p;
}

/**
 * Compares two strings starting from the end. Returns the number of matching
 * characters.
 */
size_t string_rmatch(String s1, String s2)
{
  char const *p1 = s1.end;
  char const *p2 = s2.end;
  while (s1.p < p1 && s2.p < p2 && p1[-1] == p2[-1]) {
    --p1; --p2;
  }
  return s1.end - p1;
}

/**
 * Copies a string
 */
String string_copy(Pool *pool, String string)
{
  size_t size;
  char *start;
  if (!string_size(string)) return string_null();

  size  = string_size(string);
  start = pool_aligned_alloc(pool, size, 1);
  if (!start) return string_null();
  memcpy(start, string.p, size);
  return string_init_l(start, size);
}

/**
 * Concatenates one string onto another
 */
String string_merge(Pool *pool, String a, String b)
{
  String s;
  char const *in;
  char *out;

  out = pool_aligned_alloc(pool, string_size(a) + string_size(b), 1);
  if (!out)
    return string_null();

  s = string_init_l(out, string_size(a) + string_size(b));

  for (in = a.p; in < a.end; ++in)
    *out++ = *in;
  for (in = b.p; in < b.end; ++in)
    *out++ = *in;

  return s;
}

/**
 * Opens a file, storing cleanup data and content pointers in the File
 * structure. The content pointers will be NULL if the function fails.
 * @return 0 for failure
 */
String string_load(Pool *pool, String filename)
{
  char *c_name = 0;
  FILE *fp = 0;
  long size;
  char *data;

  c_name = string_to_c(filename);
  if (!c_name) goto error;

  fp = fopen(c_name, "rb");
  if (!fp) goto error;

  if (fseek(fp, 0, SEEK_END))
    goto error;

  size = ftell(fp);
  if (size == -1L) goto error;

  if (fseek(fp, 0, SEEK_SET))
    goto error;

  data = pool_aligned_alloc(pool, size + 1, 1);
  if (!data) goto error;

  if (fread(data, 1, size, fp) != size)
    goto error;
  data[size] = 0;

  free(c_name);
  fclose(fp);
  return string_init(data, data + size);

error:
  if (c_name) free(c_name);
  if (fp)     fclose(fp);
  return string_null();
}
