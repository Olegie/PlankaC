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
tests/calculator_self_check.plk
```

They describe calculator procedures with numbered programs, `V` input
variables, `Z` intermediate variables, `R` result variables, type markers,
guarded equations, indexed values, nested fields, lists, pairs, sets, loops,
assertions, and a small set of chess-style structure examples.

## Two Execution Paths

The project now has two C execution paths.

The older path is the small runtime mirror:

```text
c/plankamath.c
c/windows_gui.c
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
lists, pairs, sets, loops, assertions, structure markers, direct
interprocedural marker checks, and multi-result procedures.

The Windows GUI now uses PlankaC first. Its function list is loaded from the
`.plk` procedure table, and calculator buttons call procedures such as
`add`, `subtract`, `multiply`, `divide_checked`, `root_checked`, and `square`
through PlankaC. If the `.plk` load fails, the GUI can still use the older C
runtime.

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
```

The generated ASM runner does not call `plankac_context_run` or the bytecode
loader. It contains native procedure functions and links a small helper runtime
for frame storage, lists, sets, pairs, math primitives, and formatting.

## Honest Project Statement

Use this statement when presenting the project:

```text
The source plans are written in .plk files using linear Plankalkuel notation.
PlankaC reads and executes the repository's .plk profile directly; the
PlankaMath GUI is one bundled host application and keeps the compact C runtime
as a fallback.
```

## PlankaC Scope

PlankaC v0.1 is deliberately small. It is useful because it checks and runs the
actual `.plk` source files. Its scope is the executable profile used in this
repository, not the whole historical Plankalkuel system.

```text
build/plankac.exe check
build/plankac.exe run calculator_demo
build/plankac.exe run divide_checked 84 0
build/plankac.exe tests
build/plankac.exe bytecode build/plankamath.pbc
build/plankac.exe checkbc build/plankamath.pbc
build/plankac.exe runbc build/plankamath.pbc set_session
build/plankac.exe cgen build/plankac_generated.c
build/plankac.exe asmgen build/plankac_asm_runtime.S
```

Expected result:

```text
PlankaC OK: 14 files, 62 procedures
R0=30
R0=0 R1=1
R0=1
Bytecode written: build/plankamath.pbc
Bytecode OK: 62 procedures
R0=2
C backend written: build/plankac_generated.c
ASM runtime written: build/plankac_asm_runtime.S
```
