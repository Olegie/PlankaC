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
gcc -Wall -Wextra -std=c89 c/plankac_common.c c/plankac_source.c c/plankac_expr.c c/plankac_bytecode.c c/plankac_runtime.c c/plankac_cli.c -o build/plankac.exe -lm
build/plankac.exe check
build/plankac.exe run calculator_demo
build/plankac.exe run divide_checked 84 0
build/plankac.exe tests
build/plankac.exe bytecode build/plankamath.pbc
build/plankac.exe checkbc build/plankamath.pbc
build/plankac.exe runbc build/plankamath.pbc set_session
build/plankac.exe cgen build/plankac_generated.c
build/plankac.exe asmgen build/plankac_asm_runtime.S
```

The modern Windows GUI links the PlankaC runtime modules directly.

The Win16 GUI is a separate target:

```text
build-win16.bat
```

It builds `build\win16\PlankaMath16.exe` with Open Watcom. That program uses
the compact PlankaMath C mirror because the full parser/backend table layout is
not realistic for a small Windows 3.x process.

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
- indexed values, nested record fields, lists, pairs, sets, loops, and assertions;
- calls to other `.plk` procedures with direct type marker checks;
- multi-result calls.

The modern GUI loads the procedure list from PlankaC and runs selected `.plk`
procedures through the same API used by external C programs. The Win16 GUI uses
the compact calculator execution layer. The DOS runner uses the same compact
layer and is intended for MS-DOS, FreeDOS, DOSBox, and DOS mode on Windows 9x.

The native ASM runner is different from the interpreter path. It contains
generated functions such as `plc_native_p10` and `plc_native_p999`, and it
links only the native helper runtime for storage, lists, sets, pairs, math, and
formatting.
