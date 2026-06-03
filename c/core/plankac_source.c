#include "plankac_internal.h"

#define PLC_MAX_TYPE_DECLS 256
#define PLC_MAX_TYPE_KEY 96
typedef struct PLC_TYPE_DECL {
    char key[PLC_MAX_TYPE_KEY];
    char type[PLC_MAX_TYPE_TEXT];
} PLC_TYPE_DECL;

static int plc_validate_type_markers(const char *line, char *err,
    unsigned err_size)
{
    return plc_validate_type_markers_in_line(line, err, err_size);
}

static void plc_strip_comment_keep_left(char *text)
{
    char *hash;

    hash = strchr(text, '#');
    if (hash != 0) {
        *hash = '\0';
    }
    plc_rtrim(text);
}

static int plc_add_type_decl(PLC_TYPE_DECL *decls, int *count,
    const char *key, const char *type, char *err, unsigned err_size)
{
    int i;

    for (i = 0; i < *count; ++i) {
        if (strcmp(decls[i].key, key) == 0) {
            if (!plc_type_markers_compatible(decls[i].type, type)) {
                plc_set_error(err, err_size, "type mismatch");
                return 0;
            }
            return 1;
        }
    }
    if (*count >= PLC_MAX_TYPE_DECLS) {
        plc_set_error(err, err_size, "too many typed references");
        return 0;
    }
    strncpy(decls[*count].key, key, sizeof(decls[*count].key) - 1);
    decls[*count].key[sizeof(decls[*count].key) - 1] = '\0';
    strncpy(decls[*count].type, type, sizeof(decls[*count].type) - 1);
    decls[*count].type[sizeof(decls[*count].type) - 1] = '\0';
    ++(*count);
    return 1;
}

static int plc_scan_typed_refs(const char *line, PLC_TYPE_DECL *decls,
    int *count, char *err, unsigned err_size)
{
    const char *p;

    p = line;
    while (*p != '\0') {
        char key[PLC_MAX_TYPE_KEY];
        char type[PLC_MAX_TYPE_TEXT];
        unsigned n;

        if ((*p != 'V' && *p != 'C' && *p != 'Z' && *p != 'R')
                || !isdigit((unsigned char)p[1])) {
            ++p;
            continue;
        }

        n = 0;
        key[n++] = *p;
        ++p;
        while (isdigit((unsigned char)*p)) {
            if (n + 1 < sizeof(key)) {
                key[n++] = *p;
            }
            ++p;
        }
        p = plc_skip_space(p);
        while (*p == '[' && p[1] != ':') {
            const char *start;

            start = p;
            ++p;
            p = plc_skip_space(p);
            while (isdigit((unsigned char)*p)) {
                ++p;
            }
            p = plc_skip_space(p);
            if (*p == ',') {
                ++p;
                p = plc_skip_space(p);
                while (isdigit((unsigned char)*p)) {
                    ++p;
                }
                p = plc_skip_space(p);
            }
            if (*p != ']') {
                plc_set_error(err, err_size, "bad indexed reference");
                return 0;
            }
            ++p;
            while (start < p && n + 1 < sizeof(key)) {
                key[n++] = *start;
                ++start;
            }
            p = plc_skip_space(p);
        }
        p = plc_skip_space(p);
        while (*p == '.') {
            if (n + 1 < sizeof(key)) {
                key[n++] = *p;
            }
            ++p;
            if (!isalpha((unsigned char)*p) && *p != '_') {
                plc_set_error(err, err_size, "bad record field");
                return 0;
            }
            while (isalnum((unsigned char)*p) || *p == '_') {
                if (n + 1 < sizeof(key)) {
                    key[n++] = *p;
                }
                ++p;
            }
        }
        key[n] = '\0';

        p = plc_skip_space(p);
        if (p[0] == '[' && p[1] == ':') {
            const char *start;
            const char *end;

            start = p;
            end = strchr(p, ']');
            if (end == 0) {
                plc_set_error(err, err_size, "bad type marker");
                return 0;
            }
            plc_copy_range(type, sizeof(type), start, end + 1);
            if (!plc_add_type_decl(decls, count, key, type,
                    err, err_size)) {
                return 0;
            }
            p = end + 1;
        }
    }
    return 1;
}

static void plc_collect_ref_types(const char *first, const char *last,
    char bank, char types[][PLC_MAX_TYPE_TEXT], int max_types)
{
    const char *p;
    int count;

    p = first;
    count = 0;
    while (p < last && *p != '\0' && count < max_types) {
        if (*p == bank && isdigit((unsigned char)p[1])) {
            ++p;
            while (p < last && isdigit((unsigned char)*p)) {
                ++p;
            }
            p = plc_skip_space(p);
            if (*p == '[' && p[1] != ':') {
                ++p;
                while (p < last && *p != '\0' && *p != ']') {
                    ++p;
                }
                if (*p == ']') {
                    ++p;
                }
                p = plc_skip_space(p);
            }
            while (*p == '.') {
                ++p;
                while (p < last && (isalnum((unsigned char)*p) || *p == '_')) {
                    ++p;
                }
                p = plc_skip_space(p);
            }
            if (p[0] == '[' && p[1] == ':') {
                const char *end;

                end = strchr(p, ']');
                if (end != 0 && end <= last) {
                    plc_copy_range(types[count], PLC_MAX_TYPE_TEXT, p, end + 1);
                    ++count;
                    p = end + 1;
                    continue;
                }
            }
            types[count][0] = '\0';
            ++count;
        } else {
            ++p;
        }
    }
}

