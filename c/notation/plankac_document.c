#include "plankac_internal.h"

static const char *plc_doc_body(const char *row, int *column)
{
    const char *p;
    int col;

    p = row;
    col = 1;
    while (*p == ' ' || *p == '\t') {
        ++p;
        ++col;
    }
    if (column != 0) {
        *column = col;
    }
    return p;
}

static int plc_doc_row_kind(const char *row)
{
    const char *p;

    p = plc_doc_body(row, 0);
    if (p[0] == 'V' && p[1] == '|') {
        return 'V';
    }
    if (p[0] == 'S' && p[1] == '|') {
        return 'S';
    }
    if (p[0] == '|') {
        return '|';
    }
    return 0;
}

static int plc_doc_arrow_column(const char *text)
{
    const char *arrow;

    arrow = strstr(text, "=>");
    if (arrow == 0) {
        return 0;
    }
    return (int)(arrow - text) + 1;
}

static int plc_doc_collect_cells(const char *text, int base_column,
    PLC_2D_ROW_MODEL *row)
{
    const char *p;
    int column;

    p = text;
    column = base_column;
    row->cell_count = 0;
    while (*p != '\0') {
        const char *first;
        const char *last;
        PLC_2D_CELL_MODEL *cell;
        unsigned len;

        while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') {
            ++p;
            ++column;
        }
        if (*p == '\0') {
            break;
        }
        if (row->cell_count >= PLC_2D_MAX_CELLS) {
            return 0;
        }
        first = p;
        while (*p != '\0' && *p != ' ' && *p != '\t'
                && *p != '\r' && *p != '\n') {
            ++p;
            ++column;
        }
        last = p;
        cell = &row->cells[row->cell_count];
        memset(cell, 0, sizeof(*cell));
        cell->row = row->row;
        cell->col_start = column - (int)(last - first);
        cell->col_end = column - 1;
        len = (unsigned)(last - first);
        if (len >= sizeof(cell->text)) {
            len = sizeof(cell->text) - 1;
        }
        memcpy(cell->text, first, len);
        cell->text[len] = '\0';
        ++row->cell_count;
    }
    return 1;
}

static int plc_doc_nearest_row(const PLC_2D_DOCUMENT *document,
    int expr_index, int kind)
{
    int best;
    int best_distance;
    int i;

    best = -1;
    best_distance = 99999;
    for (i = 0; i < document->row_count; ++i) {
        int distance;

        if (document->rows[i].kind != kind) {
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

static int plc_doc_rows_overlap(const PLC_2D_ROW_MODEL *expr,
    const PLC_2D_ROW_MODEL *typed);

static int plc_doc_nearest_overlapping_row(const PLC_2D_DOCUMENT *document,
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
                || !plc_doc_rows_overlap(expr, &document->rows[i])) {
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

static int plc_doc_rows_overlap(const PLC_2D_ROW_MODEL *expr,
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

int plc_build_2d_document(char rows[][PLC_MAX_LINE], int row_count,
    PLC_2D_DOCUMENT *document, char *err, unsigned err_size)
{
    int i;

    if (document == 0) {
        plc_set_error(err, err_size, "missing PAGE document output");
        return 0;
    }
    memset(document, 0, sizeof(*document));
    if (row_count <= 0) {
        plc_set_error(err, err_size, "PAGE has no rows");
        return 0;
    }
    if (row_count > PLC_2D_MAX_ROWS) {
        plc_set_error(err, err_size, "PAGE has too many rows");
        return 0;
    }
    document->row_count = row_count;
    for (i = 0; i < row_count; ++i) {
        PLC_2D_ROW_MODEL *row;
        const char *body;
        const char *cells;
        int row_column;
        int prefix;

        body = plc_doc_body(rows[i], &row_column);
        row = &document->rows[i];
        memset(row, 0, sizeof(*row));
        row->kind = plc_doc_row_kind(rows[i]);
        row->row = i + 1;
        row->col = row_column;
        if (body[0] == '\0') {
            continue;
        }
        if (row->kind == 0) {
            sprintf(err, "PAGE row %d column %d: expected |, V| or S|",
                i + 1, row_column);
            return 0;
        }
        prefix = row->kind == '|' ? 1 : 2;
        cells = body + prefix;
        if (!plc_doc_collect_cells(cells, row_column + prefix, row)) {
            sprintf(err, "PAGE row %d column %d: too many cells",
                i + 1, row_column);
            return 0;
        }
        if (row->kind == '|') {
            ++document->expression_count;
        }
    }
    return 1;
}

int plc_validate_2d_document(char rows[][PLC_MAX_LINE], int row_count,
    char *err, unsigned err_size)
{
    PLC_2D_DOCUMENT document;
    int i;

    if (!plc_build_2d_document(rows, row_count, &document,
            err, err_size)) {
        return 0;
    }
    for (i = 0; i < document.row_count; ++i) {
        const PLC_2D_ROW_MODEL *row;

        row = &document.rows[i];
        if (row->kind == 0) {
            continue;
        }
        if (row->kind == 'V' || row->kind == 'S') {
            if (row->cell_count <= 0) {
                sprintf(err, "PAGE row %d column %d: empty %c row",
                    row->row, row->col, row->kind);
                return 0;
            }
        }
        if (row->kind == '|') {
            int v_index;
            int s_index;
            int any_v_index;
            int any_s_index;
            int arrow_column;

            arrow_column = plc_doc_arrow_column(rows[i]);
            if (arrow_column == 0) {
                sprintf(err, "PAGE row %d column %d: missing =>",
                    row->row, row->col);
                return 0;
            }
            if (row->cell_count <= 0) {
                sprintf(err, "PAGE row %d column %d: empty expression row",
                    row->row, row->col);
                return 0;
            }
            v_index = plc_doc_nearest_overlapping_row(&document, i, 'V');
            s_index = plc_doc_nearest_overlapping_row(&document, i, 'S');
            if (v_index < 0 || s_index < 0) {
                any_v_index = plc_doc_nearest_row(&document, i, 'V');
                any_s_index = plc_doc_nearest_row(&document, i, 'S');
                if (any_v_index >= 0 && any_s_index >= 0) {
                    sprintf(err,
                        "PAGE row %d column %d: detached index/type row",
                        row->row, row->col);
                    return 0;
                }
                sprintf(err, "PAGE row %d column %d: missing V| or S| row",
                    row->row, row->col);
                return 0;
            }
        }
    }
    if (document.expression_count == 0) {
        plc_set_error(err, err_size, "PAGE has no executable row");
        return 0;
    }
    return 1;
}
