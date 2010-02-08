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

#include "parser.h"
#include "file.h"
#include "string.h"
#include <stdio.h>
#include <string.h>

struct main {
  char const *name_out;
  FileR file_in;
  FileW file_out;
};
typedef struct main Main;
void main_init(Main *m);
int main_run(Main *m, int argc, char *argv[]);
void main_close(Main *m);

void main_init(Main *m)
{
  m->name_out = 0;
  file_r_init(&m->file_in);
  file_w_init(&m->file_out);
}

void main_close(Main *m)
{
  if (m->name_out) free((char *)m->name_out);
  file_r_close(&m->file_in);
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
    printf(" Usage: tree2c <filename>\n");
    return 1;
  }

  /* Determine output file name: */
  in_name = string_init_l(argv[1], strlen(argv[1]));
  if (string_rmatch(in_name, string_init_l(".o2c", 4)) != 4) {
    printf(" Error: The input file name must end with \".o2c\".\n");
    return 1;
  }
  m->name_out = string_to_c(string_init(in_name.p, in_name.end - 4));

  /* Open input file */
  rv = file_r_open(&m->file_in, argv[1]);
  if (rv){
    printf(" Could not open file \"%s\"\n", argv[1]);
    return 1;
  }

  /* Open output file: */
  rv = file_w_open(&m->file_out, m->name_out);
  if (rv) {
    printf(" Could not open file \"%s\"\n", m->name_out);
    return 1;
  }

  /* Munchify files: */
  parser_start(string_init(m->file_in.p, m->file_in.end), string_from_c(argv[1]), &m->file_out);
  return 0;
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
