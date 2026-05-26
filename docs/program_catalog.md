# Program Catalog / Programmkatalog

## Arithmetic

```text
P10 add
P11 subtract
P12 multiply
P13 negate
P14 divide_checked
P15 modulo_checked
P16 average2
```

## Order and Sign

```text
P30 equal
P31 less
P32 maximum
P33 minimum
P34 absolute
P35 sign
P36 equal_bool
```

## Scientific Calculator Layer

```text
P50 square
P51 power2
P52 root_checked
P53 reciprocal_checked
P54 percent_of
P55 percent_change_checked
```

## High-Level Calculator Plans

```text
P70 calculator_demo
P71 chained_expression
P72 guarded_division_demo
P73 compare_and_average
```

## Memory Plans

```text
P80 memory_clear
P81 memory_store
P82 memory_add
P83 memory_subtract
P84 memory_recall
```

## Data Structures And Control Flow

```text
P110 indexed_sum
P111 record_sum
P112 loop_multiply
P113 list_session
P114 assertion_session
P115 two_dimensional_add
P116 type_model_session
```

## Chess-Style Structure Examples

```text
P120 chess_rook_move
P121 chess_bishop_move
P122 chess_queen_move
P123 chess_knight_move
```

## Relations, Sets, And Richer Structures

```text
P130 nested_record_session
P131 set_session
P132 set_logic_session
P133 pair_relation_session
P134 two_dimensional_fuller
P135 chess_piece_record
P136 chess_attack_relation
P137 logic_not_session
```

## Complex Values

```text
P140 complex_norm_session
P141 complex_add_session
P142 complex_mul_session
P143 complex_conjugate_session
P144 complex_equal_session
```

## Relation Algebra

```text
P150 set_difference_session
P151 set_subset_session
P152 relation_domain_session
P153 relation_range_session
P154 relation_has_pair_session
P155 relation_make_pair_list
P156 relation_child_heap_session
```

## Structured Values

```text
P160 record_handle_session
P161 record_make_piece
P162 record_child_heap_session
P163 record_shape_session
```

## Relation Composition

```text
P170 cartesian_product_session
P171 relation_compose_session
P172 set_quantifier_session
```

## Chess Board Structures

```text
P180 chess_square
P181 chess_piece_struct_session
P182 chess_rook_attack_map_session
P183 chess_check_session
```

## Two-Dimensional Table Notation

```text
P190 two_dimensional_original_session
P191 two_dimensional_symbolic_type_session
```

## 3D Geometry Extension

```text
P200 vec3_make_session
P201 vec3_add_dot_session
P202 vec3_cross_session
P203 vec3_normalize_session
P204 mat4_translate_session
P205 mat4_scale_session
P206 mat4_chain_session
P207 perspective_project_session
P208 triangle_normal_session
P209 three_d_pipeline_session
P210 make_engine_point
P211 vec3_child_heap_session
P212 trig_session
P213 mat4_rotate_session
```

## PlankaGUI Graphics Profiles

The `app_*` names are profile-level entry points. `plankagui.plk` and
`plankacube.plk` are loaded as separate application files, so both profiles
can use the same public application names without colliding in normal host
use.

```text
P3060 app_kind
P3061 app_canvas
P3062 app_checksum
P3063 app_timer_step
P3000 gui_canvas
P3001 gui_window_rect
P3010 gui_button_rect
P3020 gui_function_row_rect
P3040 gui_style_rgb
P3050 gui_scene_checksum
P3260 app_kind
P3261 app_canvas
P3262 app_checksum
P3263 app_timer_step
P3200 cube_canvas
P3221 cube_vertex
P3222 cube_edge
P3230 cube_model_transform
P3231 cube_project_vertex
P3240 cube_scene_checksum
```

## Value Algebra

```text
P230 list_equal_session
P231 set_equal_session
P232 pair_equal_session
P233 record_merge_session
P234 record_equal_session
P235 relation_inverse_session
P236 relation_image_session
```

## Fuller Chess Model

```text
P240 chess_board_setup_session
P241 chess_rook_attack_map_full_session
P242 chess_bishop_attack_map_full_session
P243 chess_knight_attack_map_full_session
P244 chess_queen_attack_map_full_session
P245 chess_piece_attacks_square_session
P246 chess_check_model_session
P247 chess_king_zone_session
```

## General Two-Dimensional Rows

```text
P260 two_dimensional_swapped_rows_session
P261 two_dimensional_aligned_cells_session
```

## Self-Checks

```text
P900 test_add
P901 test_subtract
P902 test_multiply
P903 test_divide_checked
P904 test_maximum
P905 test_root_checked
P906 test_division_by_zero_guard
P907 test_memory_add
P999 all_tests
```
