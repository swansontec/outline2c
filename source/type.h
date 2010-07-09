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
 * The compiler contains a dynamic type system. Some types of AST fragments
 * are interchangeable, and there is no way to know ahead of time which type
 * the user will choose to use where. This header enumerates all the valid
 * types within the system and provides a typed-pointer structure.
 */

#ifndef TYPE_H_INCLUDED
#define TYPE_H_INCLUDED

enum Type {
  TYPE_KEYWORD,

  AST_CODE,
  AST_CODE_TEXT,
  AST_OUTLINE,
  AST_OUTLINE_ITEM,
  AST_OUTLINE_TAG,
  AST_MAP,
  AST_MAP_LINE,
  AST_FOR,
  AST_FILTER,
  AST_FILTER_TAG,
  AST_FILTER_ANY,
  AST_FILTER_NOT,
  AST_FILTER_AND,
  AST_FILTER_OR,
  AST_MACRO,
  AST_MACRO_CALL,
  AST_VARIABLE,
  AST_LOOKUP,

  TYPE_END
};
typedef enum Type Type;

struct Dynamic {
  void *p;
  Type type;
};
typedef struct Dynamic Dynamic;

/**
 * All functions return 1 for success and 0 for failure. This macro checks a
 * return code and bails out if it indicates an error.
 */
#define CHECK(r) do { if (!(r)) return 0; } while(0)

/**
 * Verifies that a memory-allocating call succeeds, and prints an error message
 * otherwise.
 */
#define CHECK_MEM(r) \
  do { if (!(r)) { \
    fprintf(stderr, "error: Out of memory at %s:%d\n", __FILE__, __LINE__); \
    return 0; \
  } } while(0)

#endif
