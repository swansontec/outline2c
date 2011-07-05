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
 * Holds command-line options
 */
typedef struct {
  unsigned debug: 1;
  String name_in;
  String name_out;
} Options;

Options options_init()
{
  Options self;
  self.debug = 0;
  self.name_in = string_null();
  self.name_out = string_null();
  return self;
}

/**
 * Processes the command-line options, filling in the members of the Options
 * structure corresponding to the switches
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
    } else if (2 == string_match(s, string_from_k("-o"))) {
      self->name_out = string(s.p + 2, s.end);

    /* Input filename: */
    } else {
      if (string_size(self->name_in)) return 0;
      self->name_in = s;
    }
    ++arg;
  }
  return 1;
}
