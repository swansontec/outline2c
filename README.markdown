The outline2c utility runs before the regular C or C++ preprocessor. It generates related blocks of code from a common outline, eliminating redundancy and improving maintainability.

For example, a program with several settings might need to store its settings in memory, load the settings from disk, save the settings back to disk, and show a settings dialog. Adding a setting to the program would normally involve editing all these locations, but outline2c makes it possible to define all the settings in one place:

    \ol settings = outline {
      number width;
      number height;
      number threads;
      string username;
      string save_dir;
    }

Using simple templates, outline2c can convert this outline into the appropriate C code. Here is an example:

    struct settings {
      \ol type = map x { number {int} string {char *} }
      \ol for setting in settings { setting!type setting; }
    };

    void save_settings(struct settings *s, FILE *f)
    {
      \ol format = map x { number {"%s=%d\n"} string {"%s=%s\n"} }
      \ol for setting in settings {
        fprintf(f, setting!format, setting!quote, s->setting);
      }
    }

The output from this example would look something like this:

    struct settings {
      int width; 
      int height; 
      int threads; 
      char *username; 
      char *save_dir; 
    };

    void save_settings(struct settings *s, FILE *f)
    {
      fprintf(f, "%s=%d\n", "width", s->width); 
      fprintf(f, "%s=%d\n", "height", s->height); 
      fprintf(f, "%s=%d\n", "threads", s->threads); 
      fprintf(f, "%s=%s\n", "username", s->username); 
      fprintf(f, "%s=%s\n", "save_dir", s->save_dir); 
    }
