# Compiler Pipeline

PlankaC has a stable compiler route from source files to generated artifacts
and native executables:

```text
.plk source
    -> parser and analyzer
    -> loaded procedure table
    -> procedure AST
    -> expression AST summaries
    -> typed IR / bytecode
    -> IR reload
    -> generated C / generated x86-64 ASM / 8086 ASM source
    -> native executable through an external C toolchain
```

The important part is the IR reload step. `compile <output-prefix>` does not
only write several files from the original source table; it writes `.pbc`,
loads that `.pbc` back as the compiler IR, and emits the C/ASM artifacts from
the reloaded IR program.

For inspection, `plankac ast <output.ast>` emits the statement AST and
expression AST summaries. `plankac ir <output.ir>` emits the typed statement IR
built from that AST boundary, with operation class, type families, call shape,
expression-node counts, and lowering class.
`plankac lowering <output.txt>` writes the backend lowering plan used to see
which statements are direct scalar expressions and which ones route through
compound/runtime entry points for C, x86-64 ASM, and 8086 ASM.

## Artifact Pipeline

```bat
build\plankac.exe compile build\plankac_pipeline
```

Outputs:

```text
build\plankac_pipeline.pbc
build\plankac_pipeline.c
build\plankac_pipeline.S
build\plankac_pipeline_8086.asm
```

Expected console shape:

```text
Compiler pipeline OK
source: 29 files, 150 procedures
ir: build\plankac_pipeline.pbc
ir procedures: 150
c: build\plankac_pipeline.c
asm: build\plankac_pipeline.S
asm8086: build\plankac_pipeline_8086.asm
```

## Native C Executable

```bat
build\plankac.exe native-c build\plankac_native_c
build\plankac_native_c.exe set_session
```

This route emits the same `.pbc`, reloads it, emits generated C from the
reloaded IR, then links:

```text
build\plankac_native_c.exe
```

The generated C runner embeds the bytecode image and uses the public PlankaC
runtime API to execute procedures.

## Native ASM Executable

```bat
build\plankac.exe native-asm build\plankac_native_asm
build\plankac_native_asm.exe add 12 8
```

This route emits the same `.pbc`, reloads it, emits x86-64 assembly from the
reloaded IR, then links:

```text
build\plankac_native_asm.exe
```

The generated ASM runner contains generated procedure functions and a compact
dispatcher. It links the native helper runtime for frame storage, compound
handles, math helpers, and output formatting.

## Backend Roles

| Stage | Artifact | Role |
| --- | --- | --- |
| AST | `.ast` / in memory | statement and expression classification layer before typed IR construction |
| IR | `.ir` / `.pbc` | readable typed stream plus reloadable bytecode artifact |
| lowering report | `.lowering` / `.txt` | typed backend plan for scalar, call, contract, control, and compound operations |
| C backend | `.c` | portable runner with embedded IR/bytecode |
| x86-64 ASM backend | `.S` | generated native procedure functions plus dispatcher |
| 8086 backend | `_8086.asm` | MASM/TASM-style 16-bit source with bytecode image, direct arithmetic procedures, compound heaps, and compound procedure table |
| native C | `.exe` | linked executable from generated C |
| native ASM | `.exe` | linked executable from generated x86-64 ASM |

## Build Verification

`build.bat` checks the full route:

```text
plankac.exe compile build\plankac_pipeline
plankac.exe ast build\plankac.ast
plankac.exe ir build\plankac.ir
plankac.exe lowering build\plankac.lowering
plankac.exe native-c build\plankac_native_c
plankac_native_c.exe set_session
plankac.exe native-asm build\plankac_native_asm
plankac_native_asm.exe add 12 8
```

That keeps the compiler path part of normal verification instead of a
separate manual demonstration.

## Toolchain Boundary

PlankaC emits IR, C, and ASM itself. Native executable linking uses the
external C toolchain available in `PATH`; the default build expects GCC/MinGW.
Open Watcom remains the separate toolchain for real Win16 and DOS targets.
