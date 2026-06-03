#include "plankac_internal.h"

static const char *plc_page_row_body(const char *row, const char *prefix)
{
    const char *p;
    char *endptr;

    p = row;
    while (*p == ' ' || *p == '\t') {
        ++p;
    }
    if (*p == '[') {
        ++p;
        p = plc_skip_space(p);
        if (!isdigit((unsigned char)*p)) {
            return 0;
        }
        (void)strtol(p, &endptr, 10);
        p = plc_skip_space(endptr);
        if (*p != ',') {
            return 0;
        }
        ++p;
        p = plc_skip_space(p);
        if (!isdigit((unsigned char)*p)) {
            return 0;
        }
        (void)strtol(p, &endptr, 10);
        p = plc_skip_space(endptr);
        if (*p != ']') {
            return 0;
        }
        ++p;
        while (*p == ' ' || *p == '\t') {
            ++p;
        }
    }
    if (prefix[1] == '\0') {
        if (*p == prefix[0]) {
            return p + 1;
        }
    } else {
        if (p[0] == prefix[0] && p[1] == prefix[1]) {
            return p + 2;
        }
    }
    return 0;
}

static int plc_page_rows_overlap(const PLC_2D_ROW_MODEL *expr,
    const PLC_2D_ROW_MODEL *typed)
{
    int expr_first;
    int expr_last;
    int i;

    if (expr->cell_count <= 0 || typed->cell_count <= 0) {
        return 0;
    }
    expr_first = expr->cells[0].col_start;
    expr_last = expr->cells[expr->cell_count - 1].col_end;
    for (i = 0; i < typed->cell_count; ++i) {
        if (typed->cells[i].col_end >= expr_first
                && typed->cells[i].col_start <= expr_last) {
            return 1;
        }
    }
    return 0;
}

static int plc_page_nearest_overlapping_row(const PLC_2D_DOCUMENT *document,
    int expr_index, int kind)
{
    const PLC_2D_ROW_MODEL *expr;
    int best;
    int best_distance;
    int i;

    expr = &document->rows[expr_index];
    best = -1;
    best_distance = 99999;
    for (i = 0; i < document->row_count; ++i) {
        int distance;

        if (document->rows[i].kind != kind
                || document->rows[i].block != expr->block
                || !plc_page_rows_overlap(expr, &document->rows[i])) {
            continue;
        }
        distance = i > expr_index ? i - expr_index : expr_index - i;
        if (distance < best_distance) {
            best = i;
            best_distance = distance;
        }
    }
    return best;
}

static int plc_page_is_symbol(const char *p)
{
    if (!isupper((unsigned char)*p)) {
        return 0;
    }
    if (isalnum((unsigned char)p[1]) || p[1] == '_') {
        return 0;
    }
    return 1;
}

static char plc_page_bank_for_symbol(char symbol, int after_arrow)
{
    if (symbol == 'V' || symbol == 'Z' || symbol == 'R') {
        return symbol;
    }
    return after_arrow ? 'R' : 'Z';
}

static int plc_page_symbol_count(const char *row)
{
    int count;

    count = 0;
    while (*row != '\0') {
        if (plc_page_is_symbol(row)) {
            ++count;
        }
        ++row;
    }
    return count;
}

static int plc_page_nearest_model_cell(const PLC_2D_ROW_MODEL *row,
    int column)
{
    int best;
    int best_distance;
    int i;

    best = -1;
    best_distance = 99999;
    if (row == 0) {
        return -1;
    }
    for (i = 0; i < row->cell_count; ++i) {
        int distance;
        int center;

        if (column >= row->cells[i].col_start
                && column <= row->cells[i].col_end) {
            return i;
        }
        center = (row->cells[i].col_start + row->cells[i].col_end) / 2;
        distance = center > column ? center - column : column - center;
        if (distance < best_distance) {
            best = i;
            best_distance = distance;
        }
    }
    return best;
}

