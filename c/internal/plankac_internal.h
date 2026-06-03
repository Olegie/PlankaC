#ifndef PLANKAC_INTERNAL_H
#define PLANKAC_INTERNAL_H

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "plankac.h"

#define PLC_MAX_LINE 512
#define PLC_MAX_NAME 64
#define PLC_MAX_PROCS 192
#define PLC_MAX_STMTS 96
#define PLC_MAX_ARGS 16
#define PLC_MAX_RESULTS 8
#define PLC_MAX_VARS 64
#define PLC_MAX_DEPTH 64
#define PLC_MAX_INDEX 256
#define PLC_ARRAY_DIM 16
#define PLC_MAX_FIELDS 32
#define PLC_MAX_FIELD_NAME 64
#define PLC_MAX_TYPE_TEXT 24
#define PLC_MAX_LISTS 16
#define PLC_MAX_LIST_ITEMS 64
#define PLC_MAX_PAIRS 64
#define PLC_MAX_COMPLEX 64
#define PLC_MAX_RECORDS 32
#define PLC_MAX_RECORD_FIELDS_PER_RECORD 32
#define PLC_MAX_VEC3 64
#define PLC_MAX_MAT4 32
#define PLC_TYPE_FAMILY_NUMERIC 0
#define PLC_TYPE_FAMILY_BOOLEAN 1
#define PLC_TYPE_FAMILY_COMPLEX 2
#define PLC_TYPE_FAMILY_LIST 3
#define PLC_TYPE_FAMILY_SET 4
#define PLC_TYPE_FAMILY_PAIR 5
#define PLC_TYPE_FAMILY_RECORD 6
#define PLC_TYPE_FAMILY_VEC3 7
#define PLC_TYPE_FAMILY_MAT4 8
#define PLC_TYPE_FAMILY_UNKNOWN 9
#define PLC_MAX_VALUE_SHADOWS 512
#define PLC_2D_MAX_ROWS 96
#define PLC_2D_MAX_CELLS 64
#define PLC_2D_MAX_CELL_TEXT 64
#define PLC_VALUE_TAG_EMPTY 0
#define PLC_VALUE_TAG_NUMERIC 1
#define PLC_VALUE_TAG_BIT 2
#define PLC_VALUE_TAG_FIXED 3
#define PLC_VALUE_TAG_HANDLE 4
#define PLC_VALUE_TAG_EXCEPTION 5
#define PLC_IR_MAX_STMTS 4096
#define PLC_IR_OP_EVAL 1
#define PLC_IR_OP_CALL 2
#define PLC_IR_OP_GUARD_EVAL 3
#define PLC_IR_OP_GUARD_CALL 4
#define PLC_IR_OP_LOOP 5
#define PLC_IR_OP_ASSERT 6
#define PLC_IR_OP_REQUIRE 7
#define PLC_IR_OP_ENSURE 8
#define PLC_IR_OP_STOPIF 9
#define PLC_IR_OP_CONST 10
#define PLC_AST_OP_EVAL 1
#define PLC_AST_OP_CALL 2
#define PLC_AST_OP_GUARD_EVAL 3
#define PLC_AST_OP_GUARD_CALL 4
#define PLC_AST_OP_LOOP 5
#define PLC_AST_OP_ASSERT 6
#define PLC_AST_OP_REQUIRE 7
#define PLC_AST_OP_ENSURE 8
#define PLC_AST_OP_STOPIF 9
#define PLC_AST_OP_CONST 10
#define PLC_EXPR_AST_EMPTY 0
#define PLC_EXPR_AST_LITERAL 1
#define PLC_EXPR_AST_REF 2
#define PLC_EXPR_AST_CALL 3
#define PLC_EXPR_AST_UNARY 4
#define PLC_EXPR_AST_BINARY 5
#define PLC_EXPR_AST_PREDICATE 6
#define PLC_EXPR_AST_GROUP 7
#define PLC_EXPR_AST_TARGET_LIST 8
#define PLC_EXPR_AST_UNKNOWN 9
#define PLC_EXPR_AST_MAX_NODES 64
#define PLC_EXPR_AST_MAX_CHILDREN 8
#define PLC_EXPR_AST_NODE_TEXT 64
#define PLC_CMP_EQ 1
#define PLC_CMP_NE 2
#define PLC_CMP_LT 3
#define PLC_CMP_LE 4
#define PLC_CMP_GT 5
#define PLC_CMP_GE 6

