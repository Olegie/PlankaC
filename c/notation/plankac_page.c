#include "plankac_internal.h"

static const char *plc_page_row_body(const char *row, const char *prefix)
{
    const char *p;

    p = row;
    while (*p == ' ' || *p == '\t') {
        ++p;
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
        const char *v_body;
        const char *s_body;

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
        v_body = plc_page_row_body(rows[v_index], "V|");
        s_body = plc_page_row_body(rows[s_index], "S|");
        if (expr_body == 0 || v_body == 0 || s_body == 0) {
            plc_set_error(err, err_size, "bad two-dimensional page row");
            return 0;
        }
        if (!plc_expand_2d_statement(expr_body, v_body, s_body,
                statements[*statement_count], PLC_MAX_LINE,
                err, err_size)) {
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
