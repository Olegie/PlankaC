#include "plankac_internal.h"

static double plc_parse_expr(PLC_PARSER *parser, int *ok);

static int plc_parse_ref(const char **pp, PLC_REF *ref)
{
    const char *p;
    const char *marker_end;
    char *endptr;
    int field_len;

    p = plc_skip_space(*pp);
    memset(ref, 0, sizeof(*ref));
    if (*p != 'V' && *p != 'C' && *p != 'Z' && *p != 'R') {
        return 0;
    }
    ref->bank = *p;
    ++p;
    if (!isdigit((unsigned char)*p)) {
        return 0;
    }
    ref->index = (int)strtol(p, &endptr, 10);
    p = plc_skip_space(endptr);
    while (*p == '[' && p[1] != ':') {
        int value;

        if (ref->subscript_count >= 2) {
            return 0;
        }
        ++p;
        p = plc_skip_space(p);
        if (!isdigit((unsigned char)*p)) {
            return 0;
        }
        value = (int)strtol(p, &endptr, 10);
        ref->has_subscript = 1;
        ref->subscripts[ref->subscript_count] = value;
        ++ref->subscript_count;
        p = plc_skip_space(endptr);
        if (*p == ',') {
            ++p;
            p = plc_skip_space(p);
            if (!isdigit((unsigned char)*p) || ref->subscript_count >= 2) {
                return 0;
            }
            value = (int)strtol(p, &endptr, 10);
            ref->subscripts[ref->subscript_count] = value;
            ++ref->subscript_count;
            p = plc_skip_space(endptr);
        }
        if (*p != ']') {
            return 0;
        }
        ++p;
    }
    if (ref->has_subscript) {
        if (ref->subscript_count == 1) {
            ref->subscript = ref->subscripts[0];
        } else if (ref->subscript_count == 2) {
            ref->subscript = ref->subscripts[0] * PLC_ARRAY_DIM
                + ref->subscripts[1];
        } else {
            return 0;
        }
    }
    p = plc_skip_space(p);
    if (*p == '.') {
        field_len = 0;
        while (*p == '.') {
            if (field_len + 1 < PLC_MAX_FIELD_NAME) {
                ref->field[field_len] = *p;
                ++field_len;
            }
            ++p;
            if (!isalpha((unsigned char)*p) && *p != '_') {
                return 0;
            }
            while (isalnum((unsigned char)*p) || *p == '_') {
                if (field_len + 1 < PLC_MAX_FIELD_NAME) {
                    ref->field[field_len] = *p;
                    ++field_len;
                }
                ++p;
            }
        }
        ref->field[field_len] = '\0';
        ref->has_field = 1;
    }
    p = plc_skip_space(p);
    if (p[0] == '[' && p[1] == ':') {
        marker_end = strchr(p, ']');
        if (marker_end == 0) {
            return 0;
        }
        plc_copy_range(ref->type_marker, sizeof(ref->type_marker),
            p, marker_end + 1);
    }
    p = plc_skip_type_marker(p);
    *pp = p;
    return 1;
}

static int plc_field_index(PLC_FRAME *frame, const char *name,
    int create, char *err, unsigned err_size)
{
    int i;

    for (i = 0; i < frame->field_count; ++i) {
        if (strcmp(frame->field_names[i], name) == 0) {
            return i;
        }
    }
    if (!create) {
        plc_set_error(err, err_size, "unknown record field");
        return -1;
    }
    if (frame->field_count >= PLC_MAX_FIELDS) {
        plc_set_error(err, err_size, "too many record fields");
        return -1;
    }
    strncpy(frame->field_names[frame->field_count], name,
        PLC_MAX_FIELD_NAME - 1);
    frame->field_names[frame->field_count][PLC_MAX_FIELD_NAME - 1] = '\0';
    ++frame->field_count;
    return frame->field_count - 1;
}

