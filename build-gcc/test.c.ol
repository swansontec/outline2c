#include <stdio.h>

\ol include "test.ol";

/* List the settings in a single location: */
\ol outline settings {
  number width;
  number height;
  number threads;
  string username;
  string save_dir;
}

/* Generate a structure to hold the settings: */
typedef struct {
  \ol map type {
    number { int }
    string { char * }
  }
  \ol for setting in settings { setting!type setting; }
} Settings;

/* Generate a function to save the settings: */
void save_settings(Settings *s, FILE *f)
{
  \ol map format {
    number {"%s=%d\n"}
    string {"%s=%s\n"}
  }
  \ol for setting in settings {
    fprintf(f, setting!format, setting!quote, s->setting); }
}

int main(int argc, char *argv[])
{
  return 0;
}

/* Test nested outlines: */
\ol outline test_nesting {
  item0 { sub0; sub1; }
  item1 { sub2; sub3; }
  item2;
}
\ol for i in test_nesting {
  i: \ol for j in . { j; }
}

/* Test pasting: */
\ol outline test_paste { thing; }
\ol for i in test_paste { some\\i\\_test }
foo\\bar

/* Test case transformations: */
\ol outline test_case { _SetCPUSpeed23_FOO; _; }
\ol for i in test_case {
  lower: i!lower
  upper: i!upper
  camel: i!camel
  mixed: i!mixed
}

/* Test "for" modifiers: */
\ol outline test_for { a; b; c; x d; }
\ol for i in test_for with !x { i }
\ol for i in test_for list { i }
\ol for i in test_for reverse { i }
\ol for i in test_for reverse with !x list { i }
