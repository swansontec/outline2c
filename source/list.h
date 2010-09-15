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

#ifndef LIST_H_INCLUDED
#define LIST_H_INCLUDED

#include "type.h"
#include "pool.h"

typedef struct ListNode ListNode;
typedef struct ListBuilder ListBuilder;

/**
 * An element within a list. Not all lists are dynamically-typed, but having
 * a single dynamically-typed node struct saves the trouble of creating ad-hoc
 * node structs for those cases.
 */
struct ListNode {
  ListNode *next;
  void *p;
  Type type;
};

/**
 * This structure automates the process of building a linked list. The `first`
 * element remembers the first element in the list, and the `last` element
 * makes it possible to quickly insert new elements on the end.
 */
struct ListBuilder {
  ListNode *first;
  ListNode *last;
};

ListBuilder list_builder_init();
int list_builder_add(ListBuilder *b, Pool *pool, Type type, void *p);
int list_builder_add2(ListBuilder *b, Pool *pool, Dynamic item);

#endif
