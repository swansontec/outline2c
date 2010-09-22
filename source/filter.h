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

#ifndef FILTER_H_INCLUDED
#define FILTER_H_INCLUDED

#include "ast.h"

typedef struct FilterBuilder FilterBuilder;

int test_filter(AstFilter *test, AstOutlineItem *item);

/**
 * A stack for building filters using Dijkstra's shunting-yard algorithm
 */
struct FilterBuilder {
  Dynamic *stack;
  size_t stack_size;
  size_t stack_top;
};

int filter_builder_init(FilterBuilder *b);
void filter_builder_free(FilterBuilder *b);
Dynamic filter_builder_pop(FilterBuilder *b);

int filter_build_tag(FilterBuilder *b, Pool *pool, String tag);
int filter_build_any(FilterBuilder *b, Pool *pool);
int filter_build_not(FilterBuilder *b, Pool *pool);
int filter_build_and(FilterBuilder *b, Pool *pool);
int filter_build_or(FilterBuilder *b, Pool *pool);

#endif
