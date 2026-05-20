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
#define PLC_NATIVE_MAX_COMPLEX 128
#define PLC_NATIVE_MAX_RECORDS 64
#define PLC_NATIVE_MAX_RECORD_FIELDS 64
#define PLC_NATIVE_MAX_VEC3 128
#define PLC_NATIVE_MAX_MAT4 64

typedef struct PLC_NATIVE_FRAME {
    struct PLC_NATIVE_FRAME *heap_owner;
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
    double complex_real[PLC_NATIVE_MAX_COMPLEX];
    double complex_imag[PLC_NATIVE_MAX_COMPLEX];
    int complex_count;
    int record_keys[PLC_NATIVE_MAX_RECORDS][PLC_NATIVE_MAX_RECORD_FIELDS];
    double record_values[PLC_NATIVE_MAX_RECORDS][PLC_NATIVE_MAX_RECORD_FIELDS];
    int record_sizes[PLC_NATIVE_MAX_RECORDS];
    int record_count;
    double vec3_values[PLC_NATIVE_MAX_VEC3][3];
    int vec3_count;
    double mat4_values[PLC_NATIVE_MAX_MAT4][16];
    int mat4_count;
} PLC_NATIVE_FRAME;

static PLC_NATIVE_FRAME *plc_native_heap_frame(PLC_NATIVE_FRAME *frame)
{
    if (frame != 0 && frame->heap_owner != 0) {
        return frame->heap_owner;
    }
    return frame;
}

void *plc_native_frame_create_child(void *parent_opaque);

void *plc_native_frame_create(void)
{
    return plc_native_frame_create_child(0);
}

