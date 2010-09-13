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

#include "generate.h"
#include <string.h>

typedef struct options Options;

/*
 * Holds outline2c command-line options.
 */
struct options
{
  int debug;
  String name_in;
  String name_out;
};

void options_init(Options *self)
{
  self->debug = 0;
  self->name_in = string_null();
  self->name_out = string_null();
}

/**
 * Processes the command-line options, filling in the members of the Options
 * structure corresponding to the switches.
 * @return 0 for failure
 */
int options_parse(Options *self, int argc, char *argv[])
{
  int arg = 1;

  while (arg < argc) {
    String s = string_from_c(argv[arg]);

    /* Debug: */
    if (!strcmp(argv[arg], "-d") || !strcmp(argv[arg], "--debug")) {
      self->debug = 1;

    /* Output filename: */
    } else if (!strcmp(argv[arg], "-o")) {
      ++arg;
      if (argc <= arg) return 0;
      self->name_out = string_from_c(argv[arg]);

    /* Output filename, smushed: */
    } else if (2 == string_match(s, string_init_l("-o", 2))) {
      self->name_out = string_init(s.p + 2, s.end);

    /* Input filename: */
    } else {
      if (string_size(self->name_in)) return 0;
      self->name_in = s;
    }
    ++arg;
  }
  return 1;
}

/**
 * Program entry point. Constructs and launches the main program object.
 */
int main(int argc, char *argv[])
{
  int rv;
  Options opt;
  char *s;
  FILE *file_out;

  options_init(&opt);
  if (!options_parse(&opt, argc, argv)) {
    fprintf(stderr, "Usage: %s [-d] [-o output-file] <input-file>\n", argv[0]);
    return 1;
  }

  /* Determine output file name: */
  if (string_size(opt.name_out)) {
    s = string_to_c(opt.name_out);
  } else {
    if (string_rmatch(opt.name_in, string_init_l(".ol", 3)) != 3) {
      fprintf(stderr, "Error: If no output file is specified, the input file name must end with \".ol\".\n");
      return 1;
    }
    s = string_to_c(string_init(opt.name_in.p, opt.name_in.end - 3));
  }

  /* Open output file: */
  file_out = fopen(s, "wb");
  if (!file_out) {
    fprintf(stderr, "error: Could not open output file \"%s\"\n", s);
    free(s);
    return 1;
  }
  free(s);

  /* Munchify files: */
  rv = generate(file_out, opt.name_in, opt.debug);
  fclose(file_out);
  return !rv;
}