static double plc_get_ref(PLC_FRAME *frame, PLC_REF ref, int *ok,
    char *err, unsigned err_size)
{
    int field;
    PLC_TAGGED_VALUE tagged;

    if (ref.index < 0 || ref.index >= PLC_MAX_VARS) {
        plc_set_error(err, err_size, "variable index out of range");
        *ok = 0;
        return 0.0;
    }
    if (ref.has_subscript) {
        if (ref.subscript < 0 || ref.subscript >= PLC_MAX_INDEX) {
            plc_set_error(err, err_size, "array index out of range");
            *ok = 0;
            return 0.0;
        }
        if (plc_frame_load_tagged(frame, &ref, &tagged)) {
            return plc_tagged_to_double(&tagged);
        }
        if (ref.bank == 'V') {
            return frame->va[ref.index][ref.subscript];
        }
        if (ref.bank == 'C') {
            return frame->ca[ref.index][ref.subscript];
        }
        if (ref.bank == 'Z') {
            return frame->za[ref.index][ref.subscript];
        }
        return frame->ra[ref.index][ref.subscript];
    }
    if (ref.has_field) {
        field = plc_field_index(frame, ref.field, 0, err, err_size);
        if (field < 0) {
            *ok = 0;
            return 0.0;
        }
        if (plc_frame_load_tagged(frame, &ref, &tagged)) {
            return plc_tagged_to_double(&tagged);
        }
        if (ref.bank == 'V') {
            return frame->vf[ref.index][field];
        }
        if (ref.bank == 'C') {
            return frame->cf[ref.index][field];
        }
        if (ref.bank == 'Z') {
            return frame->zf[ref.index][field];
        }
        return frame->rf[ref.index][field];
    }
    if (plc_frame_load_tagged(frame, &ref, &tagged)) {
        return plc_tagged_to_double(&tagged);
    }
    if (ref.bank == 'V') {
        return frame->v[ref.index];
    }
    if (ref.bank == 'C') {
        return frame->c[ref.index];
    }
    if (ref.bank == 'Z') {
        return frame->z[ref.index];
    }
    return frame->r[ref.index];
}

static int plc_set_ref(PLC_FRAME *frame, PLC_REF ref, double value,
    char *err, unsigned err_size)
{
    int field;

    if (ref.index < 0 || ref.index >= PLC_MAX_VARS) {
        plc_set_error(err, err_size, "variable index out of range");
        return 0;
    }
    if (ref.bank == 'C') {
        plc_set_error(err, err_size, "C bank is constant");
        return 0;
    }
    if (ref.has_subscript) {
        if (ref.subscript < 0 || ref.subscript >= PLC_MAX_INDEX) {
            plc_set_error(err, err_size, "array index out of range");
            return 0;
        }
        if (ref.bank == 'V') {
            frame->va[ref.index][ref.subscript] = value;
        } else if (ref.bank == 'Z') {
            frame->za[ref.index][ref.subscript] = value;
        } else {
            frame->ra[ref.index][ref.subscript] = value;
        }
        return plc_frame_store_tagged(frame, &ref, value, ref.type_marker,
            err, err_size);
    }
    if (ref.has_field) {
        field = plc_field_index(frame, ref.field, 1, err, err_size);
        if (field < 0) {
            return 0;
        }
        if (ref.bank == 'V') {
            frame->vf[ref.index][field] = value;
        } else if (ref.bank == 'Z') {
            frame->zf[ref.index][field] = value;
        } else {
            frame->rf[ref.index][field] = value;
        }
        return plc_frame_store_tagged(frame, &ref, value, ref.type_marker,
            err, err_size);
    }
    if (ref.bank == 'V') {
        frame->v[ref.index] = value;
    } else if (ref.bank == 'Z') {
        frame->z[ref.index] = value;
    } else {
        frame->r[ref.index] = value;
    }
    return plc_frame_store_tagged(frame, &ref, value, ref.type_marker,
        err, err_size);
}

