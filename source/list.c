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
 * An element within a list. Not all lists are dynamically-typed, but having
 * a single dynamically-typed node struct saves the trouble of creating ad-hoc
 * node structs for those cases.
 */
typedef struct ListNode ListNode;
struct ListNode {
  ListNode *next;
  Dynamic d;
};

/**
 * Counts the nodes in a list.
 */
int list_length(ListNode *first)
{
  int i;
  for (i = 0; first; first = first->next)
    ++i;
  return i;
}

/**
 * This structure automates the process of building a linked list. The `first`
 * element remembers the first element in the list, and the `last` element
 * makes it possible to quickly insert new elements on the end.
 */
typedef struct {
  ListNode *first;
  ListNode *last;
  Pool *pool;
} ListBuilder;

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
void list_builder_add(ListBuilder *b, Dynamic value)
{
  ListNode *node = pool_new(b->pool, ListNode);
  node->next = 0;
  node->d = value;

  if (!b->first) {
    b->first = node;
    b->last = node;
  } else {
    b->last->next = node;
    b->last = node;
  }
}
