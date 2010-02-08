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

#include "file.h"
#include <malloc.h>

/**
 * Initializes a FileR structure to 0, making it safe to call file_r_clean in
 * any situation.
 */
void file_r_init(FileR *file)
{
  file->p = 0;
}

/**
 * Opens a file, storing cleanup data and content pointers in the File
 * structure. The content pointers will be NULL if the function fails.
 * @return 0 for success
 */
int file_r_open(FileR *file, char const *name)
{
  FILE *fp;
  long size;
  char *data;

  fp = fopen(name, "rb");
  if (!fp)
    return 1;

  if (fseek(fp, 0, SEEK_END))
    goto error;

  size = ftell(fp);
  if (size == -1L)
    goto error;

  if (fseek(fp, 0, SEEK_SET))
    goto error;

  data = malloc(size + 1);
  if (!data)
    goto error;

  if (fread(data, 1, size, fp) != size) {
    free(data);
    goto error;
  }

  data[size] = 0;
  file->p = data;
  file->end = data + size;
  return 0;

error:
  fclose(fp);
  return 1;
}

/**
 * Closes a file.
 */
void
file_r_close(FileR *file)
{
  if (file->p)
    free((char *)file->p);
  file->p = 0;
}

/**
 * Initializes a FileW structure to 0, making it safe to call file_w_clean in
 * any situation.
 */
void file_w_init(FileW *file)
{
  file->file = 0;
}

/**
 * Opens a file for writing.
 * @return 0 for success
 */
int file_w_open(FileW *file, char const *name)
{
  file->file = fopen(name, "wb");
  if (!file)
    return 1;
  return 0;
}

/**
 * Writes a series of bytes to a file.
 * @return 0 for success
 */
int file_w_write(FileW *file, char const *p, char const *end)
{
  return fwrite(p, 1, end - p, file->file) != end - p;
}

void file_w_close(FileW *file)
{
  if (file->file)
    fclose(file->file);
  file->file = 0;
}
