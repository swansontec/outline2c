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
 * each time it receives an allocation request, it returns a portion of the
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
 * Most pool_ functions take an explicit alignment parameter. The alignment
 * must be a power of two, and should be smaller than malloc's native
 * alignment. If the alignment is larger, malloc's native alignment will be
 * used instead and a small amount of memory might be wasted.
 *
 * These functions instantly abort the entire program when they encounter an
 * out-of-memory error.
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
 * Basing the alignment calculation on the block's base address, which comes
 * from malloc, will either meet the requested alignment or fail in a way that
 * can't possibly matter, since malloc already covers the worst case.
 */
#define ALIGN(p, base, align) ((base) + (((p) - (base) + ((align)-1)) & ~((align)-1)))
#endif

/**
 * Calculates a data type's alignment.
 */
#define alignof(type) offsetof(struct { char c; type data; }, data)

/**
 * Verifies that a memory-allocating call succeeds, and aborts the program
 * otherwise.
 */
#define CHECK_MEMORY(p) do { \
  if (!(p)) { \
    fprintf(stderr, "error: Out of memory at %s:%d\n", __FILE__, __LINE__); \
    abort(); \
  } \
} while(0)

/**
 * Helper function to add new blocks to the pool.
 */
void pool_grow(Pool *self, size_t size)
{
  char *block = (char*)malloc(size);
  CHECK_MEMORY(block);

  /* The first element in each block is a pointer to previous block: */
  *(char**)block = self->block;

  /* Place the new block in the pool: */
  self->block = block;
  self->next  = block + sizeof(char*);
  self->end   = block + size;
}

/**
 * Initializes a memory pool, allocating an initial block of the given size.
 * Each future block will have the same size as the initial block, so choose
 * wisely.
 */
Pool pool_init(size_t size)
{
  Pool self = {0, 0, 0};
  pool_grow(&self, size);
  return self;
}

/**
 * Frees all memory in the pool. The pool is no longer usable after calling
 * this function.
 */
void pool_free(Pool *self)
{
  char *block = self->block;
  while (block) {
    char *next = *(char**)block;
    free(block);
    block = next;
  }
}

/**
 * Allocates memory using malloc and adds the result to the pool's list of
 * blocks to free.
 */
void *pool_alloc_sys(Pool *self, size_t size, size_t align)
{
  size_t padding = sizeof(char*) < align ? align : sizeof(char*);
  char *block = (char*)malloc(padding + size);
  CHECK_MEMORY(block);

  /* Add the block to the list of stuff to free: */
  *(char**)block = *(char**)self->block;
  *(char**)self->block = block;

  return block + padding;
}

/**
 * Allocates memory from the pool.
 */
void *pool_alloc(Pool *self, size_t size, size_t align)
{
  char *start = ALIGN(self->next, self->block, align);
  char *end = start + size;

  /* Is the request too large? The 2nd test checks for overflow. */
  if (self->end < end || !start) {
    size_t block_size = self->end - self->block;
    /* Use malloc for large blocks: */
    if (block_size < 64*size)
      return pool_alloc_sys(self, size, align);

    /* Grow the pool: */
    pool_grow(self, block_size);
    start = ALIGN(self->next, self->block, align);
    end = start + size;
  }

  self->next = end;
  return start;
}

#define pool_new(self, type) \
  ((type*)pool_alloc(self, sizeof(type), alignof(type)))
