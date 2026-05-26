# PlankaC C API

PlankaC can be used as a small C library. A host program can load `.plk`
source files, inspect the procedure table, and run procedures by name.

Public header:

```text
c/include/plankac.h
```

Example program:

```text
examples/c_api_demo.c
```

## Build A Static Library

```text
mkdir build
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
procedures: 118
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
add(12, 8) -> R0=20
P140 complex_norm_session() -> R0=25
three_d_pipeline_session() -> R0=120
wrote build/plankac_api_demo_runtime.S
```

## Context API

Use `PLANKAC_CONTEXT` when embedding PlankaC in another C program.

```c
#include "plankac.h"

int main(void)
{
    PLANKAC_CONTEXT *ctx;
    PLANKAC_RESULT result;
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

    plankac_destroy(ctx);
    return result.value[0] == 20.0 ? 0 : 1;
}
```

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
plankac_context_run()
plankac_context_run_number()
plankac_context_summary()
plankac_context_write_bytecode()
plankac_context_write_c_backend()
plankac_context_write_asm_runtime()
plankac_context_write_asm_image()
plankac_context_write_asm8086_runtime()
plankac_write_asm8086_runtime()
plankac_format()
```

The legacy global functions remain available for simple programs and for the
PlankaMath GUI, but new embedding code should prefer the context API.

`PLANKAC_PROC_INFO` also exposes the first-level argument and result marker
strings collected from procedure headers, for example `[:32.16]` or `[:1.1]`.
That is enough for small host programs to list procedure signatures before
calling them.

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
