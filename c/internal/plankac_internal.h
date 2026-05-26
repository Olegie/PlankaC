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
#define PLC_MAX_INDEX 32
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
    PLC_STMT stmts[PLC_MAX_STMTS];
    int stmt_count;
} PLC_PROC;

typedef struct PLC_PROGRAM {
    PLC_PROC procs[PLC_MAX_PROCS];
    int proc_count;
    int source_count;
} PLC_PROGRAM;

struct PLANKAC_CONTEXT {
    PLC_PROGRAM program;
    int loaded;
};

typedef struct PLC_FRAME {
    struct PLC_FRAME *heap_owner;
    double v[PLC_MAX_VARS];
    double z[PLC_MAX_VARS];
    double r[PLC_MAX_VARS];
    double va[PLC_MAX_VARS][PLC_MAX_INDEX];
    double za[PLC_MAX_VARS][PLC_MAX_INDEX];
    double ra[PLC_MAX_VARS][PLC_MAX_INDEX];
    double vf[PLC_MAX_VARS][PLC_MAX_FIELDS];
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
    int has_field;
    char field[PLC_MAX_FIELD_NAME];
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

int plc_parse_header(const char *line, PLC_PROC *proc,
    char *err, unsigned err_size);
const PLC_PROC *plc_find_proc(const PLC_PROGRAM *program, const char *name);
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
int plc_analyze_program(const PLC_PROGRAM *program,
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

int plc_execute_proc(const PLC_PROGRAM *program, const PLC_PROC *proc,
    const double *args, int argc, PLC_VALUES *out, int depth,
    char *err, unsigned err_size);
int plc_call_proc(const PLC_PROGRAM *program, PLC_FRAME *caller_frame,
    const char *name, const double *args, int argc, PLC_VALUES *out, int depth,
    char *err, unsigned err_size);
void plc_format_value(double value, char *out);
void plc_copy_error(char *out, unsigned out_size, const char *text);
void plc_fill_proc_info(const PLC_PROC *proc, PLANKAC_PROC_INFO *info);

#endif
