#include "plankac_internal.h"

static int plc_tag_for_family(const PLC_TYPE_SPEC *spec)
{
    if (spec == 0) {
        return PLC_VALUE_TAG_NUMERIC;
    }
    if (spec->family == PLC_TYPE_FAMILY_BOOLEAN) {
        return PLC_VALUE_TAG_BIT;
    }
    if (spec->family == PLC_TYPE_FAMILY_LIST
            || spec->family == PLC_TYPE_FAMILY_SET
            || spec->family == PLC_TYPE_FAMILY_PAIR
            || spec->family == PLC_TYPE_FAMILY_RECORD
            || spec->family == PLC_TYPE_FAMILY_COMPLEX
            || spec->family == PLC_TYPE_FAMILY_VEC3
            || spec->family == PLC_TYPE_FAMILY_MAT4) {
        return PLC_VALUE_TAG_HANDLE;
    }
    if (spec->scale > 0) {
        return PLC_VALUE_TAG_FIXED;
    }
    return PLC_VALUE_TAG_NUMERIC;
}

void plc_tagged_from_double(PLC_TAGGED_VALUE *out, double value,
    const char *type_marker)
{
    PLC_TYPE_SPEC spec;
    char err[PLC_MAX_LINE];
    int has_type;

    if (out == 0) {
        return;
    }
    memset(out, 0, sizeof(*out));
    out->number = value;
    out->family = PLC_TYPE_FAMILY_NUMERIC;
    out->bits = 64;
    out->scale = 0;
    out->tag = PLC_VALUE_TAG_NUMERIC;
    has_type = 0;
    if (type_marker != 0 && type_marker[0] != '\0') {
        err[0] = '\0';
        if (plc_parse_type_marker_text(type_marker, &spec,
                err, sizeof(err))) {
            has_type = 1;
        }
    }
    if (has_type) {
        out->family = spec.family;
        out->bits = spec.bits;
        out->scale = spec.scale;
        out->tag = plc_tag_for_family(&spec);
    }
    if (out->tag == PLC_VALUE_TAG_BIT) {
        out->raw = value != 0.0 ? 1LL : 0LL;
        out->number = out->raw != 0LL ? 1.0 : 0.0;
    } else if (out->tag == PLC_VALUE_TAG_FIXED) {
        out->raw = plc_fixed_raw_from_double(value, out->scale);
        out->number = plc_fixed_double_from_raw(out->raw, out->scale);
    } else if (out->tag == PLC_VALUE_TAG_HANDLE) {
        out->handle = (int)value;
        out->raw = (long long)out->handle;
    } else {
        out->raw = (long long)value;
    }
}

double plc_tagged_to_double(const PLC_TAGGED_VALUE *value)
{
    if (value == 0) {
        return 0.0;
    }
    if (value->tag == PLC_VALUE_TAG_BIT) {
        return value->raw != 0LL ? 1.0 : 0.0;
    }
    if (value->tag == PLC_VALUE_TAG_FIXED) {
        return plc_fixed_double_from_raw(value->raw, value->scale);
    }
    if (value->tag == PLC_VALUE_TAG_HANDLE) {
        return (double)value->handle;
    }
    return value->number;
}

const char *plc_value_tag_name(int tag)
{
    if (tag == PLC_VALUE_TAG_EMPTY) {
        return "empty";
    }
    if (tag == PLC_VALUE_TAG_NUMERIC) {
        return "numeric";
    }
    if (tag == PLC_VALUE_TAG_BIT) {
        return "bit";
    }
    if (tag == PLC_VALUE_TAG_FIXED) {
        return "fixed";
    }
    if (tag == PLC_VALUE_TAG_HANDLE) {
        return "handle";
    }
    if (tag == PLC_VALUE_TAG_EXCEPTION) {
        return "exception";
    }
    return "unknown";
}

