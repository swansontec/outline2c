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
