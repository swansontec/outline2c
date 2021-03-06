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
  type_none = 0,

  type_keyword,

  type_lookup,
  type_macro,
  type_macro_call,
  type_filter_tag,
  type_filter_any,
  type_filter_not,
  type_filter_and,
  type_filter_or,
  type_outline_tag,
  type_outline_item,
  type_outline,
  type_map_line,
  type_map,
  type_for,
  type_code_text
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

#define dynamic_none() \
  (dynamic(0, 0))

#define dynamic_ok(self) \
  ((self).type != type_none)
