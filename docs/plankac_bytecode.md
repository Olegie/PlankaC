# PlankaC IR / Bytecode

PlankaC emits two readable compiler artifacts from the loaded `.plk` source
set: a statement-level typed IR report and a reloadable textual bytecode file.
The bytecode artifact is used by the compiler pipeline in addition to direct
interpretation.

Build and emit:

```text
build/plankac.exe compile build/plankac_pipeline
build/plankac.exe ir build/plankac.ir
build/plankac.exe lowering build/plankac.lowering
build/plankac.exe bytecode build/plankamath.pbc
build/plankac.exe checkbc build/plankamath.pbc
build/plankac.exe runbc build/plankamath.pbc set_session
build/plankac.exe cgen build/plankac_generated.c
build/plankac.exe asm8086 build/plankac_8086.asm
```

The generated file starts like this:

```text
PLANKAC-BYTECODE 0.1
SOURCES 29
PROCEDURES 148
PROC P10 add ARGC 2 RESULTS 1
  EVAL "(V0[:32.16] + V1[:32.16])" -> "R0[:32.16]"
END
```

The typed IR report starts like this:

```text
PLANKAC-TYPED-IR 1
sources 29
procedures 148
statements ...
```

The lowering report starts like this:

```text
PLANKAC-LOWERING 1
sources 29
procedures 148
statements ...
```

Supported bytecode operations:

```text
PROC      procedure header
EVAL      expression assignment
CALL      procedure call assignment
GEVAL     guarded expression assignment
GCALL     guarded procedure call assignment
ASSERT    assertion predicate
REQUIRE   contract precondition predicate
ENSURE    contract postcondition predicate
STOPIF    procedure stop predicate
CONST     constant-bank assignment
LOOP      repeated assignment
END       procedure end
```

The bytecode is intentionally readable. It is used by the CLI runner, the C
API through `plankac_context_load_bytecode()`, the generated C backend runner
emitted by `plankac cgen`, and the stable `compile <prefix>` route.

`compile <prefix>` writes `.pbc`, reloads it, and emits generated C, x86-64
ASM, 8086 ASM, and a DOS COM bootstrap with embedded bytecode from the
reloaded IR program.
`native-c <prefix>` and `native-asm <prefix>` use the same IR reload path
before linking executables.
`lowering <path>` records which typed IR statements are emitted through direct
scalar paths and which ones use compound runtime entry points.

`asmgen` can still be used directly. It lowers the loaded `.plk` procedures to
x86-64 assembly functions and links against the native helper runtime instead
of calling the interpreter dispatcher.

The 8086 backend also embeds this bytecode as a DOS/MASM-style `DB` image.
The assembly artifact includes direct arithmetic procedures plus exported
compound heap areas and a compound procedure table for list, set, pair,
record, relation and board-oriented procedures.
