About outline2c
===============

The outline2c utility runs before the regular C or C++ preprocessor. It provides a simple code-generation language based on outlines and code templates. Outlines give a list of items to generate, while code templates describe how to generate each item.

Outline2c helps eliminate redundancy. For example, a program with several settings could list them all in one place:

    \ol settings = outline {
      number width;
      number height;
      number speed;
      string username;
      string save_dir;
    }

Templates transform outlines into running code:

    struct settings {
      \ol for i in settings {
        \ol map i {
          number {int i;}
          string {char *i;}
        }
      }
    };

This example produces something like:

    struct settings {
      int width;
      int height;
      int speed;
      char *username;
      char *save_dir;
    };

The same outline could be again and again to generate the settings GUI, command-line options parser, config-file reader, and so forth. Adding a setting would normally involve editing all these locations, but with outline2c it only involves a one-line change.

Full documentation is in the `doc/syntax.markdown` file.

Building outline2c
==================

For Linux:

    cd build-gcc
    make
    sudo make install

Windows users can find Visual Studio 2010 project files in the build-vs2010 folder.
