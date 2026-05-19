<p align="center">
  <img src="docs/logo.svg" alt="PlankaC logo" width="560">
</p>

# PlankaC: A Small Plankalkuel Runtime In C

PlankaC is a small parser, interpreter, bytecode writer, bytecode runner, C
backend emitter, native ASM backend, and embedding API for a compact linear
Plankalkuel notation. PlankaMath is the bundled example project: a calculator
plus `.plk` plans for indexed values, nested fields, lists, pairs, sets,
loops, assertions, and chess structures.

## Contents

| Path | Purpose |
| --- | --- |
| `src/` | `.plk` plans in linear Plankalkuel notation |
| `examples/` | small `.plk` sessions |
| `tests/` | self-check programs |
| `c/` | PlankaC modules, classic C runtime, console runner, Win32/Win64 GUI, and Win16 GUI |
| `asm/` | 8086 helper routine |
| `docs/` | syntax, execution model, history, bibliography |

Important files:

| File | Purpose |
| --- | --- |
| `src/01_arithmetic.plk` | arithmetic procedures |
| `src/02_order.plk` | comparisons, minimum, maximum |
| `src/03_scientific.plk` | square, power, root, percent helpers |
| `src/04_calculator.plk` | calculator session procedures |
| `src/05_memory.plk` | memory procedures |
| `src/06_data_structures.plk` | indexed values, fields, lists, loops, assertions |
| `src/07_chess.plk` | simple chess predicates as structure examples |
| `src/08_relations_sets.plk` | nested records, pairs, sets, and chess relations |
| `c/plankamath.c` | C execution layer for the `.plk` procedures |
| `c/plankac_*.c` | PlankaC parser, runtime, bytecode, API, and CLI modules |
| `c/plankac.h` | PlankaC API for C programs and the Windows GUI |
| `c/plankamath_cli.c` | console entry point |
| `c/windows_gui.c` | modern Windows GUI with PlankaC integration |
| `c/win16_gui.c` | real Win16 GUI for Windows 3.x |
| `build-win16.bat` | Open Watcom build for `build\win16\PlankaMath16.exe` |
| `examples/c_api_demo.c` | small external C program using PlankaC as a library |
| `tests/plankac_conformance.c` | parser/runtime conformance runner |

## Example

```text
P10 add (V0[:32.16], V1[:32.16]) => R0[:32.16]
(V0[:32.16] + V1[:32.16]) => R0[:32.16]
END
```

The classic PlankaMath C layer mirrors these procedures with matching
functions:

```text
P10 add             => pm_add
P14 divide_checked  => pm_divide_checked
P52 root_checked    => pm_root_checked
P999 all_tests      => pm_all_tests
```

PlankaC is the central path. It reads the `.plk` files, builds a procedure
table, and executes the repository's `.plk` profile directly: assignments,
guards, arithmetic expressions, indexed values, record fields, lists, loops,
assertions, procedure calls, and multi-result procedures such as
`divide_checked`.

It can also emit a readable bytecode file, load it again, and run it. PlankaC
can also emit generated C and native x86-64 assembly runners from the loaded
`.plk` profile:

```text
build\plankac.exe bytecode build\plankamath.pbc
build\plankac.exe checkbc build\plankamath.pbc
build\plankac.exe runbc build\plankamath.pbc set_session
build\plankac.exe cgen build\plankac_generated.c
build\plankac.exe asmgen build\plankac_asm_runtime.S
```

The exact execution model is described in `docs/execution_model.md`.

## Requirements

On Windows:

```text
PowerShell or cmd
GCC/MinGW in PATH
```

Open Watcom can also be used for experiments with older Windows targets.

## Build

The easiest path on Windows is:

```bat
build.bat
```

Manual console build:

```powershell
New-Item -ItemType Directory -Force build
gcc -Wall -Wextra -std=c89 c\plankamath.c c\plankamath_cli.c -o build\plankamath_cli.exe -lm
gcc -Wall -Wextra -std=c89 c\plankac_common.c c\plankac_source.c c\plankac_expr.c c\plankac_bytecode.c c\plankac_runtime.c c\plankac_cli.c -o build\plankac.exe -lm
gcc -Wall -Wextra -std=c89 -c c\plankac_common.c -o build\plankac_common.o
gcc -Wall -Wextra -std=c89 -c c\plankac_source.c -o build\plankac_source.o
gcc -Wall -Wextra -std=c89 -c c\plankac_expr.c -o build\plankac_expr.o
gcc -Wall -Wextra -std=c89 -c c\plankac_bytecode.c -o build\plankac_bytecode.o
gcc -Wall -Wextra -std=c89 -c c\plankac_runtime.c -o build\plankac_runtime.o
gcc -Wall -Wextra -std=c89 -c c\plankac_native_runtime.c -o build\plankac_native_runtime.o
ar rcs build\libplankac.a build\plankac_common.o build\plankac_source.o build\plankac_expr.o build\plankac_bytecode.o build\plankac_runtime.o build\plankac_native_runtime.o
gcc -Wall -Wextra -std=c89 examples\c_api_demo.c build\libplankac.a -o build\plankac_api_demo.exe -lm
gcc -Wall -Wextra -std=c89 tests\plankac_conformance.c build\libplankac.a -o build\plankac_conformance.exe -lm
```

