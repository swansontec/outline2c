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

#include "list.h"
#include <stdio.h>

/**
 * Initializes a new ListBuilder structure.
 */
ListBuilder list_builder_init(Pool *pool)
{
  ListBuilder self;
  self.first = 0;
  self.last = 0;
  self.pool = pool;
  return self;
}

/**
 * Adds an item to the end of a list.
 */
int list_builder_add(ListBuilder *b, Type type, void *p)
{
  ListNode *node = pool_alloc(b->pool, sizeof(ListNode));
  CHECK_MEM(node);
  node->next = 0;
  node->p = p;
  node->type = type;
  if (!node->p) return 0;

  if (!b->first) {
    b->first = node;
    b->last = node;
  } else {
    b->last->next = node;
    b->last = node;
  }

  return 1;
}

static int list_out_fn(void *data, Type type, void *p)
{
  return list_builder_add(data, type, p);
}

OutRoutine list_builder_out(ListBuilder *b)
{
  OutRoutine self;
  self.code = list_out_fn;
  self.data = b;
  return self;
}