static int plc_first_type_marker(const char *text,
    char *out, unsigned out_size)
{
    const char *p;
    const char *end;

    p = strstr(text, "[:");
    if (p == 0) {
        if (out_size > 0) {
            out[0] = '\0';
        }
        return 0;
    }
    end = strchr(p, ']');
    if (end == 0) {
        if (out_size > 0) {
            out[0] = '\0';
        }
        return 0;
    }
    plc_copy_range(out, out_size, p, end + 1);
    return 1;
}

static int plc_collect_arg_types(const char *args_text,
    char types[][PLC_MAX_TYPE_TEXT], int *count)
{
    const char *p;
    const char *start;
    int depth;
    char trimmed[PLC_MAX_LINE];

    strncpy(trimmed, args_text, sizeof(trimmed) - 1);
    trimmed[sizeof(trimmed) - 1] = '\0';
    plc_trim_in_place(trimmed);
    p = args_text;
    start = args_text;
    depth = 0;
    *count = 0;
    if (trimmed[0] == '\0') {
        return 1;
    }
    while (1) {
        if (*p == '(') {
            ++depth;
        } else if (*p == ')') {
            --depth;
        }
        if ((*p == ',' && depth == 0) || *p == '\0') {
            char part[PLC_MAX_LINE];

            if (*count >= PLC_MAX_ARGS) {
                return 0;
            }
            plc_copy_range(part, sizeof(part), start, p);
            plc_trim_in_place(part);
            plc_first_type_marker(part, types[*count], PLC_MAX_TYPE_TEXT);
            ++(*count);
            if (*p == '\0') {
                return 1;
            }
            start = p + 1;
        }
        ++p;
    }
}

static int plc_collect_target_types(const char *targets,
    char types[][PLC_MAX_TYPE_TEXT], int *count)
{
    const char *p;
    const char *start;

    p = targets;
    start = targets;
    *count = 0;
    while (1) {
        if (*p == ',' || *p == '\0') {
            char part[PLC_MAX_LINE];

            if (*count >= PLC_MAX_RESULTS) {
                return 0;
            }
            plc_copy_range(part, sizeof(part), start, p);
            plc_trim_in_place(part);
            plc_first_type_marker(part, types[*count], PLC_MAX_TYPE_TEXT);
            ++(*count);
            if (*p == '\0') {
                return 1;
            }
            start = p + 1;
        }
        ++p;
    }
}

static int plc_check_call_types(const PLC_PROGRAM *program,
    const char *stmt_text, char *err, unsigned err_size)
{
    char parts[5][PLC_MAX_LINE];
    char name[PLC_MAX_NAME];
    char args_text[PLC_MAX_LINE];
    char arg_types[PLC_MAX_ARGS][PLC_MAX_TYPE_TEXT];
    char target_types[PLC_MAX_RESULTS][PLC_MAX_TYPE_TEXT];
    int arg_count;
    int target_count;
    int part_count;
    int value_part;
    int target_part;
    int i;
    const PLC_PROC *callee;
    const PLC_NATIVE_PROC *native_callee;
    int callee_argc;
    int callee_results;
    char (*callee_arg_types)[PLC_MAX_TYPE_TEXT];
    char (*callee_result_types)[PLC_MAX_TYPE_TEXT];

    if (!plc_split_arrows(stmt_text, parts, &part_count)) {
        return 1;
    }
    if (part_count == 2) {
        value_part = 0;
        target_part = 1;
    } else if (part_count == 3 && !plc_line_starts_with(parts[0], "LOOP")) {
        value_part = 1;
        target_part = 2;
    } else {
        return 1;
    }
    if (!plc_is_top_call(parts[value_part], name, sizeof(name),
            args_text, sizeof(args_text))) {
        return 1;
    }
    callee = plc_find_proc(program, name);
    native_callee = 0;
    if (callee != 0) {
        callee_argc = callee->argc;
        callee_results = callee->results;
        callee_arg_types = (char (*)[PLC_MAX_TYPE_TEXT])callee->arg_types;
        callee_result_types =
            (char (*)[PLC_MAX_TYPE_TEXT])callee->result_types;
    } else {
        native_callee = plc_find_native(program, name);
        if (native_callee == 0) {
            return 1;
        }
        callee_argc = native_callee->argc;
        callee_results = native_callee->results;
        callee_arg_types =
            (char (*)[PLC_MAX_TYPE_TEXT])native_callee->arg_types;
        callee_result_types =
            (char (*)[PLC_MAX_TYPE_TEXT])native_callee->result_types;
    }
    if (callee == 0 && native_callee == 0) {
        return 1;
    }
    if (!plc_collect_arg_types(args_text, arg_types, &arg_count)
            || !plc_collect_target_types(parts[target_part],
                target_types, &target_count)) {
        plc_set_error(err, err_size, "bad call type list");
        return 0;
    }
    if (arg_count != callee_argc || target_count != callee_results) {
        return 1;
    }
    for (i = 0; i < arg_count; ++i) {
        if (arg_types[i][0] != '\0' && callee_arg_types[i][0] != '\0'
                && !plc_type_markers_compatible(arg_types[i],
                    callee_arg_types[i])) {
            plc_set_error(err, err_size, "procedure argument type mismatch");
            return 0;
        }
    }
    for (i = 0; i < target_count; ++i) {
        if (target_types[i][0] != '\0'
                && callee_result_types[i][0] != '\0'
                && !plc_type_markers_compatible(target_types[i],
                    callee_result_types[i])) {
            plc_set_error(err, err_size, "procedure result type mismatch");
            return 0;
        }
    }
    return 1;
}

