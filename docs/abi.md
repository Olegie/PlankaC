# ABI And Embedding API

PlankaC exposes a compact C ABI for two directions:

```text
C host -> PlankaC procedure
PlankaC source -> registered C function
```

The public header is:

```text
c/include/plankac.h
```

The checked example is:

```text
examples/c_abi_demo.c
examples/host_abi.plk
```

## Host Calls PlankaC

The normal embedding path is context-based:

```c
PLANKAC_CONTEXT *ctx;
PLANKAC_RESULT result;
double args[2];
char err[256];

ctx = plankac_create();
plankac_context_load_default(ctx, err, sizeof(err));

args[0] = 12.0;
args[1] = 8.0;
plankac_context_run(ctx, "add", args, 2, &result, err, sizeof(err));

plankac_destroy(ctx);
```

Use `plankac_context_run` for named procedures and
`plankac_context_run_number` when a host intentionally follows `P` numbers.

Compiler artifacts can also be emitted from a loaded context:

```text
plankac_context_write_ir()
plankac_context_write_bytecode()
plankac_context_write_c_backend()
plankac_context_write_asm_runtime()
plankac_context_write_asm_image()
plankac_context_write_asm8086_runtime()
plankac_context_write_lowering_report()
```

## PlankaC Calls C

A host can register a native C function. The function is then callable from
`.plk` source by name:

```text
P700 host_bridge (V0[:32.16], V1[:32.16]) => R0[:32.16], R1[:1.1]
host_mad(V0[:32.16], V1[:32.16]) => R0[:32.16], R1[:1.1]
END
```

Registration:

```c
static int host_mad(void *user_data, const double *args, int argc,
    PLANKAC_RESULT *result, char *err, unsigned err_size)
{
    double offset;

    if (argc != 2 || result == 0) {
        sprintf(err, "host_mad expects two arguments");
        return PLANKAC_ERR;
    }

    offset = user_data != 0 ? *((double *)user_data) : 0.0;
    result->count = 2;
    result->value[0] = args[0] * args[1] + offset;
    result->value[1] = 0.0;
    return PLANKAC_OK;
}

const char *arg_types[] = { "[:32.16]", "[:32.16]" };
const char *result_types[] = { "[:32.16]", "[:1.1]" };
double offset = 6.0;

plankac_context_register_native(ctx, "host_mad", 2, 2,
    arg_types, result_types, host_mad, &offset, err, sizeof(err));
```

Run:

```bat
build\plankac_abi_demo.exe
```

Expected:

```text
native functions: 1
host_mad argc=2 results=2
host_bridge(6, 6) -> R0=42 R1=0
```

## Stable ABI Types

| Type | Meaning |
| --- | --- |
| `PLANKAC_CONTEXT` | loaded program context |
| `PLANKAC_RESULT` | up to `PLANKAC_MAX_RESULTS` numeric result values |
| `PLANKAC_PROC_INFO` | source procedure metadata |
| `PLANKAC_NATIVE_INFO` | registered native function metadata |
| `PLANKAC_NATIVE_FN` | callback signature for host functions callable from `.plk` |

Current ABI values are numeric `double` values. Compound values such as lists,
sets, records, complex numbers, vectors, and matrices are represented as
runtime-local handles. A native function may receive or return such handles,
but it must treat them as context-local identifiers, not process-stable
pointers.

## Registration Rules

Native functions are registered per context:

```text
plankac_context_register_native()
plankac_context_native_count()
plankac_context_get_native()
plankac_context_find_native()
```

There is also a compact global API:

```text
plankac_register_native()
plankac_native_count()
plankac_get_native()
plankac_find_native()
```

Rules:

| Rule | Reason |
| --- | --- |
| register before running source | call resolution happens at runtime by name |
| use unique names | source procedures and native functions share the call namespace |
| declare arity and result count | PlankaC checks call shape before accepting results |
| declare marker strings when known | marker metadata supports signature listing and call checks |
| return `PLANKAC_OK` or `PLANKAC_ERR` | hosts can propagate clear diagnostics |
| set `result->count` exactly | the runtime rejects unexpected result counts |

Registering before `plankac_context_load_sources` is supported. The registry is
preserved when a context loads source or bytecode.

## Backend Boundary

Native callbacks are part of the embedding/runtime ABI. Bytecode can preserve
calls by name, but a host must register the native functions before executing
that bytecode. Standalone generated C, x86-64 ASM, and 8086 ASM outputs do not
automatically emit host callback glue.

## Error Handling

A callback should write a short message to `err` and return `PLANKAC_ERR` on
failure. The interpreter prefixes source-location context around runtime
errors where available.

## Threading

Use one `PLANKAC_CONTEXT` per independent host instance. Native callbacks
receive `user_data`, so the host can keep state outside the interpreter
without global variables.
