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
#include "string.h"
#include <stdlib.h>
#include <string.h>

/* Choose an alignment technique based on the platform: */
#if defined(WIN32)
/**
 * This slightly-faster alignment routine relies on undefined behavior, but
 * it should work fine on most real-life platforms. The base parameter is not
 * used in this version of the macro.
 */
#define ALIGN(p, base, align) (char*)((uintptr_t)p + (align-1) & ~(align-1))
#else
/**
 * Aligns a pointer in a completely-portable way. This works because malloc
 * returns pointers meeting the platform's worst-case alignment requirements.
 * Basing the allignment calculation on the block's base address, which comes
 * from malloc, will either meet the requested alignment or fail in a way that
 * can't possibly matter, since malloc already covers the worst case.
 */
#define ALIGN(p, base, align) base + (p - base + (align-1) & ~(align-1))
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
#define DEFAULT_ALIGN offsetof(struct align_test, data)

/**
 * Helper function to add new blocks to the pool. This function does not modify
 * the pool if it fails.
 * @return 0 for failure
 */
static int pool_grow(Pool *p, size_t size)
{
  char *block = malloc(size);
  if (!block) return 0;

  /* The first element in each block is a pointer to previous block: */
  *(char**)block = p->block;

  /* Place the new block in the pool: */
  p->block = block;
  p->next  = block + sizeof(char*);
  p->end   = block + size;
  return 1;
}

/**
 * Initializes a memory pool, allocating an initial block of the given size.
 * Each future block will have the same size as the initial block, so choose
 * wisely.
 * @return 0 for failure
 */
int pool_init(Pool *p, size_t size)
{
  p->block = 0;
  p->next = 0;
  p->end = 0;
  return pool_grow(p, size);
}

/**
 * Frees all memory in the pool. The pool becomes un-initialized after this
 * call, requiring another call to pool_init.
 */
void pool_free(Pool *p)
{
  char *block = p->block;
  while (block) {
    char *temp = *(char**)block;
    free(block);
    block = temp;
  }
}

/**
 * Allocates memory from the pool.
 * @return address of the requested memory, or 0 for failure,
 */
void *pool_alloc(Pool *p, size_t size)
{
  return pool_aligned_alloc(p, size, DEFAULT_ALIGN);
}

/**
 * Allocates memory using malloc and adds the result to the pool's list of
 * blocks to free.
 * @return address of the requested memory, or 0 for failure,
 */
void *pool_alloc_sys(Pool *p, size_t size)
{
  return pool_aligned_alloc_sys(p, size, DEFAULT_ALIGN);
}

/**
 * Returns the remaining free space in the current block.
 * @return address of the requested memory, or 0 for failure,
 */
size_t pool_unused(Pool *p)
{
  return pool_aligned_unused(p, DEFAULT_ALIGN);
}

/**
 * A variant of pool_alloc taking an explicit alignment parameter.
 */
void *pool_aligned_alloc(Pool *p, size_t size, size_t align)
{
  char *start = ALIGN(p->next, p->block, align);
  char *end = start + size;

  /* Is the request too large? The 2nd test checks for overflow. */
  if (p->end < end || !start) {
    size_t block_size = p->end - p->block;
    /* Use malloc for large blocks: */
    if (block_size < 64*size)
      return pool_aligned_alloc_sys(p, size, align);

    /* Grow the pool: */
    if (pool_grow(p, block_size)) return 0;
    start = ALIGN(p->next, p->block, align);
    end = start + size;
  }

  p->next = end;
  return start;
}

/**
 * A variant of pool_alloc_sys taking an explicit alignment parameter.
 */
void *pool_aligned_alloc_sys(Pool *p, size_t size, size_t align)
{
  size_t padding = sizeof(char*) < align ? align : sizeof(char*);
  char *block = malloc(padding + size);
  if (!block) return 0;

  /* Add the block to the list of stuff to free: */
  *(char**)block = *(char**)p->block;
  *(char**)p->block = block;

  return block + padding;
}

/**
 * A variant of pool_unused taking an explicit alignment parameter.
 */
size_t pool_aligned_unused(Pool *p, size_t align)
{
  char *start = ALIGN(p->next, p->block, align);
  if (p->end < start || !start)
    return 0;
  return p->end - start;
}

/**
 * Copies a string.
 */
String pool_string_copy(Pool *p, String string)
{
  size_t size;
  char *start;
  if (!string_size(string)) return string_null();

  size  = string_size(string);
  start = pool_aligned_alloc(p, size, 1);
  if (!start) return string_null();
  memcpy(start, string.p, size);
  return string_init_l(start, size);
}
