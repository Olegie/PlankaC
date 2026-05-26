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
tests/calculator_self_check.plk
```

They describe calculator procedures with numbered programs, `V` input
variables, `Z` intermediate variables, `R` result variables, type markers,
guarded equations, indexed values, nested fields, handle-backed records,
lists, pairs, sets, relation composition, value algebra helpers, chess board
state, 3D vectors, 4x4 matrices, rotation, projection, loops, assertions, executable
two-dimensional table rows, and chess-style structure examples.

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
calls, `V`/`Z`/`R` variables, indexed slots, nested record fields, frame-local
lists, pairs, sets, relation predicates, relation composition, relation
inverse/image helpers, complex values, 3D vector handles, 4x4 matrix handles,
rotation helpers, structured record handles, chess board records, loops,
assertions, structure markers,
interprocedural marker/type-family checks, shared handle heaps, executable
two-dimensional rows, and multi-result procedures.

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

PlankaC also has compiler-output modes. It can emit a textual bytecode file
with procedure, expression, call, guarded-call, loop, and assertion operations,
load that bytecode back, run it, emit a generated C runner, and emit a native
x86-64 assembly runner:

```text
build/plankac.exe bytecode build/plankamath.pbc
build/plankac.exe checkbc build/plankamath.pbc
build/plankac.exe runbc build/plankamath.pbc set_session
build/plankac.exe cgen build/plankac_generated.c
build/plankac.exe asmgen build/plankac_asm_runtime.S
build/plankac.exe asm8086 build/plankac_8086.asm
```

The generated ASM runner does not call `plankac_context_run` or the bytecode
loader. It contains native procedure functions and links a small helper runtime
for frame storage, lists, sets, relations, pairs, complex values, math
primitives, and formatting.

The 8086/DOS backend is a separate emitter. It writes MASM/TASM-style 16-bit
source containing the loaded program as a bytecode image and direct 8086 near
procedures for the arithmetic core. Wider compound-value procedures are
present in the image and exposed as guarded stubs until a full 16-bit compound
runtime exists.

## Honest Project Statement

Use this statement when presenting the project:

```text
The source plans are written in .plk files using linear Plankalkuel notation.
PlankaC reads and executes the repository's .plk profile directly; the
PlankaHost layer can load additional .plk application profiles, and the
PlankaMath GUI keeps the compact C runtime as a fallback.
```

## PlankaC Scope

PlankaC v0.1 is a compact implementation of the repository's executable
language profile. It checks and runs the actual `.plk` source files, exposes a
C embedding API, writes bytecode, and can emit generated C, native x86-64
ASM, or DOS-oriented 8086 ASM source. Its implementation is split across core
loading/execution, types, notation, analyzer, backend, target, and legacy
modules. Its scope is the executable profile used in this repository; the
coverage matrix records which parts of the wider Plankalkuel source model are
implemented and which parts remain future work.

```text
build/plankac.exe check
build/plankac.exe run calculator_demo
build/plankac.exe run divide_checked 84 0
build/plankac.exe tests
build/plankac.exe run three_d_pipeline_session
build/plankac.exe bytecode build/plankamath.pbc
build/plankac.exe checkbc build/plankamath.pbc
build/plankac.exe runbc build/plankamath.pbc set_session
build/plankac.exe cgen build/plankac_generated.c
build/plankac.exe asmgen build/plankac_asm_runtime.S
build/plankac.exe asm8086 build/plankac_8086.asm
```

Expected result:

```text
PlankaC OK: 24 files, 118 procedures
R0=30
R0=0 R1=1
R0=1
R0=120
Bytecode written: build/plankamath.pbc
Bytecode OK: 118 procedures
R0=2
C backend written: build/plankac_generated.c
ASM runtime written: build/plankac_asm_runtime.S
8086 ASM written: build/plankac_8086.asm
```
