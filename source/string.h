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

#ifndef STRING_H_INCLUDED
#define STRING_H_INCLUDED

#include "typedefs.h"
#include <stdlib.h>

/**
 * A string without a null terminator. The structure consists of a pointer to
 * the beginning of the string, p, and a pointer one-past the end of the
 * string, end. This scheme makes it possible divide a longer string into
 * smaller pieces without making copies. It also makes it trivial to find a
 * string's length.
 */
struct string {
  char const *p;
  char const *end;
};
#define string_size(s) ((s).end - (s).p)
String string_init(char const *p, char const *end);
String string_init_l(char const *p, size_t size);
String string_null();
String string_from_c(char const *p);
char *string_to_c(String s);
int string_equal(String s1, String s2);
size_t string_match(String s1, String s2);
size_t string_rmatch(String s1, String s2);

#endif
