#include "plankac_internal.h"

static int plc_ast_count_commas(const char *text)
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
        } else if (*text == ')' && depth > 0) {
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

static void plc_ast_trim_copy(char *out, unsigned out_size, const char *text)
{
    strncpy(out, text, out_size - 1);
    out[out_size - 1] = '\0';
    plc_trim_in_place(out);
}

typedef struct PLC_EXPR_PARSER {
    const char *p;
    PLC_EXPR_AST_SUMMARY *summary;
} PLC_EXPR_PARSER;

static int plc_expr_is_ident_char(int c)
{
    return isalnum((unsigned char)c) || c == '_';
}

static void plc_expr_ast_clear(PLC_EXPR_AST_SUMMARY *summary)
{
    if (summary != 0) {
        memset(summary, 0, sizeof(*summary));
    }
}

static int plc_expr_ast_note(PLC_EXPR_AST_SUMMARY *summary, int kind,
    int depth)
{
    if (summary == 0) {
        return kind;
    }
    if (summary->root_kind == PLC_EXPR_AST_EMPTY) {
        summary->root_kind = kind;
    }
    ++summary->node_count;
    if (depth > summary->max_depth) {
        summary->max_depth = depth;
    }
    if (kind == PLC_EXPR_AST_CALL) {
        ++summary->call_count;
    } else if (kind == PLC_EXPR_AST_REF) {
        ++summary->ref_count;
    } else if (kind == PLC_EXPR_AST_LITERAL) {
        ++summary->literal_count;
    } else if (kind == PLC_EXPR_AST_PREDICATE) {
        ++summary->predicate_count;
    } else if (kind == PLC_EXPR_AST_BINARY
            || kind == PLC_EXPR_AST_UNARY) {
        ++summary->operator_count;
    } else if (kind == PLC_EXPR_AST_UNKNOWN) {
        ++summary->unknown_count;
    }
    return kind;
}

static int plc_expr_keyword_at(const char *p, const char *keyword)
{
    unsigned n;

    n = (unsigned)strlen(keyword);
    if (strncmp(p, keyword, n) != 0) {
        return 0;
    }
    return !plc_expr_is_ident_char((unsigned char)p[n]);
}

static int plc_expr_is_predicate_keyword(const char *p, unsigned *len)
{
    static const char *keywords[] = {
        "DOMAINSELECT", "DOMAINCOUNT", "DOMAINEXISTS", "DOMAINFORALL",
        "RANGESELECT", "RANGECOUNT", "RANGEEXISTS", "RANGEFORALL",
        "SETSELECT", "SETCOUNT", "SETEXISTS", "SETFORALL",
        "SELECT", "COUNT", "EXISTS", "FORALL",
        0
    };
    int i;

    for (i = 0; keywords[i] != 0; ++i) {
        if (plc_expr_keyword_at(p, keywords[i])) {
            if (len != 0) {
                *len = (unsigned)strlen(keywords[i]);
            }
            return 1;
        }
    }
    return 0;
}

static int plc_expr_parse_ref(const char **pp)
{
    const char *p;
    char *endptr;

    p = plc_skip_space(*pp);
    if (*p != 'V' && *p != 'C' && *p != 'Z' && *p != 'R') {
        return 0;
    }
    ++p;
    if (!isdigit((unsigned char)*p)) {
        return 0;
    }
    (void)strtol(p, &endptr, 10);
    p = plc_skip_space(endptr);
    while (*p == '[' && p[1] != ':') {
        ++p;
        p = plc_skip_space(p);
        if (!isdigit((unsigned char)*p)) {
            return 0;
        }
        (void)strtol(p, &endptr, 10);
        p = plc_skip_space(endptr);
        if (*p == ',') {
            ++p;
            p = plc_skip_space(p);
            if (!isdigit((unsigned char)*p)) {
                return 0;
            }
            (void)strtol(p, &endptr, 10);
            p = plc_skip_space(endptr);
        }
        if (*p != ']') {
            return 0;
        }
        ++p;
        p = plc_skip_space(p);
    }
    while (*p == '.') {
        ++p;
        if (!isalpha((unsigned char)*p) && *p != '_') {
            return 0;
        }
        while (plc_expr_is_ident_char((unsigned char)*p)) {
            ++p;
        }
        p = plc_skip_space(p);
    }
    p = plc_skip_type_marker(p);
    *pp = p;
    return 1;
}

static int plc_expr_op_at(const char *p, int *len)
{
    p = plc_skip_space(p);
    if (p[0] == '\0' || p[0] == ')' || p[0] == ',') {
        return 0;
    }
    if (p[0] == '=' && p[1] == '>') {
        return 0;
    }
    if ((p[0] == '!' || p[0] == '<' || p[0] == '>')
            && p[1] == '=') {
        *len = 2;
        return 3;
    }
    if (p[0] == '=' || p[0] == '<' || p[0] == '>') {
        *len = 1;
        return 3;
    }
    if (p[0] == '|' ) {
        *len = 1;
        return 1;
    }
    if (p[0] == '&') {
        *len = 1;
        return 2;
    }
    if (p[0] == '+' || p[0] == '-') {
        *len = 1;
        return 4;
    }
    if (p[0] == '*' || p[0] == '/' || p[0] == '%') {
        *len = 1;
        return 5;
    }
    if (p[0] == '^') {
        *len = 1;
        return 6;
    }
    return 0;
}

static int plc_expr_parse_expression(PLC_EXPR_PARSER *parser, int min_prec,
    int depth);

static int plc_expr_parse_identifier(const char **pp, char *name,
    unsigned name_size)
{
    const char *p;
    unsigned n;

    p = plc_skip_space(*pp);
    if (!isalpha((unsigned char)*p) && *p != '_') {
        return 0;
    }
    n = 0;
    while (plc_expr_is_ident_char((unsigned char)*p)) {
        if (n + 1 < name_size) {
            name[n++] = *p;
        }
        ++p;
    }
    name[n] = '\0';
    *pp = p;
    return 1;
}

static int plc_expr_parse_primary(PLC_EXPR_PARSER *parser, int depth)
{
    const char *p;
    char *endptr;

    p = plc_skip_space(parser->p);
    if (*p == '(') {
        int child;

        parser->p = p + 1;
        child = plc_expr_parse_expression(parser, 1, depth + 1);
        p = plc_skip_space(parser->p);
        if (*p == ')') {
            parser->p = p + 1;
        } else {
            parser->p = p;
            plc_expr_ast_note(parser->summary, PLC_EXPR_AST_UNKNOWN, depth);
        }
        (void)child;
        return plc_expr_ast_note(parser->summary,
            PLC_EXPR_AST_GROUP, depth);
    }
    if (*p == '-' || *p == '!') {
        parser->p = p + 1;
        (void)plc_expr_parse_primary(parser, depth + 1);
        return plc_expr_ast_note(parser->summary,
            PLC_EXPR_AST_UNARY, depth);
    }
    if (isdigit((unsigned char)*p) || *p == '.') {
        (void)strtod(p, &endptr);
        if (endptr == p) {
            parser->p = p + 1;
            return plc_expr_ast_note(parser->summary,
                PLC_EXPR_AST_UNKNOWN, depth);
        }
        parser->p = plc_skip_type_marker(endptr);
        return plc_expr_ast_note(parser->summary,
            PLC_EXPR_AST_LITERAL, depth);
    }
    if (*p == 'V' || *p == 'C' || *p == 'Z' || *p == 'R') {
        const char *refp;

        refp = p;
        if (plc_expr_parse_ref(&refp)) {
            parser->p = refp;
            return plc_expr_ast_note(parser->summary,
                PLC_EXPR_AST_REF, depth);
        }
    }
    if (isalpha((unsigned char)*p) || *p == '_') {
        char name[PLC_MAX_NAME];

        if (plc_expr_parse_identifier(&p, name, sizeof(name))) {
            p = plc_skip_space(p);
            if (*p == '(') {
                parser->p = p + 1;
                p = plc_skip_space(parser->p);
                if (*p != ')') {
                    while (1) {
                        (void)plc_expr_parse_expression(parser, 1,
                            depth + 1);
                        p = plc_skip_space(parser->p);
                        if (*p == ',') {
                            parser->p = p + 1;
                            continue;
                        }
                        break;
                    }
                }
                p = plc_skip_space(parser->p);
                if (*p == ')') {
                    parser->p = p + 1;
                } else {
                    parser->p = p;
                    plc_expr_ast_note(parser->summary,
                        PLC_EXPR_AST_UNKNOWN, depth);
                }
                return plc_expr_ast_note(parser->summary,
                    PLC_EXPR_AST_CALL, depth);
            }
            parser->p = p;
            return plc_expr_ast_note(parser->summary,
                PLC_EXPR_AST_UNKNOWN, depth);
        }
    }
    if (*p != '\0') {
        parser->p = p + 1;
    } else {
        parser->p = p;
    }
    return plc_expr_ast_note(parser->summary, PLC_EXPR_AST_UNKNOWN, depth);
}

static int plc_expr_parse_expression(PLC_EXPR_PARSER *parser, int min_prec,
    int depth)
{
    int left;

    left = plc_expr_parse_primary(parser, depth);
    while (1) {
        const char *op_pos;
        int op_len;
        int prec;

        op_pos = plc_skip_space(parser->p);
        op_len = 0;
        prec = plc_expr_op_at(op_pos, &op_len);
        if (prec == 0 || prec < min_prec) {
            parser->p = op_pos;
            break;
        }
        parser->p = op_pos + op_len;
        (void)plc_expr_parse_expression(parser, prec + 1, depth + 1);
        left = plc_expr_ast_note(parser->summary,
            PLC_EXPR_AST_BINARY, depth);
    }
    return left;
}

static int plc_expr_parse_predicate(PLC_EXPR_PARSER *parser, int depth)
{
    const char *p;
    unsigned keyword_len;
    int op_len;

    p = plc_skip_space(parser->p);
    keyword_len = 0;
    if (!plc_expr_is_predicate_keyword(p, &keyword_len)) {
        return 0;
    }
    parser->p = plc_skip_space(p + keyword_len);
    (void)plc_expr_parse_expression(parser, 4, depth + 1);
    p = plc_skip_space(parser->p);
    op_len = 0;
    if (plc_expr_op_at(p, &op_len) == 3) {
        parser->p = plc_skip_space(p + op_len);
        (void)plc_expr_parse_expression(parser, 1, depth + 1);
    } else {
        parser->p = p;
        plc_expr_ast_note(parser->summary, PLC_EXPR_AST_UNKNOWN, depth);
    }
    return plc_expr_ast_note(parser->summary,
        PLC_EXPR_AST_PREDICATE, depth);
}

static void plc_expr_ast_analyze_text(const char *text,
    PLC_EXPR_AST_SUMMARY *summary)
{
    PLC_EXPR_PARSER parser;
    const char *p;
    int root;

    plc_expr_ast_clear(summary);
    if (text == 0) {
        return;
    }
    p = plc_skip_space(text);
    if (plc_line_starts_with(p, "LOOP")) {
        p = plc_skip_space(p + 4);
    }
    if (*p == '\0') {
        return;
    }
    parser.p = p;
    parser.summary = summary;
    root = plc_expr_parse_predicate(&parser, 1);
    if (!root) {
        root = plc_expr_parse_expression(&parser, 1, 1);
    }
    if (root != PLC_EXPR_AST_EMPTY) {
        summary->root_kind = root;
    }
    p = plc_skip_space(parser.p);
    if (*p != '\0' && *p != ')' && *p != ',') {
        plc_expr_ast_note(summary, PLC_EXPR_AST_UNKNOWN, 1);
    }
}

static void plc_expr_ast_analyze_target(const char *text,
    PLC_EXPR_AST_SUMMARY *summary)
{
    const char *p;

    plc_expr_ast_clear(summary);
    if (text == 0) {
        return;
    }
    p = text;
    summary->root_kind = PLC_EXPR_AST_TARGET_LIST;
    while (*p != '\0') {
        const char *candidate;

        candidate = plc_skip_space(p);
        if (plc_expr_parse_ref(&candidate)) {
            p = candidate;
            plc_expr_ast_note(summary, PLC_EXPR_AST_REF, 2);
        } else {
            while (*p != '\0' && *p != ',') {
                ++p;
            }
            plc_expr_ast_note(summary, PLC_EXPR_AST_UNKNOWN, 2);
        }
        p = plc_skip_space(p);
        if (*p == ',') {
            ++p;
        }
    }
    if (summary->node_count == 0) {
        summary->root_kind = PLC_EXPR_AST_EMPTY;
    }
}

static void plc_ast_analyze_statement_exprs(PLC_AST_STMT *stmt)
{
    if (stmt->guard[0] != '\0') {
        plc_expr_ast_analyze_text(stmt->guard, &stmt->guard_expr);
    } else {
        plc_expr_ast_clear(&stmt->guard_expr);
    }
    if (stmt->value[0] != '\0') {
        plc_expr_ast_analyze_text(stmt->value, &stmt->value_expr);
    } else {
        plc_expr_ast_clear(&stmt->value_expr);
    }
    if (stmt->target[0] != '\0') {
        plc_expr_ast_analyze_target(stmt->target, &stmt->target_expr);
    } else {
        plc_expr_ast_clear(&stmt->target_expr);
    }
}

static void plc_ast_set_call_shape(const PLC_PROGRAM *program,
    PLC_AST_STMT *stmt)
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
    stmt->argc = plc_ast_count_commas(args);
    proc = plc_find_proc(program, stmt->callee);
    if (proc != 0) {
        stmt->argc = proc->argc;
        stmt->results = proc->results;
        return;
    }
    native_proc = plc_find_native(program, stmt->callee);
    if (native_proc != 0) {
        stmt->argc = native_proc->argc;
        stmt->results = native_proc->results;
    }
}

