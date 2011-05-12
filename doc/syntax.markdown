Outline to C Documentation
==========================

Escaping
--------

By default, the outline2c compiler treats its input as pain C code and simply writes it to the output file. Statements intended for outline2c must begin with the special `\ol` escape code. Outline2c ignores escape codes that occur inside C-style comments (`/**/`), C++-style comments (`//`), and strings.

Basic usage
-----------

The `outline` keyword creates an outline. Outlines can contain any number of items. Here is an example:

    \ol breakfast = outline {
      eggs;
      bacon;
      toast;
    }

This example creates an outline named "breakfast" with three items named "eggs", "bacon", and "toast".

The `for` statement generates code based on an outline. The `for` statement loops over the items in an outline and writes a block of code for each one:

    \ol for food in breakfast { int food; }

This example loops over the items in the outline named "breakfast". On each iteration, the variable named "food" is set to the current item's name. So, "food" would be "eggs" on the first iteration, "bacon" on the second iteration and "toast" on the last iteration. The code between the `{}` brackets is written to the output on each iteration, so the complete output from this example would be:

    int eggs;  int bacon;  int toast;

Renaming items
--------------

It often makes sense to modify an item's name before writing it to the output. Outline2c provides a few ways to do this.

The `\\\\` operator concatenates text onto the beginning or end of an item name:

    \ol for food in breakfast { int have_\\\\food; }

This example places the prefix "have_" on the beginning of each item name, producing the output:

    int have_eggs;  int have_bacon;  int have_toast;

Outline2c provides several transformations that modify the item name itself:

* `upper` - Converts the name to `UPPER_CASE`
* `lower` - Converts the name to `lower_case`
* `camel` - Converts the name to `CamelCase`
* `mixed` - Converst the name to `mixedCase`
* `quote` - Surrounds the name in "quotes"

The `!` operator appies one of these transformations to the item:

    \ol for food in breakfast { printf(food!quote); }

This example produces:

    printf("eggs");  printf("bacon");  printf("toast");

Concatenation and transformation can be combined. For example:

    \ol for food in breakfast {
    #define WANT_\\\\food!upper}

Generates:

    #define WANT_EGGS
    #define WANT_BACON
    #define WANT_TOAST

The case transformers handle multi-word item names intelligently. They are smart enough to transform "FrenchToast" into "FRENCH_TOAST", for example.

Advanced options
----------------

Outline2c provides a few options that modify the `for` statment's behavior. The `list` option inserts a comma between each block of code. This is useful for generating function parameter lists, enum definitions, and so forth:

    int place_order(\ol for food in breakfast list { int food });

This example produces:

    int place_order( int eggs , int bacon , int toast );

It is also possible to process the items in reverse using the `reverse` option:

    \ol for food in breakfast reverse { food }

Which produces:

    toast  bacon  eggs

Tags
----

Items within an outline can have "tags." Tags, if present, always come before the item's name:

    \ol club = outline {
      active Tim;
      Joe;
      active president Bob;
    }

The item named "Bob" has two tags in this example, "active" and "president".

The `for` statement can filter items based on tags. This is done using the `with` option:

    \ol for member in club with active { member; }

The example above only processes items in the "club" outline that have an "active" tag:

    Tim;  Bob;

The `with` option can be freely combined with the `list` and `reverse` options. Filters support several operators:

* `tag` - matches any item that has `tag`
* `!tag` - matches any item that does not have `tag`
* `tag1 & tag2` - matches any item that has both `tag1` and `tag2`
* `tag1 | tag2` - matches any item that has either `tag1` or `tag2`
* `*` - matches any item at all

Operators can be combined to form arbitrary expressions such as `(active | honorary) & !president`. The `&` operator has a higher precedence than the `|` operator.

Maps
----

Maps offer another way to filter items based on tags. A map contains a set of filters and code blocks. The map finds the first matching filter and emits the corresponding code block:

    \ol for member in club {
      \ol map member {
        president {printf("Hello, Master %s!\\n", member!quote);}
        active    {printf("Hello, %s.\\n", member!quote);}
        !active   {printf("Welcome back, %s.\\n", member!quote);}
      }}

In this example, the map selects an appropriate greeting based on the "member" item's tags, producing the following output:

    printf("Hello, %s.\\n", "Tim");
    printf("Welcome back, %s.\\n", "Joe");
    printf("Hello, Master %s!\\n", "Bob");

Outline2c generates an error message if none of the options match.

Tag values
----------

A tag can have a block of attached C-code, like this:

    \ol universe = outline {
      answer={42} mice;
    }

Here, the C-code "42" is attached to the "answer" tag. To use a tag's value in generated code, simply use the `!` operator with the name of the tag:

    \ol for i in universe { i!answer }

This example generates:

    42

Since name transforms and tag values use the same `!` operator, there is some room for ambiguity. Outline2c resolves this by giving highest priority to tag values and lowest priority to the built-in transforms.

Nested outlines
---------------

Each item within an outline can contain another outline inside it. For example:

    \ol states = outline {
      california {
        san_diego;
        sacramento;
        los_angeles;
      }
      washington { redmond; }
      washington_dc;
    }

In this example, the "california" and "washington" items each contain nested outlines.

To generate code for nested outlines, simply place one `for` statement inside another, using the current item as the outline name:

    \ol for state in states { \ol for city in state {
    printf("%s is in %s.\\n", city!quote, state!quote);}}

This example produces:

    printf("%s is in %s.\\n", "san_diego", "california");
    printf("%s is in %s.\\n", "sacramento", "california");
    printf("%s is in %s.\\n", "los_angeles", "california");
    printf("%s is in %s.\\n", "redmond", "washington");

Outline unions
--------------

The `union` keyword gathers items from several outlines into one:

    \ol both = union {outline_a, outline_b}

This line creates a new outline, called "both", which contains all the items in "outline_a" and "outline_b". Unions also understand the `with` modifier:

    \ol some = union {outline_a, outline_b with foo}

The resulting union contains all the items from "outline_a" and any items from
"outline_b" that have the tag "foo".

Macros
------

The `macro` keyword defines a macro:

    \ol do_stuff = macro(a, b) { \ol for i in a { b } }

Invoking a macro is pretty straightforward:

    do_stuff(outline_a, item_b)

This is equivalent to entering the following code directly:

    \ol for i in outline_a { item_b }
