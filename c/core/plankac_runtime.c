#include "plankac_internal.h"

const char *PLC_SOURCES[] = {
    "src/00_types.plk",
    "src/01_arithmetic.plk",
    "src/02_order.plk",
    "src/03_scientific.plk",
    "src/04_calculator.plk",
    "src/05_memory.plk",
    "src/06_data_structures.plk",
    "src/07_chess.plk",
    "src/08_relations_sets.plk",
    "src/09_complex.plk",
    "src/10_relation_algebra.plk",
    "src/11_structured_values.plk",
    "src/12_relation_composition.plk",
    "src/13_chess_board.plk",
    "src/14_two_dimensional_tables.plk",
    "src/15_3d_geometry.plk",
    "src/16_value_algebra.plk",
    "src/17_chess_model.plk",
    "src/18_two_dimensional_general.plk",
    "src/19_language_closure.plk",
    "src/20_page_table.plk",
    "src/21_chess_game.plk",
    "src/22_predicate_schema.plk",
    "src/23_chess_complete.plk",
    "examples/session_basic.plk",
    "examples/session_guarded.plk",
    "examples/session_memory.plk",
    "examples/session_scientific.plk",
    "tests/calculator_self_check.plk",
    0
};

PLC_PROGRAM g_plankac_program;
int g_plankac_loaded = 0;

const PLC_NATIVE_PROC *plc_find_native(const PLC_PROGRAM *program,
    const char *name)
{
    int i;

    if (program == 0 || name == 0) {
        return 0;
    }
    for (i = 0; i < program->native_count; ++i) {
        if (strcmp(program->natives[i].name, name) == 0) {
            return &program->natives[i];
        }
    }
    return 0;
}

int plc_register_native(PLC_PROGRAM *program, const char *name,
    int argc, int results, const char *const *arg_types,
    const char *const *result_types, PLANKAC_NATIVE_FN fn, void *user_data,
    char *err, unsigned err_size)
{
    PLC_NATIVE_PROC *native_proc;
    PLC_TYPE_SPEC type_spec;
    int i;

    if (program == 0) {
        plc_set_error(err, err_size, "missing program");
        return 0;
    }
    if (name == 0 || name[0] == '\0') {
        plc_set_error(err, err_size, "missing native function name");
        return 0;
    }
    if (strlen(name) >= PLC_MAX_NAME) {
        plc_set_error(err, err_size, "native function name is too long");
        return 0;
    }
    if (fn == 0) {
        plc_set_error(err, err_size, "missing native function pointer");
        return 0;
    }
    if (argc < 0 || argc > PLANKAC_MAX_ARGS
            || results < 0 || results > PLANKAC_MAX_RESULTS) {
        plc_set_error(err, err_size, "bad native function arity");
        return 0;
    }
    if (plc_find_proc(program, name) != 0 || plc_find_native(program, name) != 0) {
        plc_set_error(err, err_size, "duplicate native function");
        return 0;
    }
    if (program->native_count >= PLANKAC_MAX_NATIVE) {
        plc_set_error(err, err_size, "too many native functions");
        return 0;
    }
    native_proc = &program->natives[program->native_count];
    memset(native_proc, 0, sizeof(*native_proc));
    strncpy(native_proc->name, name, PLC_MAX_NAME - 1);
    native_proc->name[PLC_MAX_NAME - 1] = '\0';
    native_proc->argc = argc;
    native_proc->results = results;
    native_proc->fn = fn;
    native_proc->user_data = user_data;
    for (i = 0; i < argc; ++i) {
        if (arg_types != 0 && arg_types[i] != 0 && arg_types[i][0] != '\0') {
            if (!plc_parse_type_marker_text(arg_types[i], &type_spec,
                    err, err_size)) {
                plc_prefix_error(err, err_size, "native argument ");
                memset(native_proc, 0, sizeof(*native_proc));
                return 0;
            }
            strncpy(native_proc->arg_types[i], arg_types[i],
                PLC_MAX_TYPE_TEXT - 1);
            native_proc->arg_types[i][PLC_MAX_TYPE_TEXT - 1] = '\0';
        }
    }
    for (i = 0; i < results; ++i) {
        if (result_types != 0 && result_types[i] != 0
                && result_types[i][0] != '\0') {
            if (!plc_parse_type_marker_text(result_types[i], &type_spec,
                    err, err_size)) {
                plc_prefix_error(err, err_size, "native result ");
                memset(native_proc, 0, sizeof(*native_proc));
                return 0;
            }
            strncpy(native_proc->result_types[i], result_types[i],
                PLC_MAX_TYPE_TEXT - 1);
            native_proc->result_types[i][PLC_MAX_TYPE_TEXT - 1] = '\0';
        }
    }
    ++program->native_count;
    plc_copy_error(err, err_size, "");
    return 1;
}

static PLC_FRAME *plc_heap_frame(PLC_FRAME *frame)
{
    if (frame != 0 && frame->heap_owner != 0) {
        return frame->heap_owner;
    }
    return frame;
}

static int plc_new_list(PLC_FRAME *frame, char *err, unsigned err_size)
{
    int list_id;

    frame = plc_heap_frame(frame);
    if (frame == 0) {
        plc_set_error(err, err_size, "list allocation needs a frame");
        return -1;
    }
    if (frame->list_count >= PLC_MAX_LISTS) {
        plc_set_error(err, err_size, "too many lists");
        return -1;
    }
    list_id = frame->list_count;
    frame->list_sizes[list_id] = 0;
    ++frame->list_count;
    return list_id;
}

static int plc_list_valid(PLC_FRAME *frame, int list_id,
    char *err, unsigned err_size)
{
    frame = plc_heap_frame(frame);
    if (frame == 0) {
        plc_set_error(err, err_size, "list access needs a frame");
        return 0;
    }
    if (list_id < 0 || list_id >= frame->list_count) {
        plc_set_error(err, err_size, "bad list id");
        return 0;
    }
    return 1;
}

static int plc_list_append(PLC_FRAME *frame, int list_id, double value,
    char *err, unsigned err_size)
{
    int index;

    if (!plc_list_valid(frame, list_id, err, err_size)) {
        return 0;
    }
    if (frame->list_sizes[list_id] >= PLC_MAX_LIST_ITEMS) {
        plc_set_error(err, err_size, "list is full");
        return 0;
    }
    index = frame->list_sizes[list_id];
    frame->lists[list_id][index] = value;
    ++frame->list_sizes[list_id];
    return 1;
}

static int plc_list_contains_value(PLC_FRAME *frame, int list_id, double value)
{
    int i;

    frame = plc_heap_frame(frame);
    if (frame == 0) {
        return 0;
    }
    for (i = 0; i < frame->list_sizes[list_id]; ++i) {
        if (fabs(frame->lists[list_id][i] - value) < 0.0000001) {
            return 1;
        }
    }
    return 0;
}

static int plc_compare_where(double left, int op_code, double right)
{
    switch (op_code) {
    case PLC_CMP_EQ:
        return fabs(left - right) < 0.0000001;
    case PLC_CMP_NE:
        return fabs(left - right) >= 0.0000001;
    case PLC_CMP_LT:
        return left < right;
    case PLC_CMP_LE:
        return left <= right || fabs(left - right) < 0.0000001;
    case PLC_CMP_GT:
        return left > right;
    case PLC_CMP_GE:
        return left >= right || fabs(left - right) < 0.0000001;
    default:
        return 0;
    }
}

static int plc_new_complex(PLC_FRAME *frame, double real, double imag,
    char *err, unsigned err_size)
{
    int id;

    frame = plc_heap_frame(frame);
    if (frame == 0) {
        plc_set_error(err, err_size, "complex allocation needs a frame");
        return -1;
    }
    if (frame->complex_count >= PLC_MAX_COMPLEX) {
        plc_set_error(err, err_size, "too many complex values");
        return -1;
    }
    id = frame->complex_count;
    frame->complex_real[id] = real;
    frame->complex_imag[id] = imag;
    ++frame->complex_count;
    return id;
}

static int plc_complex_valid(PLC_FRAME *frame, int id,
    char *err, unsigned err_size)
{
    frame = plc_heap_frame(frame);
    if (frame == 0) {
        plc_set_error(err, err_size, "complex access needs a frame");
        return 0;
    }
    if (id < 0 || id >= frame->complex_count) {
        plc_set_error(err, err_size, "bad complex id");
        return 0;
    }
    return 1;
}

static int plc_new_record(PLC_FRAME *frame, char *err, unsigned err_size)
{
    int id;

    frame = plc_heap_frame(frame);
    if (frame == 0) {
        plc_set_error(err, err_size, "record allocation needs a frame");
        return -1;
    }
    if (frame->record_count >= PLC_MAX_RECORDS) {
        plc_set_error(err, err_size, "too many records");
        return -1;
    }
    id = frame->record_count;
    frame->record_sizes[id] = 0;
    ++frame->record_count;
    return id;
}

static int plc_record_valid(PLC_FRAME *frame, int id,
    char *err, unsigned err_size)
{
    frame = plc_heap_frame(frame);
    if (frame == 0) {
        plc_set_error(err, err_size, "record access needs a frame");
        return 0;
    }
    if (id < 0 || id >= frame->record_count) {
        plc_set_error(err, err_size, "bad record id");
        return 0;
    }
    return 1;
}

static int plc_record_find_key(PLC_FRAME *frame, int id, int key)
{
    int i;

    frame = plc_heap_frame(frame);
    if (frame == 0) {
        return -1;
    }
    for (i = 0; i < frame->record_sizes[id]; ++i) {
        if (frame->record_keys[id][i] == key) {
            return i;
        }
    }
    return -1;
}

static int plc_record_set_value(PLC_FRAME *frame, int id, int key,
    double value, char *err, unsigned err_size)
{
    int slot;

    frame = plc_heap_frame(frame);
    if (!plc_record_valid(frame, id, err, err_size)) {
        return 0;
    }
    slot = plc_record_find_key(frame, id, key);
    if (slot < 0) {
        if (frame->record_sizes[id] >= PLC_MAX_RECORD_FIELDS_PER_RECORD) {
            plc_set_error(err, err_size, "record is full");
            return 0;
        }
        slot = frame->record_sizes[id];
        frame->record_keys[id][slot] = key;
        ++frame->record_sizes[id];
    }
    frame->record_values[id][slot] = value;
    return 1;
}

static int plc_new_vec3(PLC_FRAME *frame, double x, double y, double z,
    char *err, unsigned err_size)
{
    int id;

    frame = plc_heap_frame(frame);
    if (frame == 0) {
        plc_set_error(err, err_size, "vec3 allocation needs a frame");
        return -1;
    }
    if (frame->vec3_count >= PLC_MAX_VEC3) {
        plc_set_error(err, err_size, "too many vec3 values");
        return -1;
    }
    id = frame->vec3_count;
    frame->vec3_values[id][0] = x;
    frame->vec3_values[id][1] = y;
    frame->vec3_values[id][2] = z;
    ++frame->vec3_count;
    return id;
}

static int plc_vec3_valid(PLC_FRAME *frame, int id,
    char *err, unsigned err_size)
{
    frame = plc_heap_frame(frame);
    if (frame == 0) {
        plc_set_error(err, err_size, "vec3 access needs a frame");
        return 0;
    }
    if (id < 0 || id >= frame->vec3_count) {
        plc_set_error(err, err_size, "bad vec3 id");
        return 0;
    }
    return 1;
}

static int plc_new_mat4(PLC_FRAME *frame, const double *values,
    char *err, unsigned err_size)
{
    int id;
    int i;

    frame = plc_heap_frame(frame);
    if (frame == 0) {
        plc_set_error(err, err_size, "mat4 allocation needs a frame");
        return -1;
    }
    if (frame->mat4_count >= PLC_MAX_MAT4) {
        plc_set_error(err, err_size, "too many mat4 values");
        return -1;
    }
    id = frame->mat4_count;
    for (i = 0; i < 16; ++i) {
        frame->mat4_values[id][i] = values[i];
    }
    ++frame->mat4_count;
    return id;
}

static int plc_mat4_valid(PLC_FRAME *frame, int id,
    char *err, unsigned err_size)
{
    frame = plc_heap_frame(frame);
    if (frame == 0) {
        plc_set_error(err, err_size, "mat4 access needs a frame");
        return 0;
    }
    if (id < 0 || id >= frame->mat4_count) {
        plc_set_error(err, err_size, "bad mat4 id");
        return 0;
    }
    return 1;
}

static int plc_chess_square_code(int file, int rank)
{
    return file * 10 + rank;
}

static int plc_chess_add_square(PLC_FRAME *frame, int list_id,
    int file, int rank, char *err, unsigned err_size)
{
    double square;

    if (file < 1 || file > 8 || rank < 1 || rank > 8) {
        return 1;
    }
    square = (double)plc_chess_square_code(file, rank);
    if (!plc_list_contains_value(frame, list_id, square)) {
        return plc_list_append(frame, list_id, square, err, err_size);
    }
    return 1;
}

static int plc_chess_attack_map(PLC_FRAME *frame, int piece_kind,
    int file, int rank, char *err, unsigned err_size)
{
    int list_id;
    int step;
    int i;
    static const int knight_delta[8][2] = {
        {1, 2}, {2, 1}, {2, -1}, {1, -2},
        {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}
    };

    list_id = plc_new_list(frame, err, err_size);
    if (list_id < 0) {
        return -1;
    }
    if (piece_kind == 4 || piece_kind == 5) {
        for (step = 1; step <= 7; ++step) {
            if (!plc_chess_add_square(frame, list_id, file + step, rank,
                    err, err_size)
                    || !plc_chess_add_square(frame, list_id, file - step, rank,
                        err, err_size)
                    || !plc_chess_add_square(frame, list_id, file, rank + step,
                        err, err_size)
                    || !plc_chess_add_square(frame, list_id, file, rank - step,
                        err, err_size)) {
                return -1;
            }
        }
    }
    if (piece_kind == 3 || piece_kind == 5) {
        for (step = 1; step <= 7; ++step) {
            if (!plc_chess_add_square(frame, list_id, file + step, rank + step,
                    err, err_size)
                    || !plc_chess_add_square(frame, list_id,
                        file + step, rank - step, err, err_size)
                    || !plc_chess_add_square(frame, list_id,
                        file - step, rank + step, err, err_size)
                    || !plc_chess_add_square(frame, list_id,
                        file - step, rank - step, err, err_size)) {
                return -1;
            }
        }
    }
    if (piece_kind == 2) {
        for (i = 0; i < 8; ++i) {
            if (!plc_chess_add_square(frame, list_id,
                    file + knight_delta[i][0], rank + knight_delta[i][1],
                    err, err_size)) {
                return -1;
            }
        }
    }
    if (piece_kind == 6) {
        int df;
        int dr;

        for (df = -1; df <= 1; ++df) {
            for (dr = -1; dr <= 1; ++dr) {
                if ((df != 0 || dr != 0)
                        && !plc_chess_add_square(frame, list_id,
                            file + df, rank + dr, err, err_size)) {
                    return -1;
                }
            }
        }
    }
    return list_id;
}

static int plc_chess_piece_kind_code(int piece)
{
    return piece / 10;
}

static int plc_chess_piece_side_code(int piece)
{
    return piece % 10;
}

static int plc_chess_board_get_piece(PLC_FRAME *frame, int board_id,
    int square, int *piece, char *err, unsigned err_size)
{
    int slot;

    frame = plc_heap_frame(frame);
    if (!plc_record_valid(frame, board_id, err, err_size)) {
        return 0;
    }
    slot = plc_record_find_key(frame, board_id, square);
    *piece = slot >= 0 ? (int)frame->record_values[board_id][slot] : 0;
    return 1;
}

static int plc_run_boolean_contract(const PLC_PROGRAM *program,
    PLC_FRAME *frame, const PLC_STMT *stmt, int depth,
    const char *keyword, const char *message,
    char *err, unsigned err_size)
{
    const char *expr;
    double guard;

    expr = plc_skip_space(plc_skip_space(stmt->text) + strlen(keyword));
    if (!plc_eval_expr_text(program, frame, depth, expr,
            &guard, err, err_size)) {
        return 0;
    }
    if (guard == 0.0) {
        plc_set_error(err, err_size, message);
        return 0;
    }
    return 1;
}

