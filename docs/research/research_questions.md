# Research Questions

These are realistic questions for PlankaC. They are intentionally phrased so
that the answer can be tested.

## RQ1: Stable Source Profile

Can a Plankalkuel-profile source corpus be reduced to a deterministic procedure
catalog that remains stable across platforms and build routes?

Current mechanism: `plankac evidence`.

## RQ2: Executable Two-Dimensional Notation

Can table notation be normalized into the same procedure model as linear source
without losing the association between expression, index row and type row?

Current mechanism: `c/notation`, page fixtures and evidence catalog checks.

## RQ3: Backend Agreement

Can source execution, bytecode execution, generated C and generated ASM preserve
the same observable procedure results for arithmetic, relation, structured
value, chess and graphics-adjacent procedures?

Current mechanism: conformance fixtures plus backend smoke tests.

## RQ4: Typed Value Families

How far can typed families such as fixed numeric values, booleans, lists, sets,
pairs, records, vectors, matrices and board states be tracked before the system
needs a deeper structural type algebra?

Current mechanism: schema analyzer, typed IR and evidence type-family counts.

## Boundary

PlankaC should not claim a complete historical Plankalkuel implementation until
the open gaps are closed and independently reviewed. The present research track
is narrower and stronger: a reproducible executable profile with measurable
source, type and backend evidence.
