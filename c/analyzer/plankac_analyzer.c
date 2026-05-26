#include "plankac_internal.h"

static int plc_analyze_type_list(const PLC_PROC *proc,
    const char types[][PLC_MAX_TYPE_TEXT], int count, const char *kind,
    char *err, unsigned err_size)
{
    int i;

    for (i = 0; i < count; ++i) {
        PLC_TYPE_SPEC spec;

        if (types[i][0] == '\0') {
            continue;
        }
        if (!plc_parse_type_marker_text(types[i], &spec, err, err_size)) {
            char prefix[160];

            sprintf(prefix, "P%d %s %s %d: ",
                proc->number, proc->name, kind, i);
            plc_prefix_error(err, err_size, prefix);
            return 0;
        }
    }
    return 1;
}

static int plc_analyze_call_marker(const PLC_PROC *caller,
    const PLC_PROC *callee, const char *actual, const char *formal,
    const char *kind, int index, char *err, unsigned err_size)
{
    (void)err_size;
    if (!plc_type_markers_compatible(actual, formal)) {
        sprintf(err,
            "P%d %s %s %d incompatible with P%d %s",
            caller->number, caller->name, kind, index,
            callee->number, callee->name);
        return 0;
    }
    return 1;
}

static int plc_analyzer_marker_family(const char *marker)
{
    PLC_TYPE_SPEC spec;
    char err[PLC_MAX_LINE];

    if (marker == 0 || marker[0] == '\0') {
        return PLC_TYPE_FAMILY_UNKNOWN;
    }
    err[0] = '\0';
    if (!plc_parse_type_marker_text(marker, &spec, err, sizeof(err))) {
        return PLC_TYPE_FAMILY_UNKNOWN;
    }
    return spec.family;
}

static int plc_analyzer_first_marker_family(const char *text)
{
    const char *p;
    const char *end;
    char marker[PLC_MAX_TYPE_TEXT];

    p = strstr(text, "[:");
    if (p == 0) {
        return PLC_TYPE_FAMILY_UNKNOWN;
    }
    --p;
    while (p > text && *p != '[') {
        --p;
    }
    if (p[0] != '[' || p[1] != ':') {
        p = strstr(text, "[:");
    }
    end = strchr(p, ']');
    if (end == 0) {
        return PLC_TYPE_FAMILY_UNKNOWN;
    }
    plc_copy_range(marker, sizeof(marker), p, end + 1);
    return plc_analyzer_marker_family(marker);
}

static int plc_analyzer_target_family(const char *targets)
{
    return plc_analyzer_first_marker_family(targets);
}

