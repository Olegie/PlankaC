# Examples

This page collects focused PlankaC source and command examples. All commands are
run from the repository root after `build.bat`.

## Arithmetic Procedure

```text
P10 add (V0[:32.16], V1[:32.16]) => R0[:32.16]
(V0[:32.16] + V1[:32.16]) => R0[:32.16]
END
```

Run:

```bat
build\plankac.exe run add 12 8
```

Expected:

```text
R0=20
```

## Guarded Division

```text
P14 divide_checked (V0[:32.16], V1[:32.16]) => R0[:32.16], R1[:1.1]
0[:32.16] => R0[:32.16]
1[:1.1] => R1[:1.1]
(V1[:32.16] != 0[:32.16]) => (V0[:32.16] / V1[:32.16]) => R0[:32.16]
(V1[:32.16] != 0[:32.16]) => 0[:1.1] => R1[:1.1]
END
```

Run:

```bat
build\plankac.exe run divide_checked 84 0
```

Expected:

```text
R0=0 R1=1
```

## Loop

```text
P112 loop_multiply (V0[:32.16], V1[:32.0]) => R0[:32.16]
0[:32.16] => R0[:32.16]
LOOP V1[:32.0] => (R0[:32.16] + V0[:32.16]) => R0[:32.16]
END
```

Run:

```bat
build\plankac.exe run loop_multiply 3 4
```

Expected:

```text
R0=12
```

## List Session

```text
P113 list_session () => R0[:32.16]
list_new() => Z1[:L32.0]
list_push(Z1[:L32.0], 7[:32.16]) => Z1[:L32.0]
list_push(Z1[:L32.0], 8[:32.16]) => Z1[:L32.0]
list_get(Z1[:L32.0], 0[:32.0]) => Z2[:32.16]
list_len(Z1[:L32.0]) => Z3[:32.0]
(Z2[:32.16] + Z3[:32.0]) => R0[:32.16]
END
```

Run:

```bat
build\plankac.exe run list_session
```

Expected:

```text
R0=9
```

## Complex Values

```bat
build\plankac.exe run complex_norm_session
build\plankac.exe run complex_equal_session
```

Expected:

```text
R0=25
R0=1
```

## Relation Algebra

```bat
build\plankac.exe run relation_inverse_session
build\plankac.exe run relation_image_session
build\plankac.exe run relation_compose_session
build\plankac.exe run relation_range_selection_session
build\plankac.exe run relation_quantifier_session
```

These examples exercise pair lists, relation inversion, image projection, and
composition, range selection, and quantified relation checks.

## Two-Dimensional Row

```text
P134 two_dimensional_fuller () => R0[:32.16]
 | Z + 1[:32.16] => R
V| 4                0
S| 32.16            32.16
END
```

Run:

```bat
build\plankac.exe run two_dimensional_fuller
```

## Chess Structures

```bat
build\plankac.exe run chess_piece_struct_session
build\plankac.exe run chess_queen_attack_map_full_session
build\plankac.exe run chess_check_model_session
build\plankac.exe run chess_checkmate_session
build\plankac.exe run chess_en_passant_session
build\plankac.exe run chess_stalemate_session
build\plankac.exe run chess_fen_signature_session
```

These procedures are structural examples: board state, piece encoding, attack
maps, legal move checks, material search, side-in-check, checkmate, stalemate,
en passant, and position signatures.

## Compiler Lowering

```bat
build\plankac.exe lowering build\plankac.lowering
```

The generated report shows the selected backend path for each typed IR
statement, including scalar paths and compound runtime entry points.

## Application Profile

Run the GUI profile as source:

```bat
build\plankac.exe runfile graphics\src\plankagui.plk app_canvas
```

Run the 3D profile as source:

```bat
build\plankac.exe runfile graphics\src\plankacube.plk cube_scene_checksum
```

## Max3 To Native

`examples/max3.plk` is the shortest end-to-end compiler example. It defines a
max-of-two helper, a max-of-three procedure, and `max3_demo`.

```text
build\plankac.exe checkfile examples\max3.plk
build\plankac.exe runfile examples\max3.plk max3_demo
build\plankac.exe compile build\plankac_pipeline
build\plankac.exe native-c build\plankac_native_c
build\plankac.exe native-asm build\plankac_native_asm
```

The full walk-through is in `docs/tutorials/max3_to_native.md`.

Open the shared host:

```bat
build\PlankaHost.exe graphics\src\plankagui.plk
build\PlankaHost.exe graphics\src\plankacube.plk
```

## External C API

```c
#include "plankac.h"

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

See `plankac_api.md` for a complete embedding example.

## Native C Function ABI

`examples/host_abi.plk` calls a C function registered by the host:

```text
P700 host_bridge (V0[:32.16], V1[:32.16]) => R0[:32.16], R1[:1.1]
host_mad(V0[:32.16], V1[:32.16]) => R0[:32.16], R1[:1.1]
END
```

Build and run:

```bat
build\plankac_abi_demo.exe
```

Expected:

```text
native functions: 1
host_mad argc=2 results=2
host_bridge(6, 6) -> R0=42 R1=0
```

See `abi.md` for the registration contract.
