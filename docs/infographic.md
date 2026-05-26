# PlankaC Infographic

This page is a compact engineering map of the repository. The numbers below
come from the current tree, not from a project pitch.

## Snapshot

| Metric | Value |
| --- | ---: |
| Loaded PlankaC source profile | 24 files |
| Repository `src/*.plk` files | 20 files |
| Graphics `.plk` profiles | 2 files |
| Loaded procedures | 118 procedures |
| C source/header modules | 37 files |
| Negative conformance fixtures | 17 files |
| Main host language | C89-oriented C |
| Main compiler artifacts | bytecode, generated C, x86-64 ASM, 8086/DOS ASM |
| 16-bit targets | Win16 GUI, DOS runner |

## Procedure Mix

```mermaid
pie showData
    title Procedure profile by domain
    "Core calculator and math" : 30
    "Structured data and relations" : 44
    "Chess model" : 12
    "3D geometry extension" : 14
    "Complex values" : 5
    "2D notation sessions" : 4
    "Self-checks" : 9
```

The calculator is the narrowest executable host, not the boundary of the
language work. The procedure profile is weighted toward data modeling,
relation algebra, typed value families, chess-domain structures, 3D geometry,
and backend/conformance checks.

## C Module Weight

```mermaid
pie showData
    title C implementation lines by module directory
    "core" : 4391
    "backends" : 2873
    "graphics" : 3250
    "targets" : 1456
    "legacy" : 515
    "analyzer" : 356
    "types" : 214
    "tools" : 204
    "internal" : 193
    "notation" : 173
    "include" : 143
```

The dominant implementation mass is concentrated in the expected subsystems:
loader/runtime logic in `c/core`, compiler-output logic in `c/backends`, and
host integrations in `graphics/c` and `c/targets`.

## Coverage Status

```mermaid
pie showData
    title Coverage matrix status count
    "supported" : 13
    "supported subset" : 15
    "partial" : 2
    "supported extension" : 2
```

The coverage matrix is intentionally strict. `docs/plankalkuel_coverage.md`
is the contract: supported means there is source syntax, runtime behavior,
and test coverage.

## Toolchain

```mermaid
flowchart LR
    PLK[".plk source plans"] --> Loader["Source loader"]
    Loader --> Notation["Linear and aligned 2D notation"]
    Notation --> Analyzer["Static analyzer"]
    Analyzer --> Table["Procedure table"]

    Table --> Run["Interpreter"]
    Table --> BC["Text bytecode"]
    Table --> CGen["Generated C"]
    Table --> ASM64["x86-64 ASM"]
    Table --> ASM86["8086/DOS ASM"]
    Table --> API["C API"]
    Table --> Host["PlankaHost app context"]

    API --> GUI["Modern Windows GUI"]
    Host --> GUI2["2D GUI profile"]
    Host --> Cube["3D cube profile"]
    BC --> BCRun["Bytecode runner"]
```

## Backend Map

| Output | Command | Artifact | Directness |
| --- | --- | --- | --- |
| Interpreter | `plankac run <proc>` | no generated file | direct execution of loaded `.plk` profile |
| Bytecode | `plankac bytecode out.pbc` | readable `.pbc` | compiler artifact, reloadable by PlankaC |
| Generated C | `plankac cgen out.c` | C runner with embedded bytecode | portable host runner |
| x86-64 ASM | `plankac asmgen out.S` | native ASM runner | generated procedures plus helper runtime |
| 8086/DOS ASM | `plankac asm8086 out.asm` | MASM/TASM-style source | arithmetic-core lowering plus full bytecode image |

## Backend Maturity

Scale: `5` means the path is practical for the current repository profile.
Lower values are useful, but intentionally narrower.

| Backend | Parser integrated | Compound values | Standalone artifact | 16-bit relevance | Current score |
| --- | ---: | ---: | ---: | ---: | --- |
| Interpreter | 5 | 5 | 1 | 1 | `##################` |
| Bytecode | 5 | 5 | 4 | 2 | `################` |
| Generated C | 5 | 5 | 4 | 2 | `################` |
| x86-64 ASM | 4 | 4 | 4 | 2 | `##############` |
| 8086/DOS ASM | 3 | 1 | 4 | 5 | `#############` |
| Win16/DOS compact hosts | 1 | 1 | 4 | 5 | `###########` |

## Execution Surfaces

```mermaid
flowchart TB
    subgraph Modern["Modern path"]
        A["plankac.exe"] --> B["Interpreter"]
        A --> C["Bytecode"]
        A --> D["Generated C"]
        A --> E["x86-64 ASM"]
        B --> F["Modern Windows GUI"]
        A --> O["runfile app procedures"]
    end

    subgraph SixteenBit["16-bit path"]
        G["Open Watcom"] --> H["Win16 GUI"]
        G --> I["DOS runner"]
        A --> J["8086 ASM source"]
    end

    subgraph Library["Library path"]
        K["libplankac.a"] --> L["External C host"]
        K --> M["Procedure metadata"]
        K --> N["Run by name or P-number"]
        K --> P["PlankaHost app launcher"]
    end
```

## Language Basis And Engineering Decisions

| Area | Source basis | PlankaC engineering decision |
| --- | --- | --- |
| Plans / procedures | numbered plans | `P<number> name (...) => ... END` |
| Value rows | `V`, `Z`, `R` classes | direct runtime variable banks |
| Type rows | structure markers | parsed marker families with width and scale |
| 2D notation | expression row plus value/type rows | executable aligned `|`, `V|`, `S|` rows |
| Chess examples | known Plankalkuel modeling domain | executable board and attack-map procedures |
| 3D geometry | outside the documented core profile | explicit PlankaC extension |
| 8086 output | 16-bit execution target | separate DOS-oriented backend artifact |

## Verification Route

```text
build.bat
build\plankac.exe check
build\plankac.exe run calculator_demo
build\plankac.exe run relation_inverse_session
build\plankac.exe run chess_queen_attack_map_full_session
build\plankac.exe run three_d_pipeline_session
build\plankac.exe runfile graphics\src\plankagui.plk app_kind
build\plankac.exe runfile graphics\src\plankacube.plk app_kind
build\plankac.exe bytecode build\plankamath.pbc
build\plankac.exe cgen build\plankac_generated.c
build\plankac.exe asmgen build\plankac_asm_runtime.S
build\plankac.exe asm8086 build\plankac_8086.asm
build\plankac_conformance.exe
```

Expected high-level result:

```text
PlankaC OK: 24 files, 118 procedures
Bytecode OK: 118 procedures
8086 ASM written: build\plankac_8086.asm
CONFORMANCE OK
```

## Remaining Implementation Work

```mermaid
flowchart LR
    A["Current PlankaC"] --> B["Full page-layout 2D parser"]
    A --> C["Deeper compound type inference"]
    A --> D["Source-accurate value model"]
    A --> E["Full chess legality model"]
    A --> F["16-bit compound runtime"]
    A --> G["More backend equivalence tests"]
```

The useful standard for this project is simple: add a feature only when it has
source examples, runtime behavior, docs, and conformance coverage.