static int plc_run_statement(const PLC_PROGRAM *program, PLC_FRAME *frame,
    const PLC_PROC *proc, const PLC_STMT *stmt, int depth,
    char *err, unsigned err_size)
{
    char parts[5][PLC_MAX_LINE];
    int count;
    PLC_VALUES values;
    double guard;
    double loop_count;
    int i;

    if (plc_line_starts_with(stmt->text, "CONST")) {
        const char *body;
        const char *eq;
        char target[PLC_MAX_LINE];
        char value_text[PLC_MAX_LINE];
        double const_value;

        body = plc_skip_space(plc_skip_space(stmt->text) + 5);
        eq = strchr(body, '=');
        if (eq == 0 || eq[1] == '>') {
            plc_set_error(err, err_size, "CONST expects target = value");
            return 0;
        }
        plc_copy_range(target, sizeof(target), body, eq);
        plc_trim_in_place(target);
        strncpy(value_text, eq + 1, sizeof(value_text) - 1);
        value_text[sizeof(value_text) - 1] = '\0';
        plc_trim_in_place(value_text);
        if (!plc_eval_expr_text(program, frame, depth, value_text,
                &const_value, err, err_size)) {
            return 0;
        }
        return plc_assign_const_text(frame, target, const_value,
            err, err_size);
    }

    if (plc_line_starts_with(stmt->text, "REQUIRE")) {
        return plc_run_boolean_contract(program, frame, stmt, depth,
            "REQUIRE", "contract requirement failed", err, err_size);
    }
    if (plc_line_starts_with(stmt->text, "ENSURE")) {
        return plc_run_boolean_contract(program, frame, stmt, depth,
            "ENSURE", "contract guarantee failed", err, err_size);
    }
    if (plc_line_starts_with(stmt->text, "STOPIF")) {
        const char *expr;

        expr = plc_skip_space(plc_skip_space(stmt->text) + 6);
        if (!plc_eval_expr_text(program, frame, depth, expr,
                &guard, err, err_size)) {
            return 0;
        }
        if (guard != 0.0) {
            frame->stopped = 1;
        }
        return 1;
    }

    if (plc_line_starts_with(stmt->text, "ASSERT")) {
        return plc_run_boolean_contract(program, frame, stmt, depth,
            "ASSERT", "assertion failed", err, err_size);
    }

    if (!plc_split_arrows(stmt->text, parts, &count)) {
        sprintf(err, "P%d %s line %d: too many arrows",
            proc->number, proc->name, stmt->line_no);
        return 0;
    }
    if (count == 2) {
        if (!plc_eval_value_text(program, frame, depth, parts[0],
                &values, err, err_size)) {
            return 0;
        }
        return plc_assign_targets(frame, parts[1], &values, err, err_size);
    }
    if (count == 3) {
        if (plc_line_starts_with(parts[0], "LOOP")) {
            const char *expr;

            expr = plc_skip_space(plc_skip_space(parts[0]) + 4);
            if (!plc_eval_expr_text(program, frame, depth, expr,
                    &loop_count, err, err_size)) {
                return 0;
            }
            if (loop_count < 0.0) {
                plc_set_error(err, err_size, "negative loop count");
                return 0;
            }
            for (i = 0; i < (int)loop_count; ++i) {
                if (!plc_eval_value_text(program, frame, depth, parts[1],
                        &values, err, err_size)) {
                    return 0;
                }
                if (!plc_assign_targets(frame, parts[2], &values,
                        err, err_size)) {
                    return 0;
                }
            }
            return 1;
        }
        if (!plc_eval_expr_text(program, frame, depth, parts[0],
                &guard, err, err_size)) {
            return 0;
        }
        if (guard == 0.0) {
            return 1;
        }
        if (!plc_eval_value_text(program, frame, depth, parts[1],
                &values, err, err_size)) {
            return 0;
        }
        return plc_assign_targets(frame, parts[2], &values, err, err_size);
    }
    sprintf(err, "P%d %s line %d: expected assignment",
        proc->number, proc->name, stmt->line_no);
    return 0;
}

static int plc_execute_proc_with_heap(const PLC_PROGRAM *program,
    const PLC_PROC *proc,
    const double *args, int argc, PLC_VALUES *out, int depth,
    PLC_FRAME *heap_owner, char *err, unsigned err_size)
{
    PLC_FRAME *frame;
    int i;

    if (depth > PLC_MAX_DEPTH) {
        plc_set_error(err, err_size, "call depth exceeded");
        return 0;
    }
    if (argc != proc->argc) {
        sprintf(err, "%s expects %d argument(s), got %d",
            proc->name, proc->argc, argc);
        return 0;
    }
    frame = (PLC_FRAME *)calloc(1, sizeof(*frame));
    if (frame == 0) {
        plc_set_error(err, err_size, "cannot allocate execution frame");
        return 0;
    }
    frame->heap_owner = heap_owner;
    for (i = 0; i < argc && i < PLC_MAX_VARS; ++i) {
        frame->v[i] = args[i];
    }
    for (i = 0; i < proc->stmt_count; ++i) {
        if (!plc_run_statement(program, frame, proc, &proc->stmts[i],
                depth, err, err_size)) {
            if (err != 0 && err[0] != '\0') {
                char prefix[160];

                sprintf(prefix, "P%d %s line %d: ",
                    proc->number, proc->name,
                    proc->stmts[i].line_no);
                plc_prefix_error(err, err_size, prefix);
            }
            free(frame);
            return 0;
        }
        if (frame->stopped) {
            break;
        }
    }
    out->count = proc->results;
    if (out->count > PLC_MAX_RESULTS) {
        plc_set_error(err, err_size, "too many result values");
        free(frame);
        return 0;
    }
    for (i = 0; i < out->count; ++i) {
        out->value[i] = frame->r[i];
    }
    free(frame);
    return 1;
}

int plc_execute_proc(const PLC_PROGRAM *program, const PLC_PROC *proc,
    const double *args, int argc, PLC_VALUES *out, int depth,
    char *err, unsigned err_size)
{
    return plc_execute_proc_with_heap(program, proc, args, argc, out, depth,
        0, err, err_size);
}

