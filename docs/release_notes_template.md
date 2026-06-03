# PlankaC Release

PlankaC is a substantial executable Plankalkuel-profile implementation in C:
parser, interpreter, typed IR, bytecode, generated C, generated x86-64 ASM,
8086/DOS-oriented ASM emission, built-in DOS COM bootstrap emission,
PlankaC DOS runner route,
embedding API, and host applications for `.plk` profiles.

## Included Assets

- Windows x64 tools and hosts: `PlankaC-<tag>-windows-x64.zip`
- Linux x64 tools and exporters: `PlankaC-<tag>-linux-x64.tar.gz`
- Source archive: `PlankaC-<tag>-source.tar.gz`
- Checksums: `SHA256SUMS`

Win16 and DOS packages, when present, are experimental platform builds created
with Open Watcom and attached separately.

## Verified Paths

```text
build.bat
make ci
plankac check
plankac tests
plankac_conformance
plankac compile build/plankac_pipeline
plankac native-c build/plankac_native_c
plankac asm8086 build/plankac_8086.asm
```

Windows CI also links and runs the generated x86-64 ASM runner. Linux CI
validates generated ASM emission structurally and runs generated C/native C,
because the current x86-64 ASM runner targets the Windows/MinGW ABI.

## Scope

This release is not described as a complete historical Plankalkuel
implementation. The honest scope is a substantial executable
Plankalkuel-profile implementation with documented extensions, conformance
fixtures, compiler artifacts, and embeddable C APIs.
