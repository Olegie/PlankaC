# Plankalkuel Coverage

This file tracks how PlankaC maps the historical Plankalkuel feature set to
the current implementation.

The coverage list is based on the project bibliography and public historical
summaries by Britannica and Horst Zuse. Those sources describe Plankalkuel as a
high-level language with procedures, two-dimensional notation, structured
data, conditionals, loops, assignment, Boolean/predicate logic, lists, arrays,
records, assertions, and exception handling.

## Coverage Matrix

| Feature | Status | PlankaC handling |
| --- | --- | --- |
| Numbered plans / procedures | supported | `P10 add`, procedure table, calls by name or `P` number |
| Subprogram calls | supported | `CALL`, `GCALL`, runtime procedure calls |
| Assignment with `=>` | supported | expression and call assignment to `R`/`Z` targets |
| `V`, `Z`, `R` variable classes | supported | inputs, intermediate values, result values |
| Structure markers | supported subset | markers such as `[:1.1]`, `[:32.0]`, `[:32.16]` are parsed and carried |
| Fixed-point calculator values | supported subset | represented as `double` in the current runtime |
| Boolean values | supported | `0` and `1`, comparisons, `&`, `|`, `!` |
| Arithmetic expressions | supported | `+`, `-`, `*`, `/`, `%`, `^`, parentheses |
| Comparisons | supported | `=`, `!=`, `<`, `<=`, `>`, `>=` |
| Guarded commands / conditionals | supported | `condition => value => target` |
| Multiple result values | supported | procedures such as `divide_checked` return `R0`, `R1` |
| Textual compiler output | supported | `plankac bytecode output.pbc`, `checkbc`, `runbc` |
| Bytecode runner | supported subset | textual bytecode can be loaded into a context and executed |
| C backend output | supported subset | `plankac cgen output.c` emits a small C runner with embedded bytecode |
| Native ASM backend | supported subset | `plankac asmgen output.S` emits x86-64 assembly procedures and a dispatcher without calling the interpreter |
| C embedding API | supported | `PLANKAC_CONTEXT`, custom source lists, bytecode loading, procedure metadata, `plankac_context_run` |
| Conformance checks | supported | valid and invalid `.plk` fixtures under `tests/conformance` |
| Two-dimensional original notation | supported subset | executable `|` statement rows are accepted; `V|` and `S|` rows are kept as notation rows |
| Arrays / indexed data | supported subset | fixed-size indexed refs such as `Z10[0][:32.16]` execute in the frame model |
| Records / hierarchical structures | supported subset | nested field refs such as `Z40.white.king.file[:32.0]` execute in the frame model |
| Lists and pairs | supported subset | frame-local list handles plus pair helpers such as `pair`, `pair_left`, `pair_list_count_first` |
| Sets / relations | supported subset | `set_new`, `set_add`, `set_contains`, `set_union`, `set_intersection`; relation examples are list-of-pair plans |
| Loops / repetition | supported subset | `LOOP count => value => target` repeats an assignment |
| Assertions | supported subset | `ASSERT (expr)` stops execution on false predicates |
| Complex numbers | documented | historical feature, not part of the current executable subset |
| Floating-point type model | partial | markers are validated, stored in procedure metadata, and checked across direct calls; values still use host `double` |
| Arithmetic exception handling | partial | calculator uses result/status pairs such as `R1[:1.1]` |
| No recursion | supported subset | static direct/indirect recursion rejection plus runtime call-depth protection |
| Chess/list examples | supported subset | executable movement predicates, nested piece records, and attack relations in `src/07_chess.plk` and `src/08_relations_sets.plk` |

## Next Implementation Targets

1. Wider type-shape inference across expressions, records, lists, and procedure calls.
2. More historical structure forms, especially richer list and set algebra.
3. Complex-number and floating-point examples from the historical material.
4. A larger chess/modeling suite beyond single-move predicates.
5. More native backend targets beyond Windows x86-64 GAS assembly.

The project should keep this matrix honest: a feature moves to `supported`
only when it has source examples, runtime behavior, and conformance coverage.
