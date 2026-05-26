# PlankaC Command Reference

PlankaC is the compact compiler, runtime, and C-facing toolchain at the center
of the project. It reads the project's `.plk` files, builds a procedure table,
executes the repository's linear Plankalkuel profile directly, and emits
bytecode/IR, generated C, generated ASM, and native executable build paths.

It is deliberately scoped: the executable model stays close to the written
Plankalkuel plans, while compiler output is kept under explicit backend and
pipeline commands.

## Build

Use the root `build.bat`, or build the module set listed in
`docs/plankac_modules.md`.

The root `build.bat` builds it together with the console runner, static
library, conformance runner, and Windows GUI.

The public interface is declared in `c/include/plankac.h`.

For library use, see `docs/plankac_api.md`, `docs/abi.md`,
`examples/c_api_demo.c`, and `examples/c_abi_demo.c`.
For module layout, see `docs/plankac_modules.md`.
For the full user-facing manual, start with `docs/index.md`.

## Commands

```text
build/plankac.exe check
build/plankac.exe compile <output-prefix>
build/plankac.exe native-c <output-prefix>
build/plankac.exe native-asm <output-prefix>
build/plankac.exe list
build/plankac.exe run <procedure|Pnumber> [args...]
build/plankac.exe checkfile <extra.plk>
build/plankac.exe runfile <extra.plk> <procedure|Pnumber> [args...]
build/plankac.exe bytecode <output.pbc>
build/plankac.exe checkbc <input.pbc>
build/plankac.exe runbc <input.pbc> <procedure|Pnumber> [args...]
build/plankac.exe ir <output.ir>
build/plankac.exe lowering <output.txt>
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
build/plankac.exe runfile graphics/src/plankagui.plk app_canvas
build/plankac.exe runfile graphics/src/plankagui.plk add 12 8
build/plankac.exe runfile graphics/src/plankacube.plk cube_scene_checksum
build/plankac.exe runfile graphics/src/plankacube.plk app_timer_step 0.5
build/plankac.exe bytecode build/plankamath.pbc
build/plankac.exe checkbc build/plankamath.pbc
build/plankac.exe runbc build/plankamath.pbc set_session
build/plankac.exe ir build/plankac.ir
build/plankac.exe lowering build/plankac.lowering
build/plankac.exe cgen build/plankac_generated.c
build/plankac.exe asmgen build/plankac_asm_runtime.S
build/plankac.exe asm8086 build/plankac_8086.asm
build/plankac.exe compile build/plankac_pipeline
build/plankac.exe native-c build/plankac_native_c
build/plankac_native_c.exe set_session
build/plankac.exe native-asm build/plankac_native_asm
build/plankac_native_asm.exe add 12 8
```

Expected output:

```text
R0=30
R0=0 R1=1
R0=16
R0=120
R0=960 R1=460
R0=20
R0=2403.500000
R0=0.545000
Bytecode written: build/plankamath.pbc
Bytecode OK: 148 procedures
R0=2
IR written: build/plankac.ir
Lowering written: build/plankac.lowering
C backend written: build/plankac_generated.c
ASM runtime written: build/plankac_asm_runtime.S
8086 ASM written: build/plankac_8086.asm
Compiler pipeline OK
native-c: build/plankac_native_c.exe
R0=2
Compiler pipeline OK
native-asm: build/plankac_native_asm.exe
R0=20
```

`compile` is the stable compiler route. It writes bytecode/IR, reloads that
IR, and emits generated C, x86-64 ASM, and 8086 ASM from the reloaded program.
`native-c` and `native-asm` use the same route and then link an executable
through the external GCC/MinGW toolchain.

`lowering` emits the backend plan for each typed IR statement. It shows which
operations use scalar/direct paths and which ones use compound runtime or ABI
entry points in generated C, x86-64 ASM, and 8086 ASM.

