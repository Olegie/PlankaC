# From `max3.plk` To Compiler Artifacts

This tutorial uses one focused source file and follows the same path that CI
checks: parse `.plk`, run it, emit bytecode, emit C, emit ASM, and link native
executables.

Source file:

```text
examples/max3.plk
```

The source defines `tutorial_max2`, `tutorial_max3`, and `max3_demo`.

## Windows

Build the toolchain:

```text
build.bat
```

Check the source:

```text
build\plankac.exe checkfile examples\max3.plk
```

Run the demo procedure through PlankaC:

```text
build\plankac.exe runfile examples\max3.plk max3_demo
```

Expected result:

```text
R0=9
```

Emit compiler artifacts:

```text
build\plankac.exe bytecode build\plankamath.pbc
build\plankac.exe cgen build\plankac_generated.c
build\plankac.exe asmgen build\plankac_asm_runtime.S
build\plankac.exe asm8086 build\plankac_8086.asm
build\plankac.exe compile build\plankac_pipeline
```

Link native runners:

```text
build\plankac.exe native-c build\plankac_native_c
build\plankac_native_c.exe set_session

build\plankac.exe native-asm build\plankac_native_asm
build\plankac_native_asm.exe add 12 8
```

## Linux

Run the same verification path through Make:

```text
make ci
```

Run the tutorial source directly:

```text
build/plankac checkfile examples/max3.plk
build/plankac runfile examples/max3.plk max3_demo
```

## What This Proves

The demo is intentionally plain. Its purpose is not to show every language
feature. Its purpose is to show the toolchain boundary:

```text
.plk source -> parser/analyzer -> procedure table -> bytecode/IR -> C/ASM -> native executable
```

That path is the useful part for people who want to write their own `.plk`
files and embed or compile them through PlankaC.
