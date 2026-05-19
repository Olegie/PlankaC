# PlankaC Bytecode

PlankaC can emit a small textual bytecode file from the loaded `.plk` source
set. This gives the project a real compiler artifact in addition to direct
interpretation.

Build and emit:

```text
build/plankac.exe bytecode build/plankamath.pbc
build/plankac.exe checkbc build/plankamath.pbc
build/plankac.exe runbc build/plankamath.pbc set_session
build/plankac.exe cgen build/plankac_generated.c
```

The generated file starts like this:

```text
PLANKAC-BYTECODE 0.1
SOURCES 14
PROCEDURES 62
PROC P10 add ARGC 2 RESULTS 1
  EVAL "(V0[:32.16] + V1[:32.16])" -> "R0[:32.16]"
END
```

Supported bytecode operations:

```text
PROC      procedure header
EVAL      expression assignment
CALL      procedure call assignment
GEVAL     guarded expression assignment
GCALL     guarded procedure call assignment
ASSERT    assertion predicate
LOOP      repeated assignment
END       procedure end
```

The bytecode is intentionally readable. It is used by the CLI runner, the C
API through `plankac_context_load_bytecode()`, and the generated C backend
runner emitted by `plankac cgen`.

The native ASM backend is separate from this bytecode path. `plankac asmgen`
lowers the loaded `.plk` procedures to x86-64 assembly functions and links
against the native helper runtime instead of calling the bytecode runner.
