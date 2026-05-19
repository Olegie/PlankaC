#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define PLC_NATIVE_MAX_VARS 64
#define PLC_NATIVE_MAX_INDEX 32
#define PLC_NATIVE_MAX_FIELDS 64
#define PLC_NATIVE_MAX_FIELD_NAME 64
#define PLC_NATIVE_MAX_LISTS 32
#define PLC_NATIVE_MAX_LIST_ITEMS 128
#define PLC_NATIVE_MAX_PAIRS 128

typedef struct PLC_NATIVE_FRAME {
    double v[PLC_NATIVE_MAX_VARS];
    double z[PLC_NATIVE_MAX_VARS];
    double r[PLC_NATIVE_MAX_VARS];
    double va[PLC_NATIVE_MAX_VARS][PLC_NATIVE_MAX_INDEX];
    double za[PLC_NATIVE_MAX_VARS][PLC_NATIVE_MAX_INDEX];
    double ra[PLC_NATIVE_MAX_VARS][PLC_NATIVE_MAX_INDEX];
    double vf[PLC_NATIVE_MAX_VARS][PLC_NATIVE_MAX_FIELDS];
    double zf[PLC_NATIVE_MAX_VARS][PLC_NATIVE_MAX_FIELDS];
    double rf[PLC_NATIVE_MAX_VARS][PLC_NATIVE_MAX_FIELDS];
    char field_names[PLC_NATIVE_MAX_FIELDS][PLC_NATIVE_MAX_FIELD_NAME];
    int field_count;
    double lists[PLC_NATIVE_MAX_LISTS][PLC_NATIVE_MAX_LIST_ITEMS];
    int list_sizes[PLC_NATIVE_MAX_LISTS];
    int list_count;
    double pair_left[PLC_NATIVE_MAX_PAIRS];
    double pair_right[PLC_NATIVE_MAX_PAIRS];
    int pair_count;
} PLC_NATIVE_FRAME;

void *plc_native_frame_create(void)
{
    PLC_NATIVE_FRAME *frame;

    frame = (PLC_NATIVE_FRAME *)calloc(1, sizeof(*frame));
    return frame;
}

void plc_native_frame_destroy(void *opaque)
{
    free(opaque);
}

static int plc_native_field(PLC_NATIVE_FRAME *frame, const char *name)
{
    int i;

    if (name == 0 || name[0] == '\0') {
        return -1;
    }
    for (i = 0; i < frame->field_count; ++i) {
        if (strcmp(frame->field_names[i], name) == 0) {
            return i;
        }
    }
    if (frame->field_count >= PLC_NATIVE_MAX_FIELDS) {
        return -1;
    }
    strncpy(frame->field_names[frame->field_count], name,
        PLC_NATIVE_MAX_FIELD_NAME - 1);
    frame->field_names[frame->field_count][PLC_NATIVE_MAX_FIELD_NAME - 1] = '\0';
    ++frame->field_count;
    return frame->field_count - 1;
}

double plc_native_get(void *opaque, int bank, int index,
    int subscript, const char *field)
{
    PLC_NATIVE_FRAME *frame;
    int field_index;

    frame = (PLC_NATIVE_FRAME *)opaque;
    if (frame == 0 || index < 0 || index >= PLC_NATIVE_MAX_VARS) {
        return 0.0;
    }
    if (field != 0 && field[0] != '\0') {
        field_index = plc_native_field(frame, field);
        if (field_index < 0) {
            return 0.0;
        }
        if (bank == 'V') {
            return frame->vf[index][field_index];
        }
        if (bank == 'Z') {
            return frame->zf[index][field_index];
        }
        return frame->rf[index][field_index];
    }
    if (subscript >= 0) {
        if (subscript >= PLC_NATIVE_MAX_INDEX) {
            return 0.0;
        }
        if (bank == 'V') {
            return frame->va[index][subscript];
        }
        if (bank == 'Z') {
            return frame->za[index][subscript];
        }
        return frame->ra[index][subscript];
    }
    if (bank == 'V') {
        return frame->v[index];
    }
    if (bank == 'Z') {
        return frame->z[index];
    }
    return frame->r[index];
}

