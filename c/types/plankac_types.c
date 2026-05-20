#include "plankac_internal.h"

static int plc_parse_type_number(const char **pp)
{
    const char *p;
    int value;

    p = *pp;
    if (!isdigit((unsigned char)*p)) {
        while (isalpha((unsigned char)*p) || *p == '_') {
            ++p;
        }
        *pp = p;
        return -1;
    }
    value = 0;
    while (isdigit((unsigned char)*p)) {
        value = value * 10 + (*p - '0');
        ++p;
    }
    *pp = p;
    return value;
}

static int plc_type_family_from_prefix(const char *prefix,
    int class_code, int bits, int scale)
{
    if (prefix[0] == '\0') {
        if (bits == 1 && scale == 1) {
            return PLC_TYPE_FAMILY_BOOLEAN;
        }
        return PLC_TYPE_FAMILY_NUMERIC;
    }
    if (strcmp(prefix, "C") == 0 || strcmp(prefix, "COMPLEX") == 0) {
        return PLC_TYPE_FAMILY_COMPLEX;
    }
    if (strcmp(prefix, "L") == 0 || strcmp(prefix, "LIST") == 0) {
        return PLC_TYPE_FAMILY_LIST;
    }
    if (strcmp(prefix, "S") == 0 || strcmp(prefix, "SET") == 0) {
        return PLC_TYPE_FAMILY_SET;
    }
    if (strcmp(prefix, "P") == 0 || strcmp(prefix, "PAIR") == 0) {
        return PLC_TYPE_FAMILY_PAIR;
    }
    if (strcmp(prefix, "Q") == 0 || strcmp(prefix, "REC") == 0
            || strcmp(prefix, "RECORD") == 0) {
        return PLC_TYPE_FAMILY_RECORD;
    }
    if (strcmp(prefix, "VEC") == 0 || strcmp(prefix, "VEC3") == 0
            || strcmp(prefix, "VECTOR") == 0) {
        return PLC_TYPE_FAMILY_VEC3;
    }
    if (strcmp(prefix, "MAT") == 0 || strcmp(prefix, "MAT4") == 0
            || strcmp(prefix, "MATRIX") == 0) {
        return PLC_TYPE_FAMILY_MAT4;
    }
    if (class_code == 1 && bits == 1 && scale == 1) {
        return PLC_TYPE_FAMILY_BOOLEAN;
    }
    return PLC_TYPE_FAMILY_UNKNOWN;
}

int plc_parse_type_marker_text(const char *text, PLC_TYPE_SPEC *spec,
    char *err, unsigned err_size)
{
    const char *p;
    const char *start;
    unsigned n;

    if (spec != 0) {
        memset(spec, 0, sizeof(*spec));
        spec->class_code = -1;
        spec->bits = -1;
        spec->scale = -1;
    }
    if (text == 0 || text[0] != '[' || text[1] != ':') {
        plc_set_error(err, err_size, "bad type marker");
        return 0;
    }
    p = text + 2;
    start = text;
    n = 0;
    while (isalpha((unsigned char)*p) || *p == '_') {
        if (spec != 0 && n + 1 < sizeof(spec->prefix)) {
            spec->prefix[n] = (char)toupper((unsigned char)*p);
            ++n;
        }
        ++p;
    }
    if (spec != 0) {
        spec->prefix[n] = '\0';
    }
    if (!isdigit((unsigned char)*p)) {
        plc_set_error(err, err_size, "bad type marker");
        return 0;
    }
    if (spec != 0) {
        spec->bits = plc_parse_type_number(&p);
    } else {
        plc_parse_type_number(&p);
    }
    if (*p != '.') {
        plc_set_error(err, err_size, "bad type marker");
        return 0;
    }
    ++p;
    if (!isdigit((unsigned char)*p) && !isalpha((unsigned char)*p)) {
        plc_set_error(err, err_size, "bad type marker");
        return 0;
    }
    if (spec != 0) {
        spec->scale = plc_parse_type_number(&p);
    } else {
        plc_parse_type_number(&p);
    }
    if (*p == '.') {
        ++p;
        if (!isdigit((unsigned char)*p) && !isalpha((unsigned char)*p)) {
            plc_set_error(err, err_size, "bad type marker");
            return 0;
        }
        if (spec != 0) {
            spec->class_code = spec->bits;
            spec->bits = spec->scale;
            spec->scale = plc_parse_type_number(&p);
        } else {
            plc_parse_type_number(&p);
        }
    }
    if (*p != ']') {
        plc_set_error(err, err_size, "bad type marker");
        return 0;
    }
    if (spec != 0) {
        plc_copy_range(spec->text, sizeof(spec->text), start, p + 1);
        spec->family = plc_type_family_from_prefix(spec->prefix,
            spec->class_code, spec->bits, spec->scale);
    }
    return 1;
}

int plc_validate_type_markers_in_line(const char *line,
    char *err, unsigned err_size)
{
    const char *p;
    const char *end;
    char marker[PLC_MAX_TYPE_TEXT];

    for (p = line; *p != '\0'; ++p) {
        if (p[0] != '[' || p[1] != ':') {
            continue;
        }
        end = strchr(p, ']');
        if (end == 0) {
            plc_set_error(err, err_size, "bad type marker");
            return 0;
        }
        plc_copy_range(marker, sizeof(marker), p, end + 1);
        if (!plc_parse_type_marker_text(marker, 0, err, err_size)) {
            return 0;
        }
        p = end;
    }
    return 1;
}

int plc_type_markers_compatible(const char *left, const char *right)
{
    PLC_TYPE_SPEC a;
    PLC_TYPE_SPEC b;
    char err[PLC_MAX_LINE];

    if (left == 0 || right == 0 || left[0] == '\0' || right[0] == '\0') {
        return 1;
    }
    err[0] = '\0';
    if (!plc_parse_type_marker_text(left, &a, err, sizeof(err))
            || !plc_parse_type_marker_text(right, &b, err, sizeof(err))) {
        return 0;
    }
    if (a.family != b.family) {
        return 0;
    }
    if (a.bits >= 0 && b.bits >= 0 && a.bits != b.bits) {
        return 0;
    }
    if (a.scale >= 0 && b.scale >= 0 && a.scale != b.scale) {
        return 0;
    }
    return 1;
}

const char *plc_type_family_name(int family)
{
    if (family == PLC_TYPE_FAMILY_NUMERIC) {
        return "numeric";
    }
    if (family == PLC_TYPE_FAMILY_BOOLEAN) {
        return "boolean";
    }
    if (family == PLC_TYPE_FAMILY_COMPLEX) {
        return "complex";
    }
    if (family == PLC_TYPE_FAMILY_LIST) {
        return "list";
    }
    if (family == PLC_TYPE_FAMILY_SET) {
        return "set";
    }
    if (family == PLC_TYPE_FAMILY_PAIR) {
        return "pair";
    }
    if (family == PLC_TYPE_FAMILY_RECORD) {
        return "record";
    }
    if (family == PLC_TYPE_FAMILY_VEC3) {
        return "vec3";
    }
    if (family == PLC_TYPE_FAMILY_MAT4) {
        return "mat4";
    }
    return "unknown";
}
