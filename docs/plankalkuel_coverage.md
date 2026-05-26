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
| `V`, `C`, `Z`, `R` variable classes | supported | inputs, constants, intermediate values, result values |
| Constant bank | supported | `CONST Cn = expression`, scalar/indexed/field refs, normal assignment to `C` rejected |
| Structure markers | supported | markers such as `[:1.1]`, `[:32.0]`, `[:32.16]`, `[:C32.16]`, `[:L32.0]`, `[:S32.0]`, `[:P32.0]`, `[:Q32.0]`, `[:VEC32.16]`, and `[:MAT32.16]` are parsed into type families |
| Fixed-point calculator values | supported profile | bit width and scale are parsed; `fixed_quantize`, `fixed_add`, `fixed_mul`, and `fixed_div_checked` use raw scaled integer helpers before returning host-visible numeric results |
| Boolean values | supported | `0` and `1`, comparisons, `&`, `|`, `!` |
| Bit-level Ja-Nein helpers | supported | `bit`, `bit_not`, `bit_and`, `bit_or`, `bit_xor`, `bits_pack4`, `bits_get` |
| Arithmetic expressions | supported | `+`, `-`, `*`, `/`, `%`, `^`, parentheses |
| Comparisons | supported | `=`, `!=`, `<`, `<=`, `>`, `>=` |
| Guarded commands / conditionals | supported | `condition => value => target` |
| Multiple result values | supported | procedures such as `divide_checked` return `R0`, `R1` |
| Typed IR | supported | source loading builds and validates a typed statement IR; `plankac ir output.ir` emits a readable IR view |
| Textual compiler output | supported | `plankac bytecode output.pbc`, `checkbc`, `runbc` |
| Bytecode runner | supported | textual bytecode can be loaded into a context and executed |
| C backend output | supported | `plankac cgen output.c` emits a compact C runner with embedded bytecode |
| Native ASM backend | supported profile | `plankac asmgen output.S` emits x86-64 assembly procedures, predicate syntax lowering, native calls, and a dispatcher without calling the interpreter |
| 8086/DOS ASM backend | supported profile | `plankac asm8086 output.asm` emits MASM/TASM-style 16-bit source with bytecode image, direct arithmetic procedures, compound value heap declarations, a compound procedure table, and a documented 16-bit compound ABI surface |
| Compiler pipeline | supported | `plankac compile prefix` writes `.pbc`, reloads IR, then emits C, x86-64 ASM, and 8086 ASM; `native-c` and `native-asm` link executables |
| Backend lowering report | supported | `plankac lowering output.txt` classifies typed IR statements for direct C lowering, native ASM, 8086 integer core, and compound runtime entry points |
| C embedding API | supported | `PLANKAC_CONTEXT`, custom source lists, bytecode loading, procedure metadata, run by name or number, IR/bytecode/C/ASM/8086 output |
| Conformance checks | supported | valid and invalid `.plk` fixtures under `tests/conformance` |
| Two-dimensional original notation | supported profile | executable `|` statement rows with aligned `V|`/`S|` tables are accepted, including swapped row order, spaced cell alignment, symbolic cells, and `PAGE`/`ENDPAGE` document validation through a row/cell document model with row/column diagnostics |
| Arrays / indexed data | supported | scalar refs, one-dimensional refs, and two-dimensional refs such as `Z10[1,2]` and `Z10[1][2]` execute in the frame model |
| Records / hierarchical structures | supported profile | nested field refs, handle-backed records, `record_set_path2`, `record_get_path2`, `record_has_path2`, and `record_shape_equal` execute in the shared frame heap |
| Lists and pairs | supported profile | shared frame-heap list handles plus pair helpers, list equality, predicate syntax (`SELECT`, `COUNT`, `EXISTS`, `FORALL`), zipped pair lists, and list-pair handles |
| Sets / relations | supported profile | `set_new`, `set_add`, `set_contains`, `set_union`, `set_intersection`, `set_difference`, `set_subset`, `set_equal`, `set_cartesian`, `relation_inverse`, `relation_image`, `relation_compose`, `relation_select_domain`, `relation_select_range`, relation domain/range/pair predicates, quantifier helpers, and deterministic relation signatures |
| Loops / repetition | supported profile | `LOOP count => value => target` repeats an assignment |
| Stop criteria | supported | `STOPIF (predicate)` exits the current procedure and preserves already written results |
| Assertions and contracts | supported | `ASSERT`, `REQUIRE`, and `ENSURE` evaluate Boolean predicates and fail execution on false values |
| Complex numbers | supported profile | `[:C32.16]` markers, handle-backed complex values, real/imag access, add/sub/mul/conjugate/norm/equality; covered by `src/09_complex.plk` and catalog entries `P140`-`P144` |
| 3D geometry extension | supported extension | not part of the documented Plankalkuel core profile; PlankaC adds `vec3`, dot/cross/normalize, `mat4`, rotation, transform, projection, pipeline examples in `src/15_3d_geometry.plk`, and a cube scene in `graphics/src/plankacube.plk` |
| PLK application host contracts | supported extension | `PlankaHost` loads the standard profile plus one `.plk` application file and dispatches `app_kind`, `app_canvas`, `app_checksum`, `app_timer_step`, GUI geometry, and cube geometry procedures through the same PlankaC context |
| Type/value model | supported profile | markers are parsed into families with width/scale pieces, checked across calls, checked by structural schema inference for lists, sets, pairs and record fields, and backed by active tagged PLC values for bits, fixed values, handles, and numeric values |
| Arithmetic exception handling | supported | direct division/modulo by zero fails execution; checked division helpers return result/status pairs such as `R1[:1.1]`; exception helper state is available through `raise_exception`, `exception_raised`, `exception_code`, and `exception_clear` |
| No recursion | supported profile | static direct/indirect recursion rejection plus runtime call-depth protection |
| Chess/list examples | supported profile | executable movement predicates, nested piece records, board-square helpers, board state records, attack maps, legal moves, legal move counts, promotion checks, castling-path checks, position and FEN-style signatures, en passant checks, move-history lists, stalemate checks, board moves, piece accessors, side-in-check, material scoring, capture search, and checkmate examples |

## Engineering Notes

The project treats a feature as supported when it has source examples, runtime
behavior, and conformance coverage. The current implementation is strongest on
linear notation, executable page/table expansion, typed procedure contracts,
compound value handles, chess-board modeling, typed IR, bytecode, lowering
classification, generated C, native x86-64 ASM, and DOS-oriented 8086 ASM
source emission.
