# PlankaC Modules

PlankaC is split into small C modules so the library can grow without turning
the interpreter into one large source file.

| File | Role |
| --- | --- |
| `c/plankac.h` | public C API |
| `c/plankac_internal.h` | internal shared types and declarations |
| `c/plankac_common.c` | string, source-line, and notation helpers |
| `c/plankac_source.c` | `.plk` source loading, bytecode loading, two-dimensional row expansion, type checks, and procedure table building |
| `c/plankac_expr.c` | expression parser, guards, calls, and assignment targets |
| `c/plankac_bytecode.c` | textual bytecode emission, generated C backend emission, and native ASM backend emission |
| `c/plankac_runtime.c` | execution engine and public API implementation |
| `c/plankac_native_runtime.c` | native backend helper runtime for generated ASM code |
| `c/plankac_cli.c` | command-line interface |
| `c/plankac.c` | short compatibility note for the split implementation |

Library builds use all modules except `c/plankac_cli.c`.

Command-line builds add `c/plankac_cli.c`.
