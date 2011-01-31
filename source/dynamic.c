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

typedef enum {
  TYPE_KEYWORD,

  AST_VARIABLE,
  AST_LOOKUP,
  AST_MACRO,
  AST_MACRO_CALL,
  AST_FILTER_TAG,
  AST_FILTER_ANY,
  AST_FILTER_NOT,
  AST_FILTER_AND,
  AST_FILTER_OR,
  AST_OUTLINE_TAG,
  AST_OUTLINE_ITEM,
  AST_OUTLINE,
  AST_MAP_LINE,
  AST_MAP,
  AST_FOR,
  AST_CODE_TEXT,

  TYPE_END
} Type;

typedef struct {
  void *p;
  Type type;
} Dynamic;

Dynamic dynamic(Type type, void *p)
{
  Dynamic self;
  self.p = p;
  self.type = type;
  return self;
}
