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
    "examples/session_basic.plk",
    "examples/session_guarded.plk",
    "examples/session_memory.plk",
    "examples/session_scientific.plk",
    "tests/calculator_self_check.plk",
    0
};

PLC_PROGRAM g_plankac_program;
int g_plankac_loaded = 0;

static int plc_new_list(PLC_FRAME *frame, char *err, unsigned err_size)
{
    int list_id;

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

    for (i = 0; i < frame->list_sizes[list_id]; ++i) {
        if (fabs(frame->lists[list_id][i] - value) < 0.0000001) {
            return 1;
        }
    }
    return 0;
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

    if (plc_line_starts_with(stmt->text, "ASSERT")) {
        const char *expr;

        expr = plc_skip_space(plc_skip_space(stmt->text) + 6);
        if (!plc_eval_expr_text(program, frame, depth, expr,
                &guard, err, err_size)) {
            return 0;
        }
        if (guard == 0.0) {
            plc_set_error(err, err_size, "assertion failed");
            return 0;
        }
        return 1;
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

int plc_execute_proc(const PLC_PROGRAM *program, const PLC_PROC *proc,
    const double *args, int argc, PLC_VALUES *out, int depth,
    char *err, unsigned err_size)
{
    PLC_FRAME frame;
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
    memset(&frame, 0, sizeof(frame));
    for (i = 0; i < argc && i < PLC_MAX_VARS; ++i) {
        frame.v[i] = args[i];
    }
    for (i = 0; i < proc->stmt_count; ++i) {
        if (!plc_run_statement(program, &frame, proc, &proc->stmts[i],
                depth, err, err_size)) {
            if (err != 0 && err[0] != '\0') {
                char prefix[160];

                sprintf(prefix, "P%d %s line %d: ",
                    proc->number, proc->name,
                    proc->stmts[i].line_no);
                plc_prefix_error(err, err_size, prefix);
            }
            return 0;
        }
    }
    out->count = proc->results;
    if (out->count > PLC_MAX_RESULTS) {
        plc_set_error(err, err_size, "too many result values");
        return 0;
    }
    for (i = 0; i < out->count; ++i) {
        out->value[i] = frame.r[i];
    }
    return 1;
}

int plc_call_proc(const PLC_PROGRAM *program, PLC_FRAME *caller_frame,
    const char *name, const double *args, int argc, PLC_VALUES *out, int depth,
    char *err, unsigned err_size)
{
    const PLC_PROC *proc;
    int list_id;
    int index;
    int other_list;
    int out_list;
    int pair_id;
    int i;

    if (strcmp(name, "sqrt") == 0) {
        if (argc != 1) {
            plc_set_error(err, err_size, "sqrt expects one argument");
            return 0;
        }
        out->count = 1;
        out->value[0] = sqrt(args[0]);
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
        if (caller_frame->list_count >= PLC_MAX_LISTS) {
            plc_set_error(err, err_size, "too many lists");
            return 0;
        }
        list_id = plc_new_list(caller_frame, err, err_size);
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
        if (!plc_list_append(caller_frame, list_id, args[1],
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
        if (!plc_list_valid(caller_frame, list_id, err, err_size)) {
            return 0;
        }
        out->count = 1;
        out->value[0] = (double)caller_frame->list_sizes[list_id];
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
        if (list_id < 0 || list_id >= caller_frame->list_count
                || index < 0 || index >= caller_frame->list_sizes[list_id]) {
            plc_set_error(err, err_size, "bad list access");
            return 0;
        }
        out->count = 1;
        out->value[0] = caller_frame->lists[list_id][index];
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
        if (!plc_list_valid(caller_frame, list_id, err, err_size)) {
            return 0;
        }
        if (caller_frame->list_sizes[list_id] <= 0) {
            plc_set_error(err, err_size, "empty list");
            return 0;
        }
        if (strcmp(name, "list_last") == 0) {
            value = caller_frame->lists[list_id]
                [caller_frame->list_sizes[list_id] - 1];
        } else {
            value = caller_frame->lists[list_id][0];
        }
        if (strcmp(name, "list_min") == 0 || strcmp(name, "list_max") == 0) {
            for (i = 1; i < caller_frame->list_sizes[list_id]; ++i) {
                if (strcmp(name, "list_min") == 0
                        && caller_frame->lists[list_id][i] < value) {
                    value = caller_frame->lists[list_id][i];
                }
                if (strcmp(name, "list_max") == 0
                        && caller_frame->lists[list_id][i] > value) {
                    value = caller_frame->lists[list_id][i];
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
        if (!plc_list_valid(caller_frame, list_id, err, err_size)
                || !plc_list_valid(caller_frame, other_list, err, err_size)) {
            return 0;
        }
        out_list = plc_new_list(caller_frame, err, err_size);
        if (out_list < 0) {
            return 0;
        }
        for (i = 0; i < caller_frame->list_sizes[list_id]; ++i) {
            if (!plc_list_append(caller_frame, out_list,
                    caller_frame->lists[list_id][i], err, err_size)) {
                return 0;
            }
        }
        for (i = 0; i < caller_frame->list_sizes[other_list]; ++i) {
            if (!plc_list_append(caller_frame, out_list,
                    caller_frame->lists[other_list][i], err, err_size)) {
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
        if (caller_frame->pair_count >= PLC_MAX_PAIRS) {
            plc_set_error(err, err_size, "too many pairs");
            return 0;
        }
        pair_id = caller_frame->pair_count;
        caller_frame->pair_left[pair_id] = args[0];
        caller_frame->pair_right[pair_id] = args[1];
        ++caller_frame->pair_count;
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
        if (pair_id < 0 || pair_id >= caller_frame->pair_count) {
            plc_set_error(err, err_size, "bad pair id");
            return 0;
        }
        out->count = 1;
        out->value[0] = strcmp(name, "pair_left") == 0
            ? caller_frame->pair_left[pair_id]
            : caller_frame->pair_right[pair_id];
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
        if (!plc_list_valid(caller_frame, list_id, err, err_size)) {
            return 0;
        }
        count = 0;
        for (i = 0; i < caller_frame->list_sizes[list_id]; ++i) {
            pair_id = (int)caller_frame->lists[list_id][i];
            if (pair_id >= 0 && pair_id < caller_frame->pair_count
                    && fabs(caller_frame->pair_left[pair_id] - args[1])
                        < 0.0000001) {
                ++count;
            }
        }
        out->count = 1;
        out->value[0] = (double)count;
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
        list_id = plc_new_list(caller_frame, err, err_size);
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
        if (!plc_list_valid(caller_frame, list_id, err, err_size)) {
            return 0;
        }
        if (!plc_list_contains_value(caller_frame, list_id, args[1])) {
            if (!plc_list_append(caller_frame, list_id, args[1],
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
        if (!plc_list_valid(caller_frame, list_id, err, err_size)) {
            return 0;
        }
        out->count = 1;
        out->value[0] = strcmp(name, "set_size") == 0
            ? (double)caller_frame->list_sizes[list_id]
            : (double)plc_list_contains_value(caller_frame, list_id, args[1]);
        return 1;
    }
    if (strcmp(name, "set_union") == 0
            || strcmp(name, "set_intersection") == 0) {
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
        if (!plc_list_valid(caller_frame, list_id, err, err_size)
                || !plc_list_valid(caller_frame, other_list, err, err_size)) {
            return 0;
        }
        out_list = plc_new_list(caller_frame, err, err_size);
        if (out_list < 0) {
            return 0;
        }
        for (i = 0; i < caller_frame->list_sizes[list_id]; ++i) {
            double value;

            value = caller_frame->lists[list_id][i];
            if (strcmp(name, "set_union") == 0
                    || plc_list_contains_value(caller_frame,
                        other_list, value)) {
                if (!plc_list_append(caller_frame, out_list, value,
                        err, err_size)) {
                    return 0;
                }
            }
        }
        if (strcmp(name, "set_union") == 0) {
            for (i = 0; i < caller_frame->list_sizes[other_list]; ++i) {
                double value;

                value = caller_frame->lists[other_list][i];
                if (!plc_list_contains_value(caller_frame, out_list, value)) {
                    if (!plc_list_append(caller_frame, out_list, value,
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
    proc = plc_find_proc(program, name);
    if (proc == 0) {
        sprintf(err, "unknown procedure: %s", name);
        return 0;
    }
    return plc_execute_proc(program, proc, args, argc, out,
        depth, err, err_size);
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

int plankac_context_run(PLANKAC_CONTEXT *ctx, const char *name,
    const double *args, int argc, PLANKAC_RESULT *result,
    char *err, unsigned err_size)
{
    const PLC_PROC *proc;
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
    proc = plc_find_proc(&ctx->program, name);
    if (proc == 0) {
        if (err_size > 0 && err != 0) {
            sprintf(err, "unknown procedure: %s", name);
        }
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

int plankac_run(const char *name, const double *args, int argc,
    PLANKAC_RESULT *result, char *err, unsigned err_size)
{
    const PLC_PROC *proc;
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
    proc = plc_find_proc(&g_plankac_program, name);
    if (proc == 0) {
        if (err_size > 0 && err != 0) {
            sprintf(err, "unknown procedure: %s", name);
        }
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

void plankac_format(double value, char *out, unsigned out_size)
{
    char local[64];

    plc_format_value(value, local);
    plc_copy_error(out, out_size, local);
}

