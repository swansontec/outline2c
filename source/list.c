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

/**
 * Initializes a new ListBuilder structure.
 */
ListBuilder list_builder_init()
{
  ListBuilder self;
  self.first = 0;
  self.last = 0;
  return self;
}

/**
 * Adds an item to the end of a list.
 */
int list_builder_add(ListBuilder *b, Pool *pool, Dynamic item)
{
  ListNode *node = pool_alloc(pool, sizeof(ListNode));
  if (!node) return 0;
  node->next = 0;
  node->p = item.p;
  node->type = item.type;

  if (!b->first) {
    b->first = node;
    b->last = node;
  } else {
    b->last->next = node;
    b->last = node;
  }

  return 1;
}