static int plc_analyzer_builtin_family(const char *name)
{
    if (strcmp(name, "chess_board_new") == 0
            || strcmp(name, "chess_board_place") == 0
            || strcmp(name, "chess_board_move") == 0
            || strcmp(name, "chess_apply_move") == 0
            || strcmp(name, "record_new") == 0
            || strcmp(name, "record_set") == 0
            || strcmp(name, "record_set_path2") == 0
            || strcmp(name, "record_merge") == 0) {
        return PLC_TYPE_FAMILY_RECORD;
    }
    if (strcmp(name, "chess_rook_attack_map") == 0
            || strcmp(name, "chess_bishop_attack_map") == 0
            || strcmp(name, "chess_knight_attack_map") == 0
            || strcmp(name, "chess_queen_attack_map") == 0
            || strcmp(name, "chess_king_attack_map") == 0) {
        return PLC_TYPE_FAMILY_LIST;
    }
    if (strcmp(name, "sqrt") == 0 || strcmp(name, "set_size") == 0
            || strcmp(name, "list_len") == 0 || strcmp(name, "list_get") == 0
            || strcmp(name, "list_first") == 0
            || strcmp(name, "list_last") == 0
            || strcmp(name, "list_min") == 0
            || strcmp(name, "list_max") == 0
            || strcmp(name, "pair_left") == 0
            || strcmp(name, "pair_right") == 0
            || strcmp(name, "pair_list_count_first") == 0
            || strcmp(name, "record_get") == 0
            || strcmp(name, "record_get_path2") == 0
            || strcmp(name, "record_size") == 0
            || strcmp(name, "complex_real") == 0
            || strcmp(name, "complex_imag") == 0
            || strcmp(name, "complex_norm2") == 0
            || strcmp(name, "bit") == 0
            || strcmp(name, "bits_pack4") == 0
            || strcmp(name, "fixed_quantize") == 0
            || strcmp(name, "fixed_add") == 0
            || strcmp(name, "fixed_mul") == 0
            || strcmp(name, "fixed_div_checked") == 0
            || strcmp(name, "arith_divide_checked") == 0
            || strcmp(name, "raise_exception") == 0
            || strcmp(name, "exception_code") == 0
            || strcmp(name, "exception_clear") == 0
            || strcmp(name, "list_count_equal") == 0
            || strcmp(name, "chess_material_score") == 0
            || strcmp(name, "chess_best_capture_score") == 0
            || strcmp(name, "chess_fen_signature") == 0
            || strcmp(name, "relation_signature") == 0
            || strcmp(name, "pair_left_list_len") == 0
            || strcmp(name, "pair_right_list_len") == 0
            || strcmp(name, "vec3_x") == 0
            || strcmp(name, "vec3_y") == 0
            || strcmp(name, "vec3_z") == 0
            || strcmp(name, "vec3_dot") == 0
            || strcmp(name, "vec3_len2") == 0
            || strcmp(name, "perspective_project_x") == 0
            || strcmp(name, "chess_square") == 0
            || strcmp(name, "chess_piece") == 0
            || strcmp(name, "chess_piece_kind") == 0
            || strcmp(name, "chess_piece_side") == 0
            || strcmp(name, "chess_board_piece") == 0) {
        return PLC_TYPE_FAMILY_NUMERIC;
    }
    if (strcmp(name, "set_contains") == 0 || strcmp(name, "set_subset") == 0
            || strcmp(name, "set_equal") == 0
            || strcmp(name, "set_exists_greater") == 0
            || strcmp(name, "set_forall_less") == 0
            || strcmp(name, "relation_has_pair") == 0
            || strcmp(name, "relation_exists_range_equal") == 0
            || strcmp(name, "relation_forall_domain_greater") == 0
            || strcmp(name, "chess_piece_attacks_square") == 0
            || strcmp(name, "list_equal") == 0
            || strcmp(name, "pair_equal") == 0
            || strcmp(name, "record_equal") == 0
            || strcmp(name, "record_has") == 0
            || strcmp(name, "record_has_path2") == 0
            || strcmp(name, "record_shape_equal") == 0
            || strcmp(name, "complex_equal") == 0) {
        return PLC_TYPE_FAMILY_BOOLEAN;
    }
    if (strcmp(name, "bit_not") == 0
            || strcmp(name, "bit_and") == 0
            || strcmp(name, "bit_or") == 0
            || strcmp(name, "bit_xor") == 0
            || strcmp(name, "bits_get") == 0
            || strcmp(name, "exception_raised") == 0
            || strcmp(name, "list_exists_equal") == 0
            || strcmp(name, "list_forall_greater") == 0
            || strcmp(name, "chess_legal_rook_move") == 0
            || strcmp(name, "chess_legal_move") == 0
            || strcmp(name, "chess_side_in_check") == 0
            || strcmp(name, "chess_checkmate_simple") == 0
            || strcmp(name, "chess_checkmate") == 0
            || strcmp(name, "chess_stalemate") == 0
            || strcmp(name, "chess_en_passant_possible") == 0) {
        return PLC_TYPE_FAMILY_BOOLEAN;
    }
    if (strcmp(name, "list_new") == 0 || strcmp(name, "list_push") == 0
            || strcmp(name, "list_concat") == 0
            || strcmp(name, "list_select_greater") == 0
            || strcmp(name, "list_zip_pairs") == 0
            || strcmp(name, "relation_domain") == 0
            || strcmp(name, "relation_range") == 0
            || strcmp(name, "relation_select_domain") == 0
            || strcmp(name, "relation_select_range") == 0
            || strcmp(name, "relation_inverse") == 0
            || strcmp(name, "relation_image") == 0
            || strcmp(name, "set_cartesian") == 0
            || strcmp(name, "relation_compose") == 0) {
        return PLC_TYPE_FAMILY_LIST;
    }
    if (strcmp(name, "set_new") == 0 || strcmp(name, "set_add") == 0
            || strcmp(name, "set_union") == 0
            || strcmp(name, "set_intersection") == 0
            || strcmp(name, "set_difference") == 0) {
        return PLC_TYPE_FAMILY_SET;
    }
    if (strcmp(name, "pair") == 0) {
        return PLC_TYPE_FAMILY_PAIR;
    }
    if (strcmp(name, "list_pair") == 0) {
        return PLC_TYPE_FAMILY_PAIR;
    }
    if (strcmp(name, "complex") == 0 || strcmp(name, "complex_add") == 0
            || strcmp(name, "complex_sub") == 0
            || strcmp(name, "complex_mul") == 0
            || strcmp(name, "complex_conj") == 0) {
        return PLC_TYPE_FAMILY_COMPLEX;
    }
    if (strcmp(name, "vec3") == 0 || strcmp(name, "vec3_add") == 0
            || strcmp(name, "vec3_sub") == 0
            || strcmp(name, "vec3_cross") == 0
            || strcmp(name, "vec3_scale") == 0
            || strcmp(name, "vec3_normalize") == 0
            || strcmp(name, "mat4_transform_point") == 0
            || strcmp(name, "perspective_project") == 0) {
        return PLC_TYPE_FAMILY_VEC3;
    }
    if (strcmp(name, "mat4_identity") == 0
            || strcmp(name, "mat4_translate") == 0
            || strcmp(name, "mat4_scale") == 0
            || strcmp(name, "mat4_mul") == 0) {
        return PLC_TYPE_FAMILY_MAT4;
    }
    return PLC_TYPE_FAMILY_UNKNOWN;
}