static int plc_set_const_ref(PLC_FRAME *frame, PLC_REF ref, double value,
    char *err, unsigned err_size)
{
    int field;

    if (ref.bank != 'C') {
        plc_set_error(err, err_size, "CONST target must use C bank");
        return 0;
    }
    if (ref.index < 0 || ref.index >= PLC_MAX_VARS) {
        plc_set_error(err, err_size, "constant index out of range");
        return 0;
    }
    if (ref.has_subscript) {
        if (ref.subscript < 0 || ref.subscript >= PLC_MAX_INDEX) {
            plc_set_error(err, err_size, "constant array index out of range");
            return 0;
        }
        frame->ca[ref.index][ref.subscript] = value;
        return plc_frame_store_tagged(frame, &ref, value, ref.type_marker,
            err, err_size);
    }
    if (ref.has_field) {
        field = plc_field_index(frame, ref.field, 1, err, err_size);
        if (field < 0) {
            return 0;
        }
        frame->cf[ref.index][field] = value;
        return plc_frame_store_tagged(frame, &ref, value, ref.type_marker,
            err, err_size);
    }
    frame->c[ref.index] = value;
    return plc_frame_store_tagged(frame, &ref, value, ref.type_marker,
        err, err_size);
}

static void plc_parser_skip(PLC_PARSER *parser)
{
    parser->p = plc_skip_space(parser->p);
}

static int plc_parse_identifier(PLC_PARSER *parser, char *name,
    unsigned name_size)
{
    unsigned n;

    plc_parser_skip(parser);
    if (!isalpha((unsigned char)*parser->p) && *parser->p != '_') {
        return 0;
    }
    n = 0;
    while (isalnum((unsigned char)*parser->p) || *parser->p == '_') {
        if (n + 1 < name_size) {
            name[n] = *parser->p;
            ++n;
        }
        ++parser->p;
    }
    name[n] = '\0';
    return 1;
}

static int plc_parse_call_args(PLC_PARSER *parser, double *args, int *argc,
    int *ok)
{
    plc_parser_skip(parser);
    if (*parser->p != '(') {
        plc_set_error(parser->err, parser->err_size, "expected '('");
        *ok = 0;
        return 0;
    }
    ++parser->p;
    *argc = 0;
    plc_parser_skip(parser);
    if (*parser->p == ')') {
        ++parser->p;
        return 1;
    }
    while (*parser->p != '\0') {
        if (*argc >= PLC_MAX_ARGS) {
            plc_set_error(parser->err, parser->err_size, "too many arguments");
            *ok = 0;
            return 0;
        }
        args[*argc] = plc_parse_expr(parser, ok);
        if (!*ok) {
            return 0;
        }
        ++(*argc);
        plc_parser_skip(parser);
        if (*parser->p == ',') {
            ++parser->p;
            continue;
        }
        if (*parser->p == ')') {
            ++parser->p;
            return 1;
        }
        plc_set_error(parser->err, parser->err_size,
            "expected ',' or ')' in argument list");
        *ok = 0;
        return 0;
    }
    plc_set_error(parser->err, parser->err_size, "unclosed argument list");
    *ok = 0;
    return 0;
}

