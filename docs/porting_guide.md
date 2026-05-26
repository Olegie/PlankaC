# Porting Guide

PlankaC is written in C89-oriented C and keeps platform code separate from
the language core. A port should preserve that split.

## What Must Be Ported

For a computation-only port, use:

```text
c/include
c/internal
c/core
c/types
c/notation
c/analyzer
c/backends
```

For command-line execution, add:

```text
c/tools/plankac_cli.c
```

For host applications, use either the public API in `c/include/plankac.h` or
the higher-level host API in `graphics/c/plankahost.h`.

For bidirectional embedding, use `docs/abi.md`: the host calls `.plk`
procedures through `plankac_context_run`, and `.plk` code calls registered C
functions through `plankac_context_register_native`.

## Platform Boundaries

| Concern | Location |
| --- | --- |
| Source loading and execution | `c/core` |
| Type markers | `c/types` |
| Two-dimensional row expansion | `c/notation` |
| Static checks | `c/analyzer` |
| Compiler outputs | `c/backends` |
| CLI and OS-specific targets | `c/targets`, `c/tools` |
| Graphics, windows, input, PNG | `graphics/c` |

Do not add platform-specific behavior to `c/core` unless it changes the
language itself.

## Embedding Checklist

1. Create a context with `plankac_create`.
2. Register host-native functions when `.plk` source needs to call C.
3. Load either the default profile or a custom source list.
4. Inspect procedures with `plankac_context_proc_count` and
   `plankac_context_get_proc`.
5. Run procedures with `plankac_context_run` or `plankac_context_run_number`.
6. Destroy the context with `plankac_destroy`.

Minimal flow:

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

Register a C function callable from `.plk`:

```c
plankac_context_register_native(ctx, "host_mad", 2, 2,
    arg_types, result_types, host_mad, user_data, err, sizeof(err));
```

## Porting PlankaHost

Use `graphics/c/plankahost.h` when the host should load one `.plk`
application profile on top of the standard profile.

Required host responsibilities:

```text
create a window or drawing surface
provide a timer
map input to scene coordinates
allocate a PMG_IMAGE pixel buffer
call plankahost_render
call plankahost_button_at for GUI profiles
call plankahost_timer_step for animated profiles
```

The `.plk` application supplies the program behavior through `app_*`,
`gui_*`, or `cube_*` procedures.

## Windows Targets

Modern Windows builds use GCC/MinGW:

```bat
build.bat
```

The Win16 target uses Open Watcom:

```bat
build-win16.bat
```

The Win16 executable is a 16-bit Windows NE program and uses the compact
PlankaMath runtime. It does not include the full parser/backend toolchain.

## DOS Target

The DOS target uses Open Watcom:

```bat
build-dos.bat
```

The output is `build\dos\PMDOS.EXE`, a 16-bit DOS MZ program with an 8.3
filename. It is a compact runner, not the full PlankaC compiler.

## Backend Porting

Bytecode and generated C are the most portable compiler outputs. The x86-64
ASM backend is platform/assembler dependent. The 8086 backend emits
MASM/TASM-style source and is intended for DOS-oriented experiments.

When adding a backend:

```text
read from PLC_PROGRAM
do not parse source again
keep backend helper code under c/backends
add a command to c/tools/plankac_cli.c
add conformance or equivalence tests
document limits in compiler_guide.md
```

## Current Runtime Constraints

The current runtime uses fixed-size frame tables for variables, indexed
values, field paths, lists, pairs, records, complex values, vectors, and
matrices. Ports should preserve bounds checks and error reporting. A deeper
bit-accurate value model is a language-core change and should be implemented
in the core runtime rather than in a platform target.