Manual Windows GUI build:

```powershell
gcc -Wall -Wextra -std=c89 c\plankac_common.c c\plankac_source.c c\plankac_expr.c c\plankac_bytecode.c c\plankac_runtime.c c\plankamath.c c\windows_gui.c -mwindows -o build\PlankaMath.exe -lm
```

Real Win16 GUI build for Windows 3.x:

```bat
build-win16.bat
```

This requires Open Watcom 1.9 or Open Watcom V2 and writes
`build\win16\PlankaMath16.exe`. That target is not a modern window with an old
look; it is a Win16 program. Because the full PlankaC parser tables are too
large for a practical Win16 host, this target uses the compact PlankaMath C
mapping of the `.plk` calculator procedures. The full PlankaC parser and
backends remain the modern toolchain path.

## Run

Source check:

```powershell
.\build\plankamath_cli.exe compile
```

Expected output:

```text
Compile OK: 14 files, 62 procedures
```

Demo:

```powershell
.\build\plankamath_cli.exe demo
```

Expected output:

```text
30
```

Self-checks:

```powershell
.\build\plankamath_cli.exe tests
```

Expected output:

```text
1
```

Guarded division example:

```powershell
.\build\plankamath_cli.exe guarded
```

Expected output:

```text
0, 1
```

Run the `.plk` interpreter:

```powershell
.\build\plankac.exe check
.\build\plankac.exe run calculator_demo
.\build\plankac.exe run divide_checked 84 0
.\build\plankac.exe tests
.\build\plankac.exe bytecode build\plankamath.pbc
.\build\plankac.exe checkbc build\plankamath.pbc
.\build\plankac.exe runbc build\plankamath.pbc set_session
.\build\plankac.exe cgen build\plankac_generated.c
.\build\plankac.exe asmgen build\plankac_asm_runtime.S
```

Expected output:

```text
PlankaC OK: 14 files, 62 procedures
R0=30
R0=0 R1=1
R0=1
Bytecode written: build\plankamath.pbc
Bytecode OK: 62 procedures
R0=2
C backend written: build\plankac_generated.c
ASM runtime written: build\plankac_asm_runtime.S
R0=2
R0=0 R1=1
R0=12
R0=2
R0=1
```

GUI:

```powershell
.\build\PlankaMath.exe
```

The GUI loads the `.plk` plans through PlankaC. The older C runtime remains in
the build as a fallback path.

Win16 GUI under Windows 3.x:

```text
build\win16\PlankaMath16.exe
```

This target is intended for a Windows 3.x / Win16 environment.
It will not start by double-clicking on Windows 10/11 x64 because 64-bit
Windows does not run Win16 programs directly. To test it on a modern PC, use
`otvdm/winevdm`:

```bat
run-win16-otvdm.bat
```

The script looks for `otvdm` in `tools\otvdm`, `C:\OTVDM`, or `PATH`.

## Use PlankaC From C

PlankaC can be embedded in another C program:

```c
#include "plankac.h"

PLANKAC_CONTEXT *ctx;
PLANKAC_RESULT result;
double args[2];
char err[256];

ctx = plankac_create();
plankac_context_load_default(ctx, err, sizeof(err));
args[0] = 12.0;
args[1] = 8.0;
plankac_context_run(ctx, "add", args, 2, &result, err, sizeof(err));
plankac_destroy(ctx);
```

See [`docs/plankac_api.md`](docs/plankac_api.md).

## Conformance

Parser and runtime edge cases are covered by:

```powershell
.\build\plankac_conformance.exe
```

See [`docs/conformance.md`](docs/conformance.md) and
[`docs/plankac_bytecode.md`](docs/plankac_bytecode.md). The broader language
coverage matrix is in [`docs/plankalkuel_coverage.md`](docs/plankalkuel_coverage.md).

## Historical Basis

PlankaC is a small tribute to Konrad Zuse's Plankalkuel. The notation and
project structure are based on historical descriptions of Plankalkuel and
later work on implementing the language.

The most useful references for this project are Zuse's 1948 paper, Zuse's
1972 *Der Plankalkuel* report, W. K. Giloi's 1997 historical analysis, and the
2000 technical report by Rojas, Goektekin, Friedland, Krueger, Langmack, and
Kuniss.

Bibliography:

[`docs/bibliography.md`](docs/bibliography.md)

Archived 2000 technical report:

https://web.archive.org/web/20060501175521/http://www.zib.de/zuse/Inhalt/Programme/Plankalkuel/Plankalkuel-Report/techreport.pdf

## License

MIT. See `LICENSE`. The file includes additional notes for PlankaC,
PlankaMath, the `.plk` examples, the C API, generated artifacts, and
historical references.