int plc_call_proc(const PLC_PROGRAM *program, PLC_FRAME *caller_frame,
    const char *name, const double *args, int argc, PLC_VALUES *out, int depth,
    char *err, unsigned err_size)
{
    const PLC_PROC *proc;
    PLC_FRAME *heap_frame;
    int list_id;
    int index;
    int other_list;
    int out_list;
    int pair_id;
    int complex_id;
    int other_complex;
    int record_id;
    int field_key;
    int vec_id;
    int other_vec;
    int mat_id;
    int other_mat;
    int i;
    const PLC_NATIVE_PROC *native_proc;

    heap_frame = plc_heap_frame(caller_frame);
    if (strcmp(name, "sqrt") == 0) {
        if (argc != 1) {
            plc_set_error(err, err_size, "sqrt expects one argument");
            return 0;
        }
        out->count = 1;
        out->value[0] = sqrt(args[0]);
        return 1;
    }
    if (strcmp(name, "sin") == 0 || strcmp(name, "cos") == 0) {
        if (argc != 1) {
            plc_set_error(err, err_size,
                "trigonometric function expects one argument");
            return 0;
        }
        out->count = 1;
        out->value[0] = strcmp(name, "sin") == 0
            ? sin(args[0]) : cos(args[0]);
        return 1;
    }
    if (strcmp(name, "bit") == 0 || strcmp(name, "bit_not") == 0) {
        int value;

        if (argc != 1) {
            plc_set_error(err, err_size, "bit operation expects one argument");
            return 0;
        }
        value = ((int)args[0]) != 0 ? 1 : 0;
        out->count = 1;
        out->value[0] = strcmp(name, "bit_not") == 0
            ? (value ? 0.0 : 1.0) : (double)value;
        return 1;
    }
    if (strcmp(name, "bit_and") == 0 || strcmp(name, "bit_or") == 0
            || strcmp(name, "bit_xor") == 0) {
        int left_bit;
        int right_bit;
        int value;

        if (argc != 2) {
            plc_set_error(err, err_size, "bit operation expects two arguments");
            return 0;
        }
        left_bit = ((int)args[0]) != 0 ? 1 : 0;
        right_bit = ((int)args[1]) != 0 ? 1 : 0;
        if (strcmp(name, "bit_and") == 0) {
            value = left_bit && right_bit;
        } else if (strcmp(name, "bit_or") == 0) {
            value = left_bit || right_bit;
        } else {
            value = left_bit != right_bit;
        }
        out->count = 1;
        out->value[0] = (double)value;
        return 1;
    }
    if (strcmp(name, "bits_pack4") == 0) {
        if (argc != 4) {
            plc_set_error(err, err_size, "bits_pack4 expects four arguments");
            return 0;
        }
        out->count = 1;
        out->value[0] = (double)plc_bit_pack4((int)args[0],
            (int)args[1], (int)args[2], (int)args[3]);
        return 1;
    }
    if (strcmp(name, "bits_get") == 0) {
        int index_value;

        if (argc != 2) {
            plc_set_error(err, err_size, "bits_get expects two arguments");
            return 0;
        }
        index_value = (int)args[1];
        if (index_value < 0 || index_value > 30) {
            plc_set_error(err, err_size, "bit index out of range");
            return 0;
        }
        out->count = 1;
        out->value[0] = (double)plc_bit_get_word((unsigned long)args[0],
            index_value);
        return 1;
    }
    if (strcmp(name, "fixed_quantize") == 0
            || strcmp(name, "fixed_add") == 0
            || strcmp(name, "fixed_mul") == 0) {
        int scale;
        double value;

        if ((strcmp(name, "fixed_quantize") == 0 && argc != 2)
                || (strcmp(name, "fixed_quantize") != 0 && argc != 3)) {
            plc_set_error(err, err_size, "bad fixed operation argument count");
            return 0;
        }
        scale = (int)args[argc - 1];
        if (scale < 0 || scale > 30) {
            plc_set_error(err, err_size, "fixed scale out of range");
            return 0;
        }
        if (strcmp(name, "fixed_quantize") == 0) {
            value = plc_fixed_quantize_bits(args[0], scale);
        } else if (strcmp(name, "fixed_add") == 0) {
            value = plc_fixed_add_bits(args[0], args[1], scale);
        } else {
            value = plc_fixed_mul_bits(args[0], args[1], scale);
        }
        out->count = 1;
        out->value[0] = plc_fixed_quantize_bits(value, scale);
        return 1;
    }
    if (strcmp(name, "fixed_div_checked") == 0
            || strcmp(name, "arith_divide_checked") == 0) {
        int scale;

        if ((strcmp(name, "fixed_div_checked") == 0 && argc != 3)
                || (strcmp(name, "arith_divide_checked") == 0 && argc != 2)) {
            plc_set_error(err, err_size,
                "bad checked division argument count");
            return 0;
        }
        out->count = 2;
        if (fabs(args[1]) < 0.0000001) {
            out->value[0] = 0.0;
            out->value[1] = 1.0;
            if (caller_frame != 0) {
                caller_frame->exception_raised = 1;
                caller_frame->exception_code = 1;
            }
            return 1;
        }
        if (strcmp(name, "fixed_div_checked") == 0) {
            scale = (int)args[2];
            if (scale < 0 || scale > 30) {
                plc_set_error(err, err_size, "fixed scale out of range");
                return 0;
            }
            if (!plc_fixed_div_bits(args[0], args[1], scale,
                    &out->value[0])) {
                out->value[0] = 0.0;
                out->value[1] = 1.0;
                if (caller_frame != 0) {
                    caller_frame->exception_raised = 1;
                    caller_frame->exception_code = 1;
                }
                return 1;
            }
        } else {
            out->value[0] = args[0] / args[1];
        }
        out->value[1] = 0.0;
        return 1;
    }
    if (strcmp(name, "raise_exception") == 0
            || strcmp(name, "exception_raised") == 0
            || strcmp(name, "exception_code") == 0
            || strcmp(name, "exception_clear") == 0) {
        if (caller_frame == 0) {
            plc_set_error(err, err_size, "exception operation needs a frame");
            return 0;
        }
        if ((strcmp(name, "raise_exception") == 0 && argc != 1)
                || (strcmp(name, "raise_exception") != 0 && argc != 0)) {
            plc_set_error(err, err_size,
                "bad exception operation argument count");
            return 0;
        }
        if (strcmp(name, "raise_exception") == 0) {
            caller_frame->exception_raised = 1;
            caller_frame->exception_code = (int)args[0];
            out->count = 1;
            out->value[0] = (double)caller_frame->exception_code;
            return 1;
        }
        if (strcmp(name, "exception_clear") == 0) {
            caller_frame->exception_raised = 0;
            caller_frame->exception_code = 0;
            out->count = 1;
            out->value[0] = 0.0;
            return 1;
        }
        out->count = 1;
        out->value[0] = strcmp(name, "exception_raised") == 0
            ? (double)caller_frame->exception_raised
            : (double)caller_frame->exception_code;
        return 1;
    }
    if (strcmp(name, "list_new") == 0) {
        if (caller_frame == 0) {
            plc_set_error(err, err_size, "list_new needs a caller frame");
            return 0;
        }
        if (argc != 0) {
            plc_set_error(err, err_size, "list_new expects no arguments");
            return 0;
        }
        if (heap_frame->list_count >= PLC_MAX_LISTS) {
            plc_set_error(err, err_size, "too many lists");
            return 0;
        }
        list_id = plc_new_list(heap_frame, err, err_size);
        if (list_id < 0) {
            return 0;
        }
        out->count = 1;
        out->value[0] = (double)list_id;
        return 1;
    }
    if (strcmp(name, "list_push") == 0) {
        if (caller_frame == 0) {
            plc_set_error(err, err_size, "list_push needs a caller frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size, "list_push expects two arguments");
            return 0;
        }
        list_id = (int)args[0];
        if (!plc_list_append(heap_frame, list_id, args[1],
                err, err_size)) {
            return 0;
        }
        out->count = 1;
        out->value[0] = (double)list_id;
        return 1;
    }
    if (strcmp(name, "list_len") == 0) {
        if (caller_frame == 0) {
            plc_set_error(err, err_size, "list_len needs a caller frame");
            return 0;
        }
        if (argc != 1) {
            plc_set_error(err, err_size, "list_len expects one argument");
            return 0;
        }
        list_id = (int)args[0];
        if (!plc_list_valid(heap_frame, list_id, err, err_size)) {
            return 0;
        }
        out->count = 1;
        out->value[0] = (double)heap_frame->list_sizes[list_id];
        return 1;
    }
    if (strcmp(name, "list_get") == 0) {
        if (caller_frame == 0) {
            plc_set_error(err, err_size, "list_get needs a caller frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size, "list_get expects two arguments");
            return 0;
        }
        list_id = (int)args[0];
        index = (int)args[1];
        if (list_id < 0 || list_id >= heap_frame->list_count
                || index < 0 || index >= heap_frame->list_sizes[list_id]) {
            plc_set_error(err, err_size, "bad list access");
            return 0;
        }
        out->count = 1;
        out->value[0] = heap_frame->lists[list_id][index];
        return 1;
    }
    if (strcmp(name, "list_first") == 0 || strcmp(name, "list_last") == 0
            || strcmp(name, "list_min") == 0 || strcmp(name, "list_max") == 0) {
        double value;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "list operation needs a caller frame");
            return 0;
        }
        if (argc != 1) {
            plc_set_error(err, err_size, "list operation expects one argument");
            return 0;
        }
        list_id = (int)args[0];
        if (!plc_list_valid(heap_frame, list_id, err, err_size)) {
            return 0;
        }
        if (heap_frame->list_sizes[list_id] <= 0) {
            plc_set_error(err, err_size, "empty list");
            return 0;
        }
        if (strcmp(name, "list_last") == 0) {
            value = heap_frame->lists[list_id]
                [heap_frame->list_sizes[list_id] - 1];
        } else {
            value = heap_frame->lists[list_id][0];
        }
        if (strcmp(name, "list_min") == 0 || strcmp(name, "list_max") == 0) {
            for (i = 1; i < heap_frame->list_sizes[list_id]; ++i) {
                if (strcmp(name, "list_min") == 0
                        && heap_frame->lists[list_id][i] < value) {
                    value = heap_frame->lists[list_id][i];
                }
                if (strcmp(name, "list_max") == 0
                        && heap_frame->lists[list_id][i] > value) {
                    value = heap_frame->lists[list_id][i];
                }
            }
        }
        out->count = 1;
        out->value[0] = value;
        return 1;
    }
    if (strcmp(name, "list_concat") == 0) {
        if (caller_frame == 0) {
            plc_set_error(err, err_size, "list_concat needs a caller frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size, "list_concat expects two arguments");
            return 0;
        }
        list_id = (int)args[0];
        other_list = (int)args[1];
        if (!plc_list_valid(heap_frame, list_id, err, err_size)
                || !plc_list_valid(heap_frame, other_list, err, err_size)) {
            return 0;
        }
        out_list = plc_new_list(heap_frame, err, err_size);
        if (out_list < 0) {
            return 0;
        }
        for (i = 0; i < heap_frame->list_sizes[list_id]; ++i) {
            if (!plc_list_append(heap_frame, out_list,
                    heap_frame->lists[list_id][i], err, err_size)) {
                return 0;
            }
        }
        for (i = 0; i < heap_frame->list_sizes[other_list]; ++i) {
            if (!plc_list_append(heap_frame, out_list,
                    heap_frame->lists[other_list][i], err, err_size)) {
                return 0;
            }
        }
        out->count = 1;
        out->value[0] = (double)out_list;
        return 1;
    }
    if (strcmp(name, "list_select_where") == 0
            || strcmp(name, "list_count_where") == 0
            || strcmp(name, "list_exists_where") == 0
            || strcmp(name, "list_forall_where") == 0
            || strcmp(name, "set_select_where") == 0
            || strcmp(name, "set_count_where") == 0
            || strcmp(name, "set_exists_where") == 0
            || strcmp(name, "set_forall_where") == 0) {
        int op_code;
        int matched;
        int count_value;
        int ok;
        int is_select;
        int is_count;
        int is_forall;
        int is_set;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "container predicate needs a frame");
            return 0;
        }
        if (argc != 3) {
            plc_set_error(err, err_size,
                "container predicate expects three arguments");
            return 0;
        }
        list_id = (int)args[0];
        op_code = (int)args[1];
        if (!plc_list_valid(heap_frame, list_id, err, err_size)) {
            return 0;
        }
        is_select = strstr(name, "_select_") != 0;
        is_count = strstr(name, "_count_") != 0;
        is_forall = strstr(name, "_forall_") != 0;
        is_set = strncmp(name, "set_", 4) == 0;
        count_value = 0;
        ok = is_forall ? 1 : 0;
        out_list = -1;
        if (is_select) {
            out_list = plc_new_list(heap_frame, err, err_size);
            if (out_list < 0) {
                return 0;
            }
        }
        for (i = 0; i < heap_frame->list_sizes[list_id]; ++i) {
            double value;

            value = heap_frame->lists[list_id][i];
            matched = plc_compare_where(value, op_code, args[2]);
            if (matched) {
                ++count_value;
                if (!is_forall) {
                    ok = 1;
                }
                if (is_select
                        && (!is_set
                            || !plc_list_contains_value(heap_frame,
                                out_list, value))
                        && !plc_list_append(heap_frame, out_list, value,
                            err, err_size)) {
                    return 0;
                }
            } else if (is_forall) {
                ok = 0;
            }
        }
        out->count = 1;
        if (is_select) {
            out->value[0] = (double)out_list;
        } else if (is_count) {
            out->value[0] = (double)count_value;
        } else {
            out->value[0] = ok ? 1.0 : 0.0;
        }
        return 1;
    }
    if (strcmp(name, "list_select_greater") == 0
            || strcmp(name, "list_count_equal") == 0
            || strcmp(name, "list_exists_equal") == 0
            || strcmp(name, "list_forall_greater") == 0) {
        double threshold;
        int count_value;
        int ok;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "list predicate needs a caller frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size, "list predicate expects two arguments");
            return 0;
        }
        list_id = (int)args[0];
        threshold = args[1];
        if (!plc_list_valid(heap_frame, list_id, err, err_size)) {
            return 0;
        }
        if (strcmp(name, "list_select_greater") == 0) {
            out_list = plc_new_list(heap_frame, err, err_size);
            if (out_list < 0) {
                return 0;
            }
            for (i = 0; i < heap_frame->list_sizes[list_id]; ++i) {
                if (heap_frame->lists[list_id][i] > threshold
                        && !plc_list_append(heap_frame, out_list,
                            heap_frame->lists[list_id][i],
                            err, err_size)) {
                    return 0;
                }
            }
            out->count = 1;
            out->value[0] = (double)out_list;
            return 1;
        }
        count_value = 0;
        ok = strcmp(name, "list_forall_greater") == 0 ? 1 : 0;
        for (i = 0; i < heap_frame->list_sizes[list_id]; ++i) {
            if (fabs(heap_frame->lists[list_id][i] - threshold)
                    < 0.0000001) {
                ++count_value;
            }
            if (strcmp(name, "list_exists_equal") == 0
                    && fabs(heap_frame->lists[list_id][i] - threshold)
                        < 0.0000001) {
                ok = 1;
            }
            if (strcmp(name, "list_forall_greater") == 0
                    && !(heap_frame->lists[list_id][i] > threshold)) {
                ok = 0;
            }
        }
        out->count = 1;
        if (strcmp(name, "list_count_equal") == 0) {
            out->value[0] = (double)count_value;
        } else {
            out->value[0] = (double)ok;
        }
        return 1;
    }
    if (strcmp(name, "list_zip_pairs") == 0
            || strcmp(name, "list_pair") == 0) {
        int size;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "list pair operation needs a frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size,
                "list pair operation expects two arguments");
            return 0;
        }
        list_id = (int)args[0];
        other_list = (int)args[1];
        if (!plc_list_valid(heap_frame, list_id, err, err_size)
                || !plc_list_valid(heap_frame, other_list, err, err_size)) {
            return 0;
        }
        if (strcmp(name, "list_pair") == 0) {
            if (heap_frame->pair_count >= PLC_MAX_PAIRS) {
                plc_set_error(err, err_size, "too many pairs");
                return 0;
            }
            pair_id = heap_frame->pair_count;
            heap_frame->pair_left[pair_id] = (double)list_id;
            heap_frame->pair_right[pair_id] = (double)other_list;
            ++heap_frame->pair_count;
            out->count = 1;
            out->value[0] = (double)pair_id;
            return 1;
        }
        size = heap_frame->list_sizes[list_id] < heap_frame->list_sizes[other_list]
            ? heap_frame->list_sizes[list_id] : heap_frame->list_sizes[other_list];
        out_list = plc_new_list(heap_frame, err, err_size);
        if (out_list < 0) {
            return 0;
        }
        for (i = 0; i < size; ++i) {
            if (heap_frame->pair_count >= PLC_MAX_PAIRS) {
                plc_set_error(err, err_size, "too many pairs");
                return 0;
            }
            pair_id = heap_frame->pair_count;
            heap_frame->pair_left[pair_id] = heap_frame->lists[list_id][i];
            heap_frame->pair_right[pair_id] = heap_frame->lists[other_list][i];
            ++heap_frame->pair_count;
            if (!plc_list_append(heap_frame, out_list, (double)pair_id,
                    err, err_size)) {
                return 0;
            }
        }
        out->count = 1;
        out->value[0] = (double)out_list;
        return 1;
    }
    if (strcmp(name, "pair") == 0) {
        if (caller_frame == 0) {
            plc_set_error(err, err_size, "pair needs a caller frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size, "pair expects two arguments");
            return 0;
        }
        if (heap_frame->pair_count >= PLC_MAX_PAIRS) {
            plc_set_error(err, err_size, "too many pairs");
            return 0;
        }
        pair_id = heap_frame->pair_count;
        heap_frame->pair_left[pair_id] = args[0];
        heap_frame->pair_right[pair_id] = args[1];
        ++heap_frame->pair_count;
        out->count = 1;
        out->value[0] = (double)pair_id;
        return 1;
    }
    if (strcmp(name, "pair_left") == 0 || strcmp(name, "pair_right") == 0) {
        if (caller_frame == 0) {
            plc_set_error(err, err_size, "pair access needs a caller frame");
            return 0;
        }
        if (argc != 1) {
            plc_set_error(err, err_size, "pair access expects one argument");
            return 0;
        }
        pair_id = (int)args[0];
        if (pair_id < 0 || pair_id >= heap_frame->pair_count) {
            plc_set_error(err, err_size, "bad pair id");
            return 0;
        }
        out->count = 1;
        out->value[0] = strcmp(name, "pair_left") == 0
            ? heap_frame->pair_left[pair_id]
            : heap_frame->pair_right[pair_id];
        return 1;
    }
    if (strcmp(name, "pair_list_count_first") == 0) {
        int count;

        if (caller_frame == 0) {
            plc_set_error(err, err_size,
                "pair_list_count_first needs a caller frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size,
                "pair_list_count_first expects two arguments");
            return 0;
        }
        list_id = (int)args[0];
        if (!plc_list_valid(heap_frame, list_id, err, err_size)) {
            return 0;
        }
        count = 0;
        for (i = 0; i < heap_frame->list_sizes[list_id]; ++i) {
            pair_id = (int)heap_frame->lists[list_id][i];
            if (pair_id >= 0 && pair_id < heap_frame->pair_count
                    && fabs(heap_frame->pair_left[pair_id] - args[1])
                        < 0.0000001) {
                ++count;
            }
        }
        out->count = 1;
        out->value[0] = (double)count;
        return 1;
    }
    if (strcmp(name, "pair_left_list_len") == 0
            || strcmp(name, "pair_right_list_len") == 0) {
        double list_value;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "pair list access needs a frame");
            return 0;
        }
        if (argc != 1) {
            plc_set_error(err, err_size, "pair list access expects one argument");
            return 0;
        }
        pair_id = (int)args[0];
        if (pair_id < 0 || pair_id >= heap_frame->pair_count) {
            plc_set_error(err, err_size, "bad pair id");
            return 0;
        }
        list_value = strcmp(name, "pair_left_list_len") == 0
            ? heap_frame->pair_left[pair_id]
            : heap_frame->pair_right[pair_id];
        list_id = (int)list_value;
        if (!plc_list_valid(heap_frame, list_id, err, err_size)) {
            return 0;
        }
        out->count = 1;
        out->value[0] = (double)heap_frame->list_sizes[list_id];
        return 1;
    }
    if (strcmp(name, "set_new") == 0) {
        if (caller_frame == 0) {
            plc_set_error(err, err_size, "set_new needs a caller frame");
            return 0;
        }
        if (argc != 0) {
            plc_set_error(err, err_size, "set_new expects no arguments");
            return 0;
        }
        list_id = plc_new_list(heap_frame, err, err_size);
        if (list_id < 0) {
            return 0;
        }
        out->count = 1;
        out->value[0] = (double)list_id;
        return 1;
    }
    if (strcmp(name, "set_add") == 0) {
        if (caller_frame == 0) {
            plc_set_error(err, err_size, "set_add needs a caller frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size, "set_add expects two arguments");
            return 0;
        }
        list_id = (int)args[0];
        if (!plc_list_valid(heap_frame, list_id, err, err_size)) {
            return 0;
        }
        if (!plc_list_contains_value(heap_frame, list_id, args[1])) {
            if (!plc_list_append(heap_frame, list_id, args[1],
                    err, err_size)) {
                return 0;
            }
        }
        out->count = 1;
        out->value[0] = (double)list_id;
        return 1;
    }
    if (strcmp(name, "set_contains") == 0 || strcmp(name, "set_size") == 0) {
        if (caller_frame == 0) {
            plc_set_error(err, err_size, "set operation needs a caller frame");
            return 0;
        }
        if (strcmp(name, "set_size") == 0 && argc != 1) {
            plc_set_error(err, err_size, "set_size expects one argument");
            return 0;
        }
        if (strcmp(name, "set_contains") == 0 && argc != 2) {
            plc_set_error(err, err_size, "set_contains expects two arguments");
            return 0;
        }
        list_id = (int)args[0];
        if (!plc_list_valid(heap_frame, list_id, err, err_size)) {
            return 0;
        }
        out->count = 1;
        out->value[0] = strcmp(name, "set_size") == 0
            ? (double)heap_frame->list_sizes[list_id]
            : (double)plc_list_contains_value(heap_frame, list_id, args[1]);
        return 1;
    }
    if (strcmp(name, "set_union") == 0
            || strcmp(name, "set_intersection") == 0
            || strcmp(name, "set_difference") == 0) {
        if (caller_frame == 0) {
            plc_set_error(err, err_size, "set operation needs a caller frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size, "set operation expects two arguments");
            return 0;
        }
        list_id = (int)args[0];
        other_list = (int)args[1];
        if (!plc_list_valid(heap_frame, list_id, err, err_size)
                || !plc_list_valid(heap_frame, other_list, err, err_size)) {
            return 0;
        }
        out_list = plc_new_list(heap_frame, err, err_size);
        if (out_list < 0) {
            return 0;
        }
        for (i = 0; i < heap_frame->list_sizes[list_id]; ++i) {
            double value;
            int include_value;

            value = heap_frame->lists[list_id][i];
            include_value = 0;
            if (strcmp(name, "set_union") == 0) {
                include_value = 1;
            } else if (strcmp(name, "set_intersection") == 0) {
                include_value = plc_list_contains_value(heap_frame,
                    other_list, value);
            } else {
                include_value = !plc_list_contains_value(heap_frame,
                    other_list, value);
            }
            if (include_value) {
                if (!plc_list_append(heap_frame, out_list, value,
                        err, err_size)) {
                    return 0;
                }
            }
        }
        if (strcmp(name, "set_union") == 0) {
            for (i = 0; i < heap_frame->list_sizes[other_list]; ++i) {
                double value;

                value = heap_frame->lists[other_list][i];
                if (!plc_list_contains_value(heap_frame, out_list, value)) {
                    if (!plc_list_append(heap_frame, out_list, value,
                            err, err_size)) {
                        return 0;
                    }
                }
            }
        }
        out->count = 1;
        out->value[0] = (double)out_list;
        return 1;
    }
    if (strcmp(name, "set_subset") == 0) {
        int ok;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "set_subset needs a caller frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size, "set_subset expects two arguments");
            return 0;
        }
        list_id = (int)args[0];
        other_list = (int)args[1];
        if (!plc_list_valid(heap_frame, list_id, err, err_size)
                || !plc_list_valid(heap_frame, other_list, err, err_size)) {
            return 0;
        }
        ok = 1;
        for (i = 0; i < heap_frame->list_sizes[list_id]; ++i) {
            if (!plc_list_contains_value(heap_frame, other_list,
                    heap_frame->lists[list_id][i])) {
                ok = 0;
            }
        }
        out->count = 1;
        out->value[0] = (double)ok;
        return 1;
    }
    if (strcmp(name, "relation_domain") == 0
            || strcmp(name, "relation_range") == 0) {
        if (caller_frame == 0) {
            plc_set_error(err, err_size, "relation projection needs a frame");
            return 0;
        }
        if (argc != 1) {
            plc_set_error(err, err_size,
                "relation projection expects one argument");
            return 0;
        }
        list_id = (int)args[0];
        if (!plc_list_valid(heap_frame, list_id, err, err_size)) {
            return 0;
        }
        out_list = plc_new_list(heap_frame, err, err_size);
        if (out_list < 0) {
            return 0;
        }
        for (i = 0; i < heap_frame->list_sizes[list_id]; ++i) {
            double value;

            pair_id = (int)heap_frame->lists[list_id][i];
            if (pair_id < 0 || pair_id >= heap_frame->pair_count) {
                plc_set_error(err, err_size, "bad pair id in relation");
                return 0;
            }
            value = strcmp(name, "relation_domain") == 0
                ? heap_frame->pair_left[pair_id]
                : heap_frame->pair_right[pair_id];
            if (!plc_list_contains_value(heap_frame, out_list, value)) {
                if (!plc_list_append(heap_frame, out_list, value,
                        err, err_size)) {
                    return 0;
                }
            }
        }
        out->count = 1;
        out->value[0] = (double)out_list;
        return 1;
    }
    if (strcmp(name, "relation_has_pair") == 0) {
        int found;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "relation_has_pair needs a frame");
            return 0;
        }
        if (argc != 3) {
            plc_set_error(err, err_size,
                "relation_has_pair expects three arguments");
            return 0;
        }
        list_id = (int)args[0];
        if (!plc_list_valid(heap_frame, list_id, err, err_size)) {
            return 0;
        }
        found = 0;
        for (i = 0; i < heap_frame->list_sizes[list_id]; ++i) {
            pair_id = (int)heap_frame->lists[list_id][i];
            if (pair_id >= 0 && pair_id < heap_frame->pair_count
                    && fabs(heap_frame->pair_left[pair_id] - args[1])
                        < 0.0000001
                    && fabs(heap_frame->pair_right[pair_id] - args[2])
                        < 0.0000001) {
                found = 1;
            }
        }
        out->count = 1;
        out->value[0] = (double)found;
        return 1;
    }
    if (strcmp(name, "relation_select_domain") == 0
            || strcmp(name, "relation_select_range") == 0) {
        if (caller_frame == 0) {
            plc_set_error(err, err_size,
                "relation selection needs a frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size,
                "relation selection expects two arguments");
            return 0;
        }
        list_id = (int)args[0];
        if (!plc_list_valid(heap_frame, list_id, err, err_size)) {
            return 0;
        }
        out_list = plc_new_list(heap_frame, err, err_size);
        if (out_list < 0) {
            return 0;
        }
        for (i = 0; i < heap_frame->list_sizes[list_id]; ++i) {
            pair_id = (int)heap_frame->lists[list_id][i];
            if (pair_id < 0 || pair_id >= heap_frame->pair_count) {
                plc_set_error(err, err_size, "bad pair id in relation");
                return 0;
            }
            if (((strcmp(name, "relation_select_domain") == 0
                        && fabs(heap_frame->pair_left[pair_id] - args[1])
                            < 0.0000001)
                    || (strcmp(name, "relation_select_range") == 0
                        && fabs(heap_frame->pair_right[pair_id] - args[1])
                            < 0.0000001))
                    && !plc_list_append(heap_frame, out_list,
                        (double)pair_id, err, err_size)) {
                return 0;
            }
        }
        out->count = 1;
        out->value[0] = (double)out_list;
        return 1;
    }
    if (strcmp(name, "relation_domain_select_where") == 0
            || strcmp(name, "relation_domain_count_where") == 0
            || strcmp(name, "relation_domain_exists_where") == 0
            || strcmp(name, "relation_domain_forall_where") == 0
            || strcmp(name, "relation_range_select_where") == 0
            || strcmp(name, "relation_range_count_where") == 0
            || strcmp(name, "relation_range_exists_where") == 0
            || strcmp(name, "relation_range_forall_where") == 0) {
        int op_code;
        int matched;
        int count_value;
        int ok;
        int is_domain;
        int is_select;
        int is_count;
        int is_forall;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "relation predicate needs a frame");
            return 0;
        }
        if (argc != 3) {
            plc_set_error(err, err_size,
                "relation predicate expects three arguments");
            return 0;
        }
        list_id = (int)args[0];
        op_code = (int)args[1];
        if (!plc_list_valid(heap_frame, list_id, err, err_size)) {
            return 0;
        }
        is_domain = strncmp(name, "relation_domain_", 16) == 0;
        is_select = strstr(name, "_select_") != 0;
        is_count = strstr(name, "_count_") != 0;
        is_forall = strstr(name, "_forall_") != 0;
        count_value = 0;
        ok = is_forall ? 1 : 0;
        out_list = -1;
        if (is_select) {
            out_list = plc_new_list(heap_frame, err, err_size);
            if (out_list < 0) {
                return 0;
            }
        }
        for (i = 0; i < heap_frame->list_sizes[list_id]; ++i) {
            double value;

            pair_id = (int)heap_frame->lists[list_id][i];
            if (pair_id < 0 || pair_id >= heap_frame->pair_count) {
                plc_set_error(err, err_size, "bad pair id in relation");
                return 0;
            }
            value = is_domain
                ? heap_frame->pair_left[pair_id]
                : heap_frame->pair_right[pair_id];
            matched = plc_compare_where(value, op_code, args[2]);
            if (matched) {
                ++count_value;
                if (!is_forall) {
                    ok = 1;
                }
                if (is_select
                        && !plc_list_append(heap_frame, out_list,
                            (double)pair_id, err, err_size)) {
                    return 0;
                }
            } else if (is_forall) {
                ok = 0;
            }
        }
        out->count = 1;
        if (is_select) {
            out->value[0] = (double)out_list;
        } else if (is_count) {
            out->value[0] = (double)count_value;
        } else {
            out->value[0] = ok ? 1.0 : 0.0;
        }
        return 1;
    }
    if (strcmp(name, "relation_exists_range_equal") == 0
            || strcmp(name, "relation_forall_domain_greater") == 0
            || strcmp(name, "relation_signature") == 0) {
        int predicate_ok;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "relation predicate needs a frame");
            return 0;
        }
        if ((strcmp(name, "relation_signature") == 0 && argc != 1)
                || (strcmp(name, "relation_signature") != 0 && argc != 2)) {
            plc_set_error(err, err_size,
                "relation predicate has bad argument count");
            return 0;
        }
        list_id = (int)args[0];
        if (!plc_list_valid(heap_frame, list_id, err, err_size)) {
            return 0;
        }
        predicate_ok = strcmp(name, "relation_forall_domain_greater") == 0
            ? 1 : 0;
        if (strcmp(name, "relation_signature") == 0) {
            int signature;

            signature = 23 + heap_frame->list_sizes[list_id] * 37;
            for (i = 0; i < heap_frame->list_sizes[list_id]; ++i) {
                pair_id = (int)heap_frame->lists[list_id][i];
                if (pair_id < 0 || pair_id >= heap_frame->pair_count) {
                    plc_set_error(err, err_size, "bad pair id in relation");
                    return 0;
                }
                signature = (signature * 31
                    + (int)heap_frame->pair_left[pair_id] * 7
                    + (int)heap_frame->pair_right[pair_id] * 11) % 32767;
            }
            out->count = 1;
            out->value[0] = (double)signature;
            return 1;
        }
        for (i = 0; i < heap_frame->list_sizes[list_id]; ++i) {
            pair_id = (int)heap_frame->lists[list_id][i];
            if (pair_id < 0 || pair_id >= heap_frame->pair_count) {
                plc_set_error(err, err_size, "bad pair id in relation");
                return 0;
            }
            if (strcmp(name, "relation_exists_range_equal") == 0
                    && fabs(heap_frame->pair_right[pair_id] - args[1])
                        < 0.0000001) {
                predicate_ok = 1;
            } else if (strcmp(name, "relation_forall_domain_greater") == 0
                    && heap_frame->pair_left[pair_id] <= args[1]) {
                predicate_ok = 0;
            }
        }
        out->count = 1;
        out->value[0] = predicate_ok ? 1.0 : 0.0;
        return 1;
    }
    if (strcmp(name, "set_cartesian") == 0
            || strcmp(name, "relation_compose") == 0) {
        if (caller_frame == 0) {
            plc_set_error(err, err_size, "relation operation needs a frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size,
                "relation operation expects two arguments");
            return 0;
        }
        list_id = (int)args[0];
        other_list = (int)args[1];
        if (!plc_list_valid(heap_frame, list_id, err, err_size)
                || !plc_list_valid(heap_frame, other_list, err, err_size)) {
            return 0;
        }
        out_list = plc_new_list(heap_frame, err, err_size);
        if (out_list < 0) {
            return 0;
        }
        if (strcmp(name, "set_cartesian") == 0) {
            int j;

            for (i = 0; i < heap_frame->list_sizes[list_id]; ++i) {
                for (j = 0; j < heap_frame->list_sizes[other_list]; ++j) {
                    if (heap_frame->pair_count >= PLC_MAX_PAIRS) {
                        plc_set_error(err, err_size, "too many pairs");
                        return 0;
                    }
                    pair_id = heap_frame->pair_count;
                    heap_frame->pair_left[pair_id] =
                        heap_frame->lists[list_id][i];
                    heap_frame->pair_right[pair_id] =
                        heap_frame->lists[other_list][j];
                    ++heap_frame->pair_count;
                    if (!plc_list_append(heap_frame, out_list,
                            (double)pair_id, err, err_size)) {
                        return 0;
                    }
                }
            }
        } else {
            int j;

            for (i = 0; i < heap_frame->list_sizes[list_id]; ++i) {
                int left_pair;

                left_pair = (int)heap_frame->lists[list_id][i];
                if (left_pair < 0 || left_pair >= heap_frame->pair_count) {
                    plc_set_error(err, err_size, "bad pair id in relation");
                    return 0;
                }
                for (j = 0; j < heap_frame->list_sizes[other_list]; ++j) {
                    int right_pair;

                    right_pair = (int)heap_frame->lists[other_list][j];
                    if (right_pair < 0
                            || right_pair >= heap_frame->pair_count) {
                        plc_set_error(err, err_size,
                            "bad pair id in relation");
                        return 0;
                    }
                    if (fabs(heap_frame->pair_right[left_pair]
                            - heap_frame->pair_left[right_pair])
                            < 0.0000001) {
                        if (heap_frame->pair_count >= PLC_MAX_PAIRS) {
                            plc_set_error(err, err_size, "too many pairs");
                            return 0;
                        }
                        pair_id = heap_frame->pair_count;
                        heap_frame->pair_left[pair_id] =
                            heap_frame->pair_left[left_pair];
                        heap_frame->pair_right[pair_id] =
                            heap_frame->pair_right[right_pair];
                        ++heap_frame->pair_count;
                        if (!plc_list_append(heap_frame, out_list,
                                (double)pair_id, err, err_size)) {
                            return 0;
                        }
                    }
                }
            }
        }
        out->count = 1;
        out->value[0] = (double)out_list;
        return 1;
    }
    if (strcmp(name, "set_exists_greater") == 0
            || strcmp(name, "set_forall_less") == 0) {
        int ok;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "quantified set check needs a frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size,
                "quantified set check expects two arguments");
            return 0;
        }
        list_id = (int)args[0];
        if (!plc_list_valid(heap_frame, list_id, err, err_size)) {
            return 0;
        }
        ok = strcmp(name, "set_forall_less") == 0 ? 1 : 0;
        for (i = 0; i < heap_frame->list_sizes[list_id]; ++i) {
            if (strcmp(name, "set_exists_greater") == 0
                    && heap_frame->lists[list_id][i] > args[1]) {
                ok = 1;
            }
            if (strcmp(name, "set_forall_less") == 0
                    && !(heap_frame->lists[list_id][i] < args[1])) {
                ok = 0;
            }
        }
        out->count = 1;
        out->value[0] = (double)ok;
        return 1;
    }
    if (strcmp(name, "record_new") == 0) {
        if (caller_frame == 0) {
            plc_set_error(err, err_size, "record_new needs a caller frame");
            return 0;
        }
        if (argc != 0) {
            plc_set_error(err, err_size, "record_new expects no arguments");
            return 0;
        }
        record_id = plc_new_record(heap_frame, err, err_size);
        if (record_id < 0) {
            return 0;
        }
        out->count = 1;
        out->value[0] = (double)record_id;
        return 1;
    }
    if (strcmp(name, "record_set") == 0) {
        if (caller_frame == 0) {
            plc_set_error(err, err_size, "record_set needs a caller frame");
            return 0;
        }
        if (argc != 3) {
            plc_set_error(err, err_size, "record_set expects three arguments");
            return 0;
        }
        record_id = (int)args[0];
        field_key = (int)args[1];
        if (!plc_record_set_value(heap_frame, record_id, field_key,
                args[2], err, err_size)) {
            return 0;
        }
        out->count = 1;
        out->value[0] = (double)record_id;
        return 1;
    }
    if (strcmp(name, "record_get") == 0 || strcmp(name, "record_has") == 0
            || strcmp(name, "record_size") == 0) {
        int slot;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "record access needs a caller frame");
            return 0;
        }
        if (strcmp(name, "record_size") == 0 && argc != 1) {
            plc_set_error(err, err_size, "record_size expects one argument");
            return 0;
        }
        if (strcmp(name, "record_size") != 0 && argc != 2) {
            plc_set_error(err, err_size, "record access expects two arguments");
            return 0;
        }
        record_id = (int)args[0];
        if (!plc_record_valid(heap_frame, record_id, err, err_size)) {
            return 0;
        }
        if (strcmp(name, "record_size") == 0) {
            out->count = 1;
            out->value[0] = (double)heap_frame->record_sizes[record_id];
            return 1;
        }
        field_key = (int)args[1];
        slot = plc_record_find_key(heap_frame, record_id, field_key);
        if (strcmp(name, "record_has") == 0) {
            out->count = 1;
            out->value[0] = slot >= 0 ? 1.0 : 0.0;
            return 1;
        }
        if (slot < 0) {
            plc_set_error(err, err_size, "missing record field");
            return 0;
        }
        out->count = 1;
        out->value[0] = heap_frame->record_values[record_id][slot];
        return 1;
    }
    if (strcmp(name, "record_set_path2") == 0
            || strcmp(name, "record_get_path2") == 0
            || strcmp(name, "record_has_path2") == 0) {
        int key1;
        int key2;
        int child_id;
        int slot;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "record path operation needs a frame");
            return 0;
        }
        if ((strcmp(name, "record_set_path2") == 0 && argc != 4)
                || (strcmp(name, "record_set_path2") != 0 && argc != 3)) {
            plc_set_error(err, err_size,
                "bad record path operation argument count");
            return 0;
        }
        record_id = (int)args[0];
        key1 = (int)args[1];
        key2 = (int)args[2];
        if (!plc_record_valid(heap_frame, record_id, err, err_size)) {
            return 0;
        }
        slot = plc_record_find_key(heap_frame, record_id, key1);
        if (slot < 0) {
            if (strcmp(name, "record_set_path2") != 0) {
                out->count = 1;
                out->value[0] = 0.0;
                return 1;
            }
            child_id = plc_new_record(heap_frame, err, err_size);
            if (child_id < 0
                    || !plc_record_set_value(heap_frame, record_id, key1,
                        (double)child_id, err, err_size)) {
                return 0;
            }
        } else {
            child_id = (int)heap_frame->record_values[record_id][slot];
        }
        if (!plc_record_valid(heap_frame, child_id, err, err_size)) {
            return 0;
        }
        if (strcmp(name, "record_set_path2") == 0) {
            if (!plc_record_set_value(heap_frame, child_id, key2,
                    args[3], err, err_size)) {
                return 0;
            }
            out->count = 1;
            out->value[0] = (double)record_id;
            return 1;
        }
        slot = plc_record_find_key(heap_frame, child_id, key2);
        if (strcmp(name, "record_has_path2") == 0) {
            out->count = 1;
            out->value[0] = slot >= 0 ? 1.0 : 0.0;
            return 1;
        }
        if (slot < 0) {
            plc_set_error(err, err_size, "missing record path");
            return 0;
        }
        out->count = 1;
        out->value[0] = heap_frame->record_values[child_id][slot];
        return 1;
    }
    if (strcmp(name, "record_shape_equal") == 0) {
        int other_record;
        int ok;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "record_shape_equal needs a frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size,
                "record_shape_equal expects two arguments");
            return 0;
        }
        record_id = (int)args[0];
        other_record = (int)args[1];
        if (!plc_record_valid(heap_frame, record_id, err, err_size)
                || !plc_record_valid(heap_frame, other_record,
                    err, err_size)) {
            return 0;
        }
        ok = heap_frame->record_sizes[record_id]
            == heap_frame->record_sizes[other_record];
        for (i = 0; ok && i < heap_frame->record_sizes[record_id]; ++i) {
            if (plc_record_find_key(heap_frame, other_record,
                    heap_frame->record_keys[record_id][i]) < 0) {
                ok = 0;
            }
        }
        out->count = 1;
        out->value[0] = ok ? 1.0 : 0.0;
        return 1;
    }
    if (strcmp(name, "complex") == 0) {
        if (caller_frame == 0) {
            plc_set_error(err, err_size, "complex needs a caller frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size, "complex expects two arguments");
            return 0;
        }
        complex_id = plc_new_complex(heap_frame, args[0], args[1],
            err, err_size);
        if (complex_id < 0) {
            return 0;
        }
        out->count = 1;
        out->value[0] = (double)complex_id;
        return 1;
    }
    if (strcmp(name, "complex_real") == 0
            || strcmp(name, "complex_imag") == 0) {
        if (caller_frame == 0) {
            plc_set_error(err, err_size, "complex access needs a caller frame");
            return 0;
        }
        if (argc != 1) {
            plc_set_error(err, err_size, "complex access expects one argument");
            return 0;
        }
        complex_id = (int)args[0];
        if (!plc_complex_valid(heap_frame, complex_id, err, err_size)) {
            return 0;
        }
        out->count = 1;
        out->value[0] = strcmp(name, "complex_real") == 0
            ? heap_frame->complex_real[complex_id]
            : heap_frame->complex_imag[complex_id];
        return 1;
    }
    if (strcmp(name, "complex_add") == 0
            || strcmp(name, "complex_sub") == 0
            || strcmp(name, "complex_mul") == 0) {
        double ar;
        double ai;
        double br;
        double bi;
        double real;
        double imag;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "complex operation needs a caller frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size, "complex operation expects two arguments");
            return 0;
        }
        complex_id = (int)args[0];
        other_complex = (int)args[1];
        if (!plc_complex_valid(heap_frame, complex_id, err, err_size)
                || !plc_complex_valid(heap_frame, other_complex,
                    err, err_size)) {
            return 0;
        }
        ar = heap_frame->complex_real[complex_id];
        ai = heap_frame->complex_imag[complex_id];
        br = heap_frame->complex_real[other_complex];
        bi = heap_frame->complex_imag[other_complex];
        if (strcmp(name, "complex_add") == 0) {
            real = ar + br;
            imag = ai + bi;
        } else if (strcmp(name, "complex_sub") == 0) {
            real = ar - br;
            imag = ai - bi;
        } else {
            real = ar * br - ai * bi;
            imag = ar * bi + ai * br;
        }
        complex_id = plc_new_complex(heap_frame, real, imag,
            err, err_size);
        if (complex_id < 0) {
            return 0;
        }
        out->count = 1;
        out->value[0] = (double)complex_id;
        return 1;
    }
    if (strcmp(name, "complex_conj") == 0
            || strcmp(name, "complex_norm2") == 0
            || strcmp(name, "complex_equal") == 0) {
        double real;
        double imag;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "complex operation needs a caller frame");
            return 0;
        }
        if ((strcmp(name, "complex_equal") == 0 && argc != 2)
                || (strcmp(name, "complex_equal") != 0 && argc != 1)) {
            plc_set_error(err, err_size, "bad complex operation argument count");
            return 0;
        }
        complex_id = (int)args[0];
        if (!plc_complex_valid(heap_frame, complex_id, err, err_size)) {
            return 0;
        }
        real = heap_frame->complex_real[complex_id];
        imag = heap_frame->complex_imag[complex_id];
        if (strcmp(name, "complex_conj") == 0) {
            complex_id = plc_new_complex(heap_frame, real, 0.0 - imag,
                err, err_size);
            if (complex_id < 0) {
                return 0;
            }
            out->count = 1;
            out->value[0] = (double)complex_id;
            return 1;
        }
        if (strcmp(name, "complex_norm2") == 0) {
            out->count = 1;
            out->value[0] = real * real + imag * imag;
            return 1;
        }
        other_complex = (int)args[1];
        if (!plc_complex_valid(heap_frame, other_complex, err, err_size)) {
            return 0;
        }
        out->count = 1;
        out->value[0] =
            fabs(real - heap_frame->complex_real[other_complex]) < 0.0000001
            && fabs(imag - heap_frame->complex_imag[other_complex])
                < 0.0000001 ? 1.0 : 0.0;
        return 1;
    }
    if (strcmp(name, "vec3") == 0) {
        if (caller_frame == 0) {
            plc_set_error(err, err_size, "vec3 needs a caller frame");
            return 0;
        }
        if (argc != 3) {
            plc_set_error(err, err_size, "vec3 expects three arguments");
            return 0;
        }
        vec_id = plc_new_vec3(heap_frame, args[0], args[1], args[2],
            err, err_size);
        if (vec_id < 0) {
            return 0;
        }
        out->count = 1;
        out->value[0] = (double)vec_id;
        return 1;
    }
    if (strcmp(name, "vec3_x") == 0 || strcmp(name, "vec3_y") == 0
            || strcmp(name, "vec3_z") == 0) {
        int component;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "vec3 access needs a caller frame");
            return 0;
        }
        if (argc != 1) {
            plc_set_error(err, err_size, "vec3 access expects one argument");
            return 0;
        }
        vec_id = (int)args[0];
        if (!plc_vec3_valid(heap_frame, vec_id, err, err_size)) {
            return 0;
        }
        component = 0;
        if (strcmp(name, "vec3_y") == 0) {
            component = 1;
        } else if (strcmp(name, "vec3_z") == 0) {
            component = 2;
        }
        out->count = 1;
        out->value[0] = heap_frame->vec3_values[vec_id][component];
        return 1;
    }
    if (strcmp(name, "vec3_add") == 0 || strcmp(name, "vec3_sub") == 0
            || strcmp(name, "vec3_dot") == 0
            || strcmp(name, "vec3_cross") == 0) {
        double ax;
        double ay;
        double az;
        double bx;
        double by;
        double bz;
        double x;
        double y;
        double z;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "vec3 operation needs a caller frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size, "vec3 operation expects two arguments");
            return 0;
        }
        vec_id = (int)args[0];
        other_vec = (int)args[1];
        if (!plc_vec3_valid(heap_frame, vec_id, err, err_size)
                || !plc_vec3_valid(heap_frame, other_vec, err, err_size)) {
            return 0;
        }
        ax = heap_frame->vec3_values[vec_id][0];
        ay = heap_frame->vec3_values[vec_id][1];
        az = heap_frame->vec3_values[vec_id][2];
        bx = heap_frame->vec3_values[other_vec][0];
        by = heap_frame->vec3_values[other_vec][1];
        bz = heap_frame->vec3_values[other_vec][2];
        if (strcmp(name, "vec3_dot") == 0) {
            out->count = 1;
            out->value[0] = ax * bx + ay * by + az * bz;
            return 1;
        }
        if (strcmp(name, "vec3_cross") == 0) {
            x = ay * bz - az * by;
            y = az * bx - ax * bz;
            z = ax * by - ay * bx;
        } else if (strcmp(name, "vec3_add") == 0) {
            x = ax + bx;
            y = ay + by;
            z = az + bz;
        } else {
            x = ax - bx;
            y = ay - by;
            z = az - bz;
        }
        vec_id = plc_new_vec3(heap_frame, x, y, z, err, err_size);
        if (vec_id < 0) {
            return 0;
        }
        out->count = 1;
        out->value[0] = (double)vec_id;
        return 1;
    }
    if (strcmp(name, "vec3_scale") == 0
            || strcmp(name, "vec3_len2") == 0
            || strcmp(name, "vec3_normalize") == 0) {
        double x;
        double y;
        double z;
        double len2;
        double len;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "vec3 operation needs a caller frame");
            return 0;
        }
        if ((strcmp(name, "vec3_scale") == 0 && argc != 2)
                || (strcmp(name, "vec3_scale") != 0 && argc != 1)) {
            plc_set_error(err, err_size, "bad vec3 operation argument count");
            return 0;
        }
        vec_id = (int)args[0];
        if (!plc_vec3_valid(heap_frame, vec_id, err, err_size)) {
            return 0;
        }
        x = heap_frame->vec3_values[vec_id][0];
        y = heap_frame->vec3_values[vec_id][1];
        z = heap_frame->vec3_values[vec_id][2];
        len2 = x * x + y * y + z * z;
        if (strcmp(name, "vec3_len2") == 0) {
            out->count = 1;
            out->value[0] = len2;
            return 1;
        }
        if (strcmp(name, "vec3_scale") == 0) {
            x *= args[1];
            y *= args[1];
            z *= args[1];
        } else {
            if (len2 <= 0.0) {
                plc_set_error(err, err_size, "cannot normalize zero vec3");
                return 0;
            }
            len = sqrt(len2);
            x /= len;
            y /= len;
            z /= len;
        }
        vec_id = plc_new_vec3(heap_frame, x, y, z, err, err_size);
        if (vec_id < 0) {
            return 0;
        }
        out->count = 1;
        out->value[0] = (double)vec_id;
        return 1;
    }
    if (strcmp(name, "mat4_identity") == 0
            || strcmp(name, "mat4_translate") == 0
            || strcmp(name, "mat4_scale") == 0
            || strcmp(name, "mat4_rotate_x") == 0
            || strcmp(name, "mat4_rotate_y") == 0
            || strcmp(name, "mat4_rotate_z") == 0) {
        double m[16];
        double angle;
        double s;
        double c;
        int is_rotate;
        int j;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "mat4 operation needs a caller frame");
            return 0;
        }
        is_rotate = strcmp(name, "mat4_rotate_x") == 0
            || strcmp(name, "mat4_rotate_y") == 0
            || strcmp(name, "mat4_rotate_z") == 0;
        if ((strcmp(name, "mat4_identity") == 0 && argc != 0)
                || ((strcmp(name, "mat4_translate") == 0
                    || strcmp(name, "mat4_scale") == 0) && argc != 3)
                || (is_rotate && argc != 1)) {
            plc_set_error(err, err_size, "bad mat4 operation argument count");
            return 0;
        }
        for (j = 0; j < 16; ++j) {
            m[j] = 0.0;
        }
        m[0] = 1.0;
        m[5] = 1.0;
        m[10] = 1.0;
        m[15] = 1.0;
        if (strcmp(name, "mat4_translate") == 0) {
            m[3] = args[0];
            m[7] = args[1];
            m[11] = args[2];
        } else if (strcmp(name, "mat4_scale") == 0) {
            m[0] = args[0];
            m[5] = args[1];
            m[10] = args[2];
        } else if (is_rotate) {
            angle = args[0];
            s = sin(angle);
            c = cos(angle);
            if (strcmp(name, "mat4_rotate_x") == 0) {
                m[5] = c;
                m[6] = 0.0 - s;
                m[9] = s;
                m[10] = c;
            } else if (strcmp(name, "mat4_rotate_y") == 0) {
                m[0] = c;
                m[2] = s;
                m[8] = 0.0 - s;
                m[10] = c;
            } else {
                m[0] = c;
                m[1] = 0.0 - s;
                m[4] = s;
                m[5] = c;
            }
        }
        mat_id = plc_new_mat4(heap_frame, m, err, err_size);
        if (mat_id < 0) {
            return 0;
        }
        out->count = 1;
        out->value[0] = (double)mat_id;
        return 1;
    }
    if (strcmp(name, "mat4_mul") == 0) {
        double m[16];
        int row;
        int col;
        int k;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "mat4_mul needs a caller frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size, "mat4_mul expects two arguments");
            return 0;
        }
        mat_id = (int)args[0];
        other_mat = (int)args[1];
        if (!plc_mat4_valid(heap_frame, mat_id, err, err_size)
                || !plc_mat4_valid(heap_frame, other_mat, err, err_size)) {
            return 0;
        }
        for (row = 0; row < 4; ++row) {
            for (col = 0; col < 4; ++col) {
                double value;

                value = 0.0;
                for (k = 0; k < 4; ++k) {
                    value += heap_frame->mat4_values[mat_id][row * 4 + k]
                        * heap_frame->mat4_values[other_mat][k * 4 + col];
                }
                m[row * 4 + col] = value;
            }
        }
        mat_id = plc_new_mat4(heap_frame, m, err, err_size);
        if (mat_id < 0) {
            return 0;
        }
        out->count = 1;
        out->value[0] = (double)mat_id;
        return 1;
    }
    if (strcmp(name, "mat4_transform_point") == 0) {
        double x;
        double y;
        double z;
        double w;
        double nx;
        double ny;
        double nz;
        const double *m;

        if (caller_frame == 0) {
            plc_set_error(err, err_size,
                "mat4_transform_point needs a caller frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size,
                "mat4_transform_point expects two arguments");
            return 0;
        }
        mat_id = (int)args[0];
        vec_id = (int)args[1];
        if (!plc_mat4_valid(heap_frame, mat_id, err, err_size)
                || !plc_vec3_valid(heap_frame, vec_id, err, err_size)) {
            return 0;
        }
        m = heap_frame->mat4_values[mat_id];
        x = heap_frame->vec3_values[vec_id][0];
        y = heap_frame->vec3_values[vec_id][1];
        z = heap_frame->vec3_values[vec_id][2];
        nx = m[0] * x + m[1] * y + m[2] * z + m[3];
        ny = m[4] * x + m[5] * y + m[6] * z + m[7];
        nz = m[8] * x + m[9] * y + m[10] * z + m[11];
        w = m[12] * x + m[13] * y + m[14] * z + m[15];
        if (fabs(w) > 0.0000001 && fabs(w - 1.0) > 0.0000001) {
            nx /= w;
            ny /= w;
            nz /= w;
        }
        vec_id = plc_new_vec3(heap_frame, nx, ny, nz, err, err_size);
        if (vec_id < 0) {
            return 0;
        }
        out->count = 1;
        out->value[0] = (double)vec_id;
        return 1;
    }
    if (strcmp(name, "perspective_project") == 0) {
        double x;
        double y;
        double z;
        double focal;

        if (caller_frame == 0) {
            plc_set_error(err, err_size,
                "perspective_project needs a caller frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size,
                "perspective_project expects two arguments");
            return 0;
        }
        vec_id = (int)args[0];
        if (!plc_vec3_valid(heap_frame, vec_id, err, err_size)) {
            return 0;
        }
        x = heap_frame->vec3_values[vec_id][0];
        y = heap_frame->vec3_values[vec_id][1];
        z = heap_frame->vec3_values[vec_id][2];
        if (fabs(z) < 0.0000001) {
            plc_set_error(err, err_size, "cannot project vec3 with zero depth");
            return 0;
        }
        focal = args[1];
        vec_id = plc_new_vec3(heap_frame, focal * x / z,
            focal * y / z, z, err, err_size);
        if (vec_id < 0) {
            return 0;
        }
        out->count = 1;
        out->value[0] = (double)vec_id;
        return 1;
    }
    if ((strcmp(name, "list_equal") == 0 || strcmp(name, "set_equal") == 0)
            && argc == 2) {
        int ok;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "list/set equality needs a frame");
            return 0;
        }
        list_id = (int)args[0];
        other_list = (int)args[1];
        if (!plc_list_valid(heap_frame, list_id, err, err_size)
                || !plc_list_valid(heap_frame, other_list, err, err_size)) {
            return 0;
        }
        ok = heap_frame->list_sizes[list_id]
            == heap_frame->list_sizes[other_list];
        if (ok && strcmp(name, "list_equal") == 0) {
            for (i = 0; i < heap_frame->list_sizes[list_id]; ++i) {
                if (fabs(heap_frame->lists[list_id][i]
                        - heap_frame->lists[other_list][i]) > 0.0000001) {
                    ok = 0;
                }
            }
        } else if (ok) {
            for (i = 0; i < heap_frame->list_sizes[list_id]; ++i) {
                if (!plc_list_contains_value(heap_frame, other_list,
                        heap_frame->lists[list_id][i])) {
                    ok = 0;
                }
            }
        }
        out->count = 1;
        out->value[0] = ok ? 1.0 : 0.0;
        return 1;
    }
    if (strcmp(name, "pair_equal") == 0) {
        int ok;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "pair_equal needs a frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size, "pair_equal expects two arguments");
            return 0;
        }
        pair_id = (int)args[0];
        other_list = (int)args[1];
        if (pair_id < 0 || pair_id >= heap_frame->pair_count
                || other_list < 0 || other_list >= heap_frame->pair_count) {
            plc_set_error(err, err_size, "bad pair id");
            return 0;
        }
        ok = fabs(heap_frame->pair_left[pair_id]
                - heap_frame->pair_left[other_list]) < 0.0000001
            && fabs(heap_frame->pair_right[pair_id]
                - heap_frame->pair_right[other_list]) < 0.0000001;
        out->count = 1;
        out->value[0] = ok ? 1.0 : 0.0;
        return 1;
    }
    if (strcmp(name, "record_equal") == 0
            || strcmp(name, "record_merge") == 0) {
        int other_record;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "record algebra needs a frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size, "record algebra expects two arguments");
            return 0;
        }
        record_id = (int)args[0];
        other_record = (int)args[1];
        if (!plc_record_valid(heap_frame, record_id, err, err_size)
                || !plc_record_valid(heap_frame, other_record,
                    err, err_size)) {
            return 0;
        }
        if (strcmp(name, "record_equal") == 0) {
            int ok;

            ok = heap_frame->record_sizes[record_id]
                == heap_frame->record_sizes[other_record];
            for (i = 0; ok && i < heap_frame->record_sizes[record_id]; ++i) {
                int slot;

                slot = plc_record_find_key(heap_frame, other_record,
                    heap_frame->record_keys[record_id][i]);
                if (slot < 0 || fabs(heap_frame->record_values[record_id][i]
                        - heap_frame->record_values[other_record][slot])
                        > 0.0000001) {
                    ok = 0;
                }
            }
            out->count = 1;
            out->value[0] = ok ? 1.0 : 0.0;
            return 1;
        }
        out_list = plc_new_record(heap_frame, err, err_size);
        if (out_list < 0) {
            return 0;
        }
        for (i = 0; i < heap_frame->record_sizes[record_id]; ++i) {
            if (!plc_record_set_value(heap_frame, out_list,
                    heap_frame->record_keys[record_id][i],
                    heap_frame->record_values[record_id][i],
                    err, err_size)) {
                return 0;
            }
        }
        for (i = 0; i < heap_frame->record_sizes[other_record]; ++i) {
            if (!plc_record_set_value(heap_frame, out_list,
                    heap_frame->record_keys[other_record][i],
                    heap_frame->record_values[other_record][i],
                    err, err_size)) {
                return 0;
            }
        }
        out->count = 1;
        out->value[0] = (double)out_list;
        return 1;
    }
    if (strcmp(name, "relation_inverse") == 0
            || strcmp(name, "relation_image") == 0) {
        if (caller_frame == 0) {
            plc_set_error(err, err_size, "relation algebra needs a frame");
            return 0;
        }
        if ((strcmp(name, "relation_inverse") == 0 && argc != 1)
                || (strcmp(name, "relation_image") == 0 && argc != 2)) {
            plc_set_error(err, err_size, "bad relation algebra argument count");
            return 0;
        }
        list_id = (int)args[0];
        if (!plc_list_valid(heap_frame, list_id, err, err_size)) {
            return 0;
        }
        out_list = plc_new_list(heap_frame, err, err_size);
        if (out_list < 0) {
            return 0;
        }
        for (i = 0; i < heap_frame->list_sizes[list_id]; ++i) {
            int relation_pair;

            relation_pair = (int)heap_frame->lists[list_id][i];
            if (relation_pair < 0 || relation_pair >= heap_frame->pair_count) {
                plc_set_error(err, err_size, "bad relation pair");
                return 0;
            }
            if (strcmp(name, "relation_inverse") == 0) {
                if (heap_frame->pair_count >= PLC_MAX_PAIRS) {
                    plc_set_error(err, err_size, "too many pairs");
                    return 0;
                }
                pair_id = heap_frame->pair_count;
                heap_frame->pair_left[pair_id] =
                    heap_frame->pair_right[relation_pair];
                heap_frame->pair_right[pair_id] =
                    heap_frame->pair_left[relation_pair];
                ++heap_frame->pair_count;
                if (!plc_list_append(heap_frame, out_list, (double)pair_id,
                        err, err_size)) {
                    return 0;
                }
            } else if (fabs(heap_frame->pair_left[relation_pair] - args[1])
                    < 0.0000001
                    && !plc_list_contains_value(heap_frame, out_list,
                        heap_frame->pair_right[relation_pair])) {
                if (!plc_list_append(heap_frame, out_list,
                        heap_frame->pair_right[relation_pair],
                        err, err_size)) {
                    return 0;
                }
            }
        }
        out->count = 1;
        out->value[0] = (double)out_list;
        return 1;
    }
    if (strcmp(name, "chess_square") == 0) {
        if (argc != 2) {
            plc_set_error(err, err_size, "chess_square expects two arguments");
            return 0;
        }
        out->count = 1;
        out->value[0] = (double)plc_chess_square_code((int)args[0],
            (int)args[1]);
        return 1;
    }
    if (strcmp(name, "chess_piece") == 0) {
        if (argc != 2) {
            plc_set_error(err, err_size, "chess_piece expects two arguments");
            return 0;
        }
        out->count = 1;
        out->value[0] = args[0] * 10.0 + args[1];
        return 1;
    }
    if (strcmp(name, "chess_piece_kind") == 0
            || strcmp(name, "chess_piece_side") == 0) {
        if (argc != 1) {
            plc_set_error(err, err_size,
                "chess piece access expects one argument");
            return 0;
        }
        out->count = 1;
        out->value[0] = strcmp(name, "chess_piece_kind") == 0
            ? (double)plc_chess_piece_kind_code((int)args[0])
            : (double)plc_chess_piece_side_code((int)args[0]);
        return 1;
    }
    if (strcmp(name, "chess_pawn_promotes") == 0) {
        if (argc != 3) {
            plc_set_error(err, err_size,
                "chess_pawn_promotes expects from, to and side");
            return 0;
        }
        out->count = 1;
        out->value[0] = plc_chess_model_pawn_promotion((int)args[0],
            (int)args[1], (int)args[2]) ? 1.0 : 0.0;
        return 1;
    }
    if (strcmp(name, "chess_en_passant_possible") == 0) {
        if (argc != 5) {
            plc_set_error(err, err_size,
                "chess_en_passant_possible expects from, to, last_from, last_to and side");
            return 0;
        }
        out->count = 1;
        out->value[0] = plc_chess_model_en_passant((int)args[0],
            (int)args[1], (int)args[2], (int)args[3],
            (int)args[4]) ? 1.0 : 0.0;
        return 1;
    }
    if (strcmp(name, "chess_board_new") == 0
            || strcmp(name, "chess_board_place") == 0
            || strcmp(name, "chess_board_piece") == 0
            || strcmp(name, "chess_board_move") == 0
            || strcmp(name, "chess_apply_move") == 0
            || strcmp(name, "chess_legal_move") == 0
            || strcmp(name, "chess_legal_rook_move") == 0) {
        int moving_piece;
        int from_square;
        int to_square;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "chess board needs a frame");
            return 0;
        }
        if (strcmp(name, "chess_board_new") == 0) {
            if (argc != 0) {
                plc_set_error(err, err_size, "chess_board_new expects no arguments");
                return 0;
            }
            record_id = plc_new_record(heap_frame, err, err_size);
            if (record_id < 0) {
                return 0;
            }
            out->count = 1;
            out->value[0] = (double)record_id;
            return 1;
        }
        if (strcmp(name, "chess_board_place") == 0) {
            if (argc != 3) {
                plc_set_error(err, err_size,
                    "chess_board_place expects three arguments");
                return 0;
            }
            record_id = (int)args[0];
            if (!plc_record_set_value(heap_frame, record_id, (int)args[1],
                    args[2], err, err_size)) {
                return 0;
            }
            out->count = 1;
            out->value[0] = (double)record_id;
            return 1;
        }
        if (strcmp(name, "chess_board_move") == 0
                || strcmp(name, "chess_apply_move") == 0
                || strcmp(name, "chess_legal_move") == 0
                || strcmp(name, "chess_legal_rook_move") == 0) {
            int legal;
            int enforce_safety;

            if (argc != 3) {
                plc_set_error(err, err_size,
                    "chess board move expects board, from, to");
                return 0;
            }
            record_id = (int)args[0];
            from_square = (int)args[1];
            to_square = (int)args[2];
            if (!plc_chess_board_get_piece(heap_frame, record_id,
                    from_square, &moving_piece, err, err_size)) {
                return 0;
            }
            enforce_safety = strcmp(name, "chess_apply_move") == 0
                || strcmp(name, "chess_legal_move") == 0;
            if (!plc_chess_model_legal_move(heap_frame, record_id,
                    from_square, to_square, enforce_safety, &legal,
                    err, err_size)) {
                return 0;
            }
            if (strcmp(name, "chess_legal_rook_move") == 0
                    || strcmp(name, "chess_legal_move") == 0) {
                if (strcmp(name, "chess_legal_rook_move") == 0
                        && plc_chess_piece_kind_code(moving_piece) != 4) {
                    legal = 0;
                }
                out->count = 1;
                out->value[0] = legal ? 1.0 : 0.0;
                return 1;
            }
            if (strcmp(name, "chess_apply_move") == 0) {
                if (!plc_chess_model_apply_move(heap_frame, record_id,
                        from_square, to_square, 1, err, err_size)) {
                    return 0;
                }
            } else if (!plc_record_set_value(heap_frame, record_id, from_square,
                    0.0, err, err_size)
                    || !plc_record_set_value(heap_frame, record_id,
                        to_square, (double)moving_piece, err, err_size)) {
                return 0;
            }
            out->count = 1;
            out->value[0] = (double)record_id;
            return 1;
        }
        if (argc != 2) {
            plc_set_error(err, err_size,
                "chess_board_piece expects two arguments");
            return 0;
        }
        record_id = (int)args[0];
        field_key = plc_record_find_key(heap_frame, record_id, (int)args[1]);
        out->count = 1;
        out->value[0] = field_key >= 0
            ? heap_frame->record_values[record_id][field_key] : 0.0;
        return 1;
    }
    if (strcmp(name, "chess_side_in_check") == 0
            || strcmp(name, "chess_checkmate_simple") == 0
            || strcmp(name, "chess_checkmate") == 0
            || strcmp(name, "chess_material_score") == 0
            || strcmp(name, "chess_best_capture_score") == 0
            || strcmp(name, "chess_legal_move_count") == 0
            || strcmp(name, "chess_position_signature") == 0
            || strcmp(name, "chess_fen_signature") == 0
            || strcmp(name, "chess_stalemate") == 0) {
        int side;
        int in_check;
        int score;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "chess check needs a frame");
            return 0;
        }
        if (strcmp(name, "chess_fen_signature") == 0) {
            if (argc != 4) {
                plc_set_error(err, err_size,
                    "chess_fen_signature expects board, side, castling and ep-square");
                return 0;
            }
            record_id = (int)args[0];
            if (!plc_chess_model_fen_signature(heap_frame, record_id,
                    (int)args[1], (int)args[2], (int)args[3],
                    &score, err, err_size)) {
                return 0;
            }
            out->count = 1;
            out->value[0] = (double)score;
            return 1;
        }
        if (strcmp(name, "chess_position_signature") == 0) {
            if (argc != 1) {
                plc_set_error(err, err_size,
                    "chess_position_signature expects board");
                return 0;
            }
            record_id = (int)args[0];
            if (!plc_chess_model_position_signature(heap_frame, record_id,
                    &score, err, err_size)) {
                return 0;
            }
            out->count = 1;
            out->value[0] = (double)score;
            return 1;
        }
        if (argc != 2) {
            plc_set_error(err, err_size,
                "chess check expects board and side");
            return 0;
        }
        record_id = (int)args[0];
        side = (int)args[1];
        if (strcmp(name, "chess_material_score") == 0) {
            if (!plc_chess_model_material_score(heap_frame, record_id,
                    side, &score, err, err_size)) {
                return 0;
            }
            out->count = 1;
            out->value[0] = (double)score;
            return 1;
        }
        if (strcmp(name, "chess_best_capture_score") == 0) {
            if (!plc_chess_model_best_capture_score(heap_frame, record_id,
                    side, &score, err, err_size)) {
                return 0;
            }
            out->count = 1;
            out->value[0] = (double)score;
            return 1;
        }
        if (strcmp(name, "chess_legal_move_count") == 0) {
            if (!plc_chess_model_legal_move_count(heap_frame, record_id,
                    side, &score, err, err_size)) {
                return 0;
            }
            out->count = 1;
            out->value[0] = (double)score;
            return 1;
        }
        if (strcmp(name, "chess_checkmate_simple") == 0
                || strcmp(name, "chess_checkmate") == 0) {
            if (!plc_chess_model_checkmate(heap_frame, record_id, side,
                    &in_check, err, err_size)) {
                return 0;
            }
            out->count = 1;
            out->value[0] = in_check ? 1.0 : 0.0;
            return 1;
        }
        if (strcmp(name, "chess_stalemate") == 0) {
            if (!plc_chess_model_stalemate(heap_frame, record_id, side,
                    &in_check, err, err_size)) {
                return 0;
            }
            out->count = 1;
            out->value[0] = in_check ? 1.0 : 0.0;
            return 1;
        }
        if (!plc_chess_model_side_in_check(heap_frame, record_id, side,
                &in_check, err, err_size)) {
            return 0;
        }
        out->count = 1;
        out->value[0] = in_check ? 1.0 : 0.0;
        return 1;
    }
    if (strcmp(name, "chess_can_castle_path") == 0) {
        int can_castle;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "chess castle check needs a frame");
            return 0;
        }
        if (argc != 4) {
            plc_set_error(err, err_size,
                "chess_can_castle_path expects board, side, king and rook");
            return 0;
        }
        if (!plc_chess_model_can_castle_path(heap_frame, (int)args[0],
                (int)args[1], (int)args[2], (int)args[3],
                &can_castle, err, err_size)) {
            return 0;
        }
        out->count = 1;
        out->value[0] = can_castle ? 1.0 : 0.0;
        return 1;
    }
    if (strcmp(name, "chess_rook_attack_map") == 0
            || strcmp(name, "chess_bishop_attack_map") == 0
            || strcmp(name, "chess_knight_attack_map") == 0
            || strcmp(name, "chess_queen_attack_map") == 0
            || strcmp(name, "chess_king_attack_map") == 0) {
        int kind;

        if (caller_frame == 0) {
            plc_set_error(err, err_size, "chess attack map needs a frame");
            return 0;
        }
        if (argc != 2) {
            plc_set_error(err, err_size,
                "chess attack map expects file and rank");
            return 0;
        }
        kind = 6;
        if (strcmp(name, "chess_rook_attack_map") == 0) {
            kind = 4;
        } else if (strcmp(name, "chess_bishop_attack_map") == 0) {
            kind = 3;
        } else if (strcmp(name, "chess_knight_attack_map") == 0) {
            kind = 2;
        } else if (strcmp(name, "chess_queen_attack_map") == 0) {
            kind = 5;
        }
        list_id = plc_chess_attack_map(heap_frame, kind,
            (int)args[0], (int)args[1], err, err_size);
        if (list_id < 0) {
            return 0;
        }
        out->count = 1;
        out->value[0] = (double)list_id;
        return 1;
    }
    if (strcmp(name, "chess_piece_attacks_square") == 0) {
        int square;

        if (caller_frame == 0) {
            plc_set_error(err, err_size,
                "chess_piece_attacks_square needs a frame");
            return 0;
        }
        if (argc != 4) {
            plc_set_error(err, err_size,
                "chess_piece_attacks_square expects kind, file, rank, square");
            return 0;
        }
        list_id = plc_chess_attack_map(heap_frame, (int)args[0],
            (int)args[1], (int)args[2], err, err_size);
        if (list_id < 0) {
            return 0;
        }
        square = (int)args[3];
        out->count = 1;
        out->value[0] = plc_list_contains_value(heap_frame, list_id,
            (double)square) ? 1.0 : 0.0;
        return 1;
    }
    native_proc = plc_find_native(program, name);
    if (native_proc != 0) {
        PLANKAC_RESULT native_result;

        if (argc != native_proc->argc) {
            sprintf(err, "%s expects %d argument(s), got %d",
                native_proc->name, native_proc->argc, argc);
            return 0;
        }
        memset(&native_result, 0, sizeof(native_result));
        if (!native_proc->fn(native_proc->user_data, args, argc,
                &native_result, err, err_size)) {
            if (err != 0 && err_size > 0 && err[0] == '\0') {
                plc_set_error(err, err_size, "native function failed");
            }
            return 0;
        }
        if (native_result.count != native_proc->results
                || native_result.count < 0
                || native_result.count > PLANKAC_MAX_RESULTS) {
            plc_set_error(err, err_size,
                "native function returned bad result count");
            return 0;
        }
        out->count = native_result.count;
        for (i = 0; i < native_result.count; ++i) {
            out->value[i] = native_result.value[i];
        }
        return 1;
    }
    proc = plc_find_proc(program, name);
    if (proc == 0) {
        sprintf(err, "unknown procedure: %s", name);
        return 0;
    }
    return plc_execute_proc_with_heap(program, proc, args, argc, out,
        depth, heap_frame, err, err_size);
}

