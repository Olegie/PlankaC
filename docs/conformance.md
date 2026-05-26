# Conformance Checks

PlankaC includes a small conformance suite for parser and runtime behavior.

Source fixtures:

```text
tests/conformance/valid_edge.plk
tests/conformance/bad_missing_end.plk
tests/conformance/bad_header.plk
tests/conformance/bad_stray_end.plk
tests/conformance/bad_duplicate_proc.plk
tests/conformance/bad_statement.plk
tests/conformance/bad_unknown_call.plk
tests/conformance/bad_type_marker.plk
tests/conformance/bad_type_mismatch.plk
tests/conformance/bad_symbolic_type_mismatch.plk
tests/conformance/bad_compound_type_mismatch.plk
tests/conformance/bad_3d_type_mismatch.plk
tests/conformance/bad_2d_missing_type.plk
tests/conformance/bad_recursion.plk
tests/conformance/bad_pnumber_recursion.plk
tests/conformance/bad_interproc_type.plk
tests/conformance/bad_assertion.plk
```

Runner:

```text
tests/plankac_conformance.c
```

Build and run:

```text
gcc -Wall -Wextra -std=c89 -Ic/include tests/plankac_conformance.c build/libplankac.a -o build/plankac_conformance.exe -lm
build/plankac_conformance.exe
```

Expected ending:

```text
CONFORMANCE OK
```

The suite checks:

- valid guarded execution;
- positive and negative guarded branches;
- wrong argument count;
- missing `END`;
- broken procedure header;
- stray `END`;
- duplicate procedure numbers;
- malformed type markers;
- local type mismatches for the same `V`/`Z`/`R` slot, including symbolic
  complex, compound, vector, and matrix markers such as `[:C32.16]`,
  `[:Q32.0]`, `[:VEC32.16]`, and `[:MAT32.16]`;
- missing type rows in executable two-dimensional notation;
- interprocedural type marker and type-family mismatches;
- direct and numbered recursion;
- invalid arrow chains;
- unknown procedure calls;
- failed assertions;
- indexed values, nested record fields, handle-backed records, lists, pairs,
  sets, relation algebra, relation composition, cartesian products,
  inverse/image helpers, value equality helpers, record merge, quantifier-like
  helpers, complex values, 3D vector/matrix transforms, rotation, projection, loops,
  executable two-dimensional rows, bytecode loading, and chess-style board
  structure examples;
- generated native ASM runner smoke checks through `build.bat`, including
  guarded multi-result output, loops, pair relations, shared handle heaps,
  records, relation composition, chess structures, two-dimensional rows,
  3D pipeline procedures, and `all_tests`.
- generated 8086/DOS assembly source through `build.bat`
  (`build/plankac_8086.asm`).