`asmgen` emits native x86-64 assembly functions for the loaded `.plk`
procedures, plus a compact command-line dispatcher. The generated program links
against `c/backends/plankac_native_runtime.c` for primitive frame storage,
lists, sets, relations, pairs, complex values, 3D vectors, 4x4 matrices, math
helpers, and output formatting. It does not call the PlankaC
interpreter or bytecode runner.

`asm8086` emits MASM/TASM-style 16-bit source. It contains the full bytecode
image, direct 8086 procedures for the arithmetic core, exported compound heap
areas, a compound procedure table, and ABI entries for list, pair, record,
board, and compound dispatch routines.

## Supported Profile

PlankaC currently supports the notation used in this repository:

- numbered procedure headers, for example `P10 add`;
- `V`, `C`, `Z`, and `R` variables;
- `CONST` declarations for the constant bank;
- structure markers such as `[:32.16]`;
- symbolic type families such as `[:C32.16]`, `[:L32.0]`, `[:S32.0]`,
  `[:P32.0]`, `[:Q32.0]`, `[:VEC32.16]`, and `[:MAT32.16]`;
- indexed variables such as `Z10[0][:32.16]`, `Z10[1,2][:32.16]`,
  and `Z10[1][2][:32.16]`;
- nested record fields such as `Z40.white.king.file[:32.0]`;
- handle-backed structured records through `record_new`, `record_set`,
  `record_get`, `record_has`, `record_size`, `record_set_path2`,
  `record_get_path2`, `record_has_path2`, and `record_shape_equal`;
- frame-local list helpers `list_new`, `list_push`, `list_len`, `list_get`,
  `list_first`, `list_last`, `list_min`, `list_max`, `list_concat`,
  `list_select_greater`, `list_count_equal`, `list_exists_equal`,
  `list_forall_greater`, `list_zip_pairs`, and `list_pair`;
- pair and set helpers such as `pair`, `pair_left`, `set_add`,
  `set_contains`, `set_union`, `set_intersection`, `set_difference`,
  `set_subset`, `set_equal`, `set_cartesian`, `relation_inverse`,
  `relation_image`, `relation_compose`, `relation_select_domain`,
  `relation_select_range`, `relation_exists_range_equal`,
  `relation_forall_domain_greater`, and `relation_signature`;
- richer value helpers such as `list_equal`, `pair_equal`, `record_merge`,
  and `record_equal`;
- chess board helpers such as `chess_square`, `chess_piece`,
  `chess_board_new`, `chess_board_place`, `chess_board_piece`, attack maps,
  legal move counts, promotion, castling-path, en passant, stalemate,
  signatures, and check predicates;
- bit/fixed helpers such as `bit_and`, `bits_pack4`, `fixed_quantize`,
  `fixed_add`, `fixed_mul`, and `fixed_div_checked`;
- modern 3D extension helpers such as `vec3`, `vec3_dot`, `vec3_cross`,
  `mat4_translate`, `mat4_scale`, `mat4_rotate_x`, `mat4_rotate_y`,
  `mat4_rotate_z`, `mat4_mul`, `mat4_transform_point`, and
  `perspective_project`;
- numeric constants with structure markers;
- arithmetic operators `+`, `-`, `*`, `/`, `%`, `^`;
- comparisons `=`, `!=`, `<`, `<=`, `>`, `>=`;
- boolean `&`, `|`, `!`;
- guarded equations of the form `condition => value => target`;
- loop equations of the form `LOOP count => value => target`;
- assertions and contracts through `ASSERT`, `REQUIRE`, and `ENSURE`;
- stop criteria through `STOPIF`;
- direct divide/modulo-by-zero failure plus checked result/status helpers;
- procedure calls with interprocedural marker/type-family checks;
- multiple result values.

The implementation is split into focused modules: core loading/execution,
type-marker handling, two-dimensional notation expansion, static analysis,
backends, targets, and the compact legacy runtime. That is important for this
project because PlankaC is meant to be inspectable as a language tool, not a
single dense demonstration file.

The bytecode format is documented in `docs/plankac_bytecode.md`.
The `.plk` application model and shared host API are documented in
`docs/plk_application_model.md` and `docs/plankahost_api.md`.