static void plc_ast_call_op(PLC_AST_STMT *stmt)
{
    if (stmt->callee[0] == '\0') {
        return;
    }
    if (stmt->op == PLC_AST_OP_EVAL) {
        stmt->op = PLC_AST_OP_CALL;
    } else if (stmt->op == PLC_AST_OP_GUARD_EVAL) {
        stmt->op = PLC_AST_OP_GUARD_CALL;
    }
}

static int plc_ast_build_statement(const PLC_PROGRAM *program,
    const PLC_PROC *proc, const PLC_STMT *source, PLC_AST_STMT *stmt,
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
    plc_ast_trim_copy(text, sizeof(text), source->text);
    plc_ast_trim_copy(stmt->source, sizeof(stmt->source), text);

    if (plc_line_starts_with(text, "ASSERT")) {
        stmt->op = PLC_AST_OP_ASSERT;
        plc_ast_trim_copy(stmt->guard, sizeof(stmt->guard), text + 6);
        plc_ast_analyze_statement_exprs(stmt);
        return 1;
    }
    if (plc_line_starts_with(text, "REQUIRE")) {
        stmt->op = PLC_AST_OP_REQUIRE;
        plc_ast_trim_copy(stmt->guard, sizeof(stmt->guard), text + 7);
        plc_ast_analyze_statement_exprs(stmt);
        return 1;
    }
    if (plc_line_starts_with(text, "ENSURE")) {
        stmt->op = PLC_AST_OP_ENSURE;
        plc_ast_trim_copy(stmt->guard, sizeof(stmt->guard), text + 6);
        plc_ast_analyze_statement_exprs(stmt);
        return 1;
    }
    if (plc_line_starts_with(text, "STOPIF")) {
        stmt->op = PLC_AST_OP_STOPIF;
        plc_ast_trim_copy(stmt->guard, sizeof(stmt->guard), text + 6);
        plc_ast_analyze_statement_exprs(stmt);
        return 1;
    }
    if (plc_line_starts_with(text, "CONST")) {
        const char *eq;

        stmt->op = PLC_AST_OP_CONST;
        eq = strchr(text, '=');
        if (eq != 0) {
            plc_copy_range(stmt->target, sizeof(stmt->target), text + 5, eq);
            plc_trim_in_place(stmt->target);
            plc_ast_trim_copy(stmt->value, sizeof(stmt->value), eq + 1);
            stmt->results = 1;
        }
        plc_ast_analyze_statement_exprs(stmt);
        return 1;
    }

    if (!plc_split_arrows(text, parts, &part_count)) {
        stmt->op = PLC_AST_OP_EVAL;
        plc_ast_trim_copy(stmt->value, sizeof(stmt->value), text);
        stmt->arrow_count = 0;
        plc_ast_set_call_shape(program, stmt);
        plc_ast_call_op(stmt);
        plc_ast_analyze_statement_exprs(stmt);
        return 1;
    }

    stmt->arrow_count = part_count - 1;
    if (part_count == 2) {
        stmt->op = PLC_AST_OP_EVAL;
        plc_ast_trim_copy(stmt->value, sizeof(stmt->value), parts[0]);
        plc_ast_trim_copy(stmt->target, sizeof(stmt->target), parts[1]);
        stmt->results = plc_ast_count_commas(stmt->target);
    } else if (part_count == 3) {
        plc_ast_trim_copy(stmt->guard, sizeof(stmt->guard), parts[0]);
        plc_ast_trim_copy(stmt->value, sizeof(stmt->value), parts[1]);
        plc_ast_trim_copy(stmt->target, sizeof(stmt->target), parts[2]);
        stmt->op = plc_line_starts_with(parts[0], "LOOP")
            ? PLC_AST_OP_LOOP : PLC_AST_OP_GUARD_EVAL;
        stmt->results = plc_ast_count_commas(stmt->target);
    } else {
        sprintf(err, "P%d %s line %d: AST expected assignment",
            proc->number, proc->name, source->line_no);
        return 0;
    }
    plc_ast_set_call_shape(program, stmt);
    plc_ast_call_op(stmt);
    plc_ast_analyze_statement_exprs(stmt);
    return 1;
}