void *plc_native_frame_create_child(void *parent_opaque)
{
    PLC_NATIVE_FRAME *frame;
    PLC_NATIVE_FRAME *parent;

    frame = (PLC_NATIVE_FRAME *)calloc(1, sizeof(*frame));
    parent = (PLC_NATIVE_FRAME *)parent_opaque;
    frame->heap_owner = plc_native_heap_frame(parent);
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

static int plc_native_new_complex(PLC_NATIVE_FRAME *frame,
    double real, double imag)
{
    int id;

    if (frame->complex_count >= PLC_NATIVE_MAX_COMPLEX) {
        return -1;
    }
    id = frame->complex_count;
    frame->complex_real[id] = real;
    frame->complex_imag[id] = imag;
    ++frame->complex_count;
    return id;
}

static int plc_native_complex_ok(PLC_NATIVE_FRAME *frame, int id)
{
    return id >= 0 && id < frame->complex_count;
}

static int plc_native_new_record(PLC_NATIVE_FRAME *frame)
{
    int id;

    if (frame->record_count >= PLC_NATIVE_MAX_RECORDS) {
        return -1;
    }
    id = frame->record_count;
    frame->record_sizes[id] = 0;
    ++frame->record_count;
    return id;
}

static int plc_native_record_ok(PLC_NATIVE_FRAME *frame, int id)
{
    return id >= 0 && id < frame->record_count;
}

static int plc_native_record_find(PLC_NATIVE_FRAME *frame, int id, int key)
{
    int i;

    if (!plc_native_record_ok(frame, id)) {
        return -1;
    }
    for (i = 0; i < frame->record_sizes[id]; ++i) {
        if (frame->record_keys[id][i] == key) {
            return i;
        }
    }
    return -1;
}

static int plc_native_record_set(PLC_NATIVE_FRAME *frame, int id,
    int key, double value)
{
    int slot;

    if (!plc_native_record_ok(frame, id)) {
        return 0;
    }
    slot = plc_native_record_find(frame, id, key);
    if (slot < 0) {
        if (frame->record_sizes[id] >= PLC_NATIVE_MAX_RECORD_FIELDS) {
            return 0;
        }
        slot = frame->record_sizes[id];
        frame->record_keys[id][slot] = key;
        frame->record_sizes[id] = slot + 1;
    }
    frame->record_values[id][slot] = value;
    return 1;
}

static int plc_native_chess_square(int file, int rank)
{
    return file * 10 + rank;
}

static int plc_native_chess_add_square(PLC_NATIVE_FRAME *frame,
    int list_id, int file, int rank)
{
    double square;

    if (file < 1 || file > 8 || rank < 1 || rank > 8) {
        return 1;
    }
    square = (double)plc_native_chess_square(file, rank);
    if (!plc_native_list_contains(frame, list_id, square)) {
        return plc_native_list_push_value(frame, list_id, square);
    }
    return 1;
}

static int plc_native_chess_attack_map(PLC_NATIVE_FRAME *frame,
    int kind, int file, int rank)
{
    int list_id;
    int step;
    int i;
    static const int knight_delta[8][2] = {
        {1, 2}, {2, 1}, {2, -1}, {1, -2},
        {-1, -2}, {-2, -1}, {-2, 1}, {-1, 2}
    };

    list_id = plc_native_new_list(frame);
    if (list_id < 0) {
        return -1;
    }
    if (kind == 4 || kind == 5) {
        for (step = 1; step <= 7; ++step) {
            if (!plc_native_chess_add_square(frame, list_id,
                    file + step, rank)
                    || !plc_native_chess_add_square(frame, list_id,
                        file - step, rank)
                    || !plc_native_chess_add_square(frame, list_id,
                        file, rank + step)
                    || !plc_native_chess_add_square(frame, list_id,
                        file, rank - step)) {
                return -1;
            }
        }
    }
    if (kind == 3 || kind == 5) {
        for (step = 1; step <= 7; ++step) {
            if (!plc_native_chess_add_square(frame, list_id,
                    file + step, rank + step)
                    || !plc_native_chess_add_square(frame, list_id,
                        file + step, rank - step)
                    || !plc_native_chess_add_square(frame, list_id,
                        file - step, rank + step)
                    || !plc_native_chess_add_square(frame, list_id,
                        file - step, rank - step)) {
                return -1;
            }
        }
    }
    if (kind == 2) {
        for (i = 0; i < 8; ++i) {
            if (!plc_native_chess_add_square(frame, list_id,
                    file + knight_delta[i][0], rank + knight_delta[i][1])) {
                return -1;
            }
        }
    }
    if (kind == 6) {
        int df;
        int dr;

        for (df = -1; df <= 1; ++df) {
            for (dr = -1; dr <= 1; ++dr) {
                if ((df != 0 || dr != 0)
                        && !plc_native_chess_add_square(frame, list_id,
                            file + df, rank + dr)) {
                    return -1;
                }
            }
        }
    }
    return list_id;
}

static int plc_native_new_vec3(PLC_NATIVE_FRAME *frame,
    double x, double y, double z)
{
    int id;

    if (frame->vec3_count >= PLC_NATIVE_MAX_VEC3) {
        return -1;
    }
    id = frame->vec3_count;
    frame->vec3_values[id][0] = x;
    frame->vec3_values[id][1] = y;
    frame->vec3_values[id][2] = z;
    ++frame->vec3_count;
    return id;
}

static int plc_native_vec3_ok(PLC_NATIVE_FRAME *frame, int id)
{
    return id >= 0 && id < frame->vec3_count;
}

static int plc_native_new_mat4(PLC_NATIVE_FRAME *frame, const double *values)
{
    int id;
    int i;

    if (frame->mat4_count >= PLC_NATIVE_MAX_MAT4) {
        return -1;
    }
    id = frame->mat4_count;
    for (i = 0; i < 16; ++i) {
        frame->mat4_values[id][i] = values[i];
    }
    ++frame->mat4_count;
    return id;
}

static int plc_native_mat4_ok(PLC_NATIVE_FRAME *frame, int id)
{
    return id >= 0 && id < frame->mat4_count;
}

double plc_native_builtin(void *opaque, const char *name,
    const double *args, int argc, int result_index)
{
    PLC_NATIVE_FRAME *frame;
    int list_id;
    int other_id;
    int out_id;
    int pair_id;
    int complex_id;
    int record_id;
    int vec_id;
    int other_vec;
    int mat_id;
    int other_mat;
    int i;

    (void)result_index;
    frame = (PLC_NATIVE_FRAME *)opaque;
    if (frame == 0) {
        return 0.0;
    }
    frame = plc_native_heap_frame(frame);
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
            || strcmp(name, "set_intersection") == 0
            || strcmp(name, "set_difference") == 0) && argc == 2) {
        list_id = (int)args[0];
        other_id = (int)args[1];
        out_id = plc_native_new_list(frame);
        if (out_id < 0 || !plc_native_list_ok(frame, list_id)
                || !plc_native_list_ok(frame, other_id)) {
            return 0.0;
        }
        for (i = 0; i < frame->list_sizes[list_id]; ++i) {
            double value;
            int include_value;

            value = frame->lists[list_id][i];
            include_value = 0;
            if (strcmp(name, "set_union") == 0) {
                include_value = 1;
            } else if (strcmp(name, "set_intersection") == 0) {
                include_value = plc_native_list_contains(frame,
                    other_id, value);
            } else {
                include_value = !plc_native_list_contains(frame,
                    other_id, value);
            }
            if (include_value) {
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
    if (strcmp(name, "set_subset") == 0 && argc == 2) {
        list_id = (int)args[0];
        other_id = (int)args[1];
        if (!plc_native_list_ok(frame, list_id)
                || !plc_native_list_ok(frame, other_id)) {
            return 0.0;
        }
        for (i = 0; i < frame->list_sizes[list_id]; ++i) {
            if (!plc_native_list_contains(frame, other_id,
                    frame->lists[list_id][i])) {
                return 0.0;
            }
        }
        return 1.0;
    }
    if ((strcmp(name, "relation_domain") == 0
            || strcmp(name, "relation_range") == 0) && argc == 1) {
        list_id = (int)args[0];
        out_id = plc_native_new_list(frame);
        if (out_id < 0 || !plc_native_list_ok(frame, list_id)) {
            return 0.0;
        }
        for (i = 0; i < frame->list_sizes[list_id]; ++i) {
            double value;

            pair_id = (int)frame->lists[list_id][i];
            if (pair_id < 0 || pair_id >= frame->pair_count) {
                return 0.0;
            }
            value = strcmp(name, "relation_domain") == 0
                ? frame->pair_left[pair_id]
                : frame->pair_right[pair_id];
            if (!plc_native_list_contains(frame, out_id, value)) {
                plc_native_list_push_value(frame, out_id, value);
            }
        }
        return (double)out_id;
    }
    if (strcmp(name, "relation_has_pair") == 0 && argc == 3) {
        list_id = (int)args[0];
        if (!plc_native_list_ok(frame, list_id)) {
            return 0.0;
        }
        for (i = 0; i < frame->list_sizes[list_id]; ++i) {
            pair_id = (int)frame->lists[list_id][i];
            if (pair_id >= 0 && pair_id < frame->pair_count
                    && fabs(frame->pair_left[pair_id] - args[1]) < 0.0000001
                    && fabs(frame->pair_right[pair_id] - args[2]) < 0.0000001) {
                return 1.0;
            }
        }
        return 0.0;
    }
    if ((strcmp(name, "set_cartesian") == 0
            || strcmp(name, "relation_compose") == 0) && argc == 2) {
        list_id = (int)args[0];
        other_id = (int)args[1];
        out_id = plc_native_new_list(frame);
        if (out_id < 0 || !plc_native_list_ok(frame, list_id)
                || !plc_native_list_ok(frame, other_id)) {
            return 0.0;
        }
        if (strcmp(name, "set_cartesian") == 0) {
            int j;

            for (i = 0; i < frame->list_sizes[list_id]; ++i) {
                for (j = 0; j < frame->list_sizes[other_id]; ++j) {
                    if (frame->pair_count >= PLC_NATIVE_MAX_PAIRS) {
                        return 0.0;
                    }
                    pair_id = frame->pair_count;
                    frame->pair_left[pair_id] = frame->lists[list_id][i];
                    frame->pair_right[pair_id] = frame->lists[other_id][j];
                    ++frame->pair_count;
                    plc_native_list_push_value(frame, out_id, (double)pair_id);
                }
            }
        } else {
            int j;

            for (i = 0; i < frame->list_sizes[list_id]; ++i) {
                int left_pair;

                left_pair = (int)frame->lists[list_id][i];
                if (left_pair < 0 || left_pair >= frame->pair_count) {
                    return 0.0;
                }
                for (j = 0; j < frame->list_sizes[other_id]; ++j) {
                    int right_pair;

                    right_pair = (int)frame->lists[other_id][j];
                    if (right_pair < 0 || right_pair >= frame->pair_count) {
                        return 0.0;
                    }
                    if (fabs(frame->pair_right[left_pair]
                            - frame->pair_left[right_pair]) < 0.0000001) {
                        if (frame->pair_count >= PLC_NATIVE_MAX_PAIRS) {
                            return 0.0;
                        }
                        pair_id = frame->pair_count;
                        frame->pair_left[pair_id] =
                            frame->pair_left[left_pair];
                        frame->pair_right[pair_id] =
                            frame->pair_right[right_pair];
                        ++frame->pair_count;
                        plc_native_list_push_value(frame, out_id,
                            (double)pair_id);
                    }
                }
            }
        }
        return (double)out_id;
    }
    if ((strcmp(name, "set_exists_greater") == 0
            || strcmp(name, "set_forall_less") == 0) && argc == 2) {
        int ok;

        list_id = (int)args[0];
        if (!plc_native_list_ok(frame, list_id)) {
            return 0.0;
        }
        ok = strcmp(name, "set_forall_less") == 0 ? 1 : 0;
        for (i = 0; i < frame->list_sizes[list_id]; ++i) {
            if (strcmp(name, "set_exists_greater") == 0
                    && frame->lists[list_id][i] > args[1]) {
                ok = 1;
            }
            if (strcmp(name, "set_forall_less") == 0
                    && !(frame->lists[list_id][i] < args[1])) {
                ok = 0;
            }
        }
        return (double)ok;
    }
    if (strcmp(name, "record_new") == 0 && argc == 0) {
        return (double)plc_native_new_record(frame);
    }
    if (strcmp(name, "record_set") == 0 && argc == 3) {
        record_id = (int)args[0];
        plc_native_record_set(frame, record_id, (int)args[1], args[2]);
        return (double)record_id;
    }
    if (strcmp(name, "record_get") == 0 && argc == 2) {
        int slot;

        record_id = (int)args[0];
        slot = plc_native_record_find(frame, record_id, (int)args[1]);
        return slot >= 0 ? frame->record_values[record_id][slot] : 0.0;
    }
    if (strcmp(name, "record_has") == 0 && argc == 2) {
        record_id = (int)args[0];
        return plc_native_record_find(frame, record_id, (int)args[1]) >= 0
            ? 1.0 : 0.0;
    }
    if (strcmp(name, "record_size") == 0 && argc == 1) {
        record_id = (int)args[0];
        return plc_native_record_ok(frame, record_id)
            ? (double)frame->record_sizes[record_id] : 0.0;
    }
    if (strcmp(name, "complex") == 0 && argc == 2) {
        return (double)plc_native_new_complex(frame, args[0], args[1]);
    }
    if (strcmp(name, "complex_real") == 0 && argc == 1) {
        complex_id = (int)args[0];
        return plc_native_complex_ok(frame, complex_id)
            ? frame->complex_real[complex_id] : 0.0;
    }
    if (strcmp(name, "complex_imag") == 0 && argc == 1) {
        complex_id = (int)args[0];
        return plc_native_complex_ok(frame, complex_id)
            ? frame->complex_imag[complex_id] : 0.0;
    }
    if ((strcmp(name, "complex_add") == 0
            || strcmp(name, "complex_sub") == 0
            || strcmp(name, "complex_mul") == 0) && argc == 2) {
        int other_complex;
        double ar;
        double ai;
        double br;
        double bi;
        double real;
        double imag;

        complex_id = (int)args[0];
        other_complex = (int)args[1];
        if (!plc_native_complex_ok(frame, complex_id)
                || !plc_native_complex_ok(frame, other_complex)) {
            return 0.0;
        }
        ar = frame->complex_real[complex_id];
        ai = frame->complex_imag[complex_id];
        br = frame->complex_real[other_complex];
        bi = frame->complex_imag[other_complex];
        if (strcmp(name, "complex_add") == 0) {
            real = ar + br;
            imag = ai + bi;
        } else if (strcmp(name, "complex_sub") == 0) {
            real = ar - br;
            imag = ai - bi;
        } else {
            real = ar * br - ai * bi;
            imag = ar * bi + ai * br;
        }
        return (double)plc_native_new_complex(frame, real, imag);
    }
    if (strcmp(name, "complex_conj") == 0 && argc == 1) {
        complex_id = (int)args[0];
        if (!plc_native_complex_ok(frame, complex_id)) {
            return 0.0;
        }
        return (double)plc_native_new_complex(frame,
            frame->complex_real[complex_id],
            0.0 - frame->complex_imag[complex_id]);
    }
    if (strcmp(name, "complex_norm2") == 0 && argc == 1) {
        double real;
        double imag;

        complex_id = (int)args[0];
        if (!plc_native_complex_ok(frame, complex_id)) {
            return 0.0;
        }
        real = frame->complex_real[complex_id];
        imag = frame->complex_imag[complex_id];
        return real * real + imag * imag;
    }
    if (strcmp(name, "complex_equal") == 0 && argc == 2) {
        int other_complex;

        complex_id = (int)args[0];
        other_complex = (int)args[1];
        if (!plc_native_complex_ok(frame, complex_id)
                || !plc_native_complex_ok(frame, other_complex)) {
            return 0.0;
        }
        return fabs(frame->complex_real[complex_id]
                - frame->complex_real[other_complex]) < 0.0000001
            && fabs(frame->complex_imag[complex_id]
                - frame->complex_imag[other_complex]) < 0.0000001
            ? 1.0 : 0.0;
    }
    if (strcmp(name, "vec3") == 0 && argc == 3) {
        return (double)plc_native_new_vec3(frame, args[0], args[1], args[2]);
    }
    if ((strcmp(name, "vec3_x") == 0 || strcmp(name, "vec3_y") == 0
            || strcmp(name, "vec3_z") == 0) && argc == 1) {
        int component;

        vec_id = (int)args[0];
        if (!plc_native_vec3_ok(frame, vec_id)) {
            return 0.0;
        }
        component = 0;
        if (strcmp(name, "vec3_y") == 0) {
            component = 1;
        } else if (strcmp(name, "vec3_z") == 0) {
            component = 2;
        }
        return frame->vec3_values[vec_id][component];
    }
    if ((strcmp(name, "vec3_add") == 0
            || strcmp(name, "vec3_sub") == 0
            || strcmp(name, "vec3_dot") == 0
            || strcmp(name, "vec3_cross") == 0) && argc == 2) {
        double ax;
        double ay;
        double az;
        double bx;
        double by;
        double bz;
        double x;
        double y;
        double z;

        vec_id = (int)args[0];
        other_vec = (int)args[1];
        if (!plc_native_vec3_ok(frame, vec_id)
                || !plc_native_vec3_ok(frame, other_vec)) {
            return 0.0;
        }
        ax = frame->vec3_values[vec_id][0];
        ay = frame->vec3_values[vec_id][1];
        az = frame->vec3_values[vec_id][2];
        bx = frame->vec3_values[other_vec][0];
        by = frame->vec3_values[other_vec][1];
        bz = frame->vec3_values[other_vec][2];
        if (strcmp(name, "vec3_dot") == 0) {
            return ax * bx + ay * by + az * bz;
        }
        if (strcmp(name, "vec3_cross") == 0) {
            x = ay * bz - az * by;
            y = az * bx - ax * bz;
            z = ax * by - ay * bx;
        } else if (strcmp(name, "vec3_add") == 0) {
            x = ax + bx;
            y = ay + by;
            z = az + bz;
        } else {
            x = ax - bx;
            y = ay - by;
            z = az - bz;
        }
        return (double)plc_native_new_vec3(frame, x, y, z);
    }
    if ((strcmp(name, "vec3_scale") == 0
            || strcmp(name, "vec3_len2") == 0
            || strcmp(name, "vec3_normalize") == 0)
            && (argc == 1 || argc == 2)) {
        double x;
        double y;
        double z;
        double len2;
        double len;

        if (strcmp(name, "vec3_scale") == 0 && argc != 2) {
            return 0.0;
        }
        if (strcmp(name, "vec3_scale") != 0 && argc != 1) {
            return 0.0;
        }
        vec_id = (int)args[0];
        if (!plc_native_vec3_ok(frame, vec_id)) {
            return 0.0;
        }
        x = frame->vec3_values[vec_id][0];
        y = frame->vec3_values[vec_id][1];
        z = frame->vec3_values[vec_id][2];
        len2 = x * x + y * y + z * z;
        if (strcmp(name, "vec3_len2") == 0) {
            return len2;
        }
        if (strcmp(name, "vec3_scale") == 0) {
            x *= args[1];
            y *= args[1];
            z *= args[1];
        } else {
            if (len2 <= 0.0) {
                return 0.0;
            }
            len = sqrt(len2);
            x /= len;
            y /= len;
            z /= len;
        }
        return (double)plc_native_new_vec3(frame, x, y, z);
    }
    if ((strcmp(name, "mat4_identity") == 0
            || strcmp(name, "mat4_translate") == 0
            || strcmp(name, "mat4_scale") == 0)
            && (argc == 0 || argc == 3)) {
        double m[16];
        int j;

        if (strcmp(name, "mat4_identity") == 0 && argc != 0) {
            return 0.0;
        }
        if (strcmp(name, "mat4_identity") != 0 && argc != 3) {
            return 0.0;
        }
        for (j = 0; j < 16; ++j) {
            m[j] = 0.0;
        }
        m[0] = 1.0;
        m[5] = 1.0;
        m[10] = 1.0;
        m[15] = 1.0;
        if (strcmp(name, "mat4_translate") == 0) {
            m[3] = args[0];
            m[7] = args[1];
            m[11] = args[2];
        } else if (strcmp(name, "mat4_scale") == 0) {
            m[0] = args[0];
            m[5] = args[1];
            m[10] = args[2];
        }
        return (double)plc_native_new_mat4(frame, m);
    }
    if (strcmp(name, "mat4_mul") == 0 && argc == 2) {
        double m[16];
        int row;
        int col;
        int k;

        mat_id = (int)args[0];
        other_mat = (int)args[1];
        if (!plc_native_mat4_ok(frame, mat_id)
                || !plc_native_mat4_ok(frame, other_mat)) {
            return 0.0;
        }
        for (row = 0; row < 4; ++row) {
            for (col = 0; col < 4; ++col) {
                double value;

                value = 0.0;
                for (k = 0; k < 4; ++k) {
                    value += frame->mat4_values[mat_id][row * 4 + k]
                        * frame->mat4_values[other_mat][k * 4 + col];
                }
                m[row * 4 + col] = value;
            }
        }
        return (double)plc_native_new_mat4(frame, m);
    }
    if (strcmp(name, "mat4_transform_point") == 0 && argc == 2) {
        double x;
        double y;
        double z;
        double w;
        double nx;
        double ny;
        double nz;
        const double *m;

        mat_id = (int)args[0];
        vec_id = (int)args[1];
        if (!plc_native_mat4_ok(frame, mat_id)
                || !plc_native_vec3_ok(frame, vec_id)) {
            return 0.0;
        }
        m = frame->mat4_values[mat_id];
        x = frame->vec3_values[vec_id][0];
        y = frame->vec3_values[vec_id][1];
        z = frame->vec3_values[vec_id][2];
        nx = m[0] * x + m[1] * y + m[2] * z + m[3];
        ny = m[4] * x + m[5] * y + m[6] * z + m[7];
        nz = m[8] * x + m[9] * y + m[10] * z + m[11];
        w = m[12] * x + m[13] * y + m[14] * z + m[15];
        if (fabs(w) > 0.0000001 && fabs(w - 1.0) > 0.0000001) {
            nx /= w;
            ny /= w;
            nz /= w;
        }
        return (double)plc_native_new_vec3(frame, nx, ny, nz);
    }
    if (strcmp(name, "perspective_project") == 0 && argc == 2) {
        double x;
        double y;
        double z;
        double focal;

        vec_id = (int)args[0];
        if (!plc_native_vec3_ok(frame, vec_id)) {
            return 0.0;
        }
        x = frame->vec3_values[vec_id][0];
        y = frame->vec3_values[vec_id][1];
        z = frame->vec3_values[vec_id][2];
        if (fabs(z) < 0.0000001) {
            return 0.0;
        }
        focal = args[1];
        return (double)plc_native_new_vec3(frame,
            focal * x / z, focal * y / z, z);
    }
    if ((strcmp(name, "list_equal") == 0 || strcmp(name, "set_equal") == 0)
            && argc == 2) {
        int ok;

        list_id = (int)args[0];
        other_id = (int)args[1];
        if (!plc_native_list_ok(frame, list_id)
                || !plc_native_list_ok(frame, other_id)) {
            return 0.0;
        }
        ok = frame->list_sizes[list_id] == frame->list_sizes[other_id];
        if (ok && strcmp(name, "list_equal") == 0) {
            for (i = 0; i < frame->list_sizes[list_id]; ++i) {
                if (fabs(frame->lists[list_id][i]
                        - frame->lists[other_id][i]) > 0.0000001) {
                    ok = 0;
                }
            }
        } else if (ok) {
            for (i = 0; i < frame->list_sizes[list_id]; ++i) {
                if (!plc_native_list_contains(frame, other_id,
                        frame->lists[list_id][i])) {
                    ok = 0;
                }
            }
        }
        return ok ? 1.0 : 0.0;
    }
    if (strcmp(name, "pair_equal") == 0 && argc == 2) {
        pair_id = (int)args[0];
        other_id = (int)args[1];
        if (pair_id < 0 || pair_id >= frame->pair_count
                || other_id < 0 || other_id >= frame->pair_count) {
            return 0.0;
        }
        return fabs(frame->pair_left[pair_id]
                - frame->pair_left[other_id]) < 0.0000001
            && fabs(frame->pair_right[pair_id]
                - frame->pair_right[other_id]) < 0.0000001
            ? 1.0 : 0.0;
    }
    if ((strcmp(name, "record_equal") == 0
            || strcmp(name, "record_merge") == 0) && argc == 2) {
        int other_record;

        record_id = (int)args[0];
        other_record = (int)args[1];
        if (!plc_native_record_ok(frame, record_id)
                || !plc_native_record_ok(frame, other_record)) {
            return 0.0;
        }
        if (strcmp(name, "record_equal") == 0) {
            int ok;

            ok = frame->record_sizes[record_id]
                == frame->record_sizes[other_record];
            for (i = 0; ok && i < frame->record_sizes[record_id]; ++i) {
                int slot;

                slot = plc_native_record_find(frame, other_record,
                    frame->record_keys[record_id][i]);
                if (slot < 0 || fabs(frame->record_values[record_id][i]
                        - frame->record_values[other_record][slot])
                        > 0.0000001) {
                    ok = 0;
                }
            }
            return ok ? 1.0 : 0.0;
        }
        out_id = plc_native_new_record(frame);
        if (out_id < 0) {
            return 0.0;
        }
        for (i = 0; i < frame->record_sizes[record_id]; ++i) {
            plc_native_record_set(frame, out_id,
                frame->record_keys[record_id][i],
                frame->record_values[record_id][i]);
        }
        for (i = 0; i < frame->record_sizes[other_record]; ++i) {
            plc_native_record_set(frame, out_id,
                frame->record_keys[other_record][i],
                frame->record_values[other_record][i]);
        }
        return (double)out_id;
    }
    if ((strcmp(name, "relation_inverse") == 0
            || strcmp(name, "relation_image") == 0)
            && (argc == 1 || argc == 2)) {
        list_id = (int)args[0];
        out_id = plc_native_new_list(frame);
        if (out_id < 0 || !plc_native_list_ok(frame, list_id)) {
            return 0.0;
        }
        if (strcmp(name, "relation_inverse") == 0 && argc != 1) {
            return 0.0;
        }
        if (strcmp(name, "relation_image") == 0 && argc != 2) {
            return 0.0;
        }
        for (i = 0; i < frame->list_sizes[list_id]; ++i) {
            int relation_pair;

            relation_pair = (int)frame->lists[list_id][i];
            if (relation_pair < 0 || relation_pair >= frame->pair_count) {
                return 0.0;
            }
            if (strcmp(name, "relation_inverse") == 0) {
                if (frame->pair_count >= PLC_NATIVE_MAX_PAIRS) {
                    return 0.0;
                }
                pair_id = frame->pair_count;
                frame->pair_left[pair_id] = frame->pair_right[relation_pair];
                frame->pair_right[pair_id] = frame->pair_left[relation_pair];
                ++frame->pair_count;
                plc_native_list_push_value(frame, out_id, (double)pair_id);
            } else if (fabs(frame->pair_left[relation_pair] - args[1])
                    < 0.0000001
                    && !plc_native_list_contains(frame, out_id,
                        frame->pair_right[relation_pair])) {
                plc_native_list_push_value(frame, out_id,
                    frame->pair_right[relation_pair]);
            }
        }
        return (double)out_id;
    }
    if (strcmp(name, "chess_square") == 0 && argc == 2) {
        return (double)plc_native_chess_square((int)args[0], (int)args[1]);
    }
    if (strcmp(name, "chess_piece") == 0 && argc == 2) {
        return args[0] * 10.0 + args[1];
    }
    if (strcmp(name, "chess_board_new") == 0 && argc == 0) {
        return (double)plc_native_new_record(frame);
    }
    if (strcmp(name, "chess_board_place") == 0 && argc == 3) {
        record_id = (int)args[0];
        plc_native_record_set(frame, record_id, (int)args[1], args[2]);
        return (double)record_id;
    }
    if (strcmp(name, "chess_board_piece") == 0 && argc == 2) {
        int slot;

        record_id = (int)args[0];
        slot = plc_native_record_find(frame, record_id, (int)args[1]);
        return slot >= 0 ? frame->record_values[record_id][slot] : 0.0;
    }
    if ((strcmp(name, "chess_rook_attack_map") == 0
            || strcmp(name, "chess_bishop_attack_map") == 0
            || strcmp(name, "chess_knight_attack_map") == 0
            || strcmp(name, "chess_queen_attack_map") == 0
            || strcmp(name, "chess_king_attack_map") == 0) && argc == 2) {
        int kind;

        kind = 6;
        if (strcmp(name, "chess_rook_attack_map") == 0) {
            kind = 4;
        } else if (strcmp(name, "chess_bishop_attack_map") == 0) {
            kind = 3;
        } else if (strcmp(name, "chess_knight_attack_map") == 0) {
            kind = 2;
        } else if (strcmp(name, "chess_queen_attack_map") == 0) {
            kind = 5;
        }
        return (double)plc_native_chess_attack_map(frame, kind,
            (int)args[0], (int)args[1]);
    }
    if (strcmp(name, "chess_piece_attacks_square") == 0 && argc == 4) {
        list_id = plc_native_chess_attack_map(frame, (int)args[0],
            (int)args[1], (int)args[2]);
        if (list_id < 0) {
            return 0.0;
        }
        return plc_native_list_contains(frame, list_id, args[3])
            ? 1.0 : 0.0;
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
