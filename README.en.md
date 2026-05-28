<p align="center">
  <img src="docs/logo.svg" alt="PlankaC logo" width="560">
</p>

# PlankaC: A Plankalkuel-Profile Toolchain In C

[![CI](https://github.com/Olegie/PlankaC/actions/workflows/ci.yml/badge.svg)](https://github.com/Olegie/PlankaC/actions/workflows/ci.yml)

PlankaC is a substantial executable Plankalkuel-profile implementation in C:
parser, interpreter, AST layer, typed IR, bytecode writer, bytecode runner,
C backend emitter, native ASM backend, 8086 ASM emitter, and embedding API
with typed results for a linear Plankalkuel notation. PlankaMath is the
bundled example profile: a calculator
plus `.plk` plans for indexed values, nested fields, lists, pairs, sets,
relations, complex values, 3D vectors, 4x4 matrices, rotation, projection,
loops, assertions, and chess structures.

## Contents

| Path | Purpose |
| --- | --- |
| `src/` | `.plk` plans in linear Plankalkuel notation |
| `examples/` | focused `.plk` sessions |
| `tests/` | self-check programs |
| `c/` | PlankaC modules, classic C runtime, console runner, DOS runner, Win32/Win64 GUI, and Win16 GUI |
| `graphics/` | PlankaGUI: graphical interfaces from `.plk` procedures |
| `asm/` | 8086 helper routine |
| `docs/` | language reference, standard library, compiler guide, examples, porting notes, execution model |
| `docs/research/` | reproducible research track and evidence protocol |

Important files:

| File | Purpose |
| --- | --- |
| `src/01_arithmetic.plk` | arithmetic procedures |
| `src/02_order.plk` | comparisons, minimum, maximum |
| `src/03_scientific.plk` | square, power, root, percent helpers |
| `src/04_calculator.plk` | calculator session procedures |
| `src/05_memory.plk` | memory procedures |
| `src/06_data_structures.plk` | indexed values, fields, lists, loops, assertions |
| `src/07_chess.plk` | chess predicates as structure examples |
| `src/08_relations_sets.plk` | nested records, pairs, sets, and chess relations |
| `src/09_complex.plk` | complex values with `[:C32.16]` markers |
| `src/10_relation_algebra.plk` | set difference, subsets, relation projections |
| `src/11_structured_values.plk` | handle-backed records and nested values |
| `src/12_relation_composition.plk` | cartesian products, relation composition, simple quantifiers |
| `src/13_chess_board.plk` | board, piece, and attack-map examples |
| `src/14_two_dimensional_tables.plk` | executable two-dimensional table rows |
| `src/15_3d_geometry.plk` | modern 3D extension with `vec3`, `mat4`, rotation, transforms, and projection |
| `src/16_value_algebra.plk` | list, set, pair, record, and relation equality |
| `src/17_chess_model.plk` | board state, piece values, attack maps, and check examples |
| `src/18_two_dimensional_general.plk` | aligned and swapped `V|`/`S|` table rows |
| `src/19_language_closure.plk` | C bank, multidimensional indices, bit/fixed values, contracts, list/relation selection, and board moves |
| `src/20_page_table.plk` | page/table notation, predicate syntax, bit-native value checks, legal moves, material search, and checkmate session |
| `src/21_chess_game.plk` | game-level chess procedures: legal move count, castling path, promotion, and position signature |
| `src/22_predicate_schema.plk` | relation range selection, relation quantifiers, and deterministic schema signatures |
| `src/23_chess_complete.plk` | en passant checks, stalemate checks, FEN-style signatures, and move-history lists |
| `c/include/plankac.h` | PlankaC API for C programs and the Windows GUI |
| `c/core/` | parser, interpreter, source loader, and API implementation |
| `c/types/` | structure markers, type families, and compatibility rules |
| `c/notation/` | two-dimensional Plankalkuel table rows, document validation, and page/table expansion |
| `c/analyzer/` | static program checks, interprocedure checks, and structural schema inference |
| `c/values/` | bit packing, tagged PLC values, and fixed-point raw-value helpers |
| `c/ir/` | AST, typed IR, and evidence data for compiler and backend boundaries |
| `c/ir/plankac_evidence.c` | deterministic evidence packets for source/profile/backend stability |
| `c/models/` | board-level chess legality, legal move count, castling path, promotion, en passant, stalemate, signatures, material, capture search, and mate checks |
| `c/backends/` | bytecode, lowering report, C backend, x86-64 ASM, and 8086/DOS ASM |
| `c/targets/` | CLI, DOS, Win16, and Windows GUI hosts |
| `c/legacy/plankamath.c` | compact fallback runtime |
| `graphics/src/plankagui.plk` | window, button, list, and palette procedures for graphical output |
| `graphics/src/plankacube.plk` | 3D scene profile with cube topology, matrix transform, and projection procedures |
| `graphics/c/plankahost.h` | shared host API for `.plk` applications |
| `graphics/c/` | PlankaHost, PlankaGUI, PlankaCube, raster, font, export, and render layer |
| `build-dos.bat` | Open Watcom build for `build\dos\PMDOS.EXE` |
| `build-win16.bat` | Open Watcom build for `build\win16\PlankaMath16.exe` |
| `examples/c_api_demo.c` | compact external C program using PlankaC as a library |
| `examples/c_abi_demo.c` | bidirectional ABI example: C registers a function and `.plk` calls it |
| `examples/host_abi.plk` | source procedure that calls a registered C function |
| `tests/plankac_conformance.c` | parser/runtime conformance runner |

See `docs/index.md` for the complete documentation map.
See `docs/architecture.md` for the layer map and dependency boundaries.
See `docs/infographic.md` for a short visual map of the project.
The `.plk` application model is described in `docs/plk_application_model.md`;
the host API is described in `docs/plankahost_api.md`.

## Documentation

| Guide | Scope |
| --- | --- |
| [`Formal Specification`](docs/spec/index.md) | grammar, value model, type rules, execution rules, errors, and backend contract |
| [`Language Reference`](docs/language_reference.md) | source files, procedures, banks, markers, expressions, calls, guards, loops, assertions, indexed references, and executable two-dimensional rows |
| [`Standard Library`](docs/standard_library.md) | bundled `.plk` procedure profile: arithmetic, calculator sessions, data structures, relations, complex values, chess structures, 3D geometry, and application profiles |
| [`Compiler Guide`](docs/compiler_guide.md) | `plankac.exe` commands, bytecode, generated C, native ASM, 8086 ASM, build artifacts, and verification commands |
| [`Compiler Pipeline`](docs/compiler_pipeline.md) | stable `.plk -> IR/bytecode -> lowering -> C/ASM -> native executable` route |
| [`Examples`](docs/examples.md) | runnable source and command examples for the implemented language profile |
| [`Porting Guide`](docs/porting_guide.md) | embedding, platform targets, Win16/DOS limits, PlankaHost integration, and backend rules |
| [`ABI And Embedding API`](docs/abi.md) | C hosts calling PlankaC procedures and `.plk` source calling registered C functions |
| [`Max3 Tutorial`](docs/tutorials/max3_to_native.md) | one `.plk` file through interpreter, bytecode, generated C, generated ASM, and native runners |
| [`Technical Report`](docs/technical_report.md) | English/German report on scope, architecture, source basis, backends, and verification |
| [`Research Track`](docs/research/README.md) | testable hypotheses, evidence protocol, and failure conditions |
| [`Release Guide`](docs/release_guide.md) | versioning, tags, release assets, checksums, and binary policy |
| [`German Project Page`](docs/de/plankac.md) | calm German description of the project and its boundaries |
| [`External Review Request`](docs/review_request.md) | text for asking language/history people for technical review |

## PlankaHost

The preferred graphical entry point is `PlankaHost.exe`. The host loads the
standard library from `src/` and then one `.plk` application. The application
declares its type through `app_kind` and exposes common procedures such as
`app_canvas`, `app_checksum`, and `app_timer_step`.

```powershell
.\build\PlankaHost.exe graphics\src\plankagui.plk
.\build\PlankaHost.exe graphics\src\plankacube.plk
```

`PlankaGUI.exe` and `PlankaCube.exe` remain as direct compatibility launchers,
but the shared host API is `PlankaHost`.

For embedded programs, `graphics/c/plankahost.h` is the practical entry
point. `plankahost_open` loads the standard PlankaC base profile, the older
calculator procedures, data-structure, relation, chess, and 3D procedures,
and then the selected application profile. A host can list procedures, look
them up by name, and execute them with `plankahost_run`. Application behavior
stays in `.plk`; C remains the boundary for windows, timers, mouse and
keyboard input, and raster output. `plankahost_demo.exe` checks the same path
without opening a window.

## PlankaGUI

PlankaGUI describes a compact calculator window in `.plk`: canvas, window
frame, status row, display, procedure list, argument fields, keypad grid, and
palette are returned by executable PlankaC procedures. The C code is split
into focused modules for loading, rasterization, bitmap text, export, and scene
composition. The PNG file is only the README/test reference image; the
interface itself is described by Plankalkuel procedures.
`build\PlankaGUI.exe` opens the scene as a Windows window.
The window is clickable and scales the rendered scene on resize while keeping
the scene aspect ratio. Button hits are resolved through the button rectangles
defined in `.plk`; calculator operations run through PlankaC procedures such
as `add`, `multiply`, `divide_checked`, `square`, and `root_checked`.

<p align="center">
  <img src="graphics/examples/plankagui.png" alt="PlankaGUI rendered interface" width="640">
</p>

## PlankaCube

PlankaCube is a compact 3D profile on the same graphics layer. Its `.plk`
source defines the canvas, viewport, palette, cube vertices, edge list,
model matrix, and perspective projection. The host provides the window timer
and raster output; geometry and projection values come from PlankaC
procedures such as `cube_vertex`, `cube_edge`, `cube_model_transform`, and
`cube_project_vertex`.

`build\PlankaCube.exe` opens a Windows window with a rotating cube. Click or
press Space to pause the animation. The export path uses the same renderer:

```powershell
.\build\plankacube_export.exe graphics\examples\plankacube.png
```

Another 3D profile can run without new C code as long as the file implements
the same `cube_*` contract:

```powershell
.\build\PlankaCube.exe graphics\src\plankacube.plk
.\build\plankac.exe runfile graphics\src\plankacube.plk cube_scene_checksum
```

<p align="center">
  <img src="graphics/examples/plankacube.png" alt="PlankaCube rendered 3D scene" width="640">
</p>

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
relations, complex values, 3D vectors, 4x4 matrices, rotation, projection, loops,
assertions, procedure calls, and multi-result procedures such as
`divide_checked`.

It also has a compiler pipeline: source is emitted as readable AST, typed IR,
and bytecode, the bytecode is loaded back, and C/ASM artifacts are emitted from
that boundary. The native commands link generated C or generated x86-64
assembly into executable programs:

```text
build\plankac.exe ast build\plankac.ast
build\plankac.exe ir build\plankac.ir
build\plankac.exe compile build\plankac_pipeline
build\plankac.exe native-c build\plankac_native_c
build\plankac_native_c.exe set_session
build\plankac.exe native-asm build\plankac_native_asm
build\plankac_native_asm.exe add 12 8
```

The exact execution model is described in `docs/execution_model.md`.

For research and external review, PlankaC can emit a deterministic evidence
packet for the loaded source profile:

```text
build\plankac.exe evidence build\plankac.evidence.json
build\plankac.exe evidencefile examples\max3.plk build\max3.evidence.json
```

The packet records the procedure catalog, V/C/Z/R bank usage, type-family
counts, statement classes, backend contract surface, and a source-profile
fingerprint. The protocol is described in `docs/research/evidence_protocol.md`.

The 3D layer is marked as a PlankaC extension. It extends
the implemented profile with vectors, matrices, rotations, transforms, and projection
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

On Linux:

```bash
make ci
```

Manual build of the main PlankaC objects:

```powershell
New-Item -ItemType Directory -Force build
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\core\plankac_common.c -o build\plankac_common.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\core\plankac_source.c -o build\plankac_source.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\core\plankac_expr.c -o build\plankac_expr.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\types\plankac_types.c -o build\plankac_types.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\notation\plankac_2d.c -o build\plankac_2d.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\notation\plankac_document.c -o build\plankac_document.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\notation\plankac_page.c -o build\plankac_page.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\analyzer\plankac_analyzer.c -o build\plankac_analyzer.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\analyzer\plankac_schema.c -o build\plankac_schema.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\values\plankac_bits.c -o build\plankac_bits.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\values\plankac_value.c -o build\plankac_value.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\models\plankac_chess_model.c -o build\plankac_chess_model.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\ir\plankac_ast.c -o build\plankac_ast.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\ir\plankac_ir.c -o build\plankac_ir.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\ir\plankac_evidence.c -o build\plankac_evidence.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\backends\plankac_lowering.c -o build\plankac_lowering.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\backends\plankac_bytecode.c -o build\plankac_bytecode.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\backends\plankac_asm8086.c -o build\plankac_asm8086.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\core\plankac_runtime.c -o build\plankac_runtime.o
gcc -Wall -Wextra -std=c89 -Ic\include -Ic\internal -c c\backends\plankac_native_runtime.c -o build\plankac_native_runtime.o
ar rcs build\libplankac.a build\plankac_common.o build\plankac_types.o build\plankac_2d.o build\plankac_document.o build\plankac_page.o build\plankac_analyzer.o build\plankac_schema.o build\plankac_bits.o build\plankac_value.o build\plankac_chess_model.o build\plankac_ast.o build\plankac_ir.o build\plankac_evidence.o build\plankac_lowering.o build\plankac_source.o build\plankac_expr.o build\plankac_bytecode.o build\plankac_asm8086.o build\plankac_runtime.o build\plankac_native_runtime.o
gcc -Wall -Wextra -std=c89 -Ic\include examples\c_api_demo.c build\libplankac.a -o build\plankac_api_demo.exe -lm
gcc -Wall -Wextra -std=c89 -Ic\include examples\c_abi_demo.c build\libplankac.a -o build\plankac_abi_demo.exe -lm
gcc -Wall -Wextra -std=c89 -Ic\include tests\plankac_conformance.c build\libplankac.a -o build\plankac_conformance.exe -lm
```

Manual Windows GUI build:

```powershell
gcc -mwindows build\plankac_common.o build\plankac_types.o build\plankac_2d.o build\plankac_document.o build\plankac_page.o build\plankac_analyzer.o build\plankac_schema.o build\plankac_bits.o build\plankac_value.o build\plankac_chess_model.o build\plankac_ast.o build\plankac_ir.o build\plankac_evidence.o build\plankac_lowering.o build\plankac_source.o build\plankac_expr.o build\plankac_bytecode.o build\plankac_asm8086.o build\plankac_runtime.o build\plankamath.o build\windows_gui.o -o build\PlankaMath.exe -lm
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

Primary PlankaC source check:

```powershell
.\build\plankac.exe check
```

Expected output:

```text
PlankaC OK: 29 files, 150 procedures
```

Compact fallback runner check:

```powershell
.\build\plankamath_cli.exe compile
```

This command reports the narrower compatibility profile used by the compact
PlankaMath runner. The full language profile is the PlankaC profile shown
above.

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
.\build\plankac.exe runfile graphics\src\plankacube.plk cube_scene_checksum
.\build\plankac.exe checkfile examples\max3.plk
.\build\plankac.exe runfile examples\max3.plk max3_demo
.\build\plankac.exe bytecode build\plankamath.pbc
.\build\plankac.exe checkbc build\plankamath.pbc
.\build\plankac.exe runbc build\plankamath.pbc set_session
.\build\plankac.exe ast build\plankac.ast
.\build\plankac.exe astfile examples\max3.plk build\max3.ast
.\build\plankac.exe ir build\plankac.ir
.\build\plankac.exe irfile examples\max3.plk build\max3.ir
.\build\plankac.exe cgen build\plankac_generated.c
.\build\plankac.exe asmgen build\plankac_asm_runtime.S
.\build\plankac.exe asm8086 build\plankac_8086.asm
.\build\plankac.exe compile build\plankac_pipeline
.\build\plankac.exe native-c build\plankac_native_c
.\build\plankac_native_c.exe set_session
.\build\plankac.exe native-asm build\plankac_native_asm
.\build\plankac_native_asm.exe add 12 8
```

Expected output:

```text
PlankaC OK: 29 files, 150 procedures
R0=30
R0=0 R1=1
R0=1
R0=120
R0=2403.500000
PlankaC file OK: 30 files, 153 procedures
R0=9
Bytecode written: build\plankamath.pbc
Bytecode OK: 150 procedures
R0=2
AST written: build\plankac.ast
AST written: build\max3.ast
IR written: build\plankac.ir
IR written: build\max3.ir
C backend written: build\plankac_generated.c
ASM runtime written: build\plankac_asm_runtime.S
8086 ASM written: build\plankac_8086.asm
Compiler pipeline OK
native-c: build\plankac_native_c.exe
R0=2
Compiler pipeline OK
native-asm: build\plankac_native_asm.exe
R0=20
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

3D GUI:

```powershell
.\build\PlankaCube.exe
```

The window reads `graphics\src\plankacube.plk`, calls the cube and projection
procedures through PlankaC, and continuously renders the scene into a Windows
raster buffer.

Shared host:

```powershell
.\build\PlankaHost.exe graphics\src\plankagui.plk
.\build\PlankaHost.exe graphics\src\plankacube.plk
```

The same host also loads the base procedures from `src/`. Pressing `+`, `*`,
`/`, `ROOT`, or `X^2` therefore calls a procedure visible in the loaded
PlankaC context instead of a separate calculator routine written for that
button.

Check the host API without opening a window:

```powershell
.\build\plankahost_demo.exe graphics\src\plankagui.plk
.\build\plankahost_demo.exe graphics\src\plankacube.plk
```

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
For graphical and application-oriented hosts, see
[`docs/plankahost_api.md`](docs/plankahost_api.md).

## Conformance

Parser and runtime edge cases are covered by:

```powershell
.\build\plankac_conformance.exe
```

See [`docs/conformance.md`](docs/conformance.md) and
[`docs/plankac_bytecode.md`](docs/plankac_bytecode.md). The broader language
coverage matrix is in [`docs/plankalkuel_coverage.md`](docs/plankalkuel_coverage.md).

## Source Basis

PlankaC is an engineering homage to Konrad Zuse's Plankalkuel. The notation and
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
