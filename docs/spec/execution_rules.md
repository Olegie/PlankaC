# Execution Rules

## Loading

The loader reads a list of `.plk` sources and builds a procedure table. During
loading it also validates duplicate procedures, type markers, recursive calls,
interprocedure call contracts, structural schemas, PAGE documents, and typed IR.

The default profile currently loads 29 source files and 150 procedures.

## Frames

Each procedure call receives a fresh frame. Arguments are copied into `V`
slots. Results are read from `R` slots after the procedure finishes. Child
calls share the heap owner of the caller so compound handles remain valid
across a call chain.

## Assignment

An assignment evaluates the left side and stores the result into each target:

```text
V0[:32.16] + V1[:32.16] => R0[:32.16]
```

Multi-result calls assign results left to right:

```text
divide_checked(V0[:32.16], V1[:32.16]) => R0[:32.16], R1[:1.1]
```

`CONST` writes only into the `C` bank. Ordinary assignment to `C` is rejected.

## Expression Execution

Scalar expressions are parsed into the bounded expression AST and evaluated
from that tree when all nodes are supported. This covers typed numeric
literals, bank references, parenthesized groups, unary and binary operators,
nested procedure calls, comparisons, Boolean operators, and predicate forms.
The legacy recursive text evaluator remains available for compatibility when a
source fragment is outside the AST execution subset.

## Guards, Loops, And Stops

A guarded assignment has three arrow-separated parts:

```text
(Z1[:32.16] < V1[:32.16]) => V1[:32.16] => Z1[:32.16]
```

The value part runs only when the guard is true. `LOOP` repeats a guarded body
using the evaluated loop count. `STOPIF` ends the current procedure when its
condition is true.

## PAGE Execution

`PAGE`/`ENDPAGE` rows are built as a row/cell document with row and column
coordinates, validated as a document, then expanded into ordinary linear
statements before analysis. This means PAGE code and linear code share the
same interpreter, typed IR, bytecode, and generated backends.

## Host ABI

C hosts call PlankaC through `plankac_context_run` or `plankac_run`. C can also
register a native function with type-marked argument/result contracts. `.plk`
source calls the registered function exactly like a normal procedure.

The graphics host keeps windowing, timers, pointer input, keyboard input, and
raster output in C. The scene profile, geometry values, checksums, and command
procedures remain in `.plk`.