static int plc_analyzer_expr_family(const PLC_PROGRAM *program,
    const char *expr)
{
    char name[PLC_MAX_NAME];
    char args[PLC_MAX_LINE];
    const PLC_PROC *proc;
    int i;

    if (plc_line_starts_with(expr, "SELECT")) {
        return PLC_TYPE_FAMILY_LIST;
    }
    if (plc_line_starts_with(expr, "EXISTS")
            || plc_line_starts_with(expr, "FORALL")) {
        return PLC_TYPE_FAMILY_BOOLEAN;
    }
    if (plc_line_starts_with(expr, "COUNT")) {
        return PLC_TYPE_FAMILY_NUMERIC;
    }
    if (plc_is_top_call(expr, name, sizeof(name), args, sizeof(args))) {
        proc = plc_find_proc(program, name);
        if (proc != 0 && proc->results > 0) {
            return plc_analyzer_marker_family(proc->result_types[0]);
        }
        return plc_analyzer_builtin_family(name);
    }
    if (strstr(expr, "!=") != 0 || strstr(expr, "<=") != 0
            || strstr(expr, ">=") != 0 || strchr(expr, '<') != 0
            || strchr(expr, '>') != 0 || strchr(expr, '=') != 0) {
        return PLC_TYPE_FAMILY_BOOLEAN;
    }
    if (strchr(expr, '+') != 0 || strchr(expr, '-') != 0
            || strchr(expr, '*') != 0 || strchr(expr, '/') != 0
            || strchr(expr, '%') != 0 || strchr(expr, '^') != 0) {
        return PLC_TYPE_FAMILY_NUMERIC;
    }
    for (i = 0; expr[i] != '\0'; ++i) {
        if (expr[i] == '&' || expr[i] == '|') {
            return PLC_TYPE_FAMILY_BOOLEAN;
        }
    }
    return plc_analyzer_first_marker_family(expr);
}

static int plc_analyzer_family_assignable(int source, int target)
{
    if (source == PLC_TYPE_FAMILY_UNKNOWN || target == PLC_TYPE_FAMILY_UNKNOWN) {
        return 1;
    }
    if (source == target) {
        return 1;
    }
    if (source == PLC_TYPE_FAMILY_BOOLEAN
            && target == PLC_TYPE_FAMILY_NUMERIC) {
        return 1;
    }
    return 0;
}

