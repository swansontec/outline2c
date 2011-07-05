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

String string(char const *p, char const *end)
{
  String self;
  self.p = p;
  self.end = end;
  return self;
}

#define string_null()    string(0, 0)
#define string_from_c(c) string((c), (c) + strlen(c))
#define string_from_k(k) string((k), (k) + sizeof(k) - 1)

#define string_size(self) ((self).end - (self).p)

/**
 * Copies the string. The new string is silently null-terminated, and can be
 * passed to C functions.
 */
String string_copy(Pool *pool, String self)
{
  size_t size = string_size(self);
  char *out = (char*)pool_alloc(pool, size + 1, 1);
  memcpy(out, self.p, size);
  out[size] = 0;
  return string(out, out + size);
}

/**
 * Concatenates one string onto another. The new string is silently null-
 * terminated, and can be passed to C functions.
 */
String string_cat(Pool *pool, String s1, String s2)
{
  size_t size = string_size(s1) + string_size(s2);
  char *out = (char*)pool_alloc(pool, size + 1, 1);
  memcpy(out, s1.p, string_size(s1));
  memcpy(out + string_size(s1), s2.p, string_size(s2));
  out[size] = 0;
  return string(out, out + size);
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
