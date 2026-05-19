# C / Windows Build

The `.plk` files are the project source. The C layer is a small host runtime
and user interface layer.

## Compiler Policy

The compiler is not bundled with this repository. Use an external GCC, MinGW,
Open Watcom, or another C89-capable toolchain, and keep generated files under
`build/`.

## Modern Console Smoke Test

Use this when a modern C compiler is available:

```text
mkdir build
gcc -Wall -Wextra -std=c89 c/plankamath.c c/plankamath_cli.c -o build/plankamath_cli.exe -lm
gcc -Wall -Wextra -std=c89 c/plankac_common.c c/plankac_source.c c/plankac_expr.c c/plankac_bytecode.c c/plankac_runtime.c c/plankac_cli.c -o build/plankac.exe -lm
gcc -Wall -Wextra -std=c89 -c c/plankac_common.c -o build/plankac_common.o
gcc -Wall -Wextra -std=c89 -c c/plankac_source.c -o build/plankac_source.o
gcc -Wall -Wextra -std=c89 -c c/plankac_expr.c -o build/plankac_expr.o
gcc -Wall -Wextra -std=c89 -c c/plankac_bytecode.c -o build/plankac_bytecode.o
gcc -Wall -Wextra -std=c89 -c c/plankac_runtime.c -o build/plankac_runtime.o
gcc -Wall -Wextra -std=c89 -c c/plankac_native_runtime.c -o build/plankac_native_runtime.o
ar rcs build/libplankac.a build/plankac_common.o build/plankac_source.o build/plankac_expr.o build/plankac_bytecode.o build/plankac_runtime.o build/plankac_native_runtime.o
gcc -Wall -Wextra -std=c89 examples/c_api_demo.c build/libplankac.a -o build/plankac_api_demo.exe -lm
gcc -Wall -Wextra -std=c89 tests/plankac_conformance.c build/libplankac.a -o build/plankac_conformance.exe -lm
build/plankamath_cli.exe compile
build/plankamath_cli.exe demo
build/plankamath_cli.exe tests
build/plankamath_cli.exe guarded
build/plankac.exe check
build/plankac.exe tests
build/plankac.exe bytecode build/plankamath.pbc
build/plankac.exe checkbc build/plankamath.pbc
build/plankac.exe runbc build/plankamath.pbc set_session
build/plankac.exe cgen build/plankac_generated.c
build/plankac.exe asmgen build/plankac_asm_runtime.S
gcc -Wall -Wextra -std=c89 -Ic build/plankac_generated.c build/libplankac.a -o build/plankac_generated.exe -lm
build/plankac_generated.exe set_session
gcc -Wall -Wextra -Ic build/plankac_asm_runtime.S build/libplankac.a -o build/plankac_asm_runtime.exe -lm
build/plankac_asm_runtime.exe set_session
build/plankac_asm_runtime.exe divide_checked 84 0
build/plankac_asm_runtime.exe loop_multiply 3 4
build/plankac_asm_runtime.exe pair_relation_session
build/plankac_asm_runtime.exe all_tests
build/plankac_api_demo.exe
build/plankac_conformance.exe
```

Expected output:

```text
Compile OK: 14 files, 62 procedures
30
1
0, 1
PlankaC OK: 14 files, 62 procedures
R0=1
Bytecode written: build/plankamath.pbc
Bytecode OK: 62 procedures
R0=2
C backend written: build/plankac_generated.c
R0=2
ASM runtime written: build/plankac_asm_runtime.S
R0=2
R0=0 R1=1
R0=12
R0=2
R0=1
procedures: 62
P0 type_sheet args=0 results=1
  first types: V0=- R0=[:1.1]
P10 add args=2 results=1
  first types: V0=[:32.16] R0=[:32.16]
P11 subtract args=2 results=1
  first types: V0=[:32.16] R0=[:32.16]
P12 multiply args=2 results=1
  first types: V0=[:32.16] R0=[:32.16]
P13 negate args=1 results=1
  first types: V0=[:32.16] R0=[:32.16]
divide_checked(84, 0) -> R0=0 R1=1
add(12, 8) -> R0=20
CONFORMANCE OK
```

## Static Library

Use this when another C program should embed PlankaC:

```text
gcc -Wall -Wextra -std=c89 -c c/plankac_common.c -o build/plankac_common.o
gcc -Wall -Wextra -std=c89 -c c/plankac_source.c -o build/plankac_source.o
gcc -Wall -Wextra -std=c89 -c c/plankac_expr.c -o build/plankac_expr.o
gcc -Wall -Wextra -std=c89 -c c/plankac_bytecode.c -o build/plankac_bytecode.o
gcc -Wall -Wextra -std=c89 -c c/plankac_runtime.c -o build/plankac_runtime.o
gcc -Wall -Wextra -std=c89 -c c/plankac_native_runtime.c -o build/plankac_native_runtime.o
ar rcs build/libplankac.a build/plankac_common.o build/plankac_source.o build/plankac_expr.o build/plankac_bytecode.o build/plankac_runtime.o build/plankac_native_runtime.o
gcc -Wall -Wextra -std=c89 examples/c_api_demo.c build/libplankac.a -o build/plankac_api_demo.exe -lm
gcc -Wall -Wextra -std=c89 tests/plankac_conformance.c build/libplankac.a -o build/plankac_conformance.exe -lm
```

The API is declared in `c/plankac.h`.

## Modern Windows GUI

The file `c/windows_gui.c` links PlankaC and runs `.plk` procedures through
the library API.

With a modern Windows C compiler, compile it as a Win32 compatibility build:

```text
gcc -Wall -Wextra -std=c89 c/plankac_common.c c/plankac_source.c c/plankac_expr.c c/plankac_bytecode.c c/plankac_runtime.c c/plankamath.c c/windows_gui.c -mwindows -o build/PlankaMath.exe -lm
```

## Real Win16 GUI

The file `c/win16_gui.c` is a separate Windows 3.x target. It is built with
Open Watcom and the compact PlankaMath C execution layer:

```text
build-win16.bat
```

Expected output:

```text
build\win16\PlankaMath16.exe
```

This is a Win16 executable target, not a modern executable with old-looking
controls. The full PlankaC parser and backend modules stay in the modern
toolchain because their current table layout is too large for a practical
Windows 3.x GUI process.

## Real DOS Runner

The file `c/dos_cli.c` is a separate 16-bit DOS target. It is built with Open
Watcom and the compact PlankaMath C execution layer:

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