static int plc_validate_call_types(const PLC_PROGRAM *program,
    char *err, unsigned err_size)
{
    int i;
    int s;

    for (i = 0; i < program->proc_count; ++i) {
        for (s = 0; s < program->procs[i].stmt_count; ++s) {
            if (!plc_check_call_types(program,
                    program->procs[i].stmts[s].text, err, err_size)) {
                char prefix[160];

                sprintf(prefix, "P%d %s line %d: ",
                    program->procs[i].number,
                    program->procs[i].name,
                    program->procs[i].stmts[s].line_no);
                plc_prefix_error(err, err_size, prefix);
                return 0;
            }
        }
    }
    return 1;
}

static int plc_text_has_call_to(const char *text, const char *name)
{
    const char *base;
    unsigned n;

    base = text;
    n = (unsigned)strlen(name);
    while (*text != '\0') {
        if ((text == base || !isalnum((unsigned char)text[-1]))
                && strncmp(text, name, n) == 0
                && text[n] == '(') {
            return 1;
        }
        ++text;
    }
    return 0;
}

static int plc_validate_no_recursion(const PLC_PROGRAM *program,
    char *err, unsigned err_size)
{
    int calls[PLC_MAX_PROCS][PLC_MAX_PROCS];
    int i;
    int j;
    int k;

    (void)err_size;

    memset(calls, 0, sizeof(calls));
    for (i = 0; i < program->proc_count; ++i) {
        int s;

        for (s = 0; s < program->procs[i].stmt_count; ++s) {
            for (j = 0; j < program->proc_count; ++j) {
                char numbered[32];

                sprintf(numbered, "P%d", program->procs[j].number);
                if (plc_text_has_call_to(program->procs[i].stmts[s].text,
                        program->procs[j].name)
                        || plc_text_has_call_to(
                            program->procs[i].stmts[s].text, numbered)) {
                    calls[i][j] = 1;
                }
            }
        }
    }
    for (k = 0; k < program->proc_count; ++k) {
        for (i = 0; i < program->proc_count; ++i) {
            for (j = 0; j < program->proc_count; ++j) {
                if (calls[i][k] && calls[k][j]) {
                    calls[i][j] = 1;
                }
            }
        }
    }
    for (i = 0; i < program->proc_count; ++i) {
        if (calls[i][i]) {
            sprintf(err, "recursive procedure rejected: %s",
                program->procs[i].name);
            return 0;
        }
    }
    return 1;
}

int plc_parse_header(const char *line, PLC_PROC *proc,
    char *err, unsigned err_size)
{
    const char *p;
    const char *open;
    const char *close;
    const char *arrow;
    char *endptr;
    int i;

    memset(proc, 0, sizeof(*proc));
    p = plc_skip_space(line);
    if (*p != 'P') {
        plc_set_error(err, err_size, "expected procedure header");
        return 0;
    }
    ++p;
    proc->number = (int)strtol(p, &endptr, 10);
    if (endptr == p) {
        plc_set_error(err, err_size, "missing procedure number");
        return 0;
    }
    p = plc_skip_space(endptr);
    i = 0;
    while (*p != '\0' && *p != '(' && !isspace((unsigned char)*p)) {
        if (i < PLC_MAX_NAME - 1) {
            proc->name[i] = *p;
            ++i;
        }
        ++p;
    }
    proc->name[i] = '\0';
    if (proc->name[0] == '\0') {
        plc_set_error(err, err_size, "missing procedure name");
        return 0;
    }
    open = strchr(p, '(');
    if (open == 0) {
        plc_set_error(err, err_size, "missing parameter list");
        return 0;
    }
    close = plc_matching_paren(open);
    if (close == 0) {
        plc_set_error(err, err_size, "unclosed parameter list");
        return 0;
    }
    arrow = strstr(close, "=>");
    if (arrow == 0) {
        plc_set_error(err, err_size, "missing result arrow");
        return 0;
    }
    proc->argc = plc_count_refs(open + 1, close, 'V');
    proc->results = plc_count_refs(arrow + 2, line + strlen(line), 'R');
    plc_collect_ref_types(open + 1, close, 'V',
        proc->arg_types, PLC_MAX_ARGS);
    plc_collect_ref_types(arrow + 2, line + strlen(line), 'R',
        proc->result_types, PLC_MAX_RESULTS);
    if (proc->results <= 0) {
        plc_set_error(err, err_size, "procedure has no result");
        return 0;
    }
    return 1;
}

const PLC_PROC *plc_find_proc(const PLC_PROGRAM *program,
    const char *name)
{
    int i;

    if (name[0] == 'P' && isdigit((unsigned char)name[1])) {
        int number;

        number = atoi(name + 1);
        for (i = 0; i < program->proc_count; ++i) {
            if (program->procs[i].number == number) {
                return &program->procs[i];
            }
        }
    }
    for (i = 0; i < program->proc_count; ++i) {
        if (strcmp(program->procs[i].name, name) == 0) {
            return &program->procs[i];
        }
    }
    return 0;
}