static double plc_parse_primary(PLC_PARSER *parser, int *ok)
{
    PLC_REF ref;
    char name[PLC_MAX_NAME];
    char *endptr;
    double value;

    plc_parser_skip(parser);
    if (*parser->p == '(') {
        ++parser->p;
        value = plc_parse_expr(parser, ok);
        if (!*ok) {
            return 0.0;
        }
        plc_parser_skip(parser);
        if (*parser->p != ')') {
            plc_set_error(parser->err, parser->err_size, "expected ')'");
            *ok = 0;
            return 0.0;
        }
        ++parser->p;
        return value;
    }

    if (*parser->p == 'V' || *parser->p == 'C'
            || *parser->p == 'Z' || *parser->p == 'R') {
        const char *p;

        p = parser->p;
        if (!plc_parse_ref(&p, &ref)) {
            plc_set_error(parser->err, parser->err_size, "bad variable reference");
            *ok = 0;
            return 0.0;
        }
        parser->p = p;
        return plc_get_ref(parser->frame, ref, ok,
            parser->err, parser->err_size);
    }

    if (isdigit((unsigned char)*parser->p) || *parser->p == '.') {
        value = strtod(parser->p, &endptr);
        if (endptr == parser->p) {
            plc_set_error(parser->err, parser->err_size, "bad number");
            *ok = 0;
            return 0.0;
        }
        parser->p = plc_skip_type_marker(endptr);
        return value;
    }

    if (plc_parse_identifier(parser, name, sizeof(name))) {
        double args[PLC_MAX_ARGS];
        int argc;
        PLC_VALUES out;

        plc_parse_call_args(parser, args, &argc, ok);
        if (!*ok) {
            return 0.0;
        }
        if (!plc_call_proc(parser->program, parser->frame, name, args, argc,
                &out, parser->depth + 1, parser->err, parser->err_size)) {
            *ok = 0;
            return 0.0;
        }
        if (out.count <= 0) {
            plc_set_error(parser->err, parser->err_size, "call returned no value");
            *ok = 0;
            return 0.0;
        }
        return out.value[0];
    }

    plc_set_error(parser->err, parser->err_size, "expected expression");
    *ok = 0;
    return 0.0;
}

static double plc_parse_unary(PLC_PARSER *parser, int *ok)
{
    plc_parser_skip(parser);
    if (*parser->p == '-') {
        ++parser->p;
        return 0.0 - plc_parse_unary(parser, ok);
    }
    if (*parser->p == '!') {
        double value;

        ++parser->p;
        value = plc_parse_unary(parser, ok);
        return value == 0.0 ? 1.0 : 0.0;
    }
    return plc_parse_primary(parser, ok);
}

static double plc_parse_power(PLC_PARSER *parser, int *ok)
{
    double left;

    left = plc_parse_unary(parser, ok);
    if (!*ok) {
        return 0.0;
    }
    plc_parser_skip(parser);
    while (*parser->p == '^') {
        double right;

        ++parser->p;
        right = plc_parse_unary(parser, ok);
        if (!*ok) {
            return 0.0;
        }
        left = pow(left, right);
        plc_parser_skip(parser);
    }
    return left;
}

static double plc_parse_mul(PLC_PARSER *parser, int *ok)
{
    double left;

    left = plc_parse_power(parser, ok);
    if (!*ok) {
        return 0.0;
    }
    plc_parser_skip(parser);
    while (*parser->p == '*' || *parser->p == '/' || *parser->p == '%') {
        char op;
        double right;

        op = *parser->p;
        ++parser->p;
        right = plc_parse_power(parser, ok);
        if (!*ok) {
            return 0.0;
        }
        if (op == '*') {
            left = left * right;
        } else if (op == '/') {
            if (fabs(right) < 0.0000001) {
                plc_set_error(parser->err, parser->err_size,
                    "arithmetic exception: divide by zero");
                *ok = 0;
                return 0.0;
            }
            left = left / right;
        } else {
            if (fabs(right) < 0.0000001) {
                plc_set_error(parser->err, parser->err_size,
                    "arithmetic exception: modulo by zero");
                *ok = 0;
                return 0.0;
            }
            left = fmod(left, right);
        }
        plc_parser_skip(parser);
    }
    return left;
}

static double plc_parse_add(PLC_PARSER *parser, int *ok)
{
    double left;

    left = plc_parse_mul(parser, ok);
    if (!*ok) {
        return 0.0;
    }
    plc_parser_skip(parser);
    while (*parser->p == '+' || *parser->p == '-') {
        char op;
        double right;

        op = *parser->p;
        ++parser->p;
        right = plc_parse_mul(parser, ok);
        if (!*ok) {
            return 0.0;
        }
        if (op == '+') {
            left = left + right;
        } else {
            left = left - right;
        }
        plc_parser_skip(parser);
    }
    return left;
}