static int plc_value_shadow_matches(const PLC_VALUE_SHADOW *shadow,
    const PLC_REF *ref)
{
    if (shadow->bank != ref->bank || shadow->index != ref->index
            || shadow->has_subscript != ref->has_subscript
            || shadow->has_field != ref->has_field) {
        return 0;
    }
    if (ref->has_subscript && shadow->subscript != ref->subscript) {
        return 0;
    }
    if (ref->has_field && strcmp(shadow->field, ref->field) != 0) {
        return 0;
    }
    return 1;
}

static PLC_TAGGED_VALUE *plc_frame_scalar_slot(PLC_FRAME *frame,
    const PLC_REF *ref)
{
    if (frame == 0 || ref == 0 || ref->has_subscript || ref->has_field
            || ref->index < 0 || ref->index >= PLC_MAX_VARS) {
        return 0;
    }
    if (ref->bank == 'V') {
        return &frame->vb[ref->index];
    }
    if (ref->bank == 'C') {
        return &frame->cb[ref->index];
    }
    if (ref->bank == 'Z') {
        return &frame->zb[ref->index];
    }
    if (ref->bank == 'R') {
        return &frame->rb[ref->index];
    }
    return 0;
}

static const PLC_TAGGED_VALUE *plc_frame_scalar_const_slot(
    const PLC_FRAME *frame, const PLC_REF *ref)
{
    if (frame == 0 || ref == 0 || ref->has_subscript || ref->has_field
            || ref->index < 0 || ref->index >= PLC_MAX_VARS) {
        return 0;
    }
    if (ref->bank == 'V') {
        return &frame->vb[ref->index];
    }
    if (ref->bank == 'C') {
        return &frame->cb[ref->index];
    }
    if (ref->bank == 'Z') {
        return &frame->zb[ref->index];
    }
    if (ref->bank == 'R') {
        return &frame->rb[ref->index];
    }
    return 0;
}

int plc_frame_store_tagged(PLC_FRAME *frame, const PLC_REF *ref,
    double value, const char *type_marker, char *err, unsigned err_size)
{
    PLC_VALUE_SHADOW *shadow;
    PLC_TAGGED_VALUE *slot;
    int i;

    if (frame == 0 || ref == 0) {
        plc_set_error(err, err_size, "tagged value storage needs a frame");
        return 0;
    }
    slot = plc_frame_scalar_slot(frame, ref);
    if (slot != 0) {
        plc_tagged_from_double(slot, value, type_marker);
        return 1;
    }
    for (i = 0; i < frame->value_shadow_count; ++i) {
        if (plc_value_shadow_matches(&frame->value_shadows[i], ref)) {
            plc_tagged_from_double(&frame->value_shadows[i].value,
                value, type_marker);
            return 1;
        }
    }
    if (frame->value_shadow_count >= PLC_MAX_VALUE_SHADOWS) {
        plc_set_error(err, err_size, "too many tagged values");
        return 0;
    }
    shadow = &frame->value_shadows[frame->value_shadow_count];
    memset(shadow, 0, sizeof(*shadow));
    shadow->bank = ref->bank;
    shadow->index = ref->index;
    shadow->has_subscript = ref->has_subscript;
    shadow->subscript = ref->subscript;
    shadow->has_field = ref->has_field;
    if (ref->has_field) {
        strncpy(shadow->field, ref->field, sizeof(shadow->field) - 1);
        shadow->field[sizeof(shadow->field) - 1] = '\0';
    }
    plc_tagged_from_double(&shadow->value, value, type_marker);
    ++frame->value_shadow_count;
    return 1;
}

int plc_frame_load_tagged(const PLC_FRAME *frame, const PLC_REF *ref,
    PLC_TAGGED_VALUE *out)
{
    const PLC_TAGGED_VALUE *slot;
    int i;

    if (frame == 0 || ref == 0 || out == 0) {
        return 0;
    }
    slot = plc_frame_scalar_const_slot(frame, ref);
    if (slot != 0 && slot->tag != PLC_VALUE_TAG_EMPTY) {
        *out = *slot;
        return 1;
    }
    for (i = 0; i < frame->value_shadow_count; ++i) {
        if (plc_value_shadow_matches(&frame->value_shadows[i], ref)) {
            *out = frame->value_shadows[i].value;
            return 1;
        }
    }
    return 0;
}
