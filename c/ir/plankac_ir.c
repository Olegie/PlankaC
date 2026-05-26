#include "plankac_internal.h"

static int plc_ir_first_type_family(const char *text)
{
    PLC_TYPE_SPEC spec;
    const char *p;
    const char *end;
    char marker[PLC_MAX_TYPE_TEXT];
    char err[PLC_MAX_LINE];

    p = strstr(text, "[:");
    if (p == 0) {
        return PLC_TYPE_FAMILY_UNKNOWN;
    }
    end = strchr(p, ']');
    if (end == 0) {
        return PLC_TYPE_FAMILY_UNKNOWN;
    }
    plc_copy_range(marker, sizeof(marker), p, end + 1);
    err[0] = '\0';
    if (!plc_parse_type_marker_text(marker, &spec, err, sizeof(err))) {
        return PLC_TYPE_FAMILY_UNKNOWN;
    }
    return spec.family;
}

static int plc_ir_count_commas(const char *text)
{
    int depth;
    int count;
    int seen;

    depth = 0;
    count = 0;
    seen = 0;
    while (*text != '\0') {
        if (*text == '(') {
            ++depth;
        } else if (*text == ')') {
            --depth;
        } else if (*text == ',' && depth == 0) {
            ++count;
        } else if (!isspace((unsigned char)*text)) {
            seen = 1;
        }
        ++text;
    }
    if (!seen) {
        return 0;
    }
    return count + 1;
}

static void plc_ir_trim_copy(char *out, unsigned out_size, const char *text)
{
    strncpy(out, text, out_size - 1);
    out[out_size - 1] = '\0';
    plc_trim_in_place(out);
}

static void plc_ir_lowering_name(const char *text, char *out,
    unsigned out_size)
{
    const char *name;

    name = "scalar";
    if (strstr(text, "list_") != 0 || strstr(text, "set_") != 0
            || strstr(text, "pair") != 0 || strstr(text, "record_") != 0
            || strstr(text, "relation_") != 0 || strstr(text, "complex") != 0
            || strstr(text, "vec3") != 0 || strstr(text, "mat4") != 0
            || strstr(text, "chess_") != 0
            || plc_line_starts_with(text, "SELECT")
            || plc_line_starts_with(text, "EXISTS")
            || plc_line_starts_with(text, "FORALL")
            || plc_line_starts_with(text, "COUNT")) {
        name = "compound";
    } else if (strstr(text, "(") != 0 && strstr(text, ")") != 0) {
        name = "call";
    }
    strncpy(out, name, out_size - 1);
    out[out_size - 1] = '\0';
}

static void plc_ir_set_call_shape(const PLC_PROGRAM *program,
    PLC_IR_STMT *stmt)
{
    char args[PLC_MAX_LINE];
    const PLC_PROC *proc;
    const PLC_NATIVE_PROC *native_proc;

    if (!plc_is_top_call(stmt->value, stmt->callee, sizeof(stmt->callee),
            args, sizeof(args))) {
        stmt->callee[0] = '\0';
        stmt->argc = 0;
        return;
    }
    stmt->argc = plc_ir_count_commas(args);
    proc = plc_find_proc(program, stmt->callee);
    if (proc != 0) {
        stmt->argc = proc->argc;
        stmt->results = proc->results;
        if (proc->results > 0) {
            stmt->value_family = plc_ir_first_type_family(
                proc->result_types[0]);
        }
        return;
    }
    native_proc = plc_find_native(program, stmt->callee);
    if (native_proc != 0) {
        stmt->argc = native_proc->argc;
        stmt->results = native_proc->results;
        if (native_proc->results > 0) {
            stmt->value_family = plc_ir_first_type_family(
                native_proc->result_types[0]);
        }
    }
}