int plc_ast_build_program(const PLC_PROGRAM *program, PLC_AST_PROGRAM *ast,
    char *err, unsigned err_size)
{
    int i;
    int s;

    if (program == 0 || ast == 0) {
        plc_set_error(err, err_size, "missing AST input");
        return 0;
    }
    memset(ast, 0, sizeof(*ast));
    ast->proc_count = program->proc_count;
    ast->source_count = program->source_count;
    for (i = 0; i < program->proc_count; ++i) {
        for (s = 0; s < program->procs[i].stmt_count; ++s) {
            if (ast->stmt_count >= PLC_IR_MAX_STMTS) {
                plc_set_error(err, err_size, "too many AST statements");
                return 0;
            }
            if (!plc_ast_build_statement(program, &program->procs[i],
                    &program->procs[i].stmts[s],
                    &ast->stmts[ast->stmt_count], err, err_size)) {
                return 0;
            }
            ++ast->stmt_count;
        }
    }
    return 1;
}

int plc_ast_validate_program(const PLC_AST_PROGRAM *ast,
    char *err, unsigned err_size)
{
    int i;

    if (ast == 0) {
        plc_set_error(err, err_size, "missing AST program");
        return 0;
    }
    for (i = 0; i < ast->stmt_count; ++i) {
        const PLC_AST_STMT *stmt;

        stmt = &ast->stmts[i];
        if ((stmt->op == PLC_AST_OP_CALL
                || stmt->op == PLC_AST_OP_GUARD_CALL)
                && stmt->callee[0] == '\0') {
            plc_set_error(err, err_size, "AST call without callee");
            return 0;
        }
        if ((stmt->op == PLC_AST_OP_EVAL || stmt->op == PLC_AST_OP_CALL
                || stmt->op == PLC_AST_OP_GUARD_EVAL
                || stmt->op == PLC_AST_OP_GUARD_CALL
                || stmt->op == PLC_AST_OP_LOOP
                || stmt->op == PLC_AST_OP_CONST)
                && stmt->value[0] == '\0') {
            plc_set_error(err, err_size, "AST value is empty");
            return 0;
        }
        if ((stmt->op == PLC_AST_OP_GUARD_EVAL
                || stmt->op == PLC_AST_OP_GUARD_CALL
                || stmt->op == PLC_AST_OP_LOOP
                || stmt->op == PLC_AST_OP_ASSERT
                || stmt->op == PLC_AST_OP_REQUIRE
                || stmt->op == PLC_AST_OP_ENSURE
                || stmt->op == PLC_AST_OP_STOPIF)
                && stmt->guard[0] == '\0') {
            plc_set_error(err, err_size, "AST guard is empty");
            return 0;
        }
    }
    return 1;
}

