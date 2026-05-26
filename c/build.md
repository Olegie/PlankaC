# C / Windows Build

The `.plk` files are the project source. The C tree is organized as host
code, library code, and backend code:

```text
c/include/   public headers
c/internal/  private PlankaC declarations
c/core/      source loader, parser, interpreter, public API
c/types/     structure marker and type-family handling
c/notation/  executable two-dimensional table notation and PAGE documents
c/analyzer/  static program checks and structural schema checks
c/values/    bit, fixed-point, and tagged value helpers
c/models/    chess board/game model helpers
c/ir/        typed IR builder and emitter
c/backends/  bytecode, lowering reports, C backend, x86-64 ASM, 8086/DOS ASM, helper runtime
c/targets/   CLI, DOS, Win16, and modern Windows hosts
c/legacy/    compact PlankaMath fallback runtime
```

## Compiler Policy

The compiler is not bundled with this repository. Use an external GCC, MinGW,
Open Watcom, or another C89-capable toolchain, and keep generated files under
`build/`.

## Standard Build

On Windows, the normal build is:

```text
build.bat
```

It builds the modern console tools, the static `libplankac.a`, the API demos,
the conformance runner, the modern Windows GUI, bytecode output, generated C,
generated native x86-64 ASM, and generated 8086/DOS ASM source.
It also verifies the compiler pipeline through generated IR, generated C,
generated x86-64 ASM, and linked native executables.

Current smoke-test output includes:

```text
PlankaC OK: 29 files, 148 procedures
Bytecode OK: 148 procedures
host_bridge(6, 6) -> R0=42 R1=0
Compiler pipeline OK
native-c: build\plankac_native_c.exe
native-asm: build\plankac_native_asm.exe
ASM runtime written: build\plankac_asm_runtime.S
8086 ASM written: build\plankac_8086.asm
CONFORMANCE OK
```

The compact PlankaMath runner is built as a fallback target. The PlankaC line
is the active `.plk` toolchain profile.

## Static Library

Use this when another C program should embed PlankaC:

```text
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/core/plankac_common.c -o build/plankac_common.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/core/plankac_source.c -o build/plankac_source.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/core/plankac_expr.c -o build/plankac_expr.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/types/plankac_types.c -o build/plankac_types.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/notation/plankac_2d.c -o build/plankac_2d.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/notation/plankac_document.c -o build/plankac_document.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/notation/plankac_page.c -o build/plankac_page.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/analyzer/plankac_analyzer.c -o build/plankac_analyzer.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/analyzer/plankac_schema.c -o build/plankac_schema.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/values/plankac_bits.c -o build/plankac_bits.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/values/plankac_value.c -o build/plankac_value.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/models/plankac_chess_model.c -o build/plankac_chess_model.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/ir/plankac_ir.c -o build/plankac_ir.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/backends/plankac_lowering.c -o build/plankac_lowering.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/backends/plankac_bytecode.c -o build/plankac_bytecode.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/backends/plankac_asm8086.c -o build/plankac_asm8086.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/core/plankac_runtime.c -o build/plankac_runtime.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/backends/plankac_native_runtime.c -o build/plankac_native_runtime.o
ar rcs build/libplankac.a build/plankac_common.o build/plankac_types.o build/plankac_2d.o build/plankac_document.o build/plankac_page.o build/plankac_analyzer.o build/plankac_schema.o build/plankac_bits.o build/plankac_value.o build/plankac_chess_model.o build/plankac_ir.o build/plankac_lowering.o build/plankac_source.o build/plankac_expr.o build/plankac_bytecode.o build/plankac_asm8086.o build/plankac_runtime.o build/plankac_native_runtime.o
gcc -Wall -Wextra -std=c89 -Ic/include examples/c_api_demo.c build/libplankac.a -o build/plankac_api_demo.exe -lm
gcc -Wall -Wextra -std=c89 -Ic/include examples/c_abi_demo.c build/libplankac.a -o build/plankac_abi_demo.exe -lm
```

The API is declared in `c/include/plankac.h`.

## Modern Windows GUI

The file `c/targets/windows_gui.c` links PlankaC and runs `.plk` procedures
through the library API. It also keeps the compact runtime as a fallback path.

## Real Win16 GUI

The file `c/targets/win16_gui.c` is a separate Windows 3.x target. It is built
with Open Watcom and the compact PlankaMath C execution layer:

```text
build-win16.bat
```

Expected output:

```text
build\win16\PlankaMath16.exe
```

The output is a 16-bit Windows NE executable for Windows 3.x and compatible
Win16 environments. It uses the compact PlankaMath C runtime; the full PlankaC
parser and backend modules stay on the modern toolchain path.

## Real DOS Runner

The file `c/targets/dos_cli.c` is a separate 16-bit DOS target. It is built
with Open Watcom and the compact PlankaMath C execution layer:

```text
build-dos.bat
```

Expected output:

```text
build\dos\PMDOS.EXE
```

The short name is intentional for 8.3 DOS filesystems. The runner supports:

```text
PMDOS demo
PMDOS tests
PMDOS guarded
PMDOS list
PMDOS run add 12 8
PMDOS run divide_checked 84 0
```
