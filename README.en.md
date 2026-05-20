<p align="center">
  <img src="docs/logo.svg" alt="PlankaC logo" width="560">
</p>

# PlankaC: A Compact Plankalkuel Runtime In C

PlankaC is a small parser, interpreter, bytecode writer, bytecode runner, C
backend emitter, native ASM backend, and embedding API for a compact linear
Plankalkuel notation. PlankaMath is the bundled example project: a calculator
plus `.plk` plans for indexed values, nested fields, lists, pairs, sets,
relations, complex values, 3D vectors, 4x4 matrices, projection, loops,
assertions, and chess structures.

## Contents

| Path | Purpose |
| --- | --- |
| `src/` | `.plk` plans in linear Plankalkuel notation |
| `examples/` | small `.plk` sessions |
| `tests/` | self-check programs |
| `c/` | PlankaC modules, classic C runtime, console runner, DOS runner, Win32/Win64 GUI, and Win16 GUI |
| `asm/` | 8086 helper routine |
| `docs/` | syntax, execution model, infographic, source basis, bibliography |

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
| `src/09_complex.plk` | complex values with `[:C32.16]` markers |
| `src/10_relation_algebra.plk` | set difference, subsets, relation projections |
| `src/11_structured_values.plk` | handle-backed records and nested values |
| `src/12_relation_composition.plk` | cartesian products, relation composition, simple quantifiers |
| `src/13_chess_board.plk` | board, piece, and attack-map examples |
| `src/14_two_dimensional_tables.plk` | executable two-dimensional table rows |
| `src/15_3d_geometry.plk` | modern 3D extension with `vec3`, `mat4`, transforms, and projection |
| `src/16_value_algebra.plk` | list, set, pair, record, and relation equality |
| `src/17_chess_model.plk` | board state, piece values, attack maps, and check examples |
| `src/18_two_dimensional_general.plk` | aligned and swapped `V|`/`S|` table rows |
| `c/include/plankac.h` | PlankaC API for C programs and the Windows GUI |
| `c/core/` | parser, interpreter, source loader, and API implementation |
| `c/types/` | structure markers, type families, and compatibility rules |
| `c/notation/` | two-dimensional Plankalkuel table rows |
| `c/analyzer/` | static program checks across procedure boundaries |
| `c/backends/` | bytecode, C backend, x86-64 ASM, and 8086/DOS ASM |
| `c/targets/` | CLI, DOS, Win16, and Windows GUI hosts |
| `c/legacy/plankamath.c` | compact fallback runtime |
| `build-dos.bat` | Open Watcom build for `build\dos\PMDOS.EXE` |
| `build-win16.bat` | Open Watcom build for `build\win16\PlankaMath16.exe` |
| `examples/c_api_demo.c` | small external C program using PlankaC as a library |
| `tests/plankac_conformance.c` | parser/runtime conformance runner |