static int plc_analyze_statement_value_types(const PLC_PROGRAM *program,
    const PLC_PROC *proc, const PLC_STMT *stmt,
    char *err, unsigned err_size)
{
    char parts[5][PLC_MAX_LINE];
    int count;
    int value_part;
    int target_part;
    int source_family;
    int target_family;

    (void)err_size;

    if (plc_line_starts_with(stmt->text, "ASSERT")
            || plc_line_starts_with(stmt->text, "REQUIRE")
            || plc_line_starts_with(stmt->text, "ENSURE")
            || plc_line_starts_with(stmt->text, "STOPIF")) {
        const char *keyword;
        int keyword_len;

        keyword = "ASSERT";
        keyword_len = 6;
        if (plc_line_starts_with(stmt->text, "REQUIRE")) {
            keyword = "REQUIRE";
            keyword_len = 7;
        } else if (plc_line_starts_with(stmt->text, "ENSURE")) {
            keyword = "ENSURE";
            keyword_len = 6;
        } else if (plc_line_starts_with(stmt->text, "STOPIF")) {
            keyword = "STOPIF";
            keyword_len = 6;
        }
        source_family = plc_analyzer_expr_family(program,
            plc_skip_space(plc_skip_space(stmt->text) + keyword_len));
        if (source_family != PLC_TYPE_FAMILY_UNKNOWN
                && source_family != PLC_TYPE_FAMILY_BOOLEAN) {
            sprintf(err, "P%d %s line %d: %s expects boolean expression",
                proc->number, proc->name, stmt->line_no, keyword);
            return 0;
        }
        return 1;
    }
    if (plc_line_starts_with(stmt->text, "CONST")) {
        const char *body;

        body = plc_skip_space(plc_skip_space(stmt->text) + 5);
        if (*body != 'C') {
            sprintf(err, "P%d %s line %d: CONST expects C-bank target",
                proc->number, proc->name, stmt->line_no);
            return 0;
        }
        return 1;
    }
    if (!plc_split_arrows(stmt->text, parts, &count)) {
        return 1;
    }
    if (count == 2) {
        value_part = 0;
        target_part = 1;
    } else if (count == 3) {
        if (plc_line_starts_with(parts[0], "LOOP")) {
            value_part = 1;
            target_part = 2;
        } else {
            source_family = plc_analyzer_expr_family(program, parts[0]);
            if (source_family != PLC_TYPE_FAMILY_UNKNOWN
                    && source_family != PLC_TYPE_FAMILY_BOOLEAN) {
                sprintf(err, "P%d %s line %d: guard expects boolean expression",
                    proc->number, proc->name, stmt->line_no);
                return 0;
            }
            value_part = 1;
            target_part = 2;
        }
    } else {
        return 1;
    }
    source_family = plc_analyzer_expr_family(program, parts[value_part]);
    target_family = plc_analyzer_target_family(parts[target_part]);
    if (!plc_analyzer_family_assignable(source_family, target_family)) {
        sprintf(err,
            "P%d %s line %d: cannot assign %s value to %s target",
            proc->number, proc->name, stmt->line_no,
            plc_type_family_name(source_family),
            plc_type_family_name(target_family));
        return 0;
    }
    return 1;
}

int plc_analyze_program(const PLC_PROGRAM *program,
    char *err, unsigned err_size)
{
    int i;
    int j;

    if (program == 0) {
        plc_set_error(err, err_size, "missing program");
        return 0;
    }
    for (i = 0; i < program->proc_count; ++i) {
        const PLC_PROC *proc;

        proc = &program->procs[i];
        if (!plc_analyze_type_list(proc, proc->arg_types,
                proc->argc, "argument", err, err_size)) {
            return 0;
        }
        if (!plc_analyze_type_list(proc, proc->result_types,
                proc->results, "result", err, err_size)) {
            return 0;
        }
        for (j = 0; j < proc->stmt_count; ++j) {
            if (!plc_validate_type_markers_in_line(proc->stmts[j].text,
                    err, err_size)) {
                char prefix[160];

                sprintf(prefix, "P%d %s line %d: ",
                    proc->number, proc->name, proc->stmts[j].line_no);
                plc_prefix_error(err, err_size, prefix);
                return 0;
            }
            if (!plc_analyze_statement_value_types(program, proc,
                    &proc->stmts[j], err, err_size)) {
                return 0;
            }
        }
    }
    for (i = 0; i < program->proc_count; ++i) {
        const PLC_PROC *caller;
        int s;

        caller = &program->procs[i];
        for (s = 0; s < caller->stmt_count; ++s) {
            char parts[5][PLC_MAX_LINE];
            char name[PLC_MAX_NAME];
            char args_text[PLC_MAX_LINE];
            int part_count;
            int value_part;
            int k;

            if (!plc_split_arrows(caller->stmts[s].text,
                    parts, &part_count)) {
                continue;
            }
            if (part_count == 2) {
                value_part = 0;
            } else if (part_count == 3
                    && !plc_line_starts_with(parts[0], "LOOP")) {
                value_part = 1;
            } else {
                continue;
            }
            if (!plc_is_top_call(parts[value_part], name, sizeof(name),
                    args_text, sizeof(args_text))) {
                continue;
            }
            for (k = 0; k < program->proc_count; ++k) {
                const PLC_PROC *callee;

                callee = &program->procs[k];
                if (strcmp(callee->name, name) == 0) {
                    int m;

                    for (m = 0; m < callee->argc; ++m) {
                        if (callee->arg_types[m][0] != '\0'
                                && !plc_analyze_call_marker(caller, callee,
                                    callee->arg_types[m],
                                    callee->arg_types[m],
                                    "argument", m, err, err_size)) {
                            return 0;
                        }
                    }
                }
            }
        }
    }
    if (!plc_analyze_structural_schemas(program, err, err_size)) {
        return 0;
    }
    return 1;
}
