#include <stdio.h>

\ol include "test.ol";

/* List the settings in a single location: */
\ol settings = outline {
  number width;
  number height;
  number threads;
  string username;
  string save_dir;
}
\ol type = map x {
  number {int x;}
  string {char *x;}
}

/* Generate a structure to hold the settings: */
typedef struct {
  \ol for setting in settings { setting!type }
} Settings;

/* Generate a function to save the settings: */
void save_settings(Settings *s, FILE *f)
{
  \ol format = map x {
    number {fprintf(f, "%s=%d\n", x!quote, s->x);}
    string {fprintf(f, "%s=%s\n", x!quote, s->x);}
  }
  \ol for setting in settings {
  setting!format}
}

int main(int argc, char *argv[])
{
  return 0;
}

/* Test nested outlines: */
\ol test_nesting = outline {
  item0 { sub0; sub1; }
  item1 { sub2; sub3; }
  item2;
}
\ol for i in test_nesting {
  i: \ol for j in i { j; }
}

/* Test pasting: */
\ol test_paste = outline { thing; }
\ol for i in test_paste { some\\i\\_test }
foo\\bar

/* Test case transformations: */
\ol test_case = outline { _SetCPUSpeed23_FOO; _; }
\ol for i in test_case {
  lower: i!lower
  upper: i!upper
  camel: i!camel
  mixed: i!mixed
}

/* Test "for" modifiers: */
\ol test_for = outline { a; b; c; x d; }
\ol for i in test_for with !x { i }
\ol for i in test_for list { i }
\ol for i in test_for reverse { i }
\ol for i in test_for reverse with !x list { i }