void plc_format_value(double value, char *out)
{
    long whole;
    double diff;

    whole = (long)value;
    diff = value - (double)whole;
    if (diff < 0.0) {
        diff = 0.0 - diff;
    }
    if (diff < 0.000001) {
        sprintf(out, "%ld", whole);
    } else {
        sprintf(out, "%.6f", value);
    }
}

void plc_copy_error(char *out, unsigned out_size, const char *text)
{
    if (out_size == 0 || out == 0) {
        return;
    }
    strncpy(out, text, out_size - 1);
    out[out_size - 1] = '\0';
}

void plc_fill_proc_info(const PLC_PROC *proc, PLANKAC_PROC_INFO *info)
{
    int i;

    info->number = proc->number;
    strncpy(info->name, proc->name, sizeof(info->name) - 1);
    info->name[sizeof(info->name) - 1] = '\0';
    info->argc = proc->argc;
    info->results = proc->results;
    info->statements = proc->stmt_count;
    for (i = 0; i < PLANKAC_MAX_ARGS; ++i) {
        info->arg_types[i][0] = '\0';
        if (i < proc->argc) {
            strncpy(info->arg_types[i], proc->arg_types[i],
                PLANKAC_MAX_TYPE_TEXT - 1);
            info->arg_types[i][PLANKAC_MAX_TYPE_TEXT - 1] = '\0';
        }
    }
    for (i = 0; i < PLANKAC_MAX_RESULTS; ++i) {
        info->result_types[i][0] = '\0';
        if (i < proc->results) {
            strncpy(info->result_types[i], proc->result_types[i],
                PLANKAC_MAX_TYPE_TEXT - 1);
            info->result_types[i][PLANKAC_MAX_TYPE_TEXT - 1] = '\0';
        }
    }
}