const char *plc_ast_op_name(int op)
{
    if (op == PLC_AST_OP_CALL) {
        return "CALL";
    }
    if (op == PLC_AST_OP_GUARD_EVAL) {
        return "GUARD_EVAL";
    }
    if (op == PLC_AST_OP_GUARD_CALL) {
        return "GUARD_CALL";
    }
    if (op == PLC_AST_OP_LOOP) {
        return "LOOP";
    }
    if (op == PLC_AST_OP_ASSERT) {
        return "ASSERT";
    }
    if (op == PLC_AST_OP_REQUIRE) {
        return "REQUIRE";
    }
    if (op == PLC_AST_OP_ENSURE) {
        return "ENSURE";
    }
    if (op == PLC_AST_OP_STOPIF) {
        return "STOPIF";
    }
    if (op == PLC_AST_OP_CONST) {
        return "CONST";
    }
    return "EVAL";
}

const char *plc_expr_ast_kind_name(int kind)
{
    if (kind == PLC_EXPR_AST_LITERAL) {
        return "LITERAL";
    }
    if (kind == PLC_EXPR_AST_REF) {
        return "REF";
    }
    if (kind == PLC_EXPR_AST_CALL) {
        return "CALL";
    }
    if (kind == PLC_EXPR_AST_UNARY) {
        return "UNARY";
    }
    if (kind == PLC_EXPR_AST_BINARY) {
        return "BINARY";
    }
    if (kind == PLC_EXPR_AST_PREDICATE) {
        return "PREDICATE";
    }
    if (kind == PLC_EXPR_AST_GROUP) {
        return "GROUP";
    }
    if (kind == PLC_EXPR_AST_TARGET_LIST) {
        return "TARGET_LIST";
    }
    if (kind == PLC_EXPR_AST_UNKNOWN) {
        return "UNKNOWN";
    }
    return "EMPTY";
}

