# Standard Library

The PlankaC standard profile is the set of `.plk` plans loaded by
`plankac.exe check`, `plankac.exe run`, and the C API default loader.

Current profile:

```text
29 files
148 procedures
```

This page summarizes the callable procedures by domain. The complete numbered
catalog is in `program_catalog.md`.

## Arithmetic

Source: `src/01_arithmetic.plk`

| Procedure | Purpose |
| --- | --- |
| `add(V0, V1)` | addition |
| `subtract(V0, V1)` | subtraction |
| `multiply(V0, V1)` | multiplication |
| `negate(V0)` | sign inversion |
| `divide_checked(V0, V1)` | division with result/status pair |
| `modulo_checked(V0, V1)` | modulo with result/status pair |
| `average2(V0, V1)` | average of two values |

## Order And Sign

Source: `src/02_order.plk`

| Procedure | Purpose |
| --- | --- |
| `equal` | numeric equality |
| `less` | less-than predicate |
| `maximum` | maximum of two values |
| `minimum` | minimum of two values |
| `absolute` | absolute value |
| `sign` | sign classification |
| `equal_bool` | Boolean equality |

## Scientific Helpers

Source: `src/03_scientific.plk`

| Procedure | Purpose |
| --- | --- |
| `square` | square of a value |
| `power2` | power helper |
| `root_checked` | square root with guard |
| `reciprocal_checked` | reciprocal with guard |
| `percent_of` | percentage of a value |
| `percent_change_checked` | percent-change helper |

## Calculator Sessions

Sources: `src/04_calculator.plk`, `src/05_memory.plk`,
`tests/calculator_self_check.plk`

Procedures include calculator demos, chained expressions, guarded division,
memory clear/store/add/subtract/recall, and self-checks.

## Data Structures

Source: `src/06_data_structures.plk`

| Procedure | Purpose |
| --- | --- |
| `indexed_sum` | indexed slot example |
| `record_sum` | field-path record example |
| `loop_multiply` | loop equation example |
| `list_session` | list allocation/push/get example |
| `assertion_session` | runtime assertion example |
| `two_dimensional_add` | executable 2D row example |
| `type_model_session` | typed-value check session |

## Complex Values

Source: `src/09_complex.plk`

| Procedure | Purpose |
| --- | --- |
| `complex_norm_session` | squared norm |
| `complex_add_session` | complex addition |
| `complex_mul_session` | complex multiplication |
| `complex_conjugate_session` | conjugation |
| `complex_equal_session` | equality |

The runtime helpers behind these procedures are `complex`, `complex_real`,
`complex_imag`, `complex_add`, `complex_sub`, `complex_mul`,
`complex_conj`, `complex_norm2`, and `complex_equal`.

## Lists, Sets, Pairs, And Relations

Sources: `src/08_relations_sets.plk`, `src/10_relation_algebra.plk`,
`src/12_relation_composition.plk`, `src/16_value_algebra.plk`,
`src/20_page_table.plk`, `src/22_predicate_schema.plk`

Runtime helpers include:

```text
list_new, list_push, list_len, list_get, list_first, list_last
list_min, list_max, list_concat, list_equal
pair, pair_left, pair_right, pair_equal
set_new, set_add, set_contains, set_size, set_union
set_intersection, set_difference, set_subset, set_equal
set_cartesian, set_exists_greater, set_forall_less
relation_domain, relation_range, relation_has_pair
relation_compose, relation_inverse, relation_image
list_select_greater, list_count_equal, list_exists_equal
list_forall_greater, list_zip_pairs, list_pair
pair_left_list_len, pair_right_list_len
relation_select_domain, relation_select_range
relation_exists_range_equal, relation_forall_domain_greater
relation_signature
SELECT, COUNT, EXISTS, FORALL
```

## Records And Structured Values

Source: `src/11_structured_values.plk`

Runtime helpers:

```text
record_new
record_set
record_get
record_has
record_size
record_merge
record_equal
record_set_path2
record_get_path2
record_has_path2
record_shape_equal
```

Nested field-path references and handle-backed records are both used in the
current profile.

## Constants, Bits, Fixed Values, And Contracts

Source: `src/19_language_closure.plk`

Runtime helpers and language forms:

```text
CONST Cn = expression
STOPIF (predicate)
REQUIRE (predicate)
ENSURE (predicate)
bit, bit_not, bit_and, bit_or, bit_xor
bits_pack4, bits_get
fixed_quantize, fixed_add, fixed_mul, fixed_div_checked
arith_divide_checked
raise_exception, exception_raised, exception_code, exception_clear
```

The source profile includes executable sessions for the `C` bank,
two-dimensional array indices, bit/fixed helpers backed by raw scaled values,
result/status arithmetic, stop criteria, and contract predicates.

## Page Tables

Sources: `src/14_two_dimensional_tables.plk`,
`src/18_two_dimensional_general.plk`, `src/20_page_table.plk`

The notation layer accepts executable `|` rows with `V|` and `S|` rows, swapped
`V|`/`S|` order, spaced alignment, and `PAGE`/`ENDPAGE` blocks containing
several executable rows bound to nearby index/type rows.

## Chess Structures

Sources: `src/07_chess.plk`, `src/13_chess_board.plk`,
`src/17_chess_model.plk`, `src/20_page_table.plk`,
`src/21_chess_game.plk`, `src/23_chess_complete.plk`

The profile includes movement predicates, board-square helpers, piece records,
attack maps, king-zone examples, legal move checks, legal move application,
board moves, piece accessors, side-in-check checks, material scoring, capture
search, and checkmate sessions.
`src/21_chess_game.plk` adds legal move counts, promotion checks, castling-path
checks, and deterministic position signatures.
`src/23_chess_complete.plk` adds en passant checks, stalemate checks,
FEN-style position signatures, and move-history list sessions.

## 3D Geometry Extension

Source: `src/15_3d_geometry.plk`

This is a PlankaC extension, not part of the documented Plankalkuel core.

Runtime helpers:

```text
vec3, vec3_x, vec3_y, vec3_z
vec3_add, vec3_sub, vec3_dot, vec3_cross
vec3_scale, vec3_len2, vec3_normalize
mat4_identity, mat4_translate, mat4_scale
mat4_rotate_x, mat4_rotate_y, mat4_rotate_z
mat4_mul, mat4_transform_point
perspective_project
sin, cos
```

## Application Profiles

Sources: `graphics/src/plankagui.plk`, `graphics/src/plankacube.plk`

These files are loaded with `runfile` or `PlankaHost`. They expose common
application procedures:

```text
app_kind
app_canvas
app_checksum
app_timer_step
```

Kind-specific profiles expose `gui_*` or `cube_*` procedures for host
rendering.
