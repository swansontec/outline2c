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

#include "pool.h"
#include <stddef.h>

/* If the platform isn't on the whitelist, fall back on the safe alignment
calculation: */
#if !defined(WIN32)
#define POOL_ALIGN_SAFE
#endif

/* Discern the platform's alignment requirements: */
struct align_test {
  char x;
  union {
    void *vp; Pool *pp;
    char c; short s; int i; long l;
    float f; double d; long double ld;
  } data;
};
#define POOL_ALIGN offsetof(struct align_test, data)

/**
 * Represents a single block of data within the pool.
 */
struct pool_block {
  char *prev;       /* A link to the previous (full) block in the pool. */
  char space[1];    /* The data storage area */
};
typedef struct pool_block PoolBlock;

/**
 * Helper function to add new blocks to the pool. This function will not modify
 * the pool if it fails.
 * @return 0 for success
 */
static int pool_grow(Pool *p, size_t size)
{
  char *block = malloc(size);
  if (!block) return 1;

  ((PoolBlock*)block)->prev = p->block;
  p->block = block;
  p->next = ((PoolBlock*)block)->space;
  p->end = block + size;
  return 0;
}

/**
 * Initializes a memory pool, allocating an initial block of the given size.
 * Each future block will have the same size as the initial block, so choose
 * wisely.
 * @return 0 for success
 */
int pool_init(Pool *p, size_t size)
{
  p->block = 0;
  p->next = 0;
  p->end = 0;
  return pool_grow(p, size);
}

/**
 * Allocates memory from the pool.
 * @return address of the requested memory, or 0 for failure,
 */
void *pool_alloc(Pool *p, size_t size)
{
  return pool_alloca(p, size, POOL_ALIGN);
}

/**
 * Allocates memory from the pool. The align parameter specifies the alignment
 * requirements for the data.
 * @return address of the requested memory, or 0 for failure,
 */
void *pool_alloca(Pool *p, size_t size, size_t align)
{
#ifdef POOL_ALIGN_SAFE
  /* Find a pointer with the required alignment. This works because malloc
  returns pointers meeting the platform's worst-case alignment requirements.
  Basing the allignment calculation on the block's base address, which comes
  from malloc, will either meet the requested alignment or fail in a way that
  can't possibly matter, since malloc already covers the worst case. This is
  the only portable way to do the alignment I can think of. */
  char *next = p->block + (p->next - p->block + (align-1) & ~(align-1));
#else
  /* This slightly-faster alignment routine relies on undefined behavior, but
  it should work fine on most real-life platforms. */
  char *next = (char*)0 + (p->next - (char*)0 + (align-1) & ~(align-1));
#endif
  char *end = next + size;

  /* Grow the block if needed: */
  if (p->end < end)
    if (pool_grow(p, p->end - p->block))
      return 0;

  p->next = end;
  return next;
}

/**
 * Frees all memory in the pool. The pool becomes un-initialized after this
 * call, requiring another call to pool_init.
 */
void pool_free(Pool *p)
{
  char *block = p->block;
  while (block) {
    char *temp = ((PoolBlock*)block)->prev;
    free(block);
    block = temp;
  }
}