static int plc_match_compare(PLC_PARSER *parser, char *op)
{
    plc_parser_skip(parser);
    if (parser->p[0] == '!' && parser->p[1] == '=') {
        *op = 'n';
        parser->p += 2;
        return 1;
    }
    if (parser->p[0] == '<' && parser->p[1] == '=') {
        *op = 'l';
        parser->p += 2;
        return 1;
    }
    if (parser->p[0] == '>' && parser->p[1] == '=') {
        *op = 'g';
        parser->p += 2;
        return 1;
    }
    if (parser->p[0] == '=') {
        *op = '=';
        ++parser->p;
        return 1;
    }
    if (parser->p[0] == '<') {
        *op = '<';
        ++parser->p;
        return 1;
    }
    if (parser->p[0] == '>') {
        *op = '>';
        ++parser->p;
        return 1;
    }
    return 0;
}

static double plc_parse_compare_expr(PLC_PARSER *parser, int *ok)
{
    double left;
    char op;

    left = plc_parse_add(parser, ok);
    if (!*ok) {
        return 0.0;
    }
    plc_parser_skip(parser);
    while (plc_match_compare(parser, &op)) {
        double right;
        int result;

        right = plc_parse_add(parser, ok);
        if (!*ok) {
            return 0.0;
        }
        result = 0;
        if (op == '=') {
            result = fabs(left - right) < 0.0000001;
        } else if (op == 'n') {
            result = fabs(left - right) >= 0.0000001;
        } else if (op == '<') {
            result = left < right;
        } else if (op == 'l') {
            result = left <= right;
        } else if (op == '>') {
            result = left > right;
        } else if (op == 'g') {
            result = left >= right;
        }
        left = result ? 1.0 : 0.0;
        plc_parser_skip(parser);
    }
    return left;
}

static double plc_parse_and(PLC_PARSER *parser, int *ok)
{
    double left;

    left = plc_parse_compare_expr(parser, ok);
    if (!*ok) {
        return 0.0;
    }
    plc_parser_skip(parser);
    while (*parser->p == '&') {
        double right;

        ++parser->p;
        right = plc_parse_compare_expr(parser, ok);
        if (!*ok) {
            return 0.0;
        }
        left = (left != 0.0 && right != 0.0) ? 1.0 : 0.0;
        plc_parser_skip(parser);
    }
    return left;
}

static double plc_parse_or(PLC_PARSER *parser, int *ok)
{
    double left;

    left = plc_parse_and(parser, ok);
    if (!*ok) {
        return 0.0;
    }
    plc_parser_skip(parser);
    while (*parser->p == '|') {
        double right;

        ++parser->p;
        right = plc_parse_and(parser, ok);
        if (!*ok) {
            return 0.0;
        }
        left = (left != 0.0 || right != 0.0) ? 1.0 : 0.0;
        plc_parser_skip(parser);
    }
    return left;
}

static double plc_parse_expr(PLC_PARSER *parser, int *ok)
{
    return plc_parse_or(parser, ok);
}

int plc_eval_expr_text(const PLC_PROGRAM *program, PLC_FRAME *frame,
    int depth, const char *text, double *value, char *err, unsigned err_size)
{
    PLC_PARSER parser;
    int ok;

    parser.p = text;
    parser.program = program;
    parser.frame = frame;
    parser.depth = depth;
    parser.err = err;
    parser.err_size = err_size;
    ok = 1;
    *value = plc_parse_expr(&parser, &ok);
    if (!ok) {
        return 0;
    }
    parser.p = plc_skip_space(parser.p);
    if (*parser.p != '\0') {
        plc_set_error(err, err_size, "unexpected text after expression");
        return 0;
    }
    return 1;
}