void plc_fill_native_info(const PLC_NATIVE_PROC *proc,
    PLANKAC_NATIVE_INFO *info)
{
    int i;

    if (proc == 0 || info == 0) {
        return;
    }
    memset(info, 0, sizeof(*info));
    strncpy(info->name, proc->name, sizeof(info->name) - 1);
    info->name[sizeof(info->name) - 1] = '\0';
    info->argc = proc->argc;
    info->results = proc->results;
    for (i = 0; i < PLANKAC_MAX_ARGS; ++i) {
        info->arg_types[i][0] = '\0';
        if (i < proc->argc) {
            strncpy(info->arg_types[i], proc->arg_types[i],
                PLANKAC_MAX_TYPE_TEXT - 1);
            info->arg_types[i][PLANKAC_MAX_TYPE_TEXT - 1] = '\0';
        }
    }
    for (i = 0; i < PLANKAC_MAX_RESULTS; ++i) {
        info->result_types[i][0] = '\0';
        if (i < proc->results) {
            strncpy(info->result_types[i], proc->result_types[i],
                PLANKAC_MAX_TYPE_TEXT - 1);
            info->result_types[i][PLANKAC_MAX_TYPE_TEXT - 1] = '\0';
        }
    }
}

PLANKAC_CONTEXT *plankac_create(void)
{
    PLANKAC_CONTEXT *ctx;

    ctx = (PLANKAC_CONTEXT *)malloc(sizeof(*ctx));
    if (ctx == 0) {
        return 0;
    }
    memset(ctx, 0, sizeof(*ctx));
    return ctx;
}

