#include "plankac_internal.h"

void plc_set_error(char *err, unsigned err_size, const char *message)
{
    if (err_size == 0 || err == 0) {
        return;
    }
    if (err[0] != '\0') {
        return;
    }
    strncpy(err, message, err_size - 1);
    err[err_size - 1] = '\0';
}

void plc_prefix_error(char *err, unsigned err_size, const char *prefix)
{
    char detail[PLC_MAX_LINE];

    if (err_size == 0 || err == 0) {
        return;
    }
    strncpy(detail, err, sizeof(detail) - 1);
    detail[sizeof(detail) - 1] = '\0';
    err[0] = '\0';
    strncat(err, prefix, err_size - 1);
    strncat(err, detail, err_size - strlen(err) - 1);
}

void plc_copy_range(char *out, unsigned out_size,
    const char *first, const char *last)
{
    unsigned n;

    if (out_size == 0) {
        return;
    }
    if (last < first) {
        last = first;
    }
    n = (unsigned)(last - first);
    if (n >= out_size) {
        n = out_size - 1;
    }
    memcpy(out, first, n);
    out[n] = '\0';
}

char *plc_ltrim(char *text)
{
    while (*text == ' ' || *text == '\t' || *text == '\r' || *text == '\n') {
        ++text;
    }
    return text;
}

void plc_rtrim(char *text)
{
    unsigned n;

    n = (unsigned)strlen(text);
    while (n > 0) {
        char c;

        c = text[n - 1];
        if (c != ' ' && c != '\t' && c != '\r' && c != '\n') {
            break;
        }
        text[n - 1] = '\0';
        --n;
    }
}

void plc_trim_in_place(char *text)
{
    char *first;

    first = plc_ltrim(text);
    if (first != text) {
        memmove(text, first, strlen(first) + 1);
    }
    plc_rtrim(text);
}

void plc_strip_comment(char *text)
{
    char *hash;

    hash = strchr(text, '#');
    if (hash != 0) {
        *hash = '\0';
    }
    plc_trim_in_place(text);
}

const char *plc_skip_space(const char *p)
{
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') {
        ++p;
    }
    return p;
}

const char *plc_skip_type_marker(const char *p)
{
    p = plc_skip_space(p);
    if (*p == '[') {
        ++p;
        while (*p != '\0' && *p != ']') {
            ++p;
        }
        if (*p == ']') {
            ++p;
        }
    }
    return p;
}

int plc_is_program_line(const char *line)
{
    line = plc_skip_space(line);
    return line[0] == 'P' && isdigit((unsigned char)line[1]);
}

int plc_is_end_line(const char *line)
{
    line = plc_skip_space(line);
    return strcmp(line, "END") == 0;
}

const char *plc_matching_paren(const char *open)
{
    const char *p;
    int depth;

    if (*open != '(') {
        return 0;
    }
    p = open;
    depth = 0;
    while (*p != '\0') {
        if (*p == '(') {
            ++depth;
        } else if (*p == ')') {
            --depth;
            if (depth == 0) {
                return p;
            }
        }
        ++p;
    }
    return 0;
}

int plc_count_refs(const char *first, const char *last, char bank)
{
    int count;
    const char *p;

    count = 0;
    p = first;
    while (p < last && *p != '\0') {
        if (*p == bank && isdigit((unsigned char)p[1])) {
            ++count;
            ++p;
            while (p < last && isdigit((unsigned char)*p)) {
                ++p;
            }
            p = plc_skip_type_marker(p);
        } else {
            ++p;
        }
    }
    return count;
}

int plc_line_starts_with(const char *text, const char *keyword)
{
    unsigned n;

    text = plc_skip_space(text);
    n = (unsigned)strlen(keyword);
    if (strncmp(text, keyword, n) != 0) {
        return 0;
    }
    return text[n] == '\0' || isspace((unsigned char)text[n]);
}
