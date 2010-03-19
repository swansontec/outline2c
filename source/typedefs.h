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
/*
 * Because C doesn't seem to allow duplicated typedefs, it makes sense to put
 * all the "typedef struct" definitions in one place to avoid clashes.
 */

#ifndef TYPEDEFS_H_INCLUDED
#define TYPEDEFS_H_INCLUDED

/* ast-builder.h */
typedef struct ast_builder AstBuilder;

/* file.h */
typedef struct file_r FileR;
typedef struct file_w FileW;

/* pool.h */
typedef struct pool Pool;

/* search.h */
typedef struct scope Scope;

/* string.h */
typedef struct string String;

#endif
