/* Test include files: */
\ol include "test.ol";
\ol for i in included { i }

/* Test maps: */
\ol test_map = map x {
  a { x!quote: type_a }
  b { x!quote: type_b }
  * { x!quote: other }
}
\ol test_map_ol = outline {
  a one; b two; c three;
}
\ol for i in test_map_ol { i!test_map }

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
\ol for i in outline { thing; } { some\\i\\_test }
foo\\bar

/* Test case transformations: */
\ol for i in outline { _SetCPUSpeed23_FOO; _; } {
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

/* Test outline unions: */
\ol for i in union{included, test_map_ol with a, outline{last;}} { i }

/* Test macros: */
\ol test_macro = macro(a, b) {a: \ol for i in b {i }}
\ol for i in test_nesting {test_macro(i, included)}