int plc_load_sources(PLC_PROGRAM *program, const char *const *sources,
    char *err, unsigned err_size)
{
    int file_index;
    int in_proc;
    PLC_PROC *current;
    PLC_TYPE_DECL type_decls[PLC_MAX_TYPE_DECLS];
    int type_decl_count;
    PLC_NATIVE_PROC native_copy[PLANKAC_MAX_NATIVE];
    int native_count;
    int native_i;

    native_count = 0;
    if (program != 0 && program->native_count > 0
            && program->native_count <= PLANKAC_MAX_NATIVE) {
        native_count = program->native_count;
        for (native_i = 0; native_i < native_count; ++native_i) {
            native_copy[native_i] = program->natives[native_i];
        }
    }
    plc_program_free(program);
    for (native_i = 0; native_i < native_count; ++native_i) {
        program->natives[native_i] = native_copy[native_i];
    }
    program->native_count = native_count;
    in_proc = 0;
    current = 0;
    type_decl_count = 0;
    if (err_size > 0 && err != 0) {
        err[0] = '\0';
    }

    if (sources == 0 || sources[0] == 0) {
        plc_set_error(err, err_size, "no source files supplied");
        return 0;
    }

    for (file_index = 0; sources[file_index] != 0; ++file_index) {
        FILE *fp;
        char line[PLC_MAX_LINE];
        int line_no;

        fp = fopen(sources[file_index], "r");
        if (fp == 0) {
            sprintf(err, "missing source: %s", sources[file_index]);
            return 0;
        }
        ++program->source_count;
        line_no = 0;
        while (fgets(line, sizeof(line), fp) != 0) {
            ++line_no;
            plc_strip_comment(line);
            if (line[0] == '\0') {
                continue;
            }
            if (!plc_validate_type_markers(line, err, err_size)) {
                char prefix[160];

                sprintf(prefix, "%s:%d ", sources[file_index], line_no);
                plc_prefix_error(err, err_size, prefix);
                fclose(fp);
                return 0;
            }
            if (plc_is_program_line(line)) {
                PLC_PROC proc;
                int i;

                if (in_proc) {
                    sprintf(err, "%s:%d nested procedure header",
                        sources[file_index], line_no);
                    fclose(fp);
                    return 0;
                }
                if (!plc_parse_header(line, &proc, err, err_size)) {
                    char prefix[160];

                    sprintf(prefix, "%s:%d ",
                        sources[file_index], line_no);
                    plc_prefix_error(err, err_size, prefix);
                    fclose(fp);
                    plc_proc_free(&proc);
                    return 0;
                }
                for (i = 0; i < program->proc_count; ++i) {
                    if (program->procs[i].number == proc.number
                            || strcmp(program->procs[i].name, proc.name) == 0) {
                        sprintf(err, "%s:%d duplicate procedure: %s",
                            sources[file_index], line_no, proc.name);
                        fclose(fp);
                        plc_proc_free(&proc);
                        return 0;
                    }
                }
                if (plc_find_native(program, proc.name) != 0) {
                    sprintf(err, "%s:%d duplicate native/procedure: %s",
                        sources[file_index], line_no, proc.name);
                    fclose(fp);
                    plc_proc_free(&proc);
                    return 0;
                }
                if (program->proc_count >= PLC_MAX_PROCS) {
                    sprintf(err, "too many procedures");
                    fclose(fp);
                    plc_proc_free(&proc);
                    return 0;
                }
                program->procs[program->proc_count] = proc;
                current = &program->procs[program->proc_count];
                ++program->proc_count;
                type_decl_count = 0;
                if (!plc_scan_typed_refs(line, type_decls,
                        &type_decl_count, err, err_size)) {
                    char prefix[160];

                    sprintf(prefix, "%s:%d ",
                        sources[file_index], line_no);
                    plc_prefix_error(err, err_size, prefix);
                    fclose(fp);
                    return 0;
                }
                in_proc = 1;
                continue;
            }
            if (plc_is_end_line(line)) {
                if (!in_proc) {
                    sprintf(err, "%s:%d stray END",
                        sources[file_index], line_no);
                    fclose(fp);
                    return 0;
                }
                in_proc = 0;
                current = 0;
                continue;
            }
            if (in_proc && strcmp(line, "PAGE") == 0) {
                char page_rows[PLC_MAX_STMTS][PLC_MAX_LINE];
                char page_stmts[PLC_MAX_STMTS][PLC_MAX_LINE];
                int page_row_count;
                int page_stmt_count;
                int page_start_line;
                int closed;
                int ps;

                page_row_count = 0;
                page_stmt_count = 0;
                page_start_line = line_no;
                closed = 0;
                while (fgets(line, sizeof(line), fp) != 0) {
                    char trimmed[PLC_MAX_LINE];

                    ++line_no;
                    plc_strip_comment_keep_left(line);
                    strncpy(trimmed, line, sizeof(trimmed) - 1);
                    trimmed[sizeof(trimmed) - 1] = '\0';
                    plc_trim_in_place(trimmed);
                    if (strcmp(trimmed, "ENDPAGE") == 0) {
                        closed = 1;
                        break;
                    }
                    if (page_row_count >= PLC_MAX_STMTS) {
                        sprintf(err, "%s:%d too many PAGE rows",
                            sources[file_index], line_no);
                        fclose(fp);
                        return 0;
                    }
                    strncpy(page_rows[page_row_count], line,
                        PLC_MAX_LINE - 1);
                    page_rows[page_row_count][PLC_MAX_LINE - 1] = '\0';
                    ++page_row_count;
                }
                if (!closed) {
                    sprintf(err, "%s:%d PAGE without ENDPAGE",
                        sources[file_index], page_start_line);
                    fclose(fp);
                    return 0;
                }
                if (!plc_expand_2d_page(page_rows, page_row_count,
                        page_stmts, &page_stmt_count, PLC_MAX_STMTS,
                        err, err_size)) {
                    char prefix[160];

                    sprintf(prefix, "%s:%d ",
                        sources[file_index], page_start_line);
                    plc_prefix_error(err, err_size, prefix);
                    fclose(fp);
                    return 0;
                }
                for (ps = 0; ps < page_stmt_count; ++ps) {
                    if (!plc_validate_type_markers(page_stmts[ps],
                            err, err_size)
                            || !plc_scan_typed_refs(page_stmts[ps],
                                type_decls, &type_decl_count,
                                err, err_size)) {
                        char prefix[160];

                        sprintf(prefix, "%s:%d ",
                            sources[file_index], page_start_line);
                        plc_prefix_error(err, err_size, prefix);
                        fclose(fp);
                        return 0;
                    }
                    if (!plc_proc_add_stmt(current, page_stmts[ps],
                            page_start_line, err, err_size)) {
                        char prefix[160];

                        sprintf(prefix, "%s:%d %s: ",
                            sources[file_index], page_start_line,
                            current->name);
                        plc_prefix_error(err, err_size, prefix);
                        fclose(fp);
                        return 0;
                    }
                }
                continue;
            }
            if (in_proc && (strncmp(line, "V|", 2) == 0
                    || strncmp(line, "S|", 2) == 0)) {
                continue;
            }
            if (in_proc && line[0] == '|') {
                char *body;
                long pos;
                char expr_copy[PLC_MAX_LINE];
                char vrow[PLC_MAX_LINE];
                char srow[PLC_MAX_LINE];

                body = plc_ltrim(line + 1);
                strncpy(expr_copy, body, sizeof(expr_copy) - 1);
                expr_copy[sizeof(expr_copy) - 1] = '\0';
                pos = ftell(fp);
                vrow[0] = '\0';
                srow[0] = '\0';
                if (fgets(vrow, sizeof(vrow), fp) != 0) {
                    char first_row[PLC_MAX_LINE];
                    char second_row[PLC_MAX_LINE];

                    ++line_no;
                    plc_strip_comment(vrow);
                    strncpy(first_row, vrow, sizeof(first_row) - 1);
                    first_row[sizeof(first_row) - 1] = '\0';
                    if ((strncmp(first_row, "V|", 2) == 0
                            || strncmp(first_row, "S|", 2) == 0)
                            && fgets(second_row, sizeof(second_row), fp) != 0) {
                        ++line_no;
                        plc_strip_comment(second_row);
                        if (strncmp(first_row, "V|", 2) == 0
                                && strncmp(second_row, "S|", 2) == 0) {
                            strncpy(vrow, first_row, sizeof(vrow) - 1);
                            vrow[sizeof(vrow) - 1] = '\0';
                            strncpy(srow, second_row, sizeof(srow) - 1);
                            srow[sizeof(srow) - 1] = '\0';
                        } else if (strncmp(first_row, "S|", 2) == 0
                                && strncmp(second_row, "V|", 2) == 0) {
                            strncpy(srow, first_row, sizeof(srow) - 1);
                            srow[sizeof(srow) - 1] = '\0';
                            strncpy(vrow, second_row, sizeof(vrow) - 1);
                            vrow[sizeof(vrow) - 1] = '\0';
                        } else {
                            sprintf(err, "%s:%d expected V|/S| rows",
                                sources[file_index], line_no);
                            fclose(fp);
                            return 0;
                        }
                        if (vrow[0] != '\0' && srow[0] != '\0') {
                            if (!plc_expand_2d_statement(expr_copy, vrow + 2,
                                    srow + 2, line, sizeof(line),
                                    err, err_size)) {
                                char prefix[160];

                                sprintf(prefix, "%s:%d ",
                                    sources[file_index], line_no);
                                plc_prefix_error(err, err_size, prefix);
                                fclose(fp);
                                return 0;
                            }
                        }
                    } else {
                        fseek(fp, pos, SEEK_SET);
                        --line_no;
                        memmove(line, body, strlen(body) + 1);
                        plc_trim_in_place(line);
                    }
                } else {
                    memmove(line, body, strlen(body) + 1);
                    plc_trim_in_place(line);
                }
                if (line[0] == '\0') {
                    continue;
                }
            }
            if (in_proc && current != 0) {
                if (!plc_scan_typed_refs(line, type_decls,
                        &type_decl_count, err, err_size)) {
                    char prefix[160];

                    sprintf(prefix, "%s:%d ",
                        sources[file_index], line_no);
                    plc_prefix_error(err, err_size, prefix);
                    fclose(fp);
                    return 0;
                }
                if (!plc_proc_add_stmt(current, line, line_no,
                        err, err_size)) {
                    char prefix[160];

                    sprintf(prefix, "%s:%d %s: ",
                        sources[file_index], line_no, current->name);
                    plc_prefix_error(err, err_size, prefix);
                    fclose(fp);
                    return 0;
                }
                continue;
            }
            sprintf(err, "%s:%d source outside procedure: %s",
                sources[file_index], line_no, line);
            fclose(fp);
            return 0;
        }
        fclose(fp);
    }

    if (in_proc) {
        sprintf(err, "source ended inside procedure");
        return 0;
    }
    if (!plc_validate_no_recursion(program, err, err_size)) {
        return 0;
    }
    if (!plc_validate_call_types(program, err, err_size)) {
        return 0;
    }
    if (!plc_analyze_program(program, err, err_size)) {
        return 0;
    }
    {
        static PLC_IR_PROGRAM ir;

        if (!plc_ir_build_program(program, &ir, err, err_size)
                || !plc_ir_validate_program(&ir, err, err_size)) {
            return 0;
        }
    }
    return 1;
}