int plc_is_top_call(const char *text, char *name, unsigned name_size,
    char *args, unsigned args_size)
{
    const char *p;
    const char *open;
    const char *close;
    unsigned n;

    p = plc_skip_space(text);
    if (!isalpha((unsigned char)*p) && *p != '_') {
        return 0;
    }
    n = 0;
    while (isalnum((unsigned char)*p) || *p == '_') {
        if (n + 1 < name_size) {
            name[n] = *p;
            ++n;
        }
        ++p;
    }
    name[n] = '\0';
    p = plc_skip_space(p);
    if (*p != '(') {
        return 0;
    }
    open = p;
    close = plc_matching_paren(open);
    if (close == 0) {
        return 0;
    }
    p = plc_skip_space(close + 1);
    if (*p != '\0') {
        return 0;
    }
    plc_copy_range(args, args_size, open + 1, close);
    return 1;
}

int plc_eval_args_text(const PLC_PROGRAM *program, PLC_FRAME *frame,
    int depth, const char *text, double *args, int *argc,
    char *err, unsigned err_size)
{
    PLC_PARSER parser;
    int ok;

    parser.p = text;
    parser.program = program;
    parser.frame = frame;
    parser.depth = depth;
    parser.err = err;
    parser.err_size = err_size;
    ok = 1;
    *argc = 0;
    parser.p = plc_skip_space(parser.p);
    if (*parser.p == '\0') {
        return 1;
    }
    while (*parser.p != '\0') {
        if (*argc >= PLC_MAX_ARGS) {
            plc_set_error(err, err_size, "too many call arguments");
            return 0;
        }
        args[*argc] = plc_parse_expr(&parser, &ok);
        if (!ok) {
            return 0;
        }
        ++(*argc);
        parser.p = plc_skip_space(parser.p);
        if (*parser.p == ',') {
            ++parser.p;
            continue;
        }
        if (*parser.p == '\0') {
            return 1;
        }
        plc_set_error(err, err_size, "expected ',' in call arguments");
        return 0;
    }
    return 1;
}

static int plc_predicate_starts_with(const char *text, const char *keyword,
    const char **body)
{
    unsigned n;

    text = plc_skip_space(text);
    n = (unsigned)strlen(keyword);
    if (strncmp(text, keyword, n) != 0) {
        return 0;
    }
    if (text[n] != '\0' && !isspace((unsigned char)text[n])) {
        return 0;
    }
    if (body != 0) {
        *body = plc_skip_space(text + n);
    }
    return 1;
}

static const char *plc_predicate_find_operator(const char *text, char op)
{
    int paren_depth;
    int bracket_depth;

    paren_depth = 0;
    bracket_depth = 0;
    while (*text != '\0') {
        if (*text == '(') {
            ++paren_depth;
        } else if (*text == ')' && paren_depth > 0) {
            --paren_depth;
        } else if (*text == '[') {
            ++bracket_depth;
        } else if (*text == ']' && bracket_depth > 0) {
            --bracket_depth;
        } else if (*text == op && paren_depth == 0 && bracket_depth == 0) {
            return text;
        }
        ++text;
    }
    return 0;
}

static int plc_eval_predicate_text(const PLC_PROGRAM *program,
    PLC_FRAME *frame, int depth, const char *text, PLC_VALUES *out,
    char *err, unsigned err_size)
{
    const char *body;
    const char *op;
    const char *call_name;
    char left_text[PLC_MAX_LINE];
    char right_text[PLC_MAX_LINE];
    double args[2];

    call_name = 0;
    body = 0;
    op = 0;
    if (plc_predicate_starts_with(text, "SELECT", &body)) {
        op = plc_predicate_find_operator(body, '>');
        call_name = "list_select_greater";
    } else if (plc_predicate_starts_with(text, "FORALL", &body)) {
        op = plc_predicate_find_operator(body, '>');
        call_name = "list_forall_greater";
    } else if (plc_predicate_starts_with(text, "EXISTS", &body)) {
        op = plc_predicate_find_operator(body, '=');
        call_name = "list_exists_equal";
    } else if (plc_predicate_starts_with(text, "COUNT", &body)) {
        op = plc_predicate_find_operator(body, '=');
        call_name = "list_count_equal";
    } else {
        return -1;
    }
    if (op == 0) {
        plc_set_error(err, err_size, "bad predicate expression");
        return 0;
    }
    plc_copy_range(left_text, sizeof(left_text), body, op);
    plc_trim_in_place(left_text);
    strncpy(right_text, op + 1, sizeof(right_text) - 1);
    right_text[sizeof(right_text) - 1] = '\0';
    plc_trim_in_place(right_text);
    if (!plc_eval_expr_text(program, frame, depth, left_text,
            &args[0], err, err_size)
            || !plc_eval_expr_text(program, frame, depth, right_text,
                &args[1], err, err_size)) {
        return 0;
    }
    return plc_call_proc(program, frame, call_name, args, 2, out,
        depth + 1, err, err_size);
}