typedef struct PLC_TYPE_SPEC {
    int family;
    int class_code;
    int bits;
    int scale;
    char prefix[12];
    char text[PLC_MAX_TYPE_TEXT];
} PLC_TYPE_SPEC;

typedef struct PLC_STMT {
    char text[PLC_MAX_LINE];
    int line_no;
} PLC_STMT;

typedef struct PLC_PROC {
    int number;
    char name[PLC_MAX_NAME];
    int argc;
    int results;
    char arg_types[PLC_MAX_ARGS][PLC_MAX_TYPE_TEXT];
    char result_types[PLC_MAX_RESULTS][PLC_MAX_TYPE_TEXT];
    PLC_STMT *stmts;
    int stmt_count;
    int stmt_capacity;
} PLC_PROC;

typedef struct PLC_NATIVE_PROC {
    char name[PLC_MAX_NAME];
    int argc;
    int results;
    char arg_types[PLC_MAX_ARGS][PLC_MAX_TYPE_TEXT];
    char result_types[PLC_MAX_RESULTS][PLC_MAX_TYPE_TEXT];
    PLANKAC_NATIVE_FN fn;
    void *user_data;
} PLC_NATIVE_PROC;

typedef struct PLC_VALUE {
    int tag;
    int family;
    int bits;
    int scale;
    long long raw;
    int handle;
    double number;
} PLC_VALUE;

typedef PLC_VALUE PLC_TAGGED_VALUE;

typedef struct PLC_VALUE_SHADOW {
    char bank;
    int index;
    int has_subscript;
    int subscript;
    int has_field;
    char field[PLC_MAX_FIELD_NAME];
    PLC_TAGGED_VALUE value;
} PLC_VALUE_SHADOW;

typedef struct PLC_EXPR_AST_SUMMARY {
    int root_kind;
    int node_count;
    int max_depth;
    int call_count;
    int ref_count;
    int literal_count;
    int predicate_count;
    int operator_count;
    int unknown_count;
} PLC_EXPR_AST_SUMMARY;

typedef struct PLC_EXPR_AST_NODE {
    int kind;
    char op[8];
    char text[PLC_EXPR_AST_NODE_TEXT];
    int child_count;
    int children[PLC_EXPR_AST_MAX_CHILDREN];
    int family;
} PLC_EXPR_AST_NODE;

typedef struct PLC_EXPR_AST_TREE {
    PLC_EXPR_AST_NODE nodes[PLC_EXPR_AST_MAX_NODES];
    int node_count;
    int root;
    int overflow;
} PLC_EXPR_AST_TREE;

typedef struct PLC_AST_STMT {
    int proc_number;
    char proc_name[PLC_MAX_NAME];
    int source_line;
    int op;
    int arrow_count;
    int argc;
    int results;
    char callee[PLC_MAX_NAME];
    char source[PLC_MAX_LINE];
    char guard[PLC_MAX_LINE];
    char value[PLC_MAX_LINE];
    char target[PLC_MAX_LINE];
    PLC_EXPR_AST_SUMMARY guard_expr;
    PLC_EXPR_AST_SUMMARY value_expr;
    PLC_EXPR_AST_SUMMARY target_expr;
    char guard_shape[PLC_MAX_LINE];
    char value_shape[PLC_MAX_LINE];
    char target_shape[PLC_MAX_LINE];
} PLC_AST_STMT;

typedef struct PLC_AST_PROGRAM {
    PLC_AST_STMT stmts[PLC_IR_MAX_STMTS];
    int stmt_count;
    int proc_count;
    int source_count;
} PLC_AST_PROGRAM;

typedef struct PLC_IR_STMT {
    int proc_number;
    char proc_name[PLC_MAX_NAME];
    int source_line;
    int op;
    int guard_family;
    int value_family;
    int target_family;
    int argc;
    int results;
    char callee[PLC_MAX_NAME];
    char guard[PLC_MAX_LINE];
    char value[PLC_MAX_LINE];
    char target[PLC_MAX_LINE];
    char lowering[PLC_MAX_NAME];
    int expr_nodes;
    int expr_depth;
    int expr_calls;
    int expr_refs;
    int expr_literals;
    int expr_predicates;
    int expr_unknowns;
    char expr_shape[PLC_MAX_LINE];
} PLC_IR_STMT;

typedef struct PLC_IR_PROGRAM {
    PLC_IR_STMT stmts[PLC_IR_MAX_STMTS];
    int stmt_count;
    int proc_count;
    int source_count;
} PLC_IR_PROGRAM;

typedef struct PLC_2D_CELL_MODEL {
    int row;
    int col_start;
    int col_end;
    char text[PLC_2D_MAX_CELL_TEXT];
} PLC_2D_CELL_MODEL;

