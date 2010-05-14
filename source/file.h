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

#ifndef FILE_H_INCLUDED
#define FILE_H_INCLUDED

#include <stdio.h>

typedef struct file_r FileR;
typedef struct file_w FileW;

/**
 * A handle to a read-only file. This should use memory-mapped files on
 * supporting platforms, but for now it uses the C standard library.
 */
struct file_r {
  char const *p;
  char const *end;
};

void file_r_init(FileR *file);
int file_r_open(FileR *file, char const *name);
void file_r_close(FileR *file);

/**
 * Represents a write-only open file.
 */
struct file_w {
  FILE *file;
};

void file_w_init(FileW *file);
int file_w_open(FileW *file, char const *name);
int file_w_write(FileW *file, char const *p, char const *end);
void file_w_close(FileW *file);

#endif
