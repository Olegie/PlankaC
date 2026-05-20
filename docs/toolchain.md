# Toolchain

PlankaC uses this structure:

```text
.plk source plans
    -> PlankaC reader/interpreter
    -> console and GUI run selected procedures

.plk source plans
    -> compact PlankaMath C mirror
    -> real Win16 Windows 3.x GUI

.plk source plans
    -> compact PlankaMath C mirror
    -> real 16-bit DOS runner

.plk source plans
    -> PlankaC bytecode
    -> bytecode runner or generated C host runner

.plk source plans
    -> native x86-64 ASM procedures
    -> generated ASM command-line runner plus native runtime helpers

.plk source plans
    -> 8086/DOS ASM source
    -> MASM/TASM-style bytecode image plus direct arithmetic procedures

.plk source plans
    -> C host runtime mirror
    -> fallback execution path
```

The classic PlankaMath C layer mirrors the calculator procedures from the
`.plk` plans:

- `P10 add` -> `pm_add`
- `P14 divide_checked` -> `pm_divide_checked`
- `P52 root_checked` -> `pm_root_checked`
- `P999 all_tests` -> `pm_all_tests`

See `docs/execution_model.md` for the exact execution model.

PlankaC is built from the module set under `c/`:

```text
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal c/core/plankac_common.c c/types/plankac_types.c c/notation/plankac_2d.c c/analyzer/plankac_analyzer.c c/core/plankac_source.c c/core/plankac_expr.c c/backends/plankac_bytecode.c c/backends/plankac_asm8086.c c/core/plankac_runtime.c c/tools/plankac_cli.c -o build/plankac.exe -lm
build/plankac.exe check
build/plankac.exe run calculator_demo
build/plankac.exe run divide_checked 84 0
build/plankac.exe tests
build/plankac.exe bytecode build/plankamath.pbc
build/plankac.exe checkbc build/plankamath.pbc
build/plankac.exe runbc build/plankamath.pbc set_session
build/plankac.exe cgen build/plankac_generated.c
build/plankac.exe asmgen build/plankac_asm_runtime.S
build/plankac.exe asm8086 build/plankac_8086.asm
```

The modern Windows GUI links the PlankaC runtime modules directly.

The Win16 GUI is a separate target:

```text
build-win16.bat
```

It builds `build\win16\PlankaMath16.exe` with Open Watcom. The output is a
16-bit Windows NE executable for Windows 3.x and compatible Win16
environments. It uses the compact PlankaMath C runtime; the full parser and
backend modules remain on the modern PlankaC toolchain path.

The DOS runner is also a separate target:

```text
build-dos.bat
```

It builds `build\dos\PMDOS.EXE`, a 16-bit DOS MZ executable with an 8.3
filename. It runs the calculator demo, self-tests, guarded division example,
procedure listing, and named PlankaMath procedures through the compact C
execution layer.

External C programs can use the same API:

```text
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/core/plankac_common.c -o build/plankac_common.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/core/plankac_source.c -o build/plankac_source.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/core/plankac_expr.c -o build/plankac_expr.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/types/plankac_types.c -o build/plankac_types.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/notation/plankac_2d.c -o build/plankac_2d.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/analyzer/plankac_analyzer.c -o build/plankac_analyzer.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/backends/plankac_bytecode.c -o build/plankac_bytecode.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/backends/plankac_asm8086.c -o build/plankac_asm8086.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/core/plankac_runtime.c -o build/plankac_runtime.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/backends/plankac_native_runtime.c -o build/plankac_native_runtime.o
ar rcs build/libplankac.a build/plankac_common.o build/plankac_source.o build/plankac_expr.o build/plankac_types.o build/plankac_2d.o build/plankac_analyzer.o build/plankac_bytecode.o build/plankac_asm8086.o build/plankac_runtime.o build/plankac_native_runtime.o
gcc -Wall -Wextra -std=c89 -Ic/include examples/c_api_demo.c build/libplankac.a -o build/plankac_api_demo.exe -lm
gcc -Wall -Wextra -std=c89 -Ic/include tests/plankac_conformance.c build/libplankac.a -o build/plankac_conformance.exe -lm
```

See `docs/plankac_api.md`.
See `docs/plankac_bytecode.md` for compiler output and `docs/conformance.md`
for parser/runtime edge tests.

The current interpreter supports the executable profile used by this
repository:

- procedure headers and `END`;
- `V`, `Z`, and `R` variables with structure markers;
- numeric constants with structure markers;
- arithmetic and comparison expressions;
- boolean `&`, `|`, `!`;
- guarded equations;
- indexed values, nested record fields, handle-backed records, lists, pairs,
  sets, relations, relation composition, value algebra helpers, chess board
  helpers, complex values, 3D vectors, 4x4 matrices, projection, loops, and
  assertions;
- executable two-dimensional table rows;
- calls to other `.plk` procedures with interprocedural type-family checks;
- multi-result calls.

The modern GUI loads the procedure list from PlankaC and runs selected `.plk`
procedures through the same API used by external C programs. The Win16 GUI uses
the compact calculator execution layer. The DOS runner uses the same compact
layer and is intended for MS-DOS, FreeDOS, DOSBox, and DOS mode on Windows 9x.

The native ASM runner is different from the interpreter path. It contains
generated functions such as `plc_native_p10` and `plc_native_p999`, and it
links only the native helper runtime for storage, lists, sets, relations,
pairs, complex values, 3D vectors, 4x4 matrices, math, and formatting.
Generated procedures pass a shared heap frame through nested `.plk` calls so
list, pair, set, record, complex, vector, and matrix handles can cross
procedure boundaries.

The 8086 backend is a separate DOS-oriented source emitter:

```text
build/plankac.exe asm8086 build/plankac_8086.asm
```

It writes MASM/TASM-style 16-bit assembly with a bytecode image for the loaded
program and direct near procedures for the arithmetic core (`add`, `subtract`,
`multiply`, `divide_checked`, comparisons, min/max/sign, and similar small
plans). Wider compound-value procedures stay present in the bytecode image and
emit guarded stubs until there is a full 16-bit compound runtime.
