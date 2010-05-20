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

#include "string.h"
#include <stddef.h>

typedef struct pool Pool;

/**
 * A memory pool. The memory pool provides efficient allocation, but does not
 * offer any way to free individual elements. Instead, the entire pool must be
 * freed at once.
 *
 * The memory pool works by allocating a large block of memory up-front. Then,
 * each time it recieves an allocation request, it returns a portion of the
 * block. When the block becomes full, the pool allocates another block and
 * continues in the same way. The only record-keeping is a pointer to the
 * remaining free space in the current block, plus a list of used blocks for
 * later freeing.
 *
 * This strategy works well when the requested objects are much smaller than
 * the block size. Attempting to allocate objects larger than the block size
 * will always fail, and attempting to allocate objects near the block size
 * can waste large amounts of memory. The pool_alloc function deals with this
 * problem by falling back on malloc for certain large requests. To force a
 * request to come from malloc rather than the the pool, use the pool_alloc_sys
 * function. The memory will be freed when the pool itself is freed, as usual.
 *
 * To determine how much free space is available in the current block, use the
 * pool_unused function.
 *
 * Most pool_ functions have an pool_aligned_ alternative. These alternatives
 * take an explicit alignment parameter. The alignment must be a power of two,
 * and should be smaller than malloc's native allignment. If the allignment is
 * larger, malloc's native alignment will be used instead and a small amount of
 * memory might be wasted.
 *
 * The other functions use a default alignment suitable for any normal C data
 * type.
 */
struct pool {
  char *block;  /* The current block. */
  char *next;   /* The next free location in the current block */
  char *end;    /* One-past the end of the current block */
};

int     pool_init       (Pool *p, size_t size);
void    pool_free       (Pool *p);

void   *pool_alloc      (Pool *p, size_t size);
void   *pool_alloc_sys  (Pool *p, size_t size);
size_t  pool_unused     (Pool *p);

void   *pool_aligned_alloc      (Pool *p, size_t size, size_t align);
void   *pool_aligned_alloc_sys  (Pool *p, size_t size, size_t align);
size_t  pool_aligned_unused     (Pool *p, size_t align);

String  pool_string_copy        (Pool *p, String string);

#endif