int plc_load_program(PLC_PROGRAM *program, char *err,
    unsigned err_size)
{
    return plc_load_sources(program, PLC_SOURCES, err, err_size);
}

static int plc_read_bytecode_quoted(const char **pp, char *out,
    unsigned out_size)
{
    const char *p;
    unsigned n;

    p = plc_skip_space(*pp);
    if (*p != '"') {
        return 0;
    }
    ++p;
    n = 0;
    while (*p != '\0' && *p != '"') {
        char c;

        c = *p;
        if (c == '\\' && p[1] != '\0') {
            ++p;
            c = *p;
        }
        if (n + 1 < out_size) {
            out[n] = c;
            ++n;
        }
        ++p;
    }
    if (*p != '"') {
        return 0;
    }
    out[n] = '\0';
    *pp = p + 1;
    return 1;
}

static int plc_read_bytecode_arrow(const char **pp)
{
    const char *p;

    p = plc_skip_space(*pp);
    if (p[0] != '-' || p[1] != '>') {
        return 0;
    }
    *pp = p + 2;
    return 1;
}

static int plc_bytecode_stmt(PLC_PROC *proc, int line_no,
    const char *text, char *err, unsigned err_size)
{
    return plc_proc_add_stmt(proc, text, line_no, err, err_size);
}

