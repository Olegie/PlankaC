# PlankaC Modules

PlankaC is split into focused C modules so the library can grow without turning
the interpreter into one large source file.

See `docs/index.md` for the complete documentation map and
`docs/architecture.md` for the layer boundaries and dependency direction.

| File | Role |
| --- | --- |
| `c/include/plankac.h` | public PlankaC C API |
| `c/include/plankamath.h` | compact PlankaMath fallback API |
| `c/internal/plankac_internal.h` | internal shared types and declarations |
| `c/core/plankac_common.c` | string, source-line, and notation helpers |
| `c/core/plankac_source.c` | `.plk` source loading, bytecode loading, and procedure table building |
| `c/core/plankac_expr.c` | expression parser, guards, calls, and assignment targets |
| `c/core/plankac_runtime.c` | interpreter, public API implementation, value handles, lists, pairs, sets, relations, and complex values |
| `c/types/plankac_types.c` | structure marker parser, type families, and compatibility rules |
| `c/notation/plankac_2d.c` | executable two-dimensional table row expansion |
| `c/notation/plankac_document.c` | PAGE row/cell document model and validation with row/column diagnostics |
| `c/notation/plankac_page.c` | `PAGE`/`ENDPAGE` parser for several spatially bound executable rows |
| `c/analyzer/plankac_analyzer.c` | static checks after source loading, including type marker and call compatibility checks |
| `c/analyzer/plankac_schema.c` | structural schema inference for list/set elements, pairs, and record fields |
| `c/values/plankac_bits.c` | bit packing and raw fixed-point helper operations |
| `c/values/plankac_value.c` | active tagged PLC value storage for banks, fixed values, bits, and handles |
| `c/models/plankac_chess_model.c` | board-level chess legality, legal move count, castling path, promotion, en passant, stalemate, signatures, check, mate, material, and capture search |
| `c/ir/plankac_ir.c` | typed IR construction, validation, and readable IR emission |
| `c/backends/plankac_bytecode.c` | textual bytecode emission, generated C backend emission, and native x86-64 ASM backend emission |
| `c/backends/plankac_lowering.c` | typed backend lowering report for C, x86-64 ASM, and 8086 paths |
| `c/backends/plankac_asm8086.c` | MASM/TASM-style 8086/DOS source emission |
| `c/backends/plankac_native_runtime.c` | native backend helper runtime for generated ASM code |
| `c/tools/plankac_cli.c` | command-line interface |
| `c/tools/plankac.c` | short compatibility note for the split implementation |
| `c/targets/plankamath_cli.c` | compact console host |
| `c/targets/windows_gui.c` | modern Windows GUI host |
| `c/targets/win16_gui.c` | real Win16 GUI host |
| `c/targets/dos_cli.c` | real DOS CLI host |
| `c/legacy/plankamath.c` | compact fallback runtime mirror |

Library builds use `c/core`, `c/types`, `c/notation`, `c/analyzer`,
`c/values`, `c/models`, `c/ir`, `c/backends`, `c/include`, and
`c/internal`.

Embedding examples are kept under `examples/`: `c_api_demo.c` shows C calling
PlankaC procedures, and `c_abi_demo.c` with `host_abi.plk` shows `.plk`
calling a registered C function through the public ABI.

Command-line builds add `c/tools/plankac_cli.c`. GUI, Win16, and DOS targets
live under `c/targets` and link only the pieces they need.

The graphical application layer is split under `graphics/c`:

| File | Role |
| --- | --- |
| `graphics/c/plankahost.h` | shared application host API |
| `graphics/c/plankahost.c` | loads the base profile plus one `.plk` application and exposes procedure/run/render helpers |
| `graphics/c/plankahost_window.c` | Windows launcher for GUI and 3D application profiles |
| `graphics/c/plankahost_demo.c` | command-line proof for host loading, metadata, and base-procedure dispatch |
| `graphics/c/plankagui_*` | 2D scene loader, rasterizer, bitmap text, PNG export, and renderer |
| `graphics/c/plankacube_*` | 3D scene loader, projection renderer, export host, and window host |