void plankac_destroy(PLANKAC_CONTEXT *ctx)
{
    if (ctx != 0) {
        free(ctx);
    }
}

int plankac_context_load_sources(PLANKAC_CONTEXT *ctx,
    const char *const *sources, char *err, unsigned err_size)
{
    char local_err[PLC_MAX_LINE];

    if (ctx == 0) {
        plc_copy_error(err, err_size, "missing PlankaC context");
        return PLANKAC_ERR;
    }
    local_err[0] = '\0';
    if (!plc_load_sources(&ctx->program, sources, local_err,
            sizeof(local_err))) {
        ctx->loaded = 0;
        plc_copy_error(err, err_size, local_err);
        return PLANKAC_ERR;
    }
    ctx->loaded = 1;
    plc_copy_error(err, err_size, "");
    return PLANKAC_OK;
}

int plankac_context_load_default(PLANKAC_CONTEXT *ctx,
    char *err, unsigned err_size)
{
    return plankac_context_load_sources(ctx, PLC_SOURCES, err, err_size);
}

int plankac_context_load_bytecode(PLANKAC_CONTEXT *ctx,
    const char *path, char *err, unsigned err_size)
{
    char local_err[PLC_MAX_LINE];

    if (ctx == 0) {
        plc_copy_error(err, err_size, "missing PlankaC context");
        return PLANKAC_ERR;
    }
    if (path == 0 || path[0] == '\0') {
        plc_copy_error(err, err_size, "missing bytecode input path");
        return PLANKAC_ERR;
    }
    local_err[0] = '\0';
    if (!plc_load_bytecode(&ctx->program, path, local_err,
            sizeof(local_err))) {
        ctx->loaded = 0;
        plc_copy_error(err, err_size, local_err);
        return PLANKAC_ERR;
    }
    ctx->loaded = 1;
    plc_copy_error(err, err_size, "");
    return PLANKAC_OK;
}

