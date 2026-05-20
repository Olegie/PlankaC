# Plankalkuel Coverage

This file tracks how PlankaC maps the Plankalkuel feature set described in
the source literature to the current implementation.

The coverage list is based on the project bibliography and public reference
summaries by Britannica and Horst Zuse. Those sources describe Plankalkuel as
a high-level language with procedures, two-dimensional notation, structured
data, conditionals, loops, assignment, Boolean/predicate logic, lists, arrays,
records, assertions, and exception handling.

## Coverage Matrix

| Feature | Status | PlankaC handling |
| --- | --- | --- |
| Numbered plans / procedures | supported | `P10 add`, procedure table, calls by name or `P` number |
| Subprogram calls | supported | `CALL`, `GCALL`, runtime procedure calls |
| Assignment with `=>` | supported | expression and call assignment to `R`/`Z` targets |
| `V`, `Z`, `R` variable classes | supported | inputs, intermediate values, result values |
| Structure markers | supported | markers such as `[:1.1]`, `[:32.0]`, `[:32.16]`, `[:C32.16]`, `[:L32.0]`, `[:S32.0]`, `[:P32.0]`, `[:Q32.0]`, `[:VEC32.16]`, and `[:MAT32.16]` are parsed into type families |
| Fixed-point calculator values | supported subset | bit width and scale are parsed; runtime arithmetic still uses host `double` |
| Boolean values | supported | `0` and `1`, comparisons, `&`, `|`, `!` |
| Arithmetic expressions | supported | `+`, `-`, `*`, `/`, `%`, `^`, parentheses |
| Comparisons | supported | `=`, `!=`, `<`, `<=`, `>`, `>=` |
| Guarded commands / conditionals | supported | `condition => value => target` |
| Multiple result values | supported | procedures such as `divide_checked` return `R0`, `R1` |
| Textual compiler output | supported | `plankac bytecode output.pbc`, `checkbc`, `runbc` |
| Bytecode runner | supported subset | textual bytecode can be loaded into a context and executed |
| C backend output | supported subset | `plankac cgen output.c` emits a small C runner with embedded bytecode |
| Native ASM backend | supported subset | `plankac asmgen output.S` emits x86-64 assembly procedures and a dispatcher without calling the interpreter |
| 8086/DOS ASM backend | supported subset | `plankac asm8086 output.asm` emits MASM/TASM-style 16-bit source with a full bytecode image and direct arithmetic-core procedures |
| C embedding API | supported | `PLANKAC_CONTEXT`, custom source lists, bytecode loading, procedure metadata, run by name or number, bytecode/C/ASM/8086 output |
| Conformance checks | supported | valid and invalid `.plk` fixtures under `tests/conformance` |
| Two-dimensional original notation | supported subset | executable `|` statement rows with aligned `V|`/`S|` tables are accepted, including swapped row order and spaced cell alignment; full page-layout parsing is not claimed |
| Arrays / indexed data | supported subset | fixed-size indexed refs such as `Z10[0][:32.16]` execute in the frame model |
| Records / hierarchical structures | supported subset | nested field refs and handle-backed records execute in the shared frame heap |
| Lists and pairs | supported subset | shared frame-heap list handles plus pair helpers such as `pair`, `pair_left`, `pair_equal`, `list_equal`, `pair_list_count_first`, and relation pair lists |
| Sets / relations | supported subset | `set_new`, `set_add`, `set_contains`, `set_union`, `set_intersection`, `set_difference`, `set_subset`, `set_equal`, `set_cartesian`, `relation_inverse`, `relation_image`, `relation_compose`, relation domain/range/pair predicates, and simple quantifier-like helpers |
| Loops / repetition | supported subset | `LOOP count => value => target` repeats an assignment |
| Assertions | supported subset | `ASSERT (expr)` stops execution on false predicates |
| Complex numbers | supported subset | `[:C32.16]` markers, handle-backed complex values, real/imag access, add/sub/mul/conjugate/norm/equality |
| 3D geometry extension | supported extension | not part of the documented Plankalkuel core profile; PlankaC adds `vec3`, dot/cross/normalize, `mat4`, transform, projection, and pipeline examples in `src/15_3d_geometry.plk` |
| Type/value model | partial | markers are parsed into families with width/scale pieces, stored in metadata, and checked across calls; values still use compact numeric/handle runtime storage |
| Arithmetic exception handling | partial | calculator uses result/status pairs such as `R1[:1.1]` |
| No recursion | supported subset | static direct/indirect recursion rejection plus runtime call-depth protection |
| Chess/list examples | supported subset | executable movement predicates, nested piece records, board-square helpers, board state records, attack maps, and check-style examples in `src/07_chess.plk`, `src/08_relations_sets.plk`, `src/13_chess_board.plk`, and `src/17_chess_model.plk` |

## Next Implementation Targets

1. Full page-layout parser for arbitrary two-dimensional forms, not only
   aligned executable rows.
2. More type inference across nested records, list element shapes, set element
   shapes, relation domains/ranges, and procedure contracts.
3. A stricter source-accurate value model for compound values instead of the
   current compact numeric/handle runtime.
4. A larger 3D/geometry layer if the project becomes an engine-oriented
   library: camera model, clipping, meshes, raster output, and scene graphs.
5. A larger chess/modeling suite, including fuller board state and legality
   rules.
6. A full 16-bit compound runtime if the 8086/DOS backend should execute more
   than the arithmetic core directly.

The project should keep this matrix honest: a feature moves to `supported`
only when it has source examples, runtime behavior, and conformance coverage.