static int plc_expr_ast_summary_nodes(const PLC_EXPR_AST_SUMMARY *summary)
{
    if (summary == 0) {
        return 0;
    }
    return summary->node_count;
}

static void plc_ast_emit_expr_summary(FILE *out, const char *label,
    const PLC_EXPR_AST_SUMMARY *summary, const char *text)
{
    if (summary == 0 || summary->root_kind == PLC_EXPR_AST_EMPTY) {
        return;
    }
    fprintf(out,
        "  %s root=%s nodes=%d depth=%d calls=%d refs=%d literals=%d predicates=%d operators=%d unknown=%d\n",
        label, plc_expr_ast_kind_name(summary->root_kind),
        summary->node_count, summary->max_depth, summary->call_count,
        summary->ref_count, summary->literal_count,
        summary->predicate_count, summary->operator_count,
        summary->unknown_count);
    if (text != 0 && text[0] != '\0') {
        fprintf(out, "    text %s\n", text);
    }
}

int plc_emit_ast(const PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size)
{
    static PLC_AST_PROGRAM ast;
    FILE *out;
    int i;
    int expression_nodes;

    if (!plc_ast_build_program(program, &ast, err, err_size)
            || !plc_ast_validate_program(&ast, err, err_size)) {
        return 0;
    }
    out = fopen(path, "w");
    if (out == 0) {
        sprintf(err, "cannot open AST output: %s", path);
        return 0;
    }
    expression_nodes = 0;
    for (i = 0; i < ast.stmt_count; ++i) {
        expression_nodes += plc_expr_ast_summary_nodes(
            &ast.stmts[i].guard_expr);
        expression_nodes += plc_expr_ast_summary_nodes(
            &ast.stmts[i].value_expr);
        expression_nodes += plc_expr_ast_summary_nodes(
            &ast.stmts[i].target_expr);
    }
    fprintf(out, "PLANKAC-AST 1\n");
    fprintf(out, "sources %d\n", ast.source_count);
    fprintf(out, "procedures %d\n", ast.proc_count);
    fprintf(out, "statements %d\n", ast.stmt_count);
    fprintf(out, "expression_nodes %d\n\n", expression_nodes);
    for (i = 0; i < ast.stmt_count; ++i) {
        const PLC_AST_STMT *stmt;

        stmt = &ast.stmts[i];
        fprintf(out, "P%d %s line %d op=%s arrows=%d",
            stmt->proc_number, stmt->proc_name, stmt->source_line,
            plc_ast_op_name(stmt->op), stmt->arrow_count);
        if (stmt->callee[0] != '\0') {
            fprintf(out, " callee=%s argc=%d results=%d",
                stmt->callee, stmt->argc, stmt->results);
        }
        fprintf(out, "\n");
        plc_ast_emit_expr_summary(out, "guard",
            &stmt->guard_expr, stmt->guard);
        plc_ast_emit_expr_summary(out, "value",
            &stmt->value_expr, stmt->value);
        plc_ast_emit_expr_summary(out, "target",
            &stmt->target_expr, stmt->target);
    }
    fclose(out);
    plc_copy_error(err, err_size, "");
    return 1;
}