static int plc_ir_build_statement(const PLC_PROGRAM *program,
    const PLC_PROC *proc, const PLC_STMT *source, PLC_IR_STMT *stmt,
    char *err, unsigned err_size)
{
    char parts[5][PLC_MAX_LINE];
    char text[PLC_MAX_LINE];
    int part_count;

    (void)err;
    (void)err_size;
    memset(stmt, 0, sizeof(*stmt));
    stmt->proc_number = proc->number;
    strncpy(stmt->proc_name, proc->name, sizeof(stmt->proc_name) - 1);
    stmt->proc_name[sizeof(stmt->proc_name) - 1] = '\0';
    stmt->source_line = source->line_no;
    stmt->guard_family = PLC_TYPE_FAMILY_UNKNOWN;
    stmt->value_family = PLC_TYPE_FAMILY_UNKNOWN;
    stmt->target_family = PLC_TYPE_FAMILY_UNKNOWN;
    plc_ir_trim_copy(text, sizeof(text), source->text);

    if (plc_line_starts_with(text, "ASSERT")) {
        stmt->op = PLC_IR_OP_ASSERT;
        plc_ir_trim_copy(stmt->guard, sizeof(stmt->guard), text + 6);
        stmt->guard_family = PLC_TYPE_FAMILY_BOOLEAN;
        strcpy(stmt->lowering, "contract");
        return 1;
    }
    if (plc_line_starts_with(text, "REQUIRE")) {
        stmt->op = PLC_IR_OP_REQUIRE;
        plc_ir_trim_copy(stmt->guard, sizeof(stmt->guard), text + 7);
        stmt->guard_family = PLC_TYPE_FAMILY_BOOLEAN;
        strcpy(stmt->lowering, "contract");
        return 1;
    }
    if (plc_line_starts_with(text, "ENSURE")) {
        stmt->op = PLC_IR_OP_ENSURE;
        plc_ir_trim_copy(stmt->guard, sizeof(stmt->guard), text + 6);
        stmt->guard_family = PLC_TYPE_FAMILY_BOOLEAN;
        strcpy(stmt->lowering, "contract");
        return 1;
    }
    if (plc_line_starts_with(text, "STOPIF")) {
        stmt->op = PLC_IR_OP_STOPIF;
        plc_ir_trim_copy(stmt->guard, sizeof(stmt->guard), text + 6);
        stmt->guard_family = PLC_TYPE_FAMILY_BOOLEAN;
        strcpy(stmt->lowering, "control");
        return 1;
    }
    if (plc_line_starts_with(text, "CONST")) {
        const char *eq;

        stmt->op = PLC_IR_OP_CONST;
        eq = strchr(text, '=');
        if (eq != 0) {
            plc_copy_range(stmt->target, sizeof(stmt->target),
                text + 5, eq);
            plc_trim_in_place(stmt->target);
            plc_ir_trim_copy(stmt->value, sizeof(stmt->value), eq + 1);
        }
        stmt->value_family = plc_ir_first_type_family(stmt->value);
        stmt->target_family = plc_ir_first_type_family(stmt->target);
        strcpy(stmt->lowering, "constant");
        return 1;
    }

    if (!plc_split_arrows(text, parts, &part_count)) {
        stmt->op = PLC_IR_OP_EVAL;
        plc_ir_trim_copy(stmt->value, sizeof(stmt->value), text);
        stmt->value_family = plc_ir_first_type_family(stmt->value);
        plc_ir_lowering_name(stmt->value, stmt->lowering,
            sizeof(stmt->lowering));
        return 1;
    }
    if (part_count == 2) {
        plc_ir_trim_copy(stmt->value, sizeof(stmt->value), parts[0]);
        plc_ir_trim_copy(stmt->target, sizeof(stmt->target), parts[1]);
        stmt->op = PLC_IR_OP_EVAL;
    } else {
        plc_ir_trim_copy(stmt->guard, sizeof(stmt->guard), parts[0]);
        plc_ir_trim_copy(stmt->value, sizeof(stmt->value), parts[1]);
        plc_ir_trim_copy(stmt->target, sizeof(stmt->target), parts[2]);
        stmt->op = plc_line_starts_with(parts[0], "LOOP")
            ? PLC_IR_OP_LOOP : PLC_IR_OP_GUARD_EVAL;
        stmt->guard_family = PLC_TYPE_FAMILY_BOOLEAN;
    }
    stmt->value_family = plc_ir_first_type_family(stmt->value);
    stmt->target_family = plc_ir_first_type_family(stmt->target);
    stmt->results = plc_ir_count_commas(stmt->target);
    plc_ir_set_call_shape(program, stmt);
    if (stmt->callee[0] != '\0') {
        if (stmt->op == PLC_IR_OP_GUARD_EVAL) {
            stmt->op = PLC_IR_OP_GUARD_CALL;
        } else if (stmt->op != PLC_IR_OP_LOOP) {
            stmt->op = PLC_IR_OP_CALL;
        }
    }
    plc_ir_lowering_name(stmt->value, stmt->lowering,
        sizeof(stmt->lowering));
    return 1;
}

