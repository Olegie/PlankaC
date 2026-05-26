# PlankaC Modules

PlankaC is split into small C modules so the library can grow without turning
the interpreter into one large source file.

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
| `c/analyzer/plankac_analyzer.c` | static checks after source loading, including type marker and call compatibility checks |
| `c/backends/plankac_bytecode.c` | textual bytecode emission, generated C backend emission, and native x86-64 ASM backend emission |
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
`c/backends`, `c/include`, and `c/internal`.

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