int plankac_context_load_bytecode_text(PLANKAC_CONTEXT *ctx,
    const char *text, char *err, unsigned err_size)
{
    char local_err[PLC_MAX_LINE];

    if (ctx == 0) {
        plc_copy_error(err, err_size, "missing PlankaC context");
        return PLANKAC_ERR;
    }
    if (text == 0) {
        plc_copy_error(err, err_size, "missing bytecode text");
        return PLANKAC_ERR;
    }
    local_err[0] = '\0';
    if (!plc_load_bytecode_text(&ctx->program, text, local_err,
            sizeof(local_err))) {
        ctx->loaded = 0;
        plc_copy_error(err, err_size, local_err);
        return PLANKAC_ERR;
    }
    ctx->loaded = 1;
    plc_copy_error(err, err_size, "");
    return PLANKAC_OK;
}

int plankac_context_proc_count(PLANKAC_CONTEXT *ctx)
{
    if (ctx == 0 || !ctx->loaded) {
        return 0;
    }
    return ctx->program.proc_count;
}

int plankac_context_get_proc(PLANKAC_CONTEXT *ctx, int index,
    PLANKAC_PROC_INFO *info)
{
    if (ctx == 0 || info == 0 || !ctx->loaded) {
        return PLANKAC_ERR;
    }
    if (index < 0 || index >= ctx->program.proc_count) {
        return PLANKAC_ERR;
    }
    plc_fill_proc_info(&ctx->program.procs[index], info);
    return PLANKAC_OK;
}

int plankac_context_find_proc(PLANKAC_CONTEXT *ctx, const char *name,
    PLANKAC_PROC_INFO *info)
{
    const PLC_PROC *proc;

    if (ctx == 0 || info == 0 || !ctx->loaded) {
        return PLANKAC_ERR;
    }
    proc = plc_find_proc(&ctx->program, name);
    if (proc == 0) {
        return PLANKAC_ERR;
    }
    plc_fill_proc_info(proc, info);
    return PLANKAC_OK;
}

int plankac_context_register_native(PLANKAC_CONTEXT *ctx, const char *name,
    int argc, int results, const char *const *arg_types,
    const char *const *result_types, PLANKAC_NATIVE_FN fn, void *user_data,
    char *err, unsigned err_size)
{
    if (ctx == 0) {
        plc_copy_error(err, err_size, "missing PlankaC context");
        return PLANKAC_ERR;
    }
    if (!plc_register_native(&ctx->program, name, argc, results,
            arg_types, result_types, fn, user_data, err, err_size)) {
        return PLANKAC_ERR;
    }
    return PLANKAC_OK;
}

int plankac_context_native_count(PLANKAC_CONTEXT *ctx)
{
    if (ctx == 0) {
        return 0;
    }
    return ctx->program.native_count;
}

int plankac_context_get_native(PLANKAC_CONTEXT *ctx, int index,
    PLANKAC_NATIVE_INFO *info)
{
    if (ctx == 0 || info == 0) {
        return PLANKAC_ERR;
    }
    if (index < 0 || index >= ctx->program.native_count) {
        return PLANKAC_ERR;
    }
    plc_fill_native_info(&ctx->program.natives[index], info);
    return PLANKAC_OK;
}

int plankac_context_find_native(PLANKAC_CONTEXT *ctx, const char *name,
    PLANKAC_NATIVE_INFO *info)
{
    const PLC_NATIVE_PROC *proc;

    if (ctx == 0 || info == 0) {
        return PLANKAC_ERR;
    }
    proc = plc_find_native(&ctx->program, name);
    if (proc == 0) {
        return PLANKAC_ERR;
    }
    plc_fill_native_info(proc, info);
    return PLANKAC_OK;
}

static int plankac_context_run_proc(PLANKAC_CONTEXT *ctx,
    const PLC_PROC *proc, const double *args, int argc, PLANKAC_RESULT *result,
    char *err, unsigned err_size)
{
    PLC_VALUES out;
    int i;

    if (ctx == 0 || !ctx->loaded) {
        plc_copy_error(err, err_size, "PlankaC context is not loaded");
        return PLANKAC_ERR;
    }
    if (result == 0) {
        plc_copy_error(err, err_size, "missing result storage");
        return PLANKAC_ERR;
    }
    result->count = 0;
    if (proc == 0) {
        plc_copy_error(err, err_size, "unknown procedure");
        return PLANKAC_ERR;
    }
    if (!plc_execute_proc(&ctx->program, proc, args, argc, &out,
            0, err, err_size)) {
        return PLANKAC_ERR;
    }
    result->count = out.count;
    for (i = 0; i < out.count && i < PLANKAC_MAX_RESULTS; ++i) {
        result->value[i] = out.value[i];
    }
    return PLANKAC_OK;
}

static void plankac_fill_public_value(PLANKAC_VALUE *out, double value,
    const char *type_marker)
{
    PLC_TAGGED_VALUE tagged;

    if (out == 0) {
        return;
    }
    memset(out, 0, sizeof(*out));
    plc_tagged_from_double(&tagged, value, type_marker);
    out->tag = tagged.tag;
    out->family = tagged.family;
    out->bits = tagged.bits;
    out->scale = tagged.scale;
    out->raw = tagged.raw;
    out->handle = tagged.handle;
    out->number = tagged.number;
    if (type_marker != 0) {
        strncpy(out->type_text, type_marker, sizeof(out->type_text) - 1);
        out->type_text[sizeof(out->type_text) - 1] = '\0';
    }
}

static int plankac_context_run_proc_typed(PLANKAC_CONTEXT *ctx,
    const PLC_PROC *proc, const double *args, int argc,
    PLANKAC_TYPED_RESULT *result, char *err, unsigned err_size)
{
    PLANKAC_RESULT plain;
    int i;

    if (result == 0) {
        plc_copy_error(err, err_size, "missing typed result storage");
        return PLANKAC_ERR;
    }
    memset(result, 0, sizeof(*result));
    if (!plankac_context_run_proc(ctx, proc, args, argc, &plain,
            err, err_size)) {
        return PLANKAC_ERR;
    }
    result->count = plain.count;
    for (i = 0; i < plain.count && i < PLANKAC_MAX_RESULTS; ++i) {
        plankac_fill_public_value(&result->value[i], plain.value[i],
            i < proc->results ? proc->result_types[i] : "");
    }
    return PLANKAC_OK;
}

int plankac_context_run(PLANKAC_CONTEXT *ctx, const char *name,
    const double *args, int argc, PLANKAC_RESULT *result,
    char *err, unsigned err_size)
{
    const PLC_PROC *proc;

    if (ctx == 0 || !ctx->loaded) {
        plc_copy_error(err, err_size, "PlankaC context is not loaded");
        return PLANKAC_ERR;
    }
    proc = plc_find_proc(&ctx->program, name);
    if (proc == 0) {
        if (err_size > 0 && err != 0) {
            sprintf(err, "unknown procedure: %s", name);
        }
        return PLANKAC_ERR;
    }
    return plankac_context_run_proc(ctx, proc, args, argc, result,
        err, err_size);
}

int plankac_context_run_typed(PLANKAC_CONTEXT *ctx, const char *name,
    const double *args, int argc, PLANKAC_TYPED_RESULT *result,
    char *err, unsigned err_size)
{
    const PLC_PROC *proc;

    if (ctx == 0 || !ctx->loaded) {
        plc_copy_error(err, err_size, "PlankaC context is not loaded");
        return PLANKAC_ERR;
    }
    proc = plc_find_proc(&ctx->program, name);
    if (proc == 0) {
        if (err_size > 0 && err != 0) {
            sprintf(err, "unknown procedure: %s", name);
        }
        return PLANKAC_ERR;
    }
    return plankac_context_run_proc_typed(ctx, proc, args, argc, result,
        err, err_size);
}

int plankac_context_run_number(PLANKAC_CONTEXT *ctx, int number,
    const double *args, int argc, PLANKAC_RESULT *result,
    char *err, unsigned err_size)
{
    const PLC_PROC *proc;
    int i;

    if (ctx == 0 || !ctx->loaded) {
        plc_copy_error(err, err_size, "PlankaC context is not loaded");
        return PLANKAC_ERR;
    }
    proc = 0;
    for (i = 0; i < ctx->program.proc_count; ++i) {
        if (ctx->program.procs[i].number == number) {
            proc = &ctx->program.procs[i];
            break;
        }
    }
    if (proc == 0) {
        if (err_size > 0 && err != 0) {
            sprintf(err, "unknown procedure number: P%d", number);
        }
        return PLANKAC_ERR;
    }
    return plankac_context_run_proc(ctx, proc, args, argc, result,
        err, err_size);
}

int plankac_context_run_number_typed(PLANKAC_CONTEXT *ctx, int number,
    const double *args, int argc, PLANKAC_TYPED_RESULT *result,
    char *err, unsigned err_size)
{
    const PLC_PROC *proc;
    int i;

    if (ctx == 0 || !ctx->loaded) {
        plc_copy_error(err, err_size, "PlankaC context is not loaded");
        return PLANKAC_ERR;
    }
    proc = 0;
    for (i = 0; i < ctx->program.proc_count; ++i) {
        if (ctx->program.procs[i].number == number) {
            proc = &ctx->program.procs[i];
            break;
        }
    }
    if (proc == 0) {
        if (err_size > 0 && err != 0) {
            sprintf(err, "unknown procedure number: P%d", number);
        }
        return PLANKAC_ERR;
    }
    return plankac_context_run_proc_typed(ctx, proc, args, argc, result,
        err, err_size);
}

int plankac_context_summary(PLANKAC_CONTEXT *ctx,
    char *out, unsigned out_size)
{
    if (ctx == 0 || !ctx->loaded) {
        plc_copy_error(out, out_size, "PlankaC context is not loaded");
        return PLANKAC_ERR;
    }
    if (out_size > 0 && out != 0) {
        sprintf(out, "PlankaC OK: %d files, %d procedures",
            ctx->program.source_count, ctx->program.proc_count);
    }
    return PLANKAC_OK;
}

int plankac_context_write_bytecode(PLANKAC_CONTEXT *ctx,
    const char *path, char *err, unsigned err_size)
{
    if (ctx == 0 || !ctx->loaded) {
        plc_copy_error(err, err_size, "PlankaC context is not loaded");
        return PLANKAC_ERR;
    }
    if (path == 0 || path[0] == '\0') {
        plc_copy_error(err, err_size, "missing bytecode output path");
        return PLANKAC_ERR;
    }
    if (!plc_emit_bytecode(&ctx->program, path, err, err_size)) {
        return PLANKAC_ERR;
    }
    plc_copy_error(err, err_size, "");
    return PLANKAC_OK;
}

int plankac_context_write_c_backend(PLANKAC_CONTEXT *ctx,
    const char *path, char *err, unsigned err_size)
{
    if (ctx == 0 || !ctx->loaded) {
        plc_copy_error(err, err_size, "PlankaC context is not loaded");
        return PLANKAC_ERR;
    }
    if (path == 0 || path[0] == '\0') {
        plc_copy_error(err, err_size, "missing C backend output path");
        return PLANKAC_ERR;
    }
    if (!plc_emit_c_backend(&ctx->program, path, err, err_size)) {
        return PLANKAC_ERR;
    }
    plc_copy_error(err, err_size, "");
    return PLANKAC_OK;
}

