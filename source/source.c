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
 * Holds line and column information
 */
typedef struct {
  unsigned line;
  unsigned column;
} Location;

/**
 * Obtains line and column numbers given a pointer into a file. This is not
 * fast, but printing errors is hardly a bottleneck. This routine counts from
 * 0, but most text editors start from 1. It might make sense to add 1 to the
 * returned values.
 */
Location location_init(char const *start, char const *location)
{
  Location self;
  char const *p;

  self.line = 0;
  self.column = 0;
  for (p = start; p < location; ++p) {
    if (*p == '\n') {
      ++self.line;
      self.column = 0;
    } else if (*p == '\t') {
      self.column += 8;
      self.column -= self.column % 8;
    } else {
      ++self.column;
    }
  }
  return self;
}

/**
 * A stream of input text feeding the parser
 */
typedef struct {
  String filename;
  String data;
  char const *cursor;
} Source;

/**
 * Loads a file into a Source structure
 */
int source_load(Source *self, Pool *pool, String filename)
{
  char *c_name = 0;
  FILE *fp = 0;
  long size;
  char *data;

  c_name = string_to_c(filename);
  if (!c_name) goto error;

  fp = fopen(c_name, "rb");
  if (!fp) goto error;

  if (fseek(fp, 0, SEEK_END))
    goto error;

  size = ftell(fp);
  if (size == -1L) goto error;

  if (fseek(fp, 0, SEEK_SET))
    goto error;

  data = pool_alloc(pool, size + 1, 1);
  if (!data) goto error;

  if (fread(data, 1, size, fp) != size)
    goto error;
  data[size] = 0;

  self->filename = filename;
  self->data = string_init(data, data + size);
  self->cursor = self->data.p;

  free(c_name);
  fclose(fp);
  return 1;

error:
  if (c_name) free(c_name);
  if (fp)     fclose(fp);
  return 0;
}

/**
 * Prints an error message related to current cursor location
 */
int source_error(Source *self, char const *location, char const *message)
{
  Location l = location_init(self->data.p, location);
  fwrite(self->filename.p, string_size(self->filename), 1, stderr);
  fprintf(stderr, ":%d:%d: error: %s\n", l.line + 1, l.column + 1, message);
  return 0;
}
