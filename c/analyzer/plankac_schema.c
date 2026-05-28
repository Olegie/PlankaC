#include "plankac_internal.h"

#define PLC_SCHEMA_MAX 256

typedef struct PLC_SCHEMA_ENTRY {
    char key[96];
    int family;
    int element_family;
    int left_family;
    int right_family;
    int domain_family;
    int range_family;
} PLC_SCHEMA_ENTRY;

typedef struct PLC_SCHEMA_FIELD {
    char owner[96];
    char field[96];
    int family;
} PLC_SCHEMA_FIELD;

static int plc_schema_marker_family(const char *marker)
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

static int plc_schema_first_marker_family(const char *text)
{
    const char *p;
    const char *end;
    char marker[PLC_MAX_TYPE_TEXT];

    p = strstr(text, "[:");
    if (p == 0) {
        return PLC_TYPE_FAMILY_UNKNOWN;
    }
    end = strchr(p, ']');
    if (end == 0) {
        return PLC_TYPE_FAMILY_UNKNOWN;
    }
    plc_copy_range(marker, sizeof(marker), p, end + 1);
    return plc_schema_marker_family(marker);
}

static int plc_schema_builtin_family(const char *name)
{
    if (strcmp(name, "list_new") == 0 || strcmp(name, "list_push") == 0
            || strcmp(name, "list_concat") == 0
            || strcmp(name, "list_select_greater") == 0
            || strcmp(name, "list_select_where") == 0
            || strcmp(name, "list_zip_pairs") == 0
            || strcmp(name, "relation_domain") == 0
            || strcmp(name, "relation_range") == 0
            || strcmp(name, "relation_select_domain") == 0
            || strcmp(name, "relation_select_range") == 0
            || strcmp(name, "relation_domain_select_where") == 0
            || strcmp(name, "relation_range_select_where") == 0
            || strcmp(name, "relation_inverse") == 0
            || strcmp(name, "relation_image") == 0
            || strcmp(name, "set_cartesian") == 0
            || strcmp(name, "relation_compose") == 0) {
        return PLC_TYPE_FAMILY_LIST;
    }
    if (strcmp(name, "set_new") == 0 || strcmp(name, "set_add") == 0
            || strcmp(name, "set_union") == 0
            || strcmp(name, "set_intersection") == 0
            || strcmp(name, "set_difference") == 0
            || strcmp(name, "set_select_where") == 0) {
        return PLC_TYPE_FAMILY_SET;
    }
    if (strcmp(name, "record_new") == 0 || strcmp(name, "record_set") == 0
            || strcmp(name, "record_set_path2") == 0
            || strcmp(name, "record_merge") == 0
            || strcmp(name, "chess_board_new") == 0
            || strcmp(name, "chess_board_place") == 0
            || strcmp(name, "chess_board_move") == 0
            || strcmp(name, "chess_apply_move") == 0) {
        return PLC_TYPE_FAMILY_RECORD;
    }
    if (strcmp(name, "pair") == 0 || strcmp(name, "list_pair") == 0) {
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
    if (strcmp(name, "set_contains") == 0 || strcmp(name, "set_subset") == 0
            || strcmp(name, "set_equal") == 0
            || strcmp(name, "relation_has_pair") == 0
            || strcmp(name, "relation_exists_range_equal") == 0
            || strcmp(name, "relation_forall_domain_greater") == 0
            || strcmp(name, "relation_domain_exists_where") == 0
            || strcmp(name, "relation_domain_forall_where") == 0
            || strcmp(name, "relation_range_exists_where") == 0
            || strcmp(name, "relation_range_forall_where") == 0
            || strcmp(name, "record_has") == 0
            || strcmp(name, "record_has_path2") == 0
            || strcmp(name, "record_shape_equal") == 0
            || strcmp(name, "bits_get") == 0
            || strcmp(name, "list_exists_equal") == 0
            || strcmp(name, "list_forall_greater") == 0
            || strcmp(name, "list_exists_where") == 0
            || strcmp(name, "list_forall_where") == 0
            || strcmp(name, "set_exists_where") == 0
            || strcmp(name, "set_forall_where") == 0
            || strcmp(name, "chess_legal_move") == 0
            || strcmp(name, "chess_legal_rook_move") == 0
            || strcmp(name, "chess_side_in_check") == 0
            || strcmp(name, "chess_checkmate") == 0
            || strcmp(name, "chess_stalemate") == 0
            || strcmp(name, "chess_en_passant_possible") == 0) {
        return PLC_TYPE_FAMILY_BOOLEAN;
    }
    return PLC_TYPE_FAMILY_NUMERIC;
}

static int plc_schema_ref_key(const char *text, char *out, unsigned out_size)
{
    const char *p;
    unsigned n;

    p = plc_skip_space(text);
    if ((*p != 'V' && *p != 'C' && *p != 'Z' && *p != 'R')
            || !isdigit((unsigned char)p[1])) {
        if (out_size > 0) {
            out[0] = '\0';
        }
        return 0;
    }
    n = 0;
    while (*p != '\0') {
        if (p[0] == '[' && p[1] == ':') {
            break;
        }
        if (*p == ' ' || *p == '\t' || *p == ',') {
            break;
        }
        if (n + 1 < out_size) {
            out[n] = *p;
            ++n;
        }
        ++p;
    }
    out[n] = '\0';
    return 1;
}

static PLC_SCHEMA_ENTRY *plc_schema_find_entry(PLC_SCHEMA_ENTRY *entries,
    int count, const char *key)
{
    int i;

    for (i = 0; i < count; ++i) {
        if (strcmp(entries[i].key, key) == 0) {
            return &entries[i];
        }
    }
    return 0;
}

static PLC_SCHEMA_ENTRY *plc_schema_entry(PLC_SCHEMA_ENTRY *entries,
    int *count, const char *key, char *err, unsigned err_size)
{
    PLC_SCHEMA_ENTRY *entry;

    entry = plc_schema_find_entry(entries, *count, key);
    if (entry != 0) {
        return entry;
    }
    if (*count >= PLC_SCHEMA_MAX) {
        plc_set_error(err, err_size, "too many structural schema entries");
        return 0;
    }
    entry = &entries[*count];
    memset(entry, 0, sizeof(*entry));
    strncpy(entry->key, key, sizeof(entry->key) - 1);
    entry->key[sizeof(entry->key) - 1] = '\0';
    entry->family = PLC_TYPE_FAMILY_UNKNOWN;
    entry->element_family = PLC_TYPE_FAMILY_UNKNOWN;
    entry->left_family = PLC_TYPE_FAMILY_UNKNOWN;
    entry->right_family = PLC_TYPE_FAMILY_UNKNOWN;
    entry->domain_family = PLC_TYPE_FAMILY_UNKNOWN;
    entry->range_family = PLC_TYPE_FAMILY_UNKNOWN;
    ++(*count);
    return entry;
}

static int plc_schema_assign_family(PLC_SCHEMA_ENTRY *entry, int family,
    char *err, unsigned err_size)
{
    if (entry == 0 || family == PLC_TYPE_FAMILY_UNKNOWN) {
        return 1;
    }
    if (entry->family == PLC_TYPE_FAMILY_UNKNOWN) {
        entry->family = family;
        return 1;
    }
    if (entry->family == family) {
        return 1;
    }
    if (entry->family == PLC_TYPE_FAMILY_NUMERIC
            && family == PLC_TYPE_FAMILY_BOOLEAN) {
        return 1;
    }
    plc_set_error(err, err_size, "structural schema family mismatch");
    return 0;
}

static int plc_schema_split_args(const char *text,
    char args[][PLC_MAX_LINE], int *count)
{
    const char *p;
    const char *start;
    int depth;

    p = text;
    start = text;
    depth = 0;
    *count = 0;
    while (1) {
        if (*p == '(') {
            ++depth;
        } else if (*p == ')') {
            --depth;
        }
        if ((*p == ',' && depth == 0) || *p == '\0') {
            if (*count >= PLC_MAX_ARGS) {
                return 0;
            }
            plc_copy_range(args[*count], PLC_MAX_LINE, start, p);
            plc_trim_in_place(args[*count]);
            ++(*count);
            if (*p == '\0') {
                return 1;
            }
            start = p + 1;
        }
        ++p;
    }
}

static int plc_schema_expr_family(const PLC_PROGRAM *program,
    PLC_SCHEMA_ENTRY *entries, int entry_count, const char *expr)
{
    char key[96];
    char name[PLC_MAX_NAME];
    char args_text[PLC_MAX_LINE];
    PLC_SCHEMA_ENTRY *entry;
    const PLC_PROC *proc;
    int family;

    if (plc_line_starts_with(expr, "SELECT")
            || plc_line_starts_with(expr, "DOMAINSELECT")
            || plc_line_starts_with(expr, "RANGESELECT")) {
        return PLC_TYPE_FAMILY_LIST;
    }
    if (plc_line_starts_with(expr, "SETSELECT")) {
        return PLC_TYPE_FAMILY_SET;
    }
    if (plc_line_starts_with(expr, "EXISTS")
            || plc_line_starts_with(expr, "FORALL")
            || plc_line_starts_with(expr, "SETEXISTS")
            || plc_line_starts_with(expr, "SETFORALL")
            || plc_line_starts_with(expr, "DOMAINEXISTS")
            || plc_line_starts_with(expr, "DOMAINFORALL")
            || plc_line_starts_with(expr, "RANGEEXISTS")
            || plc_line_starts_with(expr, "RANGEFORALL")) {
        return PLC_TYPE_FAMILY_BOOLEAN;
    }
    if (plc_line_starts_with(expr, "COUNT")
            || plc_line_starts_with(expr, "SETCOUNT")
            || plc_line_starts_with(expr, "DOMAINCOUNT")
            || plc_line_starts_with(expr, "RANGECOUNT")) {
        return PLC_TYPE_FAMILY_NUMERIC;
    }
    family = plc_schema_first_marker_family(expr);
    if (family != PLC_TYPE_FAMILY_UNKNOWN) {
        return family;
    }
    if (plc_schema_ref_key(expr, key, sizeof(key))) {
        entry = plc_schema_find_entry(entries, entry_count, key);
        if (entry != 0) {
            return entry->family;
        }
    }
    if (plc_is_top_call(expr, name, sizeof(name), args_text,
            sizeof(args_text))) {
        proc = plc_find_proc(program, name);
        if (proc != 0 && proc->results > 0) {
            return plc_schema_marker_family(proc->result_types[0]);
        }
        return plc_schema_builtin_family(name);
    }
    return PLC_TYPE_FAMILY_UNKNOWN;
}

static int plc_schema_set_container_element(const PLC_PROC *proc,
    const PLC_STMT *stmt, PLC_SCHEMA_ENTRY *entry, int family,
    const char *kind, char *err, unsigned err_size)
{
    (void)err_size;
    if (entry == 0 || family == PLC_TYPE_FAMILY_UNKNOWN) {
        return 1;
    }
    if (entry->element_family == PLC_TYPE_FAMILY_UNKNOWN) {
        entry->element_family = family;
        return 1;
    }
    if (entry->element_family == family
            || (entry->element_family == PLC_TYPE_FAMILY_NUMERIC
                && family == PLC_TYPE_FAMILY_BOOLEAN)) {
        return 1;
    }
    sprintf(err, "P%d %s line %d: %s element schema mismatch",
        proc->number, proc->name, stmt->line_no, kind);
    return 0;
}

static int plc_schema_family_compatible(int existing, int family)
{
    if (existing == PLC_TYPE_FAMILY_UNKNOWN || family == PLC_TYPE_FAMILY_UNKNOWN
            || existing == family) {
        return 1;
    }
    if (existing == PLC_TYPE_FAMILY_NUMERIC
            && family == PLC_TYPE_FAMILY_BOOLEAN) {
        return 1;
    }
    return 0;
}

static int plc_schema_set_relation_side(const PLC_PROC *proc,
    const PLC_STMT *stmt, int *slot, int family, const char *side,
    char *err, unsigned err_size)
{
    (void)err_size;
    if (family == PLC_TYPE_FAMILY_UNKNOWN) {
        return 1;
    }
    if (*slot == PLC_TYPE_FAMILY_UNKNOWN) {
        *slot = family;
        return 1;
    }
    if (plc_schema_family_compatible(*slot, family)) {
        return 1;
    }
    sprintf(err, "P%d %s line %d: relation %s schema mismatch",
        proc->number, proc->name, stmt->line_no, side);
    return 0;
}

static PLC_SCHEMA_FIELD *plc_schema_find_field(PLC_SCHEMA_FIELD *fields,
    int count, const char *owner, const char *field)
{
    int i;

    for (i = 0; i < count; ++i) {
        if (strcmp(fields[i].owner, owner) == 0
                && strcmp(fields[i].field, field) == 0) {
            return &fields[i];
        }
    }
    return 0;
}

static int plc_schema_set_record_field(const PLC_PROC *proc,
    const PLC_STMT *stmt, PLC_SCHEMA_FIELD *fields, int *field_count,
    const char *owner, const char *field, int family,
    char *err, unsigned err_size)
{
    PLC_SCHEMA_FIELD *entry;

    (void)err_size;
    if (family == PLC_TYPE_FAMILY_UNKNOWN) {
        return 1;
    }
    entry = plc_schema_find_field(fields, *field_count, owner, field);
    if (entry == 0) {
        if (*field_count >= PLC_SCHEMA_MAX) {
            plc_set_error(err, err_size, "too many record schema fields");
            return 0;
        }
        entry = &fields[*field_count];
        memset(entry, 0, sizeof(*entry));
        strncpy(entry->owner, owner, sizeof(entry->owner) - 1);
        entry->owner[sizeof(entry->owner) - 1] = '\0';
        strncpy(entry->field, field, sizeof(entry->field) - 1);
        entry->field[sizeof(entry->field) - 1] = '\0';
        entry->family = family;
        ++(*field_count);
        return 1;
    }
    if (entry->family == family
            || (entry->family == PLC_TYPE_FAMILY_NUMERIC
                && family == PLC_TYPE_FAMILY_BOOLEAN)) {
        return 1;
    }
    sprintf(err, "P%d %s line %d: record field schema mismatch",
        proc->number, proc->name, stmt->line_no);
    return 0;
}

static int plc_schema_analyze_statement(const PLC_PROGRAM *program,
    const PLC_PROC *proc, const PLC_STMT *stmt,
    PLC_SCHEMA_ENTRY *entries, int *entry_count,
    PLC_SCHEMA_FIELD *fields, int *field_count,
    char *err, unsigned err_size)
{
    char parts[5][PLC_MAX_LINE];
    char name[PLC_MAX_NAME];
    char args_text[PLC_MAX_LINE];
    char args[PLC_MAX_ARGS][PLC_MAX_LINE];
    char target_key[96];
    char first_key[96];
    int part_count;
    int value_part;
    int target_part;
    int argc;
    int family;
    PLC_SCHEMA_ENTRY *target;
    PLC_SCHEMA_ENTRY *container;

    if (!plc_split_arrows(stmt->text, parts, &part_count)) {
        return 1;
    }
    if (part_count == 2) {
        value_part = 0;
        target_part = 1;
    } else if (part_count == 3) {
        value_part = 1;
        target_part = 2;
    } else {
        return 1;
    }
    target = 0;
    if (plc_schema_ref_key(parts[target_part], target_key,
            sizeof(target_key))) {
        target = plc_schema_entry(entries, entry_count, target_key,
            err, err_size);
        if (target == 0) {
            return 0;
        }
    }
    family = plc_schema_expr_family(program, entries, *entry_count,
        parts[value_part]);
    if (target != 0
            && !plc_schema_assign_family(target, family, err, err_size)) {
        char prefix[160];

        sprintf(prefix, "P%d %s line %d: ", proc->number, proc->name,
            stmt->line_no);
        plc_prefix_error(err, err_size, prefix);
        return 0;
    }
    if (!plc_is_top_call(parts[value_part], name, sizeof(name), args_text,
            sizeof(args_text))) {
        return 1;
    }
    if (!plc_schema_split_args(args_text, args, &argc)) {
        plc_set_error(err, err_size, "bad structural call argument list");
        return 0;
    }
    if ((strcmp(name, "list_push") == 0 || strcmp(name, "set_add") == 0)
            && argc == 2
            && plc_schema_ref_key(args[0], first_key, sizeof(first_key))) {
        int element_family;

        container = plc_schema_entry(entries, entry_count, first_key,
            err, err_size);
        if (container == 0) {
            return 0;
        }
        element_family = plc_schema_expr_family(program, entries,
            *entry_count, args[1]);
        if (!plc_schema_set_container_element(proc, stmt, container,
                element_family,
                strcmp(name, "list_push") == 0 ? "list" : "set",
                err, err_size)) {
            return 0;
        }
        if (element_family == PLC_TYPE_FAMILY_PAIR
                && plc_schema_ref_key(args[1], target_key,
                    sizeof(target_key))) {
            PLC_SCHEMA_ENTRY *pair_entry;

            pair_entry = plc_schema_find_entry(entries, *entry_count,
                target_key);
            if (pair_entry != 0) {
                if (!plc_schema_set_relation_side(proc, stmt,
                        &container->domain_family, pair_entry->left_family,
                        "domain", err, err_size)
                        || !plc_schema_set_relation_side(proc, stmt,
                            &container->range_family,
                            pair_entry->right_family, "range",
                            err, err_size)) {
                    return 0;
                }
            }
        }
    }
    if (strcmp(name, "pair") == 0 && argc == 2 && target != 0) {
        target->left_family = plc_schema_expr_family(program, entries,
            *entry_count, args[0]);
        target->right_family = plc_schema_expr_family(program, entries,
            *entry_count, args[1]);
    }
    if (strcmp(name, "record_set") == 0 && argc == 3
            && plc_schema_ref_key(args[0], first_key, sizeof(first_key))) {
        char field[96];
        int value_family;

        strncpy(field, args[1], sizeof(field) - 1);
        field[sizeof(field) - 1] = '\0';
        plc_trim_in_place(field);
        value_family = plc_schema_expr_family(program, entries,
            *entry_count, args[2]);
        if (!plc_schema_set_record_field(proc, stmt, fields, field_count,
                first_key, field, value_family, err, err_size)) {
            return 0;
        }
    }
    return 1;
}

int plc_analyze_structural_schemas(const PLC_PROGRAM *program,
    char *err, unsigned err_size)
{
    int i;

    for (i = 0; i < program->proc_count; ++i) {
        PLC_SCHEMA_ENTRY entries[PLC_SCHEMA_MAX];
        PLC_SCHEMA_FIELD fields[PLC_SCHEMA_MAX];
        int entry_count;
        int field_count;
        int s;

        memset(entries, 0, sizeof(entries));
        memset(fields, 0, sizeof(fields));
        entry_count = 0;
        field_count = 0;
        for (s = 0; s < program->procs[i].stmt_count; ++s) {
            if (!plc_schema_analyze_statement(program, &program->procs[i],
                    &program->procs[i].stmts[s], entries, &entry_count,
                    fields, &field_count, err, err_size)) {
                return 0;
            }
        }
    }
    return 1;
}
