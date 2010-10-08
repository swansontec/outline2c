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

#ifndef LWL_H_INCLUDED
#define LWL_H_INCLUDED

#include "context.h"

typedef struct Keyword Keyword;

/**
 * Represents an LWL keyword implemented in C
 */
struct Keyword {
  int (*code)(Context *ctx, OutRoutine or);
};
Keyword *keyword_new(Pool *p, int (*code)(Context *ctx, OutRoutine or));

int lwl_parse_value(Context *ctx, OutRoutine or);
int lwl_parse_line(Context *ctx, OutRoutine or);

#endif