static int plc_stmt_append(char *stmt, unsigned stmt_size,
    const char *part, char *err, unsigned err_size)
{
    unsigned have;
    unsigned add;

    have = (unsigned)strlen(stmt);
    add = (unsigned)strlen(part);
    if (have + add + 1 >= stmt_size) {
        plc_set_error(err, err_size, "bytecode statement too long");
        return 0;
    }
    strcat(stmt, part);
    return 1;
}

static int plc_parse_bytecode_statement(PLC_PROC *proc, const char *line,
    int line_no, char *err, unsigned err_size)
{
    const char *p;
    char guard[PLC_MAX_LINE];
    char expr[PLC_MAX_LINE];
    char target[PLC_MAX_LINE];
    char args[PLC_MAX_LINE];
    char name[PLC_MAX_NAME];
    char stmt[PLC_MAX_LINE];
    unsigned n;

    p = plc_skip_space(line);
    if (strncmp(p, "EVAL", 4) == 0 && isspace((unsigned char)p[4])) {
        p += 4;
        if (!plc_read_bytecode_quoted(&p, expr, sizeof(expr))
                || !plc_read_bytecode_arrow(&p)
                || !plc_read_bytecode_quoted(&p, target, sizeof(target))) {
            plc_set_error(err, err_size, "bad EVAL bytecode");
            return 0;
        }
        stmt[0] = '\0';
        if (!plc_stmt_append(stmt, sizeof(stmt), expr, err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), " => ",
                    err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), target,
                    err, err_size)) {
            return 0;
        }
        return plc_bytecode_stmt(proc, line_no, stmt, err, err_size);
    }
    if (strncmp(p, "CALL", 4) == 0 && isspace((unsigned char)p[4])) {
        p = plc_skip_space(p + 4);
        n = 0;
        while ((isalnum((unsigned char)*p) || *p == '_')
                && n + 1 < sizeof(name)) {
            name[n] = *p;
            ++n;
            ++p;
        }
        name[n] = '\0';
        if (name[0] == '\0'
                || !plc_read_bytecode_quoted(&p, args, sizeof(args))
                || !plc_read_bytecode_arrow(&p)
                || !plc_read_bytecode_quoted(&p, target, sizeof(target))) {
            plc_set_error(err, err_size, "bad CALL bytecode");
            return 0;
        }
        stmt[0] = '\0';
        if (!plc_stmt_append(stmt, sizeof(stmt), name, err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), "(", err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), args, err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), ") => ",
                    err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), target,
                    err, err_size)) {
            return 0;
        }
        return plc_bytecode_stmt(proc, line_no, stmt, err, err_size);
    }
    if (strncmp(p, "GEVAL", 5) == 0 && isspace((unsigned char)p[5])) {
        p += 5;
        if (!plc_read_bytecode_quoted(&p, guard, sizeof(guard))
                || !plc_read_bytecode_quoted(&p, expr, sizeof(expr))
                || !plc_read_bytecode_arrow(&p)
                || !plc_read_bytecode_quoted(&p, target, sizeof(target))) {
            plc_set_error(err, err_size, "bad GEVAL bytecode");
            return 0;
        }
        stmt[0] = '\0';
        if (!plc_stmt_append(stmt, sizeof(stmt), guard, err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), " => ",
                    err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), expr, err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), " => ",
                    err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), target,
                    err, err_size)) {
            return 0;
        }
        return plc_bytecode_stmt(proc, line_no, stmt, err, err_size);
    }
    if (strncmp(p, "GCALL", 5) == 0 && isspace((unsigned char)p[5])) {
        p = plc_skip_space(p + 5);
        if (!plc_read_bytecode_quoted(&p, guard, sizeof(guard))) {
            plc_set_error(err, err_size, "bad GCALL guard");
            return 0;
        }
        p = plc_skip_space(p);
        n = 0;
        while ((isalnum((unsigned char)*p) || *p == '_')
                && n + 1 < sizeof(name)) {
            name[n] = *p;
            ++n;
            ++p;
        }
        name[n] = '\0';
        if (name[0] == '\0'
                || !plc_read_bytecode_quoted(&p, args, sizeof(args))
                || !plc_read_bytecode_arrow(&p)
                || !plc_read_bytecode_quoted(&p, target, sizeof(target))) {
            plc_set_error(err, err_size, "bad GCALL bytecode");
            return 0;
        }
        stmt[0] = '\0';
        if (!plc_stmt_append(stmt, sizeof(stmt), guard, err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), " => ",
                    err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), name, err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), "(", err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), args, err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), ") => ",
                    err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), target,
                    err, err_size)) {
            return 0;
        }
        return plc_bytecode_stmt(proc, line_no, stmt, err, err_size);
    }
    if (strncmp(p, "CONST", 5) == 0 && isspace((unsigned char)p[5])) {
        p += 5;
        if (!plc_read_bytecode_quoted(&p, target, sizeof(target))) {
            plc_set_error(err, err_size, "bad CONST target");
            return 0;
        }
        p = plc_skip_space(p);
        if (*p != '=') {
            plc_set_error(err, err_size, "bad CONST bytecode");
            return 0;
        }
        ++p;
        if (!plc_read_bytecode_quoted(&p, expr, sizeof(expr))) {
            plc_set_error(err, err_size, "bad CONST expression");
            return 0;
        }
        stmt[0] = '\0';
        if (!plc_stmt_append(stmt, sizeof(stmt), "CONST ", err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), target, err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), " = ",
                    err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), expr,
                    err, err_size)) {
            return 0;
        }
        return plc_bytecode_stmt(proc, line_no, stmt, err, err_size);
    }
    if ((strncmp(p, "ASSERT", 6) == 0 && isspace((unsigned char)p[6]))
            || (strncmp(p, "REQUIRE", 7) == 0
                && isspace((unsigned char)p[7]))
            || (strncmp(p, "ENSURE", 6) == 0
                && isspace((unsigned char)p[6]))
            || (strncmp(p, "STOPIF", 6) == 0
                && isspace((unsigned char)p[6]))) {
        char keyword[16];
        int keyword_len;

        keyword_len = 6;
        strcpy(keyword, "ASSERT");
        if (strncmp(p, "REQUIRE", 7) == 0) {
            keyword_len = 7;
            strcpy(keyword, "REQUIRE");
        } else if (strncmp(p, "ENSURE", 6) == 0) {
            keyword_len = 6;
            strcpy(keyword, "ENSURE");
        } else if (strncmp(p, "STOPIF", 6) == 0) {
            keyword_len = 6;
            strcpy(keyword, "STOPIF");
        }
        p += keyword_len;
        if (!plc_read_bytecode_quoted(&p, expr, sizeof(expr))) {
            plc_set_error(err, err_size, "bad predicate bytecode");
            return 0;
        }
        stmt[0] = '\0';
        if (!plc_stmt_append(stmt, sizeof(stmt), keyword, err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), " ", err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), expr,
                    err, err_size)) {
            return 0;
        }
        return plc_bytecode_stmt(proc, line_no, stmt, err, err_size);
    }
    if (strncmp(p, "LOOP", 4) == 0 && isspace((unsigned char)p[4])) {
        p += 4;
        if (!plc_read_bytecode_quoted(&p, guard, sizeof(guard))
                || !plc_read_bytecode_quoted(&p, expr, sizeof(expr))
                || !plc_read_bytecode_arrow(&p)
                || !plc_read_bytecode_quoted(&p, target, sizeof(target))) {
            plc_set_error(err, err_size, "bad LOOP bytecode");
            return 0;
        }
        stmt[0] = '\0';
        if (!plc_stmt_append(stmt, sizeof(stmt), "LOOP ", err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), guard, err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), " => ",
                    err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), expr, err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), " => ",
                    err, err_size)
                || !plc_stmt_append(stmt, sizeof(stmt), target,
                    err, err_size)) {
            return 0;
        }
        return plc_bytecode_stmt(proc, line_no, stmt, err, err_size);
    }
    plc_set_error(err, err_size, "unknown bytecode operation");
    return 0;
}

