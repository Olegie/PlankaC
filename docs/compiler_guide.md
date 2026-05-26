# Compiler Guide

PlankaC has one source loader and several execution or output paths. All of
them start from the same loaded `.plk` procedure table.

## Build

On Windows with GCC/MinGW in `PATH`:

```bat
build.bat
```

The build produces:

```text
build\plankac.exe
build\plankamath_cli.exe
build\libplankac.a
build\plankac_conformance.exe
build\plankac_api_demo.exe
build\plankac_abi_demo.exe
build\PlankaMath.exe
build\PlankaGUI.exe
build\PlankaCube.exe
build\PlankaHost.exe
```

Win16 and DOS targets are built separately:

```bat
build-win16.bat
build-dos.bat
```

## Source Checking

```bat
build\plankac.exe check
```

Expected profile:

```text
PlankaC OK: 29 files, 148 procedures
```

Check one additional application file:

```bat
build\plankac.exe checkfile graphics\src\plankagui.plk
```

## Running Procedures

Run by name:

```bat
build\plankac.exe run add 12 8
```

Run by number:

```bat
build\plankac.exe run P73 12 20
```

Run an application profile on top of the standard profile:

```bat
build\plankac.exe runfile graphics\src\plankacube.plk app_kind
```

## Compiler Pipeline

Emit the stable compiler artifact set:

```bat
build\plankac.exe compile build\plankac_pipeline
```

This writes bytecode/IR first, reloads that IR, and emits C/ASM artifacts from
the reloaded program:

```text
build\plankac_pipeline.pbc
build\plankac_pipeline.c
build\plankac_pipeline.S
build\plankac_pipeline_8086.asm
```

Inspect the backend lowering plan:

```bat
build\plankac.exe lowering build\plankac.lowering
```

The report classifies typed IR statements into scalar expression paths,
procedure calls, contracts, control operations, and compound runtime entry
points for C, x86-64 ASM, and 8086 ASM.

Build a native executable through generated C:

```bat
build\plankac.exe native-c build\plankac_native_c
build\plankac_native_c.exe set_session
```

Build a native executable through generated x86-64 ASM:

```bat
build\plankac.exe native-asm build\plankac_native_asm
build\plankac_native_asm.exe add 12 8
```

The native commands emit the same IR/C/ASM artifact set and then call the
external GCC/MinGW toolchain to link an executable. The full route is
documented in `compiler_pipeline.md`.

## ABI Demo

The ABI demo verifies both embedding directions:

```bat
build\plankac_abi_demo.exe
```

It registers `host_mad` in C, loads `examples\host_abi.plk`, and runs
`host_bridge`, which calls the registered C function from `.plk`.

## Bytecode

Emit readable typed IR:

```bat
build\plankac.exe ir build\plankac.ir
```

Emit readable bytecode:

```bat
build\plankac.exe bytecode build\plankamath.pbc
```

Check bytecode:

```bat
build\plankac.exe checkbc build\plankamath.pbc
```

Run bytecode:

```bat
build\plankac.exe runbc build\plankamath.pbc set_session
```

The bytecode format is documented in `plankac_bytecode.md`.

## Generated C

```bat
build\plankac.exe cgen build\plankac_generated.c
gcc -Wall -Wextra -std=c89 -Ic\include build\plankac_generated.c build\libplankac.a -o build\plankac_generated.exe -lm
build\plankac_generated.exe calculator_demo
```

The generated C runner embeds the bytecode image and uses the public runtime
library to execute procedures.

## Native x86-64 ASM

```bat
build\plankac.exe asmgen build\plankac_asm_runtime.S
gcc -Wall -Wextra -Ic\include build\plankac_asm_runtime.S build\libplankac.a -o build\plankac_asm_runtime.exe -lm
build\plankac_asm_runtime.exe all_tests
```

The generated ASM runner contains native procedure functions and links helper
runtime code for frame storage, compound handles, math helpers, and output
formatting. It does not call the interpreter dispatcher for generated
procedures.

## 8086/DOS ASM

```bat
build\plankac.exe asm8086 build\plankac_8086.asm
```

This emits MASM/TASM-style 16-bit assembly source. The backend writes the full
program image, direct arithmetic procedures, exported compound heap areas, and
a compound procedure table for list, set, pair, record, relation and
board-oriented procedures.

## Conformance

```bat
build\plankac_conformance.exe
```

The conformance runner checks valid edge cases, invalid source fixtures,
runtime behavior, bytecode execution, compiler pipeline output, native C and
native ASM executable smoke checks, extended data structures, relation helpers,
chess examples, 3D procedures, and parser recovery cases.

## Output Rule

Compiler-output work should stay under `c/backends`. Platform launchers should
stay under `c/targets` or `graphics/c`. Source-language behavior belongs in
`c/core`, `c/types`, `c/notation`, or `c/analyzer`.
