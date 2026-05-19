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
gcc -Wall -Wextra -std=c89 tests/plankac_conformance.c build/libplankac.a -o build/plankac_conformance.exe -lm
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
- local type mismatches for the same `V`/`Z`/`R` slot;
- direct interprocedural type marker mismatches;
- direct and numbered recursion;
- invalid arrow chains;
- unknown procedure calls;
- failed assertions;
- indexed values, nested record fields, lists, pairs, sets, loops, executable
  two-dimensional rows, bytecode loading, and chess-style structure examples.
- generated native ASM runner smoke checks through `build.bat`, including
  guarded multi-result output, loops, pair relations, and `all_tests`.