int plc_eval_value_text(const PLC_PROGRAM *program, PLC_FRAME *frame,
    int depth, const char *text, PLC_VALUES *out,
    char *err, unsigned err_size)
{
    char name[PLC_MAX_NAME];
    char args_text[PLC_MAX_LINE];
    double args[PLC_MAX_ARGS];
    int argc;
    double value;
    int predicate_result;

    predicate_result = plc_eval_predicate_text(program, frame, depth, text,
        out, err, err_size);
    if (predicate_result >= 0) {
        return predicate_result;
    }
    if (plc_is_top_call(text, name, sizeof(name), args_text, sizeof(args_text))) {
        if (!plc_eval_args_text(program, frame, depth, args_text,
                args, &argc, err, err_size)) {
            return 0;
        }
        return plc_call_proc(program, frame, name, args, argc, out,
            depth + 1, err, err_size);
    }
    if (!plc_eval_expr_text(program, frame, depth, text, &value,
            err, err_size)) {
        return 0;
    }
    out->count = 1;
    out->value[0] = value;
    return 1;
}

int plc_assign_const_text(PLC_FRAME *frame, const char *text,
    double value, char *err, unsigned err_size)
{
    const char *p;
    PLC_REF ref;

    p = text;
    if (!plc_parse_ref(&p, &ref)) {
        plc_set_error(err, err_size, "bad constant reference");
        return 0;
    }
    p = plc_skip_space(p);
    if (*p != '\0') {
        plc_set_error(err, err_size, "unexpected text after constant reference");
        return 0;
    }
    return plc_set_const_ref(frame, ref, value, err, err_size);
}

int plc_assign_targets(PLC_FRAME *frame, const char *text,
    const PLC_VALUES *values, char *err, unsigned err_size)
{
    const char *p;
    int index;

    p = text;
    index = 0;
    while (*p != '\0') {
        PLC_REF ref;

        if (index >= values->count) {
            plc_set_error(err, err_size, "not enough values for targets");
            return 0;
        }
        if (!plc_parse_ref(&p, &ref)) {
            plc_set_error(err, err_size, "bad assignment target");
            return 0;
        }
        if (!plc_set_ref(frame, ref, values->value[index],
                err, err_size)) {
            return 0;
        }
        ++index;
        p = plc_skip_space(p);
        if (*p == ',') {
            ++p;
            continue;
        }
        if (*p == '\0') {
            break;
        }
        plc_set_error(err, err_size, "expected ',' in target list");
        return 0;
    }
    if (index != values->count) {
        plc_set_error(err, err_size, "too many values for targets");
        return 0;
    }
    return 1;
}

int plc_split_arrows(const char *text, char parts[][PLC_MAX_LINE],
    int *count)
{
    const char *p;
    const char *arrow;

    p = text;
    *count = 0;
    while ((arrow = strstr(p, "=>")) != 0) {
        if (*count >= 4) {
            return 0;
        }
        plc_copy_range(parts[*count], PLC_MAX_LINE, p, arrow);
        plc_trim_in_place(parts[*count]);
        ++(*count);
        p = arrow + 2;
    }
    if (*count >= 4) {
        return 0;
    }
    strcpy(parts[*count], p);
    plc_trim_in_place(parts[*count]);
    ++(*count);
    return 1;
}
