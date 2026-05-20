#include "plankac_internal.h"

typedef struct PLC_2D_CELL {
    int start;
    int end;
    char text[PLC_MAX_TYPE_TEXT];
} PLC_2D_CELL;

static int plc_2d_collect_cells(const char *row, PLC_2D_CELL *cells,
    int max_cells)
{
    int count;
    int pos;

    count = 0;
    pos = 0;
    while (row[pos] != '\0') {
        unsigned n;

        while (row[pos] != '\0' && isspace((unsigned char)row[pos])) {
            ++pos;
        }
        if (row[pos] == '\0') {
            break;
        }
        if (count >= max_cells) {
            break;
        }
        cells[count].start = pos;
        n = 0;
        while (row[pos] != '\0' && !isspace((unsigned char)row[pos])) {
            if (n + 1 < sizeof(cells[count].text)) {
                cells[count].text[n] = row[pos];
                ++n;
            }
            ++pos;
        }
        cells[count].text[n] = '\0';
        cells[count].end = pos > cells[count].start ? pos - 1 : pos;
        ++count;
    }
    return count;
}

static int plc_2d_nearest_cell(const PLC_2D_CELL *cells, int count,
    int column)
{
    int best;
    int best_distance;
    int i;

    best = -1;
    best_distance = 99999;
    for (i = 0; i < count; ++i) {
        int distance;
        int center;

        if (column >= cells[i].start && column <= cells[i].end) {
            return i;
        }
        center = (cells[i].start + cells[i].end) / 2;
        distance = center > column ? center - column : column - center;
        if (distance < best_distance) {
            best = i;
            best_distance = distance;
        }
    }
    return best;
}

static int plc_2d_is_symbol(const char *p)
{
    if (!isupper((unsigned char)*p)) {
        return 0;
    }
    if (isalnum((unsigned char)p[1]) || p[1] == '_') {
        return 0;
    }
    return 1;
}

static char plc_2d_bank_for_symbol(char symbol, int after_arrow)
{
    if (symbol == 'V' || symbol == 'Z' || symbol == 'R') {
        return symbol;
    }
    return after_arrow ? 'R' : 'Z';
}

static int plc_2d_symbol_count(const char *row)
{
    int count;

    count = 0;
    while (*row != '\0') {
        if (plc_2d_is_symbol(row)) {
            ++count;
        }
        ++row;
    }
    return count;
}

int plc_expand_2d_statement(const char *expr_row, const char *index_row,
    const char *type_row, char *out, unsigned out_size,
    char *err, unsigned err_size)
{
    const char *expr;
    PLC_2D_CELL index_cells[PLC_MAX_ARGS + PLC_MAX_RESULTS];
    PLC_2D_CELL type_cells[PLC_MAX_ARGS + PLC_MAX_RESULTS];
    int index_count;
    int type_count;
    int symbol_count;
    unsigned n;
    int after_arrow;

    expr = plc_skip_space(expr_row);
    index_count = plc_2d_collect_cells(index_row, index_cells,
        PLC_MAX_ARGS + PLC_MAX_RESULTS);
    type_count = plc_2d_collect_cells(type_row, type_cells,
        PLC_MAX_ARGS + PLC_MAX_RESULTS);
    symbol_count = plc_2d_symbol_count(expr);
    if (symbol_count > 0
            && (index_count < symbol_count || type_count < symbol_count)) {
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
        if (plc_2d_is_symbol(expr)) {
            char ref[128];
            char bank;
            char marker[PLC_MAX_TYPE_TEXT + 4];
            int column;
            int index_cell;
            int type_cell;

            column = (int)(expr - plc_skip_space(expr_row));
            index_cell = plc_2d_nearest_cell(index_cells, index_count, column);
            type_cell = plc_2d_nearest_cell(type_cells, type_count, column);
            if (index_cell < 0 || type_cell < 0) {
                plc_set_error(err, err_size, "bad two-dimensional row");
                return 0;
            }
            bank = plc_2d_bank_for_symbol(*expr, after_arrow);
            if (strncmp(type_cells[type_cell].text, "[:", 2) == 0) {
                strncpy(marker, type_cells[type_cell].text, sizeof(marker) - 1);
                marker[sizeof(marker) - 1] = '\0';
            } else {
                sprintf(marker, "[:%s]", type_cells[type_cell].text);
            }
            sprintf(ref, "%c%s%s", bank, index_cells[index_cell].text, marker);
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
