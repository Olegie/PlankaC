# PlankaC C API

PlankaC can be used as a compact C library. A host program can load `.plk`
source files, inspect the procedure table, and run procedures by name.

Public header:

```text
c/include/plankac.h
```

Example program:

```text
examples/c_api_demo.c
examples/c_abi_demo.c
```

## Build A Static Library

```text
mkdir build
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
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/ir/plankac_ast.c -o build/plankac_ast.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/ir/plankac_ir.c -o build/plankac_ir.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/ir/plankac_evidence.c -o build/plankac_evidence.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/backends/plankac_lowering.c -o build/plankac_lowering.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/backends/plankac_bytecode.c -o build/plankac_bytecode.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/backends/plankac_asm8086.c -o build/plankac_asm8086.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/backends/dos/plankac_doscom.c -o build/plankac_doscom.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/core/plankac_runtime.c -o build/plankac_runtime.o
gcc -Wall -Wextra -std=c89 -Ic/include -Ic/internal -c c/backends/plankac_native_runtime.c -o build/plankac_native_runtime.o
ar rcs build/libplankac.a build/plankac_common.o build/plankac_types.o build/plankac_2d.o build/plankac_document.o build/plankac_page.o build/plankac_analyzer.o build/plankac_schema.o build/plankac_bits.o build/plankac_value.o build/plankac_chess_model.o build/plankac_ast.o build/plankac_ir.o build/plankac_evidence.o build/plankac_lowering.o build/plankac_source.o build/plankac_expr.o build/plankac_bytecode.o build/plankac_asm8086.o build/plankac_doscom.o build/plankac_runtime.o build/plankac_native_runtime.o
```

Link an application:

```text
gcc -Wall -Wextra -std=c89 -Ic/include examples/c_api_demo.c build/libplankac.a -o build/plankac_api_demo.exe -lm
```

Run:

```text
build/plankac_api_demo.exe
```

Expected output:

```text
procedures: 150
found P140 complex_norm_session args=0 results=1
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
typed divide status -> tag=2 raw=1 type=[:1.1]
add(12, 8) -> R0=20
P140 complex_norm_session() -> R0=25
three_d_pipeline_session() -> R0=120
wrote build/plankac_api_demo_runtime.S
wrote build/plankac_api_demo_dos.com
```

## Context API

Use `PLANKAC_CONTEXT` when embedding PlankaC in another C program.

```c
#include "plankac.h"

int main(void)
{
    PLANKAC_CONTEXT *ctx;
    PLANKAC_RESULT result;
    PLANKAC_TYPED_RESULT typed_result;
    double args[2];
    char err[256];

    ctx = plankac_create();
    if (ctx == 0) {
        return 1;
    }

    if (!plankac_context_load_default(ctx, err, sizeof(err))) {
        plankac_destroy(ctx);
        return 1;
    }

    args[0] = 12.0;
    args[1] = 8.0;
    if (!plankac_context_run(ctx, "add", args, 2,
            &result, err, sizeof(err))) {
        plankac_destroy(ctx);
        return 1;
    }

    if (!plankac_context_run_typed(ctx, "add", args, 2,
            &typed_result, err, sizeof(err))) {
        plankac_destroy(ctx);
        return 1;
    }

    plankac_context_write_doscom(ctx,
        "build/plankac_api_demo_dos.com", err, sizeof(err));

    plankac_destroy(ctx);
    return result.value[0] == 20.0 ? 0 : 1;
}
```

`plankac_context_write_doscom()` writes a DOS `.COM` bootstrap with the loaded
program's bytecode image embedded after the banner. For full DOS execution use
the `PLANKACD.EXE` runner built by `build-dos-plankac.bat`.

Load a custom source set:

```c
const char *sources[] = {
    "my_plans/arithmetic.plk",
    "my_plans/session.plk",
    0
};

plankac_context_load_sources(ctx, sources, err, sizeof(err));
```

Source paths are resolved by the host process, so pass paths relative to the
current working directory or absolute paths from your application.

## Native Function ABI

The context API also lets `.plk` source call C functions registered by the
host. This is the bridge for platform services, device I/O, application state,
or host-provided math routines without rewriting the host in `.plk`.