void plc_native_set(void *opaque, int bank, int index,
    int subscript, const char *field, const double *value)
{
    PLC_NATIVE_FRAME *frame;
    int field_index;
    double actual;

    frame = (PLC_NATIVE_FRAME *)opaque;
    if (frame == 0 || value == 0 || index < 0
            || index >= PLC_NATIVE_MAX_VARS) {
        return;
    }
    actual = *value;
    if (field != 0 && field[0] != '\0') {
        field_index = plc_native_field(frame, field);
        if (field_index < 0) {
            return;
        }
        if (bank == 'V') {
            frame->vf[index][field_index] = actual;
        } else if (bank == 'Z') {
            frame->zf[index][field_index] = actual;
        } else {
            frame->rf[index][field_index] = actual;
        }
        return;
    }
    if (subscript >= 0) {
        if (subscript >= PLC_NATIVE_MAX_INDEX) {
            return;
        }
        if (bank == 'V') {
            frame->va[index][subscript] = actual;
        } else if (bank == 'Z') {
            frame->za[index][subscript] = actual;
        } else {
            frame->ra[index][subscript] = actual;
        }
        return;
    }
    if (bank == 'V') {
        frame->v[index] = actual;
    } else if (bank == 'Z') {
        frame->z[index] = actual;
    } else {
        frame->r[index] = actual;
    }
}

static int plc_native_new_list(PLC_NATIVE_FRAME *frame)
{
    int id;

    if (frame->list_count >= PLC_NATIVE_MAX_LISTS) {
        return -1;
    }
    id = frame->list_count;
    frame->list_sizes[id] = 0;
    ++frame->list_count;
    return id;
}

static int plc_native_list_ok(PLC_NATIVE_FRAME *frame, int id)
{
    return id >= 0 && id < frame->list_count;
}

static int plc_native_list_push_value(PLC_NATIVE_FRAME *frame,
    int id, double value)
{
    int size;

    if (!plc_native_list_ok(frame, id)) {
        return 0;
    }
    size = frame->list_sizes[id];
    if (size >= PLC_NATIVE_MAX_LIST_ITEMS) {
        return 0;
    }
    frame->lists[id][size] = value;
    frame->list_sizes[id] = size + 1;
    return 1;
}

static int plc_native_list_contains(PLC_NATIVE_FRAME *frame,
    int id, double value)
{
    int i;

    if (!plc_native_list_ok(frame, id)) {
        return 0;
    }
    for (i = 0; i < frame->list_sizes[id]; ++i) {
        if (fabs(frame->lists[id][i] - value) < 0.0000001) {
            return 1;
        }
    }
    return 0;
}

