# Source Basis

This project follows a conservative linear Plankalkuel profile based on the
historical descriptions of Zuse's language and later implementation work.

## Historical Anchors

- Zuse worked on a larger logic/programming document in 1942/43 and continued
  with the Plankalkuel document in 1945.
- Plankalkuel is described as a high-level imperative programming language.
- Its programs behave as reusable, non-recursive functions.
- Variable types are written when variables are used, instead of in a modern
  declaration block.
- Main constructs include assignment, arithmetic and logical operations,
  guarded commands, and loops.
- The original notation is two-dimensional; this project uses a linear profile
  for plain-text source files.
- The FU Berlin / Feinarbeit implementation from 2000 is treated as the
  main modern implementation reference for this project's linear profile.

## Adopted Rules

1. Programs are written as numbered plans: `P10`, `P20`, `P30`.
2. Inputs use `V` variables.
3. Intermediate values use `Z` variables.
4. Results use `R` variables.
5. Constants are written as typed values such as `2[:32.16]`.
6. Every variable use carries a structure marker.
7. Assignments are program equations with `=>`.
8. Guarded equations use `condition => value => target`.
9. Procedures are called by value.
10. Procedures are non-recursive.
11. Error cases are represented by result/status pairs, not exceptions.
12. No modern host language is part of the `.plk` source.

## Scope

The project started as calculator procedures: arithmetic, comparisons,
scientific helpers, guarded division, memory operations, and self-checks.
PlankaC now also contains a small executable language profile for indexed
values, record fields, frame-local lists, loops, assertions, and chess-style
movement predicates.

Implementation coverage is tracked in `docs/plankalkuel_coverage.md`. That
matrix is the boundary between supported PlankaC behavior, partial historical
modeling, and features kept as future work.

## Type Markers / Typmarkierungen

```text
[:1.1]       Ja-Nein-Wert, one-bit boolean
[:32.0]      signed whole number
[:32.16]     fixed-point calculator number
```

The compact `[:...]` notation is used as a linear equivalent of the
two-dimensional `S` structure row.
