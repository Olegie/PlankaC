# Evidence Protocol

The evidence protocol is a compact, machine-readable profile of a loaded
PlankaC program. It exists to make language work auditable.

## Format

```json
{
  "format": "plankac-evidence-v1",
  "version": "1.1.0",
  "fingerprint": "plc-...",
  "source_files": 29,
  "procedures": 148,
  "statements": 0,
  "statement_kinds": {},
  "banks": {},
  "type_families": {},
  "backend_contract": {},
  "catalog": []
}
```

The fingerprint is derived from the PlankaC version, loaded source count,
procedure numbers, procedure names, arities, result counts, type markers,
statement text and source line numbers. It is intended as a stability signal,
not as a cryptographic hash.

## Interpretation

The protocol separates three questions:

1. Did the loader see the same language profile?
2. Did typed source structure survive into a catalog that can be inspected?
3. Are the promised backend surfaces present for the same loaded program?

This is deliberately stricter than a screenshot or a calculator demo. A demo can
work while the language profile silently changes. The evidence packet makes that
change visible.

## Minimal Acceptance

A valid evidence packet must contain:

- `format = plankac-evidence-v1`
- a `fingerprint`
- positive `procedures`
- a non-empty `catalog`
- backend contract entries for bytecode, IR, C, x86-64 ASM and 8086 ASM

The conformance runner writes `build/conformance_evidence.json` and verifies the
basic shape through the public embedding API.
