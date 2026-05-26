# Conformance Checks

PlankaC includes a focused conformance suite for parser, analyzer, runtime,
ABI, backend, and source-layout behavior.

Source fixtures:

```text
tests/conformance/valid_edge.plk
tests/conformance/valid/page_table_document.plk
tests/conformance/runtime/tagged_fixed_runtime.plk
tests/conformance/backend_equivalence/predicate_backend.plk
tests/conformance/zuse_examples/linear_assignment_table.plk
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
tests/conformance/bad_const_target.plk
tests/conformance/bad_require_contract.plk
tests/conformance/bad_divide_zero.plk
tests/conformance/invalid/bad_page_without_type.plk
examples/host_abi.plk
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
- local type mismatches for the same `V`/`C`/`Z`/`R` slot, including symbolic
  complex, compound, vector, and matrix markers such as `[:C32.16]`,
  `[:Q32.0]`, `[:VEC32.16]`, and `[:MAT32.16]`;
- missing type rows in executable two-dimensional notation;
- PAGE document diagnostics with row/column recovery;
- valid PAGE table loading from the `valid/` conformance tree;
- interprocedural type marker and type-family mismatches;
- direct and numbered recursion;
- invalid arrow chains;
- unknown procedure calls;
- failed assertions;
- invalid `CONST` targets;
- failed contract requirements;
- direct arithmetic divide-by-zero exceptions;
- registered native C callback execution through `examples/host_abi.plk`;
- typed IR emission through `plankac ir`;
- backend lowering report emission through `plankac lowering`;
- compiler pipeline output through `plankac compile`, including IR reload and
  generated C, x86-64 ASM, and 8086 ASM artifacts;
- linked native executables through `plankac native-c` and
  `plankac native-asm`;
- indexed values, nested record fields, handle-backed records, lists, pairs,
  constants in the `C` bank, two-dimensional indexed refs, bit/fixed helpers,
  result/status arithmetic exceptions, stop criteria, contract predicates,
  sets, relation algebra, relation composition, cartesian products,
  inverse/image helpers, value equality helpers, record merge, nested record
  paths, list selection/counting/existence/forall helpers, list pairs,
  relation selection by domain/range, relation quantifier helpers, relation
  signatures, complex values, 3D vector/matrix transforms, rotation,
  projection, loops, executable two-dimensional rows, PAGE documents, bytecode
  loading, and chess board/game structure examples;
- legal move counts, promotion checks, en passant checks, castling-path checks,
  position/FEN-style signatures, move-history lists, stalemate checks, material
  search, and mate checks;
- generated native ASM runner smoke checks through `build.bat`, including
  guarded multi-result output, loops, pair relations, shared handle heaps,
  records, relation composition, chess structures, two-dimensional rows,
  3D pipeline procedures, and `all_tests`.
- generated 8086/DOS assembly source through `build.bat`
  (`build/plankac_8086.asm`).