int plankac_context_write_asm_runtime(PLANKAC_CONTEXT *ctx,
    const char *path, char *err, unsigned err_size)
{
    if (ctx == 0 || !ctx->loaded) {
        plc_copy_error(err, err_size, "PlankaC context is not loaded");
        return PLANKAC_ERR;
    }
    if (path == 0 || path[0] == '\0') {
        plc_copy_error(err, err_size, "missing ASM runtime output path");
        return PLANKAC_ERR;
    }
    if (!plc_emit_asm_runtime(&ctx->program, path, err, err_size)) {
        return PLANKAC_ERR;
    }
    plc_copy_error(err, err_size, "");
    return PLANKAC_OK;
}

int plankac_context_write_asm_image(PLANKAC_CONTEXT *ctx,
    const char *path, char *err, unsigned err_size)
{
    if (ctx == 0 || !ctx->loaded) {
        plc_copy_error(err, err_size, "PlankaC context is not loaded");
        return PLANKAC_ERR;
    }
    if (path == 0 || path[0] == '\0') {
        plc_copy_error(err, err_size, "missing ASM image output path");
        return PLANKAC_ERR;
    }
    if (!plc_emit_asm_image(&ctx->program, path, err, err_size)) {
        return PLANKAC_ERR;
    }
    plc_copy_error(err, err_size, "");
    return PLANKAC_OK;
}

int plankac_context_write_asm8086_runtime(PLANKAC_CONTEXT *ctx,
    const char *path, char *err, unsigned err_size)
{
    if (ctx == 0 || !ctx->loaded) {
        plc_copy_error(err, err_size, "PlankaC context is not loaded");
        return PLANKAC_ERR;
    }
    if (path == 0 || path[0] == '\0') {
        plc_copy_error(err, err_size, "missing 8086 ASM output path");
        return PLANKAC_ERR;
    }
    if (!plc_emit_asm8086_runtime(&ctx->program, path, err, err_size)) {
        return PLANKAC_ERR;
    }
    plc_copy_error(err, err_size, "");
    return PLANKAC_OK;
}

int plankac_context_write_ir(PLANKAC_CONTEXT *ctx,
    const char *path, char *err, unsigned err_size)
{
    if (ctx == 0 || !ctx->loaded) {
        plc_copy_error(err, err_size, "PlankaC context is not loaded");
        return PLANKAC_ERR;
    }
    if (path == 0 || path[0] == '\0') {
        plc_copy_error(err, err_size, "missing IR output path");
        return PLANKAC_ERR;
    }
    if (!plc_emit_ir(&ctx->program, path, err, err_size)) {
        return PLANKAC_ERR;
    }
    plc_copy_error(err, err_size, "");
    return PLANKAC_OK;
}

int plankac_context_write_ast(PLANKAC_CONTEXT *ctx,
    const char *path, char *err, unsigned err_size)
{
    if (ctx == 0 || !ctx->loaded) {
        plc_copy_error(err, err_size, "PlankaC context is not loaded");
        return PLANKAC_ERR;
    }
    if (path == 0 || path[0] == '\0') {
        plc_copy_error(err, err_size, "missing AST output path");
        return PLANKAC_ERR;
    }
    if (!plc_emit_ast(&ctx->program, path, err, err_size)) {
        return PLANKAC_ERR;
    }
    plc_copy_error(err, err_size, "");
    return PLANKAC_OK;
}

int plankac_context_write_evidence(PLANKAC_CONTEXT *ctx,
    const char *path, char *err, unsigned err_size)
{
    if (ctx == 0 || !ctx->loaded) {
        plc_copy_error(err, err_size, "PlankaC context is not loaded");
        return PLANKAC_ERR;
    }
    if (path == 0 || path[0] == '\0') {
        plc_copy_error(err, err_size, "missing evidence output path");
        return PLANKAC_ERR;
    }
    if (!plc_emit_evidence(&ctx->program, path, err, err_size)) {
        return PLANKAC_ERR;
    }
    plc_copy_error(err, err_size, "");
    return PLANKAC_OK;
}

int plankac_context_write_lowering_report(PLANKAC_CONTEXT *ctx,
    const char *path, char *err, unsigned err_size)
{
    if (ctx == 0 || !ctx->loaded) {
        plc_copy_error(err, err_size, "PlankaC context is not loaded");
        return PLANKAC_ERR;
    }
    if (path == 0 || path[0] == '\0') {
        plc_copy_error(err, err_size, "missing lowering output path");
        return PLANKAC_ERR;
    }
    if (!plc_emit_lowering_report(&ctx->program, path, err, err_size)) {
        return PLANKAC_ERR;
    }
    plc_copy_error(err, err_size, "");
    return PLANKAC_OK;
}

int plankac_reload(char *err, unsigned err_size)
{
    char local_err[PLC_MAX_LINE];

    local_err[0] = '\0';
    if (!plc_load_program(&g_plankac_program, local_err, sizeof(local_err))) {
        g_plankac_loaded = 0;
        plc_copy_error(err, err_size, local_err);
        return PLANKAC_ERR;
    }
    g_plankac_loaded = 1;
    plc_copy_error(err, err_size, "");
    return PLANKAC_OK;
}

int plankac_load(char *err, unsigned err_size)
{
    if (g_plankac_loaded) {
        plc_copy_error(err, err_size, "");
        return PLANKAC_OK;
    }
    return plankac_reload(err, err_size);
}

int plankac_compile_summary(char *out, unsigned out_size)
{
    char err[PLC_MAX_LINE];

    err[0] = '\0';
    if (!plankac_reload(err, sizeof(err))) {
        plc_copy_error(out, out_size, err);
        return PLANKAC_ERR;
    }
    if (out_size > 0 && out != 0) {
        sprintf(out, "PlankaC OK: %d files, %d procedures",
            g_plankac_program.source_count, g_plankac_program.proc_count);
    }
    return PLANKAC_OK;
}

int plankac_proc_count(void)
{
    char err[PLC_MAX_LINE];

    if (!plankac_load(err, sizeof(err))) {
        return 0;
    }
    return g_plankac_program.proc_count;
}

int plankac_get_proc(int index, PLANKAC_PROC_INFO *info)
{
    char err[PLC_MAX_LINE];

    if (info == 0) {
        return PLANKAC_ERR;
    }
    if (!plankac_load(err, sizeof(err))) {
        return PLANKAC_ERR;
    }
    if (index < 0 || index >= g_plankac_program.proc_count) {
        return PLANKAC_ERR;
    }
    plc_fill_proc_info(&g_plankac_program.procs[index], info);
    return PLANKAC_OK;
}

int plankac_find_proc(const char *name, PLANKAC_PROC_INFO *info)
{
    const PLC_PROC *proc;
    char err[PLC_MAX_LINE];

    if (info == 0) {
        return PLANKAC_ERR;
    }
    if (!plankac_load(err, sizeof(err))) {
        return PLANKAC_ERR;
    }
    proc = plc_find_proc(&g_plankac_program, name);
    if (proc == 0) {
        return PLANKAC_ERR;
    }
    plc_fill_proc_info(proc, info);
    return PLANKAC_OK;
}

int plankac_register_native(const char *name, int argc, int results,
    const char *const *arg_types, const char *const *result_types,
    PLANKAC_NATIVE_FN fn, void *user_data, char *err, unsigned err_size)
{
    if (!plc_register_native(&g_plankac_program, name, argc, results,
            arg_types, result_types, fn, user_data, err, err_size)) {
        return PLANKAC_ERR;
    }
    return PLANKAC_OK;
}

int plankac_native_count(void)
{
    return g_plankac_program.native_count;
}

int plankac_get_native(int index, PLANKAC_NATIVE_INFO *info)
{
    if (info == 0) {
        return PLANKAC_ERR;
    }
    if (index < 0 || index >= g_plankac_program.native_count) {
        return PLANKAC_ERR;
    }
    plc_fill_native_info(&g_plankac_program.natives[index], info);
    return PLANKAC_OK;
}

int plankac_find_native(const char *name, PLANKAC_NATIVE_INFO *info)
{
    const PLC_NATIVE_PROC *proc;

    if (info == 0) {
        return PLANKAC_ERR;
    }
    proc = plc_find_native(&g_plankac_program, name);
    if (proc == 0) {
        return PLANKAC_ERR;
    }
    plc_fill_native_info(proc, info);
    return PLANKAC_OK;
}

static int plankac_run_proc(const PLC_PROC *proc, const double *args, int argc,
    PLANKAC_RESULT *result, char *err, unsigned err_size)
{
    PLC_VALUES out;
    int i;

    if (result == 0) {
        plc_copy_error(err, err_size, "missing result storage");
        return PLANKAC_ERR;
    }
    result->count = 0;
    if (!plankac_load(err, err_size)) {
        return PLANKAC_ERR;
    }
    if (proc == 0) {
        plc_copy_error(err, err_size, "unknown procedure");
        return PLANKAC_ERR;
    }
    if (!plc_execute_proc(&g_plankac_program, proc, args, argc, &out,
            0, err, err_size)) {
        return PLANKAC_ERR;
    }
    result->count = out.count;
    for (i = 0; i < out.count && i < PLANKAC_MAX_RESULTS; ++i) {
        result->value[i] = out.value[i];
    }
    return PLANKAC_OK;
}

static int plankac_run_proc_typed(const PLC_PROC *proc, const double *args,
    int argc, PLANKAC_TYPED_RESULT *result, char *err, unsigned err_size)
{
    PLANKAC_RESULT plain;
    int i;

    if (result == 0) {
        plc_copy_error(err, err_size, "missing typed result storage");
        return PLANKAC_ERR;
    }
    memset(result, 0, sizeof(*result));
    if (!plankac_run_proc(proc, args, argc, &plain, err, err_size)) {
        return PLANKAC_ERR;
    }
    result->count = plain.count;
    for (i = 0; i < plain.count && i < PLANKAC_MAX_RESULTS; ++i) {
        plankac_fill_public_value(&result->value[i], plain.value[i],
            i < proc->results ? proc->result_types[i] : "");
    }
    return PLANKAC_OK;
}

int plankac_run(const char *name, const double *args, int argc,
    PLANKAC_RESULT *result, char *err, unsigned err_size)
{
    const PLC_PROC *proc;

    if (!plankac_load(err, err_size)) {
        return PLANKAC_ERR;
    }
    proc = plc_find_proc(&g_plankac_program, name);
    if (proc == 0) {
        if (err_size > 0 && err != 0) {
            sprintf(err, "unknown procedure: %s", name);
        }
        return PLANKAC_ERR;
    }
    return plankac_run_proc(proc, args, argc, result, err, err_size);
}

int plankac_run_typed(const char *name, const double *args, int argc,
    PLANKAC_TYPED_RESULT *result, char *err, unsigned err_size)
{
    const PLC_PROC *proc;

    if (!plankac_load(err, err_size)) {
        return PLANKAC_ERR;
    }
    proc = plc_find_proc(&g_plankac_program, name);
    if (proc == 0) {
        if (err_size > 0 && err != 0) {
            sprintf(err, "unknown procedure: %s", name);
        }
        return PLANKAC_ERR;
    }
    return plankac_run_proc_typed(proc, args, argc, result, err, err_size);
}

int plankac_run_number(int number, const double *args, int argc,
    PLANKAC_RESULT *result, char *err, unsigned err_size)
{
    const PLC_PROC *proc;
    int i;

    if (!plankac_load(err, err_size)) {
        return PLANKAC_ERR;
    }
    proc = 0;
    for (i = 0; i < g_plankac_program.proc_count; ++i) {
        if (g_plankac_program.procs[i].number == number) {
            proc = &g_plankac_program.procs[i];
            break;
        }
    }
    if (proc == 0) {
        if (err_size > 0 && err != 0) {
            sprintf(err, "unknown procedure number: P%d", number);
        }
        return PLANKAC_ERR;
    }
    return plankac_run_proc(proc, args, argc, result, err, err_size);
}

int plankac_run_number_typed(int number, const double *args, int argc,
    PLANKAC_TYPED_RESULT *result, char *err, unsigned err_size)
{
    const PLC_PROC *proc;
    int i;

    if (!plankac_load(err, err_size)) {
        return PLANKAC_ERR;
    }
    proc = 0;
    for (i = 0; i < g_plankac_program.proc_count; ++i) {
        if (g_plankac_program.procs[i].number == number) {
            proc = &g_plankac_program.procs[i];
            break;
        }
    }
    if (proc == 0) {
        if (err_size > 0 && err != 0) {
            sprintf(err, "unknown procedure number: P%d", number);
        }
        return PLANKAC_ERR;
    }
    return plankac_run_proc_typed(proc, args, argc, result, err, err_size);
}

int plankac_write_bytecode(const char *path, char *err, unsigned err_size)
{
    if (!plankac_load(err, err_size)) {
        return PLANKAC_ERR;
    }
    if (!plc_emit_bytecode(&g_plankac_program, path, err, err_size)) {
        return PLANKAC_ERR;
    }
    plc_copy_error(err, err_size, "");
    return PLANKAC_OK;
}

int plankac_write_c_backend(const char *path, char *err, unsigned err_size)
{
    if (!plankac_load(err, err_size)) {
        return PLANKAC_ERR;
    }
    if (!plc_emit_c_backend(&g_plankac_program, path, err, err_size)) {
        return PLANKAC_ERR;
    }
    plc_copy_error(err, err_size, "");
    return PLANKAC_OK;
}

int plankac_write_asm_runtime(const char *path, char *err, unsigned err_size)
{
    if (!plankac_load(err, err_size)) {
        return PLANKAC_ERR;
    }
    if (!plc_emit_asm_runtime(&g_plankac_program, path, err, err_size)) {
        return PLANKAC_ERR;
    }
    plc_copy_error(err, err_size, "");
    return PLANKAC_OK;
}

int plankac_write_asm_image(const char *path, char *err, unsigned err_size)
{
    if (!plankac_load(err, err_size)) {
        return PLANKAC_ERR;
    }
    if (!plc_emit_asm_image(&g_plankac_program, path, err, err_size)) {
        return PLANKAC_ERR;
    }
    plc_copy_error(err, err_size, "");
    return PLANKAC_OK;
}

int plankac_write_asm8086_runtime(const char *path,
    char *err, unsigned err_size)
{
    if (!plankac_load(err, err_size)) {
        return PLANKAC_ERR;
    }
    if (!plc_emit_asm8086_runtime(&g_plankac_program, path, err, err_size)) {
        return PLANKAC_ERR;
    }
    plc_copy_error(err, err_size, "");
    return PLANKAC_OK;
}

int plankac_write_ir(const char *path, char *err, unsigned err_size)
{
    if (!plankac_load(err, err_size)) {
        return PLANKAC_ERR;
    }
    if (!plc_emit_ir(&g_plankac_program, path, err, err_size)) {
        return PLANKAC_ERR;
    }
    plc_copy_error(err, err_size, "");
    return PLANKAC_OK;
}

int plankac_write_ast(const char *path, char *err, unsigned err_size)
{
    if (!plankac_load(err, err_size)) {
        return PLANKAC_ERR;
    }
    if (!plc_emit_ast(&g_plankac_program, path, err, err_size)) {
        return PLANKAC_ERR;
    }
    plc_copy_error(err, err_size, "");
    return PLANKAC_OK;
}

int plankac_write_evidence(const char *path, char *err, unsigned err_size)
{
    if (!plankac_load(err, err_size)) {
        return PLANKAC_ERR;
    }
    if (!plc_emit_evidence(&g_plankac_program, path, err, err_size)) {
        return PLANKAC_ERR;
    }
    plc_copy_error(err, err_size, "");
    return PLANKAC_OK;
}

int plankac_write_lowering_report(const char *path,
    char *err, unsigned err_size)
{
    if (!plankac_load(err, err_size)) {
        return PLANKAC_ERR;
    }
    if (!plc_emit_lowering_report(&g_plankac_program, path,
            err, err_size)) {
        return PLANKAC_ERR;
    }
    plc_copy_error(err, err_size, "");
    return PLANKAC_OK;
}

void plankac_format(double value, char *out, unsigned out_size)
{
    char local[64];

    plc_format_value(value, local);
    plc_copy_error(out, out_size, local);
}