static int plc_page_expand_model_statement(const char *expr_body,
    const PLC_2D_ROW_MODEL *expr_model, const PLC_2D_ROW_MODEL *index_model,
    const PLC_2D_ROW_MODEL *type_model, char *out, unsigned out_size,
    char *err, unsigned err_size)
{
    const char *expr;
    int symbol_count;
    unsigned n;
    int after_arrow;

    expr = expr_body;
    symbol_count = plc_page_symbol_count(expr);
    if (symbol_count > 0
            && (index_model->cell_count < symbol_count
                || type_model->cell_count < symbol_count)) {
        plc_set_error(err, err_size,
            "bad two-dimensional row: V/S rows do not cover every symbol");
        return 0;
    }
    n = 0;
    after_arrow = 0;
    if (out_size > 0) {
        out[0] = '\0';
    }
    while (*expr != '\0') {
        if (expr[0] == '=' && expr[1] == '>') {
            after_arrow = 1;
            if (n + 3 >= out_size) {
                plc_set_error(err, err_size, "two-dimensional row too long");
                return 0;
            }
            out[n++] = *expr++;
            out[n++] = *expr++;
            out[n] = '\0';
            continue;
        }
        if (plc_page_is_symbol(expr)) {
            char ref[PLC_MAX_LINE];
            char marker[PLC_MAX_LINE];
            char bank;
            int logical_column;
            int index_cell;
            int type_cell;

            logical_column = expr_model->col + 1
                + (int)(expr - expr_body);
            index_cell = plc_page_nearest_model_cell(index_model,
                logical_column);
            type_cell = plc_page_nearest_model_cell(type_model,
                logical_column);
            if (index_cell < 0 || type_cell < 0) {
                plc_set_error(err, err_size, "bad two-dimensional row");
                return 0;
            }
            bank = plc_page_bank_for_symbol(*expr, after_arrow);
            if (strncmp(type_model->cells[type_cell].text, "[:", 2) == 0) {
                strncpy(marker, type_model->cells[type_cell].text,
                    sizeof(marker) - 1);
                marker[sizeof(marker) - 1] = '\0';
            } else {
                sprintf(marker, "[:%s]", type_model->cells[type_cell].text);
            }
            if (1 + strlen(index_model->cells[index_cell].text)
                    + strlen(marker) + 1 >= sizeof(ref)) {
                plc_set_error(err, err_size, "two-dimensional row too long");
                return 0;
            }
            ref[0] = bank;
            ref[1] = '\0';
            strcat(ref, index_model->cells[index_cell].text);
            strcat(ref, marker);
            if (n + strlen(ref) + 1 >= out_size) {
                plc_set_error(err, err_size, "two-dimensional row too long");
                return 0;
            }
            strcpy(out + n, ref);
            n += (unsigned)strlen(ref);
            ++expr;
            continue;
        }
        if (n + 2 >= out_size) {
            plc_set_error(err, err_size, "two-dimensional row too long");
            return 0;
        }
        out[n++] = *expr++;
        out[n] = '\0';
    }
    plc_trim_in_place(out);
    return 1;
}

int plc_expand_2d_page(char rows[][PLC_MAX_LINE], int row_count,
    char statements[][PLC_MAX_LINE], int *statement_count,
    int max_statements, char *err, unsigned err_size)
{
    PLC_2D_DOCUMENT document;
    int i;

    if (statement_count == 0) {
        plc_set_error(err, err_size, "bad two-dimensional page output");
        return 0;
    }
    if (!plc_validate_2d_document(rows, row_count, err, err_size)) {
        return 0;
    }
    if (!plc_build_2d_document(rows, row_count, &document, err, err_size)) {
        return 0;
    }
    *statement_count = 0;
    for (i = 0; i < row_count; ++i) {
        int v_index;
        int s_index;
        const char *expr_body;

        if (document.rows[i].kind != '|') {
            continue;
        }
        if (*statement_count >= max_statements) {
            plc_set_error(err, err_size,
                "two-dimensional page produced too many statements");
            return 0;
        }
        v_index = plc_page_nearest_overlapping_row(&document, i, 'V');
        s_index = plc_page_nearest_overlapping_row(&document, i, 'S');
        if (v_index < 0 || s_index < 0) {
            plc_set_error(err, err_size,
                "two-dimensional page needs V| and S| rows");
            return 0;
        }
        expr_body = plc_page_row_body(rows[i], "|");
        if (expr_body == 0) {
            plc_set_error(err, err_size, "bad two-dimensional page row");
            return 0;
        }
        if (!plc_page_expand_model_statement(expr_body,
                &document.rows[i], &document.rows[v_index],
                &document.rows[s_index], statements[*statement_count],
                PLC_MAX_LINE, err, err_size)) {
            return 0;
        }
        ++(*statement_count);
    }
    if (*statement_count == 0) {
        plc_set_error(err, err_size,
            "two-dimensional page has no executable row");
        return 0;
    }
    return 1;
}
