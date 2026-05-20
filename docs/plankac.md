# PlankaC v0.1

PlankaC is the compact C reader/interpreter at the center of the project. It
reads the project's `.plk` files, builds a procedure table, and executes the
repository's linear Plankalkuel profile directly.

It is deliberately scoped: it exists to make the repository testable from the
written Plankalkuel plans and keeps the executable model close to those source
files.

## Build

Use the root `build.bat`, or build the module set listed in
`docs/plankac_modules.md`.

The root `build.bat` builds it together with the console runner, static
library, conformance runner, and Windows GUI.

The public interface is declared in `c/include/plankac.h`.

For library use, see `docs/plankac_api.md` and `examples/c_api_demo.c`.
For module layout, see `docs/plankac_modules.md`.

## Commands

```text
build/plankac.exe check
build/plankac.exe compile
build/plankac.exe list
build/plankac.exe run <procedure|Pnumber> [args...]
build/plankac.exe bytecode <output.pbc>
build/plankac.exe checkbc <input.pbc>
build/plankac.exe runbc <input.pbc> <procedure|Pnumber> [args...]
build/plankac.exe cgen <output.c>
build/plankac.exe asmgen <output.S>
build/plankac.exe asmimage <output.asm>
build/plankac.exe asm8086 <output.asm>
build/plankac.exe demo
build/plankac.exe tests
```

Examples:

```text
build/plankac.exe run calculator_demo
build/plankac.exe run divide_checked 84 0
build/plankac.exe run P73 12 20
build/plankac.exe run three_d_pipeline_session
build/plankac.exe bytecode build/plankamath.pbc
build/plankac.exe checkbc build/plankamath.pbc
build/plankac.exe runbc build/plankamath.pbc set_session
build/plankac.exe cgen build/plankac_generated.c
build/plankac.exe asmgen build/plankac_asm_runtime.S
build/plankac.exe asm8086 build/plankac_8086.asm
```

Expected output:

```text
R0=30
R0=0 R1=1
R0=16
R0=120
Bytecode written: build/plankamath.pbc
Bytecode OK: 116 procedures
R0=2
C backend written: build/plankac_generated.c
ASM runtime written: build/plankac_asm_runtime.S
8086 ASM written: build/plankac_8086.asm
```

`asmgen` emits native x86-64 assembly functions for the loaded `.plk`
procedures, plus a compact command-line dispatcher. The generated program links
against `c/backends/plankac_native_runtime.c` for primitive frame storage,
lists, sets, relations, pairs, complex values, 3D vectors, 4x4 matrices, math
helpers, and output formatting. It does not call the PlankaC
interpreter or bytecode runner.

`asm8086` emits MASM/TASM-style 16-bit source for DOS-oriented experiments. It
contains the full bytecode image plus direct 8086 procedures for the arithmetic
core. Compound values are still kept in the bytecode image rather than fully
lowered to 16-bit machine code.

## Supported Subset

PlankaC currently supports the notation used in this repository:

- numbered procedure headers, for example `P10 add`;
- `V`, `Z`, and `R` variables;
- structure markers such as `[:32.16]`;
- symbolic type families such as `[:C32.16]`, `[:L32.0]`, `[:S32.0]`,
  `[:P32.0]`, `[:Q32.0]`, `[:VEC32.16]`, and `[:MAT32.16]`;
- indexed variables such as `Z10[0][:32.16]`;
- nested record fields such as `Z40.white.king.file[:32.0]`;
- handle-backed structured records through `record_new`, `record_set`,
  `record_get`, `record_has`, and `record_size`;
- frame-local list helpers `list_new`, `list_push`, `list_len`, `list_get`,
  `list_first`, `list_last`, `list_min`, `list_max`, `list_concat`;
- pair and set helpers such as `pair`, `pair_left`, `set_add`,
  `set_contains`, `set_union`, `set_intersection`, `set_difference`,
  `set_subset`, `set_equal`, `set_cartesian`, `relation_inverse`,
  `relation_image`, and `relation_compose`;
- richer value helpers such as `list_equal`, `pair_equal`, `record_merge`,
  and `record_equal`;
- chess board helpers such as `chess_square`, `chess_piece`,
  `chess_board_new`, `chess_board_place`, `chess_board_piece`, attack maps,
  and simple check predicates;
- modern 3D extension helpers such as `vec3`, `vec3_dot`, `vec3_cross`,
  `mat4_translate`, `mat4_scale`, `mat4_mul`, `mat4_transform_point`, and
  `perspective_project`;
- numeric constants with structure markers;
- arithmetic operators `+`, `-`, `*`, `/`, `%`, `^`;
- comparisons `=`, `!=`, `<`, `<=`, `>`, `>=`;
- boolean `&`, `|`, `!`;
- guarded equations of the form `condition => value => target`;
- loop equations of the form `LOOP count => value => target`;
- assertions of the form `ASSERT (expr)`;
- procedure calls with interprocedural marker/type-family checks;
- multiple result values.

The implementation is split into small modules: core loading/execution,
type-marker handling, two-dimensional notation expansion, static analysis,
backends, targets, and the compact legacy runtime. That is important for this
project because PlankaC is meant to be inspectable as a language tool, not a
single dense demonstration file.

The bytecode format is documented in `docs/plankac_bytecode.md`.
