#include <stdio.h>

/* List the settings in a single location: */
\ol settings = outline {
  number width;
  number height;
  number threads;
  string username;
  string save_dir;
}

/* Generate a structure to hold the settings: */
typedef struct {
  \ol type = map x {
    number {int x;}
    string {char *x;}
  }
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
  Settings s = {400, 300, 3, "me", "home"};
  save_settings(&s, stdout);
  return 0;
}
