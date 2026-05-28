# PlankaC Research Track

This directory is for reproducible work around executable Plankalkuel-profile
programs. It is not a place for priority claims. The rule is simple: a claim
belongs here only if it has an input corpus, a command, an output artifact and a
failure condition.

## Current Hypothesis

A Plankalkuel-profile program can be represented as a stable executable
procedure catalog whose observable structure survives the route:

```text
.plk source -> loaded procedure table -> bytecode/IR -> C/ASM/8086 emission
```

The research value is not that the route exists. The value is that the route can
be measured. PlankaC now emits an evidence packet with a deterministic
fingerprint, procedure catalog, statement-class counts, bank usage, type-family
counts and backend contract metadata.

## Reproducible Command

```bat
build\plankac.exe evidence build\plankac.evidence.json
build\plankac.exe evidencefile examples\max3.plk build\max3.evidence.json
```

On Linux:

```sh
make ci
build/plankac evidence build/plankac.evidence.json
```

## What Counts As Evidence

The evidence packet records:

- PlankaC version and deterministic source fingerprint.
- Number of loaded source files and procedures.
- Procedure catalog with P-number, name, arity, result count and statement
  count.
- V/C/Z/R bank references observed in statements.
- Type-family usage across declarations and statements.
- Statement classes: assignments, guarded assignments, procedure calls,
  contracts, assertions, loops, stop criteria and constants.
- Backend contract surface for bytecode, typed IR, generated C, generated
  x86-64 ASM and generated 8086 ASM.

## Failure Condition

The hypothesis weakens if two platforms load the same source corpus but produce
different evidence fingerprints, or if bytecode reload changes the observable
procedure catalog. In that case the project has found a compiler-stability bug,
not a documentation issue.

## Possible Research Contribution

If the evidence protocol remains stable across a larger corpus, it can become a
practical method for studying source-oriented Plankalkuel profiles without
depending on one handwritten interpreter path. That is a defensible research
contribution: a reproducible normalization and measurement route for a
Plankalkuel-derived executable notation.