int plc_ir_build_program(const PLC_PROGRAM *program, PLC_IR_PROGRAM *ir,
    char *err, unsigned err_size)
{
    int i;
    int s;

    if (program == 0 || ir == 0) {
        plc_set_error(err, err_size, "missing IR input");
        return 0;
    }
    memset(ir, 0, sizeof(*ir));
    ir->proc_count = program->proc_count;
    ir->source_count = program->source_count;
    for (i = 0; i < program->proc_count; ++i) {
        for (s = 0; s < program->procs[i].stmt_count; ++s) {
            if (ir->stmt_count >= PLC_IR_MAX_STMTS) {
                plc_set_error(err, err_size, "too many IR statements");
                return 0;
            }
            if (!plc_ir_build_statement(program, &program->procs[i],
                    &program->procs[i].stmts[s],
                    &ir->stmts[ir->stmt_count], err, err_size)) {
                return 0;
            }
            ++ir->stmt_count;
        }
    }
    return 1;
}

int plc_ir_validate_program(const PLC_IR_PROGRAM *ir,
    char *err, unsigned err_size)
{
    int i;

    if (ir == 0) {
        plc_set_error(err, err_size, "missing IR program");
        return 0;
    }
    for (i = 0; i < ir->stmt_count; ++i) {
        const PLC_IR_STMT *stmt;

        stmt = &ir->stmts[i];
        if ((stmt->op == PLC_IR_OP_CALL
                || stmt->op == PLC_IR_OP_GUARD_CALL)
                && stmt->callee[0] == '\0') {
            plc_set_error(err, err_size, "IR call without callee");
            return 0;
        }
        if ((stmt->op == PLC_IR_OP_EVAL || stmt->op == PLC_IR_OP_CALL
                || stmt->op == PLC_IR_OP_GUARD_EVAL
                || stmt->op == PLC_IR_OP_GUARD_CALL
                || stmt->op == PLC_IR_OP_LOOP
                || stmt->op == PLC_IR_OP_CONST)
                && stmt->value[0] == '\0') {
            plc_set_error(err, err_size, "IR value is empty");
            return 0;
        }
    }
    return 1;
}

static const char *plc_ir_op_name(int op)
{
    if (op == PLC_IR_OP_CALL) {
        return "CALL";
    }
    if (op == PLC_IR_OP_GUARD_EVAL) {
        return "GUARD_EVAL";
    }
    if (op == PLC_IR_OP_GUARD_CALL) {
        return "GUARD_CALL";
    }
    if (op == PLC_IR_OP_LOOP) {
        return "LOOP";
    }
    if (op == PLC_IR_OP_ASSERT) {
        return "ASSERT";
    }
    if (op == PLC_IR_OP_REQUIRE) {
        return "REQUIRE";
    }
    if (op == PLC_IR_OP_ENSURE) {
        return "ENSURE";
    }
    if (op == PLC_IR_OP_STOPIF) {
        return "STOPIF";
    }
    if (op == PLC_IR_OP_CONST) {
        return "CONST";
    }
    return "EVAL";
}

int plc_emit_ir(const PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size)
{
    static PLC_IR_PROGRAM ir;
    FILE *out;
    int i;

    if (!plc_ir_build_program(program, &ir, err, err_size)
            || !plc_ir_validate_program(&ir, err, err_size)) {
        return 0;
    }
    out = fopen(path, "w");
    if (out == 0) {
        sprintf(err, "cannot open IR output: %s", path);
        return 0;
    }
    fprintf(out, "PLANKAC-TYPED-IR 1\n");
    fprintf(out, "sources %d\n", ir.source_count);
    fprintf(out, "procedures %d\n", ir.proc_count);
    fprintf(out, "statements %d\n\n", ir.stmt_count);
    for (i = 0; i < ir.stmt_count; ++i) {
        PLC_IR_STMT *stmt;

        stmt = &ir.stmts[i];
        fprintf(out, "P%d %s line %d %s lower=%s",
            stmt->proc_number, stmt->proc_name, stmt->source_line,
            plc_ir_op_name(stmt->op), stmt->lowering);
        if (stmt->callee[0] != '\0') {
            fprintf(out, " callee=%s argc=%d results=%d",
                stmt->callee, stmt->argc, stmt->results);
        }
        fprintf(out, "\n");
        if (stmt->guard[0] != '\0') {
            fprintf(out, "  guard[%s] %s\n",
                plc_type_family_name(stmt->guard_family), stmt->guard);
        }
        if (stmt->value[0] != '\0') {
            fprintf(out, "  value[%s] %s\n",
                plc_type_family_name(stmt->value_family), stmt->value);
        }
        if (stmt->target[0] != '\0') {
            fprintf(out, "  target[%s] %s\n",
                plc_type_family_name(stmt->target_family), stmt->target);
        }
    }
    fclose(out);
    plc_copy_error(err, err_size, "");
    return 1;
}