double plc_native_builtin(void *opaque, const char *name,
    const double *args, int argc, int result_index)
{
    PLC_NATIVE_FRAME *frame;
    int list_id;
    int other_id;
    int out_id;
    int pair_id;
    int i;

    (void)result_index;
    frame = (PLC_NATIVE_FRAME *)opaque;
    if (strcmp(name, "sqrt") == 0 && argc == 1) {
        return sqrt(args[0]);
    }
    if (strcmp(name, "list_new") == 0 && argc == 0) {
        return (double)plc_native_new_list(frame);
    }
    if (strcmp(name, "list_push") == 0 && argc == 2) {
        list_id = (int)args[0];
        plc_native_list_push_value(frame, list_id, args[1]);
        return (double)list_id;
    }
    if (strcmp(name, "list_len") == 0 && argc == 1) {
        list_id = (int)args[0];
        return plc_native_list_ok(frame, list_id)
            ? (double)frame->list_sizes[list_id] : 0.0;
    }
    if (strcmp(name, "list_get") == 0 && argc == 2) {
        list_id = (int)args[0];
        i = (int)args[1];
        if (!plc_native_list_ok(frame, list_id)
                || i < 0 || i >= frame->list_sizes[list_id]) {
            return 0.0;
        }
        return frame->lists[list_id][i];
    }
    if ((strcmp(name, "list_first") == 0 || strcmp(name, "list_last") == 0
            || strcmp(name, "list_min") == 0 || strcmp(name, "list_max") == 0)
            && argc == 1) {
        double value;

        list_id = (int)args[0];
        if (!plc_native_list_ok(frame, list_id)
                || frame->list_sizes[list_id] <= 0) {
            return 0.0;
        }
        value = strcmp(name, "list_last") == 0
            ? frame->lists[list_id][frame->list_sizes[list_id] - 1]
            : frame->lists[list_id][0];
        if (strcmp(name, "list_min") == 0 || strcmp(name, "list_max") == 0) {
            for (i = 1; i < frame->list_sizes[list_id]; ++i) {
                if (strcmp(name, "list_min") == 0
                        && frame->lists[list_id][i] < value) {
                    value = frame->lists[list_id][i];
                }
                if (strcmp(name, "list_max") == 0
                        && frame->lists[list_id][i] > value) {
                    value = frame->lists[list_id][i];
                }
            }
        }
        return value;
    }
    if (strcmp(name, "list_concat") == 0 && argc == 2) {
        list_id = (int)args[0];
        other_id = (int)args[1];
        out_id = plc_native_new_list(frame);
        if (out_id < 0 || !plc_native_list_ok(frame, list_id)
                || !plc_native_list_ok(frame, other_id)) {
            return 0.0;
        }
        for (i = 0; i < frame->list_sizes[list_id]; ++i) {
            plc_native_list_push_value(frame, out_id, frame->lists[list_id][i]);
        }
        for (i = 0; i < frame->list_sizes[other_id]; ++i) {
            plc_native_list_push_value(frame, out_id, frame->lists[other_id][i]);
        }
        return (double)out_id;
    }
    if (strcmp(name, "pair") == 0 && argc == 2) {
        if (frame->pair_count >= PLC_NATIVE_MAX_PAIRS) {
            return 0.0;
        }
        pair_id = frame->pair_count;
        frame->pair_left[pair_id] = args[0];
        frame->pair_right[pair_id] = args[1];
        ++frame->pair_count;
        return (double)pair_id;
    }
    if ((strcmp(name, "pair_left") == 0 || strcmp(name, "pair_right") == 0)
            && argc == 1) {
        pair_id = (int)args[0];
        if (pair_id < 0 || pair_id >= frame->pair_count) {
            return 0.0;
        }
        return strcmp(name, "pair_left") == 0
            ? frame->pair_left[pair_id] : frame->pair_right[pair_id];
    }
    if (strcmp(name, "pair_list_count_first") == 0 && argc == 2) {
        int count;

        list_id = (int)args[0];
        if (!plc_native_list_ok(frame, list_id)) {
            return 0.0;
        }
        count = 0;
        for (i = 0; i < frame->list_sizes[list_id]; ++i) {
            pair_id = (int)frame->lists[list_id][i];
            if (pair_id >= 0 && pair_id < frame->pair_count
                    && fabs(frame->pair_left[pair_id] - args[1]) < 0.0000001) {
                ++count;
            }
        }
        return (double)count;
    }
    if (strcmp(name, "set_new") == 0 && argc == 0) {
        return (double)plc_native_new_list(frame);
    }
    if (strcmp(name, "set_add") == 0 && argc == 2) {
        list_id = (int)args[0];
        if (!plc_native_list_contains(frame, list_id, args[1])) {
            plc_native_list_push_value(frame, list_id, args[1]);
        }
        return (double)list_id;
    }
    if (strcmp(name, "set_contains") == 0 && argc == 2) {
        return (double)plc_native_list_contains(frame, (int)args[0], args[1]);
    }
    if (strcmp(name, "set_size") == 0 && argc == 1) {
        list_id = (int)args[0];
        return plc_native_list_ok(frame, list_id)
            ? (double)frame->list_sizes[list_id] : 0.0;
    }
    if ((strcmp(name, "set_union") == 0
            || strcmp(name, "set_intersection") == 0) && argc == 2) {
        list_id = (int)args[0];
        other_id = (int)args[1];
        out_id = plc_native_new_list(frame);
        if (out_id < 0 || !plc_native_list_ok(frame, list_id)
                || !plc_native_list_ok(frame, other_id)) {
            return 0.0;
        }
        for (i = 0; i < frame->list_sizes[list_id]; ++i) {
            double value;

            value = frame->lists[list_id][i];
            if (strcmp(name, "set_union") == 0
                    || plc_native_list_contains(frame, other_id, value)) {
                plc_native_list_push_value(frame, out_id, value);
            }
        }
        if (strcmp(name, "set_union") == 0) {
            for (i = 0; i < frame->list_sizes[other_id]; ++i) {
                double value;

                value = frame->lists[other_id][i];
                if (!plc_native_list_contains(frame, out_id, value)) {
                    plc_native_list_push_value(frame, out_id, value);
                }
            }
        }
        return (double)out_id;
    }
    return 0.0;
}

void plc_native_format(double value, char *out, unsigned out_size)
{
    long whole;
    double diff;

    if (out == 0 || out_size == 0) {
        return;
    }
    whole = (long)value;
    diff = value - (double)whole;
    if (diff < 0.0) {
        diff = 0.0 - diff;
    }
    if (diff < 0.000001) {
        sprintf(out, "%ld", whole);
    } else {
        sprintf(out, "%.6f", value);
    }
}