static int plc_process_bytecode_line(PLC_PROGRAM *program, PLC_PROC **current,
    const char *origin, int line_no, char *line, char *err, unsigned err_size)
{
    char name[PLC_MAX_NAME];
    int number;
    int argc;
    int results;

    plc_trim_in_place(line);
    if (line[0] == '\0') {
        return 1;
    }
    if (strncmp(line, "PLANKAC-BYTECODE", 16) == 0
            || strncmp(line, "SOURCES ", 8) == 0
            || strncmp(line, "PROCEDURES ", 11) == 0) {
        return 1;
    }
    if (strncmp(line, "PROC ", 5) == 0) {
        if (*current != 0) {
            sprintf(err, "%s:%d nested bytecode PROC", origin, line_no);
            return 0;
        }
        if (program->proc_count >= PLC_MAX_PROCS) {
            sprintf(err, "%s:%d too many bytecode procedures",
                origin, line_no);
            return 0;
        }
        name[0] = '\0';
        if (sscanf(line, "PROC P%d %63s ARGC %d RESULTS %d",
                &number, name, &argc, &results) != 4) {
            sprintf(err, "%s:%d bad bytecode PROC", origin, line_no);
            return 0;
        }
        *current = &program->procs[program->proc_count];
        memset(*current, 0, sizeof(**current));
        (*current)->number = number;
        strncpy((*current)->name, name, sizeof((*current)->name) - 1);
        (*current)->argc = argc;
        (*current)->results = results;
        ++program->proc_count;
        return 1;
    }
    if (strcmp(line, "END") == 0) {
        if (*current == 0) {
            sprintf(err, "%s:%d stray bytecode END", origin, line_no);
            return 0;
        }
        *current = 0;
        return 1;
    }
    if (*current == 0) {
        sprintf(err, "%s:%d bytecode outside procedure", origin, line_no);
        return 0;
    }
    if (!plc_parse_bytecode_statement(*current, line,
            line_no, err, err_size)) {
        char prefix[160];

        sprintf(prefix, "%s:%d ", origin, line_no);
        plc_prefix_error(err, err_size, prefix);
        return 0;
    }
    return 1;
}

