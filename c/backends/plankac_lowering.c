#include "plankac_internal.h"

static const char *plc_lowering_op_kind(const PLC_IR_STMT *stmt)
{
    if (stmt->lowering[0] != '\0') {
        return stmt->lowering;
    }
    if (stmt->op == PLC_IR_OP_ASSERT || stmt->op == PLC_IR_OP_REQUIRE
            || stmt->op == PLC_IR_OP_ENSURE) {
        return "contract";
    }
    if (stmt->op == PLC_IR_OP_STOPIF || stmt->op == PLC_IR_OP_LOOP) {
        return "control";
    }
    return "scalar";
}

static int plc_lowering_needs_heap(const PLC_IR_STMT *stmt)
{
    if (stmt->value_family == PLC_TYPE_FAMILY_LIST
            || stmt->value_family == PLC_TYPE_FAMILY_SET
            || stmt->value_family == PLC_TYPE_FAMILY_PAIR
            || stmt->value_family == PLC_TYPE_FAMILY_RECORD
            || stmt->value_family == PLC_TYPE_FAMILY_COMPLEX
            || stmt->value_family == PLC_TYPE_FAMILY_VEC3
            || stmt->value_family == PLC_TYPE_FAMILY_MAT4
            || stmt->target_family == PLC_TYPE_FAMILY_LIST
            || stmt->target_family == PLC_TYPE_FAMILY_SET
            || stmt->target_family == PLC_TYPE_FAMILY_PAIR
            || stmt->target_family == PLC_TYPE_FAMILY_RECORD
            || stmt->target_family == PLC_TYPE_FAMILY_COMPLEX
            || stmt->target_family == PLC_TYPE_FAMILY_VEC3
            || stmt->target_family == PLC_TYPE_FAMILY_MAT4) {
        return 1;
    }
    if (strcmp(plc_lowering_op_kind(stmt), "list") == 0
            || strcmp(plc_lowering_op_kind(stmt), "set") == 0
            || strcmp(plc_lowering_op_kind(stmt), "pair") == 0
            || strcmp(plc_lowering_op_kind(stmt), "record") == 0
            || strcmp(plc_lowering_op_kind(stmt), "relation") == 0
            || strcmp(plc_lowering_op_kind(stmt), "predicate") == 0
            || strcmp(plc_lowering_op_kind(stmt), "chess") == 0
            || strcmp(plc_lowering_op_kind(stmt), "geometry") == 0
            || strcmp(plc_lowering_op_kind(stmt), "complex") == 0
            || strcmp(plc_lowering_op_kind(stmt), "compound") == 0) {
        return 1;
    }
    return 0;
}

static const char *plc_lowering_c_path(const PLC_IR_STMT *stmt)
{
    if (plc_lowering_needs_heap(stmt)) {
        return "abi-runtime-call";
    }
    if (stmt->op == PLC_IR_OP_CALL || stmt->op == PLC_IR_OP_GUARD_CALL) {
        return "procedure-call";
    }
    return "direct-expression";
}

static const char *plc_lowering_asm_path(const PLC_IR_STMT *stmt)
{
    if (plc_lowering_needs_heap(stmt)) {
        return "helper-dispatch";
    }
    if (stmt->op == PLC_IR_OP_CALL || stmt->op == PLC_IR_OP_GUARD_CALL) {
        return "near-call";
    }
    return "register-expression";
}

static const char *plc_lowering_8086_path(const PLC_IR_STMT *stmt)
{
    if (plc_lowering_needs_heap(stmt)) {
        return "compound-runtime-entry";
    }
    if (stmt->op == PLC_IR_OP_CALL || stmt->op == PLC_IR_OP_GUARD_CALL) {
        return "near-proc";
    }
    return "integer-core";
}

int plc_emit_lowering_report(const PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size)
{
    static PLC_IR_PROGRAM ir;
    FILE *out;
    int i;
    int scalar_count;
    int compound_count;

    if (!plc_ir_build_program(program, &ir, err, err_size)
            || !plc_ir_validate_program(&ir, err, err_size)) {
        return 0;
    }
    out = fopen(path, "w");
    if (out == 0) {
        sprintf(err, "cannot open lowering output: %s", path);
        return 0;
    }
    scalar_count = 0;
    compound_count = 0;
    fprintf(out, "PLANKAC-LOWERING 1\n");
    fprintf(out, "sources %d\n", ir.source_count);
    fprintf(out, "procedures %d\n", ir.proc_count);
    fprintf(out, "statements %d\n\n", ir.stmt_count);
    for (i = 0; i < ir.stmt_count; ++i) {
        const PLC_IR_STMT *stmt;

        stmt = &ir.stmts[i];
        if (plc_lowering_needs_heap(stmt)) {
            ++compound_count;
        } else {
            ++scalar_count;
        }
        fprintf(out,
            "P%d %s line %d kind=%s value=%s target=%s c=%s asm=%s asm8086=%s expr_nodes=%d calls=%d predicates=%d",
            stmt->proc_number, stmt->proc_name, stmt->source_line,
            plc_lowering_op_kind(stmt),
            plc_type_family_name(stmt->value_family),
            plc_type_family_name(stmt->target_family),
            plc_lowering_c_path(stmt),
            plc_lowering_asm_path(stmt),
            plc_lowering_8086_path(stmt),
            stmt->expr_nodes, stmt->expr_calls, stmt->expr_predicates);
        if (stmt->callee[0] != '\0') {
            fprintf(out, " callee=%s", stmt->callee);
        }
        fprintf(out, "\n");
        if (stmt->expr_shape[0] != '\0') {
            fprintf(out, "  ast %s\n", stmt->expr_shape);
        }
    }
    fprintf(out, "\nsummary scalar=%d compound=%d\n",
        scalar_count, compound_count);
    fclose(out);
    plc_copy_error(err, err_size, "");
    return 1;
}
