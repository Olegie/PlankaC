# PlankaC v0.1

PlankaC is the small C reader/interpreter at the center of the project. It
reads the project's `.plk` files, builds a procedure table, and executes the
repository's linear Plankalkuel profile directly.

It is intentionally narrow: it exists to make the repository testable from the
written Plankalkuel plans and keeps the executable model close to those source
files.

## Build

Use the root `build.bat`, or build the module set listed in
`docs/plankac_modules.md`.

The root `build.bat` builds it together with the console runner, static
library, conformance runner, and Windows GUI.

The public interface is declared in `c/plankac.h`.

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
build/plankac.exe demo
build/plankac.exe tests
```

Examples:

```text
build/plankac.exe run calculator_demo
build/plankac.exe run divide_checked 84 0
build/plankac.exe run P73 12 20
build/plankac.exe bytecode build/plankamath.pbc
build/plankac.exe checkbc build/plankamath.pbc
build/plankac.exe runbc build/plankamath.pbc set_session
build/plankac.exe cgen build/plankac_generated.c
build/plankac.exe asmgen build/plankac_asm_runtime.S
```

Expected output:

```text
R0=30
R0=0 R1=1
R0=16
Bytecode written: build/plankamath.pbc
Bytecode OK: 62 procedures
R0=2
C backend written: build/plankac_generated.c
ASM runtime written: build/plankac_asm_runtime.S
```

`asmgen` emits native x86-64 assembly functions for the loaded `.plk`
procedures, plus a small command-line dispatcher. The generated program links
against `c/plankac_native_runtime.c` for primitive frame storage, lists, sets,
pairs, math helpers, and output formatting. It does not call the PlankaC
interpreter or bytecode runner.

## Supported Subset

PlankaC currently supports the notation used in this repository:

- numbered procedure headers, for example `P10 add`;
- `V`, `Z`, and `R` variables;
- structure markers such as `[:32.16]`;
- indexed variables such as `Z10[0][:32.16]`;
- nested record fields such as `Z40.white.king.file[:32.0]`;
- frame-local list helpers `list_new`, `list_push`, `list_len`, `list_get`,
  `list_first`, `list_last`, `list_min`, `list_max`, `list_concat`;
- pair and set helpers such as `pair`, `pair_left`, `set_add`,
  `set_contains`, `set_union`, `set_intersection`;
- numeric constants with structure markers;
- arithmetic operators `+`, `-`, `*`, `/`, `%`, `^`;
- comparisons `=`, `!=`, `<`, `<=`, `>`, `>=`;
- boolean `&`, `|`, `!`;
- guarded equations of the form `condition => value => target`;
- loop equations of the form `LOOP count => value => target`;
- assertions of the form `ASSERT (expr)`;
- procedure calls with direct interprocedural marker checks;
- multiple result values.

The interpreter is split into small modules, which keeps the parser, source
loader, bytecode writer, runtime, and API readable as separate pieces. That is
useful for a historical/educational project: the source plans and the
executable model can be compared without a large compiler framework in between.

The bytecode format is documented in `docs/plankac_bytecode.md`.