int plc_load_bytecode(PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size)
{
    FILE *fp;
    PLC_PROC *current;
    char line[PLC_MAX_LINE];
    int line_no;
    PLC_NATIVE_PROC native_copy[PLANKAC_MAX_NATIVE];
    int native_count;
    int native_i;

    native_count = 0;
    if (program != 0 && program->native_count > 0
            && program->native_count <= PLANKAC_MAX_NATIVE) {
        native_count = program->native_count;
        for (native_i = 0; native_i < native_count; ++native_i) {
            native_copy[native_i] = program->natives[native_i];
        }
    }
    plc_program_free(program);
    for (native_i = 0; native_i < native_count; ++native_i) {
        program->natives[native_i] = native_copy[native_i];
    }
    program->native_count = native_count;
    fp = fopen(path, "r");
    if (fp == 0) {
        sprintf(err, "cannot open bytecode input: %s", path);
        return 0;
    }
    current = 0;
    line_no = 0;
    while (fgets(line, sizeof(line), fp) != 0) {
        ++line_no;
        if (!plc_process_bytecode_line(program, &current, path, line_no,
                line, err, err_size)) {
            fclose(fp);
            return 0;
        }
    }
    fclose(fp);
    if (current != 0) {
        plc_set_error(err, err_size, "bytecode ended inside procedure");
        return 0;
    }
    program->source_count = 1;
    return 1;
}

int plc_load_bytecode_text(PLC_PROGRAM *program, const char *text,
    char *err, unsigned err_size)
{
    PLC_PROC *current;
    char line[PLC_MAX_LINE];
    int line_no;
    PLC_NATIVE_PROC native_copy[PLANKAC_MAX_NATIVE];
    int native_count;
    int native_i;

    if (text == 0) {
        plc_set_error(err, err_size, "missing bytecode text");
        return 0;
    }
    native_count = 0;
    if (program != 0 && program->native_count > 0
            && program->native_count <= PLANKAC_MAX_NATIVE) {
        native_count = program->native_count;
        for (native_i = 0; native_i < native_count; ++native_i) {
            native_copy[native_i] = program->natives[native_i];
        }
    }
    plc_program_free(program);
    for (native_i = 0; native_i < native_count; ++native_i) {
        program->natives[native_i] = native_copy[native_i];
    }
    program->native_count = native_count;
    current = 0;
    line_no = 0;
    while (*text != '\0') {
        unsigned n;

        n = 0;
        while (text[n] != '\0' && text[n] != '\n') {
            if (n + 1 >= sizeof(line)) {
                plc_set_error(err, err_size, "bytecode line too long");
                return 0;
            }
            line[n] = text[n];
            ++n;
        }
        line[n] = '\0';
        ++line_no;
        if (!plc_process_bytecode_line(program, &current, "<bytecode>",
                line_no, line, err, err_size)) {
            return 0;
        }
        text += n;
        if (*text == '\n') {
            ++text;
        }
    }
    if (current != 0) {
        plc_set_error(err, err_size, "bytecode ended inside procedure");
        return 0;
    }
    program->source_count = 1;
    return 1;
}
