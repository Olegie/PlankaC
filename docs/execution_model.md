# Execution Model

PlankaC is honest about what runs and what is specified.

## What Is The .plk Source

The `.plk` files are the main source/specification layer of the project.

Examples:

```text
src/01_arithmetic.plk
src/02_order.plk
src/03_scientific.plk
src/04_calculator.plk
src/08_relations_sets.plk
src/09_complex.plk
src/10_relation_algebra.plk
src/11_structured_values.plk
src/12_relation_composition.plk
src/13_chess_board.plk
src/14_two_dimensional_tables.plk
src/15_3d_geometry.plk
src/16_value_algebra.plk
src/17_chess_model.plk
src/18_two_dimensional_general.plk
src/19_language_closure.plk
src/20_page_table.plk
src/21_chess_game.plk
src/22_predicate_schema.plk
src/23_chess_complete.plk
tests/calculator_self_check.plk
```

They describe calculator procedures with numbered programs, `V` input
variables, `C` constants, `Z` intermediate variables, `R` result variables, type markers,
guarded equations, indexed values, nested fields, handle-backed records,
lists, pairs, sets, relation composition, value algebra helpers, chess board
state, 3D vectors, 4x4 matrices, rotation, projection, loops, assertions,
contracts, stop criteria, bit/fixed helpers, arithmetic exception helpers,
executable two-dimensional table rows, relation quantifiers, schema
signatures, game-state chess procedures, and chess-style structure examples.

## Two Execution Paths

The project has two C execution paths.

The legacy path is the compact runtime mirror:

```text
c/legacy/plankamath.c
c/targets/windows_gui.c
```

The C runtime mirrors the `.plk` procedures manually:

```text
P10 add              -> pm_add
P14 divide_checked   -> pm_divide_checked
P52 root_checked     -> pm_root_checked
P999 all_tests       -> pm_all_tests
```

The GUI keeps this runtime linked as a fallback path.

The central path is PlankaC. Its modules read the `.plk` files, build a
procedure table, and execute the repository's `.plk` profile directly. It
handles assignments, guarded equations, arithmetic expressions, procedure
calls, `V`/`C`/`Z`/`R` variables, indexed slots, nested record fields, frame-local
lists, pairs, sets, relation predicates, relation composition, relation
inverse/image helpers, complex values, 3D vector handles, 4x4 matrix handles,
rotation helpers, structured record handles, chess board records, loops,
assertions, contracts, stop criteria, bit/fixed helpers, structure markers,
interprocedural marker/type-family checks, shared handle heaps, executable
two-dimensional rows, and multi-result procedures.
The two-dimensional notation layer builds a row/cell document model for
`PAGE` blocks before expansion, so diagnostics and alignment checks are tied
to document coordinates rather than only to linear strings.

The Windows GUI uses PlankaC first. Its function list is loaded from the
`.plk` procedure table, and calculator buttons call procedures such as
`add`, `subtract`, `multiply`, `divide_checked`, `root_checked`, and `square`
through PlankaC. If the `.plk` load fails, the GUI can still use the legacy C
runtime.

PlankaHost is the shared application path. It loads the standard PlankaC
profile, then one application file such as `graphics/src/plankagui.plk` or
`graphics/src/plankacube.plk`. The same context contains the older calculator
procedures, the structured-value and relation procedures, chess procedures,
3D helpers, and the selected application procedures. `PlankaHost.exe` uses
`app_kind`, `app_canvas`, `app_checksum`, and `app_timer_step` to choose and
drive the host surface. For the cube profile, vertices, edge topology, model
matrices, and projection values are evaluated from `.plk` procedures.

PlankaC also has compiler-output modes. It can emit a textual IR/bytecode file
with procedure, expression, call, guarded-call, loop, and assertion operations,
load that bytecode back, run it, emit generated C, emit native x86-64
assembly, emit 8086 assembly source, and link native executables through an
external C toolchain:

```text
build/plankac.exe compile build/plankac_pipeline
build/plankac.exe native-c build/plankac_native_c
build/plankac.exe native-asm build/plankac_native_asm
build/plankac.exe bytecode build/plankamath.pbc
build/plankac.exe checkbc build/plankamath.pbc
build/plankac.exe runbc build/plankamath.pbc set_session
build/plankac.exe lowering build/plankac.lowering
build/plankac.exe cgen build/plankac_generated.c
build/plankac.exe asmgen build/plankac_asm_runtime.S
build/plankac.exe asm8086 build/plankac_8086.asm
```

The `compile` command is the stable route: `.plk` source is emitted to
`.pbc`, the `.pbc` is loaded back as IR, and C/ASM artifacts are emitted from
that reloaded program.
The `lowering` command writes the backend plan for scalar expressions,
procedure calls, contracts, control statements, and compound runtime entry
points.

The generated ASM runner does not call `plankac_context_run` or the bytecode
loader. It contains native procedure functions and links a helper runtime
for frame storage, lists, sets, relations, pairs, complex values, math
primitives, and formatting.

The 8086/DOS backend is a separate emitter. It writes MASM/TASM-style 16-bit
source containing the loaded program as a bytecode image and direct 8086 near
procedures for the arithmetic core. Compound-value procedures are present in
the image, listed in the compound procedure table, and exposed through a
documented 16-bit ABI surface for list, pair, record, board, and compound
dispatch routines.

The DOS COM backend is a narrower binary emitter under `c/backends/dos`. It
writes a small 8086 `.COM` bootstrap directly, without an external assembler,
and appends the generated PlankaC bytecode image. Full DOS execution is handled
by the `PLANKACD.EXE` target built with `build-dos-plankac.bat`; the same
source is verified by the modern `plankacd_host.exe` build.

## Honest Project Statement

Use this statement when presenting the project:

```text
The source plans are written in .plk files using linear Plankalkuel notation.
PlankaC reads and executes the repository's .plk profile directly; the
PlankaHost layer can load additional .plk application profiles, and the
PlankaMath GUI keeps the compact C runtime as a fallback.
```

## PlankaC Scope

PlankaC 1.1.0 is a substantial executable Plankalkuel-profile implementation
in C. It checks and runs the actual `.plk` source files, exposes a C embedding
API, writes bytecode and typed IR, emits generated C, generated x86-64 ASM,
DOS-oriented 8086 ASM source, DOS COM bootstrap binaries, a PlankaC DOS runner,
and native executable builds through the compiler pipeline. Its implementation is split across core loading/execution, types,
notation, analyzer, values, IR, backend, target, graphics, and legacy modules.
Its scope is the executable profile used in this repository; the coverage
matrix records the exact feature surface and backend boundaries.

```text
build/plankac.exe check
build/plankac.exe run calculator_demo
build/plankac.exe run divide_checked 84 0
build/plankac.exe tests
build/plankac.exe run three_d_pipeline_session
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
build/plankac.exe native-asm build/plankac_native_asm
```

Expected result:

```text
PlankaC OK: 29 files, 150 procedures
R0=30
R0=0 R1=1
R0=1
R0=120
Bytecode written: build/plankamath.pbc
Bytecode OK: 150 procedures
R0=2
IR written: build/plankac.ir
Lowering written: build/plankac.lowering
C backend written: build/plankac_generated.c
ASM runtime written: build/plankac_asm_runtime.S
8086 ASM written: build/plankac_8086.asm
```