typedef struct PLC_2D_ROW_MODEL {
    int kind;
    int row;
    int col;
    int block;
    int cell_count;
    PLC_2D_CELL_MODEL cells[PLC_2D_MAX_CELLS];
} PLC_2D_ROW_MODEL;

typedef struct PLC_2D_DOCUMENT {
    int row_count;
    int expression_count;
    int block_count;
    PLC_2D_ROW_MODEL rows[PLC_2D_MAX_ROWS];
} PLC_2D_DOCUMENT;

typedef struct PLC_PROGRAM {
    PLC_PROC procs[PLC_MAX_PROCS];
    int proc_count;
    int source_count;
    PLC_NATIVE_PROC natives[PLANKAC_MAX_NATIVE];
    int native_count;
} PLC_PROGRAM;

struct PLANKAC_CONTEXT {
    PLC_PROGRAM program;
    int loaded;
};

typedef struct PLC_FRAME {
    struct PLC_FRAME *heap_owner;
    double v[PLC_MAX_VARS];
    double c[PLC_MAX_VARS];
    double z[PLC_MAX_VARS];
    double r[PLC_MAX_VARS];
    PLC_TAGGED_VALUE vb[PLC_MAX_VARS];
    PLC_TAGGED_VALUE cb[PLC_MAX_VARS];
    PLC_TAGGED_VALUE zb[PLC_MAX_VARS];
    PLC_TAGGED_VALUE rb[PLC_MAX_VARS];
    double va[PLC_MAX_VARS][PLC_MAX_INDEX];
    double ca[PLC_MAX_VARS][PLC_MAX_INDEX];
    double za[PLC_MAX_VARS][PLC_MAX_INDEX];
    double ra[PLC_MAX_VARS][PLC_MAX_INDEX];
    double vf[PLC_MAX_VARS][PLC_MAX_FIELDS];
    double cf[PLC_MAX_VARS][PLC_MAX_FIELDS];
    double zf[PLC_MAX_VARS][PLC_MAX_FIELDS];
    double rf[PLC_MAX_VARS][PLC_MAX_FIELDS];
    char field_names[PLC_MAX_FIELDS][PLC_MAX_FIELD_NAME];
    int field_count;
    double lists[PLC_MAX_LISTS][PLC_MAX_LIST_ITEMS];
    int list_sizes[PLC_MAX_LISTS];
    int list_count;
    double pair_left[PLC_MAX_PAIRS];
    double pair_right[PLC_MAX_PAIRS];
    int pair_count;
    double complex_real[PLC_MAX_COMPLEX];
    double complex_imag[PLC_MAX_COMPLEX];
    int complex_count;
    int record_keys[PLC_MAX_RECORDS][PLC_MAX_RECORD_FIELDS_PER_RECORD];
    double record_values[PLC_MAX_RECORDS][PLC_MAX_RECORD_FIELDS_PER_RECORD];
    int record_sizes[PLC_MAX_RECORDS];
    int record_count;
    double vec3_values[PLC_MAX_VEC3][3];
    int vec3_count;
    double mat4_values[PLC_MAX_MAT4][16];
    int mat4_count;
    PLC_VALUE_SHADOW value_shadows[PLC_MAX_VALUE_SHADOWS];
    int value_shadow_count;
    int exception_raised;
    int exception_code;
    int stopped;
} PLC_FRAME;

typedef struct PLC_VALUES {
    double value[PLC_MAX_RESULTS];
    int count;
} PLC_VALUES;

typedef struct PLC_REF {
    char bank;
    int index;
    int has_subscript;
    int subscript;
    int subscript_count;
    int subscripts[2];
    int has_field;
    char field[PLC_MAX_FIELD_NAME];
    char type_marker[PLC_MAX_TYPE_TEXT];
} PLC_REF;

typedef struct PLC_PARSER {
    const char *p;
    const PLC_PROGRAM *program;
    PLC_FRAME *frame;
    int depth;
    char *err;
    unsigned err_size;
} PLC_PARSER;

extern const char *PLC_SOURCES[];
extern PLC_PROGRAM g_plankac_program;
extern int g_plankac_loaded;

void plc_set_error(char *err, unsigned err_size, const char *message);
void plc_prefix_error(char *err, unsigned err_size, const char *prefix);
void plc_copy_range(char *out, unsigned out_size,
    const char *first, const char *last);
