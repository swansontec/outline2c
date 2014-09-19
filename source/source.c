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
 * A source file
 */
typedef struct Source {
  String filename;
  String data;
  char const *cursor;
  struct Source *next;
} Source;

/**
 * A global linked list of Source structures. This makes it possible to find
 * line and column information in any file using only a character pointer.
 */
Source *source_list = 0;

/**
 * Loads a file into a Source structure
 */
Source *source_load(Pool *pool, String filename)
{
  FILE *fp = 0;
  long size;
  char *data;
  Source *self;

  filename = string_copy(pool, filename);

  fp = fopen(filename.p, "rb");
  if (!fp) return 0;

  if (fseek(fp, 0, SEEK_END))
    goto error;

  size = ftell(fp);
  if (size == -1L) goto error;

  if (fseek(fp, 0, SEEK_SET))
    goto error;

  data = (char*)pool_alloc(pool, size + 1, 1);

  if (fread(data, 1, size, fp) != size)
    goto error;
  data[size] = 0;

  self = pool_new(pool, Source);
  self->filename = filename;
  self->data = string(data, data + size);
  self->cursor = self->data.p;
  self->next = source_list;
  source_list = self;

  fclose(fp);
  return self;

error:
  fclose(fp);
  return 0;
}

/**
 * Formats and prints a source location
 */
int source_location(FILE *stream, char const *location)
{
  unsigned line;
  unsigned column;
  char const *p;

  Source *self = source_list;
  while (self && (location < self->data.p || self->data.end < location))
    self = self->next;
  if (!self)
    return 0;

  line = 0;
  column = 0;
  for (p = self->data.p; p < location; ++p) {
    if (*p == '\n') {
      ++line;
      column = 0;
    } else if (*p == '\t') {
      column += 8;
      column -= column % 8;
    } else {
      ++column;
    }
  }

  return 0 < fprintf(stream, "%s:%d:%d: ", self->filename.p, line + 1, column + 1);
}

/**
 * Prints an error message related to a source file
 */
int source_error(char const *location, char const *message)
{
  source_location(stderr, location);
  fprintf(stderr, "%s\n", message);
  return 0;
}