See `docs/infographic.md` for a short visual map of the project.

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
guards, arithmetic expressions, indexed values, record fields, lists, sets,
relations, complex values, 3D vectors, 4x4 matrices, projection, loops,
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
build\plankac.exe asm8086 build\plankac_8086.asm
```

The exact execution model is described in `docs/execution_model.md`.

The 3D layer is deliberately marked as a modern PlankaC extension. It extends
the implemented profile with vectors, matrices, transforms, and projection
without assigning that extension to the documented Plankalkuel core.

## Requirements

On Windows:

```text
PowerShell or cmd
GCC/MinGW in PATH
```

Open Watcom can also be used for Win16 and DOS targets.

## Build

The easiest path on Windows is:

```bat
build.bat
```

Manual build of the main PlankaC objects:

```powershell
New-Item -ItemType Directory -Force build
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\core\plankac_common.c -o build\plankac_common.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\core\plankac_source.c -o build\plankac_source.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\core\plankac_expr.c -o build\plankac_expr.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\types\plankac_types.c -o build\plankac_types.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\notation\plankac_2d.c -o build\plankac_2d.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\analyzer\plankac_analyzer.c -o build\plankac_analyzer.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\backends\plankac_bytecode.c -o build\plankac_bytecode.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\backends\plankac_asm8086.c -o build\plankac_asm8086.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\core\plankac_runtime.c -o build\plankac_runtime.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\backends\plankac_native_runtime.c -o build\plankac_native_runtime.o
ar rcs build\libplankac.a build\plankac_common.o build\plankac_source.o build\plankac_expr.o build\plankac_types.o build\plankac_2d.o build\plankac_analyzer.o build\plankac_bytecode.o build\plankac_asm8086.o build\plankac_runtime.o build\plankac_native_runtime.o
gcc -Wall -Wextra -std=c89 -Ic\include examples\c_api_demo.c build\libplankac.a -o build\plankac_api_demo.exe -lm
gcc -Wall -Wextra -std=c89 -Ic\include tests\plankac_conformance.c build\libplankac.a -o build\plankac_conformance.exe -lm
```

Manual Windows GUI build:

```powershell
gcc -mwindows build\plankac_common.o build\plankac_types.o build\plankac_2d.o build\plankac_analyzer.o build\plankac_source.o build\plankac_expr.o build\plankac_bytecode.o build\plankac_asm8086.o build\plankac_runtime.o build\plankamath.o build\windows_gui.o -o build\PlankaMath.exe -lm
```

Real Win16 GUI build for Windows 3.x:

```bat
build-win16.bat
```

This requires Open Watcom 1.9 or Open Watcom V2 and writes
`build\win16\PlankaMath16.exe`, a 16-bit Windows NE executable. The target is
meant for Windows 3.x and compatible Win16 environments. It uses the compact
PlankaMath C runtime; the parser, bytecode runner, C backend, and ASM backend
remain part of the modern PlankaC toolchain.

Real 16-bit DOS runner build:

```bat
build-dos.bat
```

This writes `build\dos\PMDOS.EXE`. The short 8.3 filename is intentional so
the executable is comfortable in classic DOS environments without long
filenames.

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
.\build\plankac.exe run three_d_pipeline_session
.\build\plankac.exe bytecode build\plankamath.pbc
.\build\plankac.exe checkbc build\plankamath.pbc
.\build\plankac.exe runbc build\plankamath.pbc set_session
.\build\plankac.exe cgen build\plankac_generated.c
.\build\plankac.exe asmgen build\plankac_asm_runtime.S
.\build\plankac.exe asm8086 build\plankac_8086.asm
```

Expected output:

```text
PlankaC OK: 24 files, 116 procedures
R0=30
R0=0 R1=1
R0=1
R0=120
Bytecode written: build\plankamath.pbc
Bytecode OK: 116 procedures
R0=2
C backend written: build\plankac_generated.c
ASM runtime written: build\plankac_asm_runtime.S
8086 ASM written: build\plankac_8086.asm
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

The GUI loads the `.plk` plans through PlankaC. The legacy C runtime remains
in the build as a fallback path.

Win16 GUI under Windows 3.x:

```text
build\win16\PlankaMath16.exe
```

64-bit Windows does not include the Win16 subsystem. For local testing on a
current Windows machine, run the Win16 build through `otvdm/winevdm`:

```bat
run-win16-otvdm.bat
```

The helper looks for `otvdm` under `tools\otvdm`, `C:\OTVDM`, and `PATH`.

DOS runner:

```text
PMDOS demo
PMDOS tests
PMDOS run add 12 8
PMDOS run divide_checked 84 0
```

On a modern Windows PC, run the DOS target through DOSBox:

```bat
run-dos-dosbox.bat demo
run-dos-dosbox.bat run add 12 8
```

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

## Source Basis

PlankaC is a small tribute to Konrad Zuse's Plankalkuel. The notation and
project structure follow Zuse's Plankalkuel work and later implementation and
analysis literature on the language.

The most useful references for this project are Zuse's 1948 paper, Zuse's
1972 *Der Plankalkuel* report, W. K. Giloi's 1997 analysis, and the 2000
technical report by Rojas, Goektekin, Friedland, Krueger, Langmack, and
Kuniss.

Bibliography:

[`docs/bibliography.md`](docs/bibliography.md)

Archived 2000 technical report:

https://web.archive.org/web/20060501175521/http://www.zib.de/zuse/Inhalt/Programme/Plankalkuel/Plankalkuel-Report/techreport.pdf

## License

MIT. See `LICENSE`. The file includes additional notes for PlankaC,
PlankaMath, the `.plk` examples, the C API, generated artifacts, and
source references.