char *plc_ltrim(char *text);
void plc_rtrim(char *text);
void plc_trim_in_place(char *text);
void plc_strip_comment(char *text);
const char *plc_skip_space(const char *p);
const char *plc_skip_type_marker(const char *p);
int plc_is_program_line(const char *line);
int plc_is_end_line(const char *line);
const char *plc_matching_paren(const char *open);
int plc_count_refs(const char *first, const char *last, char bank);
void plc_proc_free(PLC_PROC *proc);
void plc_program_free(PLC_PROGRAM *program);
int plc_proc_add_stmt(PLC_PROC *proc, const char *text, int line_no,
    char *err, unsigned err_size);

int plc_parse_header(const char *line, PLC_PROC *proc,
    char *err, unsigned err_size);
const PLC_PROC *plc_find_proc(const PLC_PROGRAM *program, const char *name);
const PLC_NATIVE_PROC *plc_find_native(const PLC_PROGRAM *program,
    const char *name);
int plc_register_native(PLC_PROGRAM *program, const char *name,
    int argc, int results, const char *const *arg_types,
    const char *const *result_types, PLANKAC_NATIVE_FN fn, void *user_data,
    char *err, unsigned err_size);
int plc_load_sources(PLC_PROGRAM *program, const char *const *sources,
    char *err, unsigned err_size);
int plc_load_program(PLC_PROGRAM *program, char *err, unsigned err_size);
int plc_load_bytecode(PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size);
int plc_load_bytecode_text(PLC_PROGRAM *program, const char *text,
    char *err, unsigned err_size);

int plc_eval_expr_text(const PLC_PROGRAM *program, PLC_FRAME *frame,
    int depth, const char *text, double *value, char *err, unsigned err_size);
int plc_is_top_call(const char *text, char *name, unsigned name_size,
    char *args, unsigned args_size);
int plc_eval_args_text(const PLC_PROGRAM *program, PLC_FRAME *frame,
    int depth, const char *text, double *args, int *argc,
    char *err, unsigned err_size);
int plc_eval_value_text(const PLC_PROGRAM *program, PLC_FRAME *frame,
    int depth, const char *text, PLC_VALUES *out,
    char *err, unsigned err_size);
int plc_assign_const_text(PLC_FRAME *frame, const char *text,
    double value, char *err, unsigned err_size);
int plc_assign_targets(PLC_FRAME *frame, const char *text,
    const PLC_VALUES *values, char *err, unsigned err_size);
int plc_split_arrows(const char *text, char parts[][PLC_MAX_LINE],
    int *count);
int plc_line_starts_with(const char *text, const char *keyword);
int plc_parse_type_marker_text(const char *text, PLC_TYPE_SPEC *spec,
    char *err, unsigned err_size);
int plc_validate_type_markers_in_line(const char *line,
    char *err, unsigned err_size);
int plc_type_markers_compatible(const char *left, const char *right);
const char *plc_type_family_name(int family);
int plc_expand_2d_statement(const char *expr_row, const char *index_row,
    const char *type_row, char *out, unsigned out_size,
    char *err, unsigned err_size);
int plc_expand_2d_page(char rows[][PLC_MAX_LINE], int row_count,
    char statements[][PLC_MAX_LINE], int *statement_count,
    int max_statements, char *err, unsigned err_size);
int plc_validate_2d_document(char rows[][PLC_MAX_LINE], int row_count,
    char *err, unsigned err_size);
int plc_build_2d_document(char rows[][PLC_MAX_LINE], int row_count,
    PLC_2D_DOCUMENT *document, char *err, unsigned err_size);
int plc_analyze_program(const PLC_PROGRAM *program,
    char *err, unsigned err_size);
int plc_analyze_structural_schemas(const PLC_PROGRAM *program,
    char *err, unsigned err_size);

void plc_tagged_from_double(PLC_TAGGED_VALUE *out, double value,
    const char *type_marker);
double plc_tagged_to_double(const PLC_TAGGED_VALUE *value);
const char *plc_value_tag_name(int tag);
int plc_frame_store_tagged(PLC_FRAME *frame, const PLC_REF *ref,
    double value, const char *type_marker, char *err, unsigned err_size);
int plc_frame_load_tagged(const PLC_FRAME *frame, const PLC_REF *ref,
    PLC_TAGGED_VALUE *out);

int plc_ast_build_program(const PLC_PROGRAM *program, PLC_AST_PROGRAM *ast,
    char *err, unsigned err_size);
int plc_ast_validate_program(const PLC_AST_PROGRAM *ast,
    char *err, unsigned err_size);
