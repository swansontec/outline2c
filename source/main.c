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
#include "string.h"
#include "generate.h"
#include <stdio.h>
#include <string.h>

struct main {
  char const *name_out;
  FileW file_out;
};
typedef struct main Main;
void main_init(Main *m);
int main_run(Main *m, int argc, char *argv[]);
void main_close(Main *m);

void main_init(Main *m)
{
  m->name_out = 0;
  file_w_init(&m->file_out);
}

void main_close(Main *m)
{
  if (m->name_out) free((char *)m->name_out);
  file_w_close(&m->file_out);
}

/**
 * Program body. Processes command-line options and launches the parser.
 */
int main_run(Main *m, int argc, char *argv[])
{
  int rv;
  String in_name;

  /* Count options: */
  if (argc != 2) {
    printf(" Usage: %s <filename>\n", argv[0]);
    return 1;
  }

  /* Determine output file name: */
  in_name = string_from_c(argv[1]);
  if (string_rmatch(in_name, string_init_l(".ol", 3)) != 3) {
    printf(" Error: The input file name must end with \".ol\".\n");
    return 1;
  }
  m->name_out = string_to_c(string_init(in_name.p, in_name.end - 3));

  /* Open output file: */
  rv = file_w_open(&m->file_out, m->name_out);
  if (rv) {
    printf(" Could not open file \"%s\"\n", m->name_out);
    return 1;
  }

  /* Munchify files: */
  return generate(&m->file_out, argv[1]);
}

/**
 * Program entry point. Constructs and launches the main program object.
 */
int main(int argc, char *argv[])
{
  int rv;
  Main program;
  main_init(&program);
  rv = main_run(&program, argc, argv);
  main_close(&program);
  return rv;
}