Callback type:

```c
typedef int (*PLANKAC_NATIVE_FN)(void *user_data,
    const double *args, int argc, PLANKAC_RESULT *result,
    char *err, unsigned err_size);
```

Register a function:

```c
const char *arg_types[] = { "[:32.16]", "[:32.16]" };
const char *result_types[] = { "[:32.16]", "[:1.1]" };

plankac_context_register_native(ctx, "host_mad", 2, 2,
    arg_types, result_types, host_mad, user_data, err, sizeof(err));
```

Call it from `.plk`:

```text
P700 host_bridge (V0[:32.16], V1[:32.16]) => R0[:32.16], R1[:1.1]
host_mad(V0[:32.16], V1[:32.16]) => R0[:32.16], R1[:1.1]
END
```

The checked demo is:

```text
build/plankac_abi_demo.exe
```

For the ABI contract and backend boundary, see `docs/abi.md`.

## Main Functions

```text
plankac_create()
plankac_destroy()
plankac_context_load_default()
plankac_context_load_sources()
plankac_context_load_bytecode()
plankac_context_load_bytecode_text()
plankac_context_proc_count()
plankac_context_get_proc()
plankac_context_find_proc()
plankac_context_register_native()
plankac_context_native_count()
plankac_context_get_native()
plankac_context_find_native()
plankac_context_run()
plankac_context_run_typed()
plankac_context_run_number()
plankac_context_run_number_typed()
plankac_context_summary()
plankac_context_write_bytecode()
plankac_context_write_c_backend()
plankac_context_write_asm_runtime()
plankac_context_write_asm_image()
plankac_context_write_asm8086_runtime()
plankac_context_write_ast()
plankac_context_write_ir()
plankac_context_write_evidence()
plankac_context_write_lowering_report()
plankac_write_asm8086_runtime()
plankac_write_ast()
plankac_write_ir()
plankac_write_evidence()
plankac_write_lowering_report()
plankac_register_native()
plankac_native_count()
plankac_get_native()
plankac_find_native()
plankac_run_typed()
plankac_run_number_typed()
plankac_format()
```

The legacy global functions remain available for simple programs and for the
PlankaMath GUI, but new embedding code should prefer the context API.

`PLANKAC_PROC_INFO` also exposes the first-level argument and result marker
strings collected from procedure headers, for example `[:32.16]` or `[:1.1]`.
That is enough for host programs to list procedure signatures before
calling them.

Use `PLANKAC_TYPED_RESULT` when the host needs the value model rather than
only ABI doubles. `plankac_context_run_typed` and `plankac_run_typed` return
`tag`, `family`, `bits`, `scale`, `raw`, `handle`, `number`, and `type_text`
for each result slot. Boolean `[:1.1]` results return `PLANKAC_VALUE_BIT`;
fixed-point markers return `PLANKAC_VALUE_FIXED`; compound families return
`PLANKAC_VALUE_HANDLE`. `raw` is a wide integer field; bundled MinGW/GCC
examples print it with `%lld`.

Use `plankac_context_write_ast` or `plankac_write_ast` when a host wants the
compiler inspection artifact. The AST output includes statement operation
classes and expression-node summaries. It is useful for editors, workbenches,
diagnostic tools, and backend comparison tests.

## PlankaHost API

Graphical or application-oriented hosts can use the higher-level API in:

```text
graphics/c/plankahost.h
```

`PlankaHost` loads the standard PlankaC source profile plus one application
file. The loaded context therefore contains the older calculator procedures,
data-structure procedures, relation helpers, chess procedures, 3D helpers, and
the selected application procedures.

Main functions:

```text
plankahost_open()
plankahost_close()
plankahost_proc_count()
plankahost_get_proc()
plankahost_find_proc()
plankahost_run()
plankahost_render()
plankahost_button_at()
plankahost_timer_step()
```

Use the core context API when you need a pure computation library. Use
`PlankaHost` when you want a complete loaded application profile with render,
input, timer, and procedure-dispatch helpers. See `docs/plankahost_api.md`.