const char *plc_ast_op_name(int op);
const char *plc_expr_ast_kind_name(int kind);
int plc_expr_ast_build_text(const char *text, PLC_EXPR_AST_TREE *tree,
    PLC_EXPR_AST_SUMMARY *summary);
int plc_expr_ast_build_target(const char *text, PLC_EXPR_AST_TREE *tree,
    PLC_EXPR_AST_SUMMARY *summary);
void plc_expr_ast_serialize(const PLC_EXPR_AST_TREE *tree,
    char *out, unsigned out_size);
int plc_emit_ast(const PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size);

int plc_ir_build_program(const PLC_PROGRAM *program, PLC_IR_PROGRAM *ir,
    char *err, unsigned err_size);
int plc_ir_validate_program(const PLC_IR_PROGRAM *ir,
    char *err, unsigned err_size);
int plc_emit_ir(const PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size);
int plc_emit_evidence(const PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size);
int plc_emit_lowering_report(const PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size);

unsigned long plc_bit_pack4(int a, int b, int c, int d);
int plc_bit_get_word(unsigned long word, int index);
long long plc_fixed_raw_from_double(double value, int scale);
double plc_fixed_double_from_raw(long long raw, int scale);
long long plc_fixed_raw_add(long long left, long long right);
long long plc_fixed_raw_mul(long long left, long long right, int scale);
int plc_fixed_raw_div_checked(long long left, long long right, int scale,
    long long *out);
double plc_fixed_quantize_bits(double value, int scale);
double plc_fixed_add_bits(double left, double right, int scale);
double plc_fixed_mul_bits(double left, double right, int scale);
int plc_fixed_div_bits(double left, double right, int scale, double *out);

int plc_chess_model_legal_move(PLC_FRAME *frame, int board_id,
    int from_square, int to_square, int enforce_king_safety,
    int *legal, char *err, unsigned err_size);
int plc_chess_model_apply_move(PLC_FRAME *frame, int board_id,
    int from_square, int to_square, int enforce_king_safety,
    char *err, unsigned err_size);
int plc_chess_model_side_in_check(PLC_FRAME *frame, int board_id, int side,
    int *in_check, char *err, unsigned err_size);
int plc_chess_model_checkmate(PLC_FRAME *frame, int board_id, int side,
    int *is_mate, char *err, unsigned err_size);
int plc_chess_model_material_score(PLC_FRAME *frame, int board_id, int side,
    int *score, char *err, unsigned err_size);
int plc_chess_model_best_capture_score(PLC_FRAME *frame, int board_id,
    int side, int *score, char *err, unsigned err_size);
int plc_chess_model_legal_move_count(PLC_FRAME *frame, int board_id,
    int side, int *count, char *err, unsigned err_size);
int plc_chess_model_pawn_promotion(int from_square, int to_square,
    int side);
int plc_chess_model_can_castle_path(PLC_FRAME *frame, int board_id,
    int side, int king_from, int rook_from, int *can_castle,
    char *err, unsigned err_size);
int plc_chess_model_position_signature(PLC_FRAME *frame, int board_id,
    int *signature, char *err, unsigned err_size);
int plc_chess_model_stalemate(PLC_FRAME *frame, int board_id, int side,
    int *is_stalemate, char *err, unsigned err_size);
int plc_chess_model_en_passant(int from_square, int to_square,
    int last_from, int last_to, int side);
int plc_chess_model_fen_signature(PLC_FRAME *frame, int board_id,
    int side, int castling, int ep_square, int *signature,
    char *err, unsigned err_size);

int plc_emit_bytecode(const PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size);
int plc_emit_c_backend(const PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size);
int plc_emit_asm_image(const PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size);
int plc_emit_asm_runtime(const PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size);
int plc_emit_asm8086_runtime(const PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size);
int plc_emit_doscom(const PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size);

int plc_execute_proc(const PLC_PROGRAM *program, const PLC_PROC *proc,
    const double *args, int argc, PLC_VALUES *out, int depth,
    char *err, unsigned err_size);
int plc_call_proc(const PLC_PROGRAM *program, PLC_FRAME *caller_frame,
    const char *name, const double *args, int argc, PLC_VALUES *out, int depth,
    char *err, unsigned err_size);
void plc_format_value(double value, char *out);
void plc_copy_error(char *out, unsigned out_size, const char *text);
void plc_fill_proc_info(const PLC_PROC *proc, PLANKAC_PROC_INFO *info);
void plc_fill_native_info(const PLC_NATIVE_PROC *proc,
    PLANKAC_NATIVE_INFO *info);

#endif
