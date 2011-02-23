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
typedef struct {
  char *block;  /* The current block. */
  char *next;   /* The next free location in the current block */
  char *end;    /* One-past the end of the current block */
} Pool;

/* Choose an alignment technique based on the platform: */
#if defined(WIN32)
/**
 * This slightly-faster alignment routine relies on undefined behavior, but
 * it should work fine on most real-life platforms. The base parameter is not
 * used in this version of the macro.
 */
#define ALIGN(p, base, align) ((char*)(((uintptr_t)(p) + ((align)-1)) & ~((align)-1)))
#else
/**
 * Aligns a pointer in a completely-portable way. This works because malloc
 * returns pointers meeting the platform's worst-case alignment requirements.
 * Basing the allignment calculation on the block's base address, which comes
 * from malloc, will either meet the requested alignment or fail in a way that
 * can't possibly matter, since malloc already covers the worst case.
 */
#define ALIGN(p, base, align) ((base) + (((p) - (base) + ((align)-1)) & ~((align)-1)))
#endif

/* Discern the platform's alignment requirements: */
typedef struct {
  char x;
  union {
    void *vp; Pool *pp;
    char c; short s; int i; long l;
    float f; double d; long double ld;
  } data;
} AlignTest;
#define DEFAULT_ALIGN offsetof(AlignTest, data)

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
 * Allocates memory using malloc and adds the result to the pool's list of
 * blocks to free.
 */
void *pool_alloc_sys(Pool *p, size_t size, size_t align)
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
 * Allocates memory from the pool.
 */
void *pool_alloc(Pool *p, size_t size, size_t align)
{
  char *start = ALIGN(p->next, p->block, align);
  char *end = start + size;

  /* Is the request too large? The 2nd test checks for overflow. */
  if (p->end < end || !start) {
    size_t block_size = p->end - p->block;
    /* Use malloc for large blocks: */
    if (block_size < 64*size)
      return pool_alloc_sys(p, size, align);

    /* Grow the pool: */
    if (!pool_grow(p, block_size)) return 0;
    start = ALIGN(p->next, p->block, align);
    end = start + size;
  }

  p->next = end;
  return start;
}

#define pool_new(pool, type) \
  ((type*)pool_alloc(pool, sizeof(type), DEFAULT_ALIGN))

/**
 * Returns the remaining free space in the current block.
 */
size_t pool_unused(Pool *p, size_t align)
{
  char *start = ALIGN(p->next, p->block, align);
  if (p->end < start || !start)
    return 0;
  return p->end - start;
}
