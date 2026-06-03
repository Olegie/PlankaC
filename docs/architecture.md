# Architecture

PlankaC is organized as a compact language toolchain rather than a single
calculator program. The repository keeps the source language, parser,
runtime, analysis, compiler outputs, platform targets, and graphical hosts in
separate layers.

## Layer Map

```text
.plk source files
    -> c/core
    -> c/types
    -> c/notation
    -> c/analyzer
    -> procedure table
    -> c/ir
    -> interpreter / bytecode / C backend / ASM backends / host APIs
```

| Layer | Directory | Responsibility |
| --- | --- | --- |
| Public API | `c/include` | Stable C-facing declarations for embedding PlankaC |
| Internal model | `c/internal` | Shared parser/runtime/backend structures |
| Core | `c/core` | Source loading, header parsing, expression parsing, interpreter, context API |
| Types | `c/types` | Structure markers, type families, width/scale parsing, compatibility checks |
| Notation | `c/notation` | Executable two-dimensional row expansion, PAGE row/cell document modeling, validation, and page/table expansion |
| Analyzer | `c/analyzer` | Static checks after loading, including call, marker, and structural schema consistency |
| Values | `c/values` | Bit packing, tagged PLC value storage, and raw fixed-point value helpers |
| Models | `c/models` | Board-level chess legality, legal move counts, castling paths, promotion, en passant checks, signatures, stalemate, check, mate, material, and capture search |
| Typed IR | `c/ir` | Statement-level IR construction, validation, and readable IR emission |
| Backends | `c/backends` | Text bytecode, lowering reports, generated C, x86-64 ASM, 8086/DOS ASM, DOS COM, native helper runtime |
| Tools | `c/tools` | Command-line interface for checking, running, and emitting artifacts |
| Targets | `c/targets` | Console, DOS, Win16, and Windows GUI launchers |
| Compact runtime | `c/legacy` | Small PlankaMath fallback runtime for narrow platform targets |
| Graphics hosts | `graphics/c` | PlankaHost, PlankaGUI, PlankaCube, raster, font, PNG, and window integration |
| Source profile | `src`, `examples`, `tests` | Executable `.plk` plans and conformance programs |

## Dependency Direction

The intended dependency direction is one-way:

```text
source files
    -> core parser/runtime
    -> type and notation helpers
    -> analyzer
    -> backends or hosts
```

Backends and hosts should depend on the loaded procedure table and public API.
They should not introduce new source-language rules. If a feature changes
what `.plk` means, it belongs in `c/core`, `c/types`, `c/notation`, or
`c/analyzer`, not in a target-specific file.

Platform code belongs in `c/targets` or `graphics/c`. Win16, DOS, Win32, PNG
export, and animated windows are host concerns. They may call procedures, list
metadata, draw pixels, or dispatch input, but the program model must remain in
the loaded `.plk` profile.

## Runtime Path

The interpreter path is:

```text
plankac_context_load_sources()
    -> source lines
    -> procedures
    -> static analysis
    -> typed IR validation
    -> plankac_context_run()
    -> PLC_FRAME
    -> PLANKAC_RESULT
```

`PLC_FRAME` stores the current execution frame. The current implementation
uses numeric slots for host-facing compatibility, active tagged PLC value
entries for bits/fixed values/handles, and handle tables for lists, pairs,
sets, records, complex values, vectors, and matrices. Bit and fixed-point
helpers use raw scaled integer operations at the value-helper layer before
results are returned to the host-visible frame.

## Compiler Outputs

The backend layer emits several forms from the same loaded procedure table:

```text
typed IR     -> readable statement-level compiler contract
bytecode     -> readable procedure image, reloadable by PlankaC
lowering     -> backend plan for scalar and compound operations
generated C  -> portable runner with embedded bytecode
x86-64 ASM   -> generated native procedure dispatcher plus helper runtime
8086 ASM     -> DOS-oriented assembly source with direct arithmetic procedures and compound tables
DOS COM      -> direct 8086 bootstrap plus embedded bytecode emitted without an assembler
DOS runner   -> PlankaC API runner for a real 16-bit DOS MZ executable
native exe   -> linked generated C or linked generated x86-64 ASM
```

The backends are deliberately placed under `c/backends` so their limits are
visible. Improving compiler output should not require editing the GUI, DOS, or
Win16 targets.

The `compile <prefix>` command is the stable compiler route: it emits
bytecode/IR, reloads the IR, then emits generated C, x86-64 ASM, 8086 ASM,
and a DOS COM bootstrap from that reloaded program. `build-dos-plankac.bat`
uses the API runner target for the full DOS executable route. `lowering <path>`
exposes the backend plan for
inspection. `native-c <prefix>` and `native-asm <prefix>` add the external
toolchain link step.

## Host Path

PlankaHost is the shared application host:

```text
PlankaHost.exe <application.plk>
    -> standard PlankaC profile
    -> application profile
    -> app_kind / app_canvas / app_checksum / app_timer_step
    -> renderer or input bridge
```

This keeps application behavior in `.plk` while C handles windows, timers,
mouse and keyboard input, and pixel buffers. `PlankaGUI.exe` and
`PlankaCube.exe` are direct launchers for specific profiles; `PlankaHost` is
the reusable host API.

## Language Core Versus Extensions

The language core tracks the Plankalkuel-derived profile: plans,
`V`/`C`/`Z`/`R` variables, type rows/markers, assignment equations, constants,
guards, calls, loops, assertions, contracts, stop criteria, structured values,
lists, pairs, sets, relations, predicate forms, relation/schema signatures,
bit/fixed helpers, exception helpers, page/table notation, and board-level
chess models.

The 3D geometry and graphical application contracts are PlankaC extensions.
They are useful host/application examples, but they should not hide missing
work in the core language profile. Core coverage is tracked in
`docs/plankalkuel_coverage.md`.

## Documentation Set

The public manual is split into five stable entry points:

| Document | Role |
| --- | --- |
| `docs/language_reference.md` | syntax and execution rules for `.plk` source |
| `docs/standard_library.md` | callable procedure profile and runtime helper domains |
| `docs/compiler_guide.md` | CLI, bytecode, generated C, ASM, and verification workflow |
| `docs/compiler_pipeline.md` | stable source-to-IR-to-native route |
| `docs/dos_backend.md` | DOS-oriented ASM route, built-in COM bootstrap emission, and PlankaC DOS runner |
| `docs/examples.md` | source and command examples |
| `docs/porting_guide.md` | embedding and platform porting notes |
| `docs/abi.md` | bidirectional C ABI and native function registration |

`docs/index.md` is the top-level map for the full documentation set.

## Engineering Rule

A feature is considered part of the serious toolchain only when it has:

```text
source syntax
runtime behavior
tests or conformance coverage
documentation
```

This rule keeps the repository useful as a language implementation instead of
only a collection of demos.
