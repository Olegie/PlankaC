# Documentation Index

This directory is the documentation set for PlankaC as a language toolchain.
Start with the primary guides, then use the engineering references for
implementation details and verification.

## Primary Guides

| Document | Purpose |
| --- | --- |
| `spec/index.md` | formal language specification: grammar, value model, type rules, execution rules, errors, and backend contract |
| `language_reference.md` | Source syntax, procedure model, variables, expressions, calls, guards, loops, assertions, and type markers |
| `standard_library.md` | Built-in procedures and the bundled `.plk` procedure profile |
| `compiler_guide.md` | Command-line tools, bytecode, lowering, generated C, x86-64 ASM, 8086 ASM, and verification commands |
| `compiler_pipeline.md` | Stable `.plk -> IR/bytecode -> lowering -> C/ASM -> native executable` route |
| `examples.md` | Small source and command examples for arithmetic, guards, data structures, relations, chess, graphics, and 3D |
| `porting_guide.md` | Embedding, platform targets, Win16/DOS notes, host integration, and constraints |
| `abi.md` | C ABI for calling PlankaC from C and registered C functions from `.plk` |
| `tutorials/max3_to_native.md` | End-to-end `.plk` tutorial from source to bytecode, generated C, generated ASM, and native runners |
| `technical_report.md` | English/German technical report on scope, source basis, architecture, backends, and verification |
| `de/plankac.md` | German project page |

## Reading Order

For language users:

```text
spec/index.md -> language_reference.md -> standard_library.md -> examples.md
```

For compiler and host work:

```text
architecture.md -> compiler_guide.md -> compiler_pipeline.md -> abi.md -> plankac_api.md -> plankahost_api.md
```

For ports:

```text
porting_guide.md -> toolchain.md -> conformance.md
```

## Engineering References

| Document | Purpose |
| --- | --- |
| `architecture.md` | Layer map and dependency direction |
| `spec/backend_contract.md` | typed IR, lowering, and backend contract |
| `execution_model.md` | What runs, how `.plk` source is loaded, and where fallback hosts fit |
| `plankalkuel_coverage.md` | Coverage matrix against the Plankalkuel-derived feature set |
| `plankac_api.md` | C embedding API |
| `plankahost_api.md` | Shared `.plk` application host API |
| `plankac_bytecode.md` | Text bytecode format and runner |
| `plankac_modules.md` | C module list |
| `program_catalog.md` | Numbered procedure catalog |
| `conformance.md` | Valid and invalid parser/runtime fixtures |
| `release_guide.md` | Tagging, binary policy, release assets, and checksum workflow |
| `release_artifacts_note.md` | Public note for release bundles and experimental platform builds |
| `review_request.md` | Draft issue/discussion text for outside technical review |
| `bibliography.md` | Source literature |
