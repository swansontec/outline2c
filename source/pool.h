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

#ifndef POOL_H_INCLUDED
#define POOL_H_INCLUDED

#include "typedefs.h"
#include <stdlib.h>

/**
 * A memory pool. The memory pool provides fast allocation, but does not offer
 * any way to free individual elements. Instead, the entire pool must be freed
 * at once.
 */
struct pool {
  char *block;  /* The current block. */
  char *next;   /* The next free location in the current block */
  char *end;    /* One-past the end of the current block */
};
int pool_init(Pool *p, size_t size);
void *pool_alloc(Pool *p, size_t size);
void *pool_alloca(Pool *p, size_t size, size_t align);
void pool_free(Pool *p);

#endif
