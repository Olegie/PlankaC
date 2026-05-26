#include "plankac_internal.h"

static long plc_fixed_scale_factor_bits(int scale)
{
    long factor;
    int i;

    factor = 1L;
    for (i = 0; i < scale && i < 30; ++i) {
        factor *= 2L;
    }
    return factor;
}

unsigned long plc_bit_pack4(int a, int b, int c, int d)
{
    unsigned long word;

    word = 0UL;
    if (a != 0) {
        word |= 8UL;
    }
    if (b != 0) {
        word |= 4UL;
    }
    if (c != 0) {
        word |= 2UL;
    }
    if (d != 0) {
        word |= 1UL;
    }
    return word;
}

int plc_bit_get_word(unsigned long word, int index)
{
    if (index < 0 || index >= 31) {
        return 0;
    }
    return (word & (1UL << index)) != 0UL ? 1 : 0;
}

long plc_fixed_raw_from_double(double value, int scale)
{
    double factor;

    factor = (double)plc_fixed_scale_factor_bits(scale);
    if (value >= 0.0) {
        return (long)(value * factor + 0.5);
    }
    return (long)(value * factor - 0.5);
}

double plc_fixed_double_from_raw(long raw, int scale)
{
    return (double)raw / (double)plc_fixed_scale_factor_bits(scale);
}

long plc_fixed_raw_add(long left, long right)
{
    return left + right;
}

long plc_fixed_raw_mul(long left, long right, int scale)
{
    long factor;

    factor = plc_fixed_scale_factor_bits(scale);
    if (factor == 0L) {
        return 0L;
    }
    return (left * right) / factor;
}

int plc_fixed_raw_div_checked(long left, long right, int scale, long *out)
{
    long factor;

    if (right == 0L) {
        if (out != 0) {
            *out = 0L;
        }
        return 0;
    }
    factor = plc_fixed_scale_factor_bits(scale);
    if (out != 0) {
        *out = (left * factor) / right;
    }
    return 1;
}

double plc_fixed_quantize_bits(double value, int scale)
{
    return plc_fixed_double_from_raw(
        plc_fixed_raw_from_double(value, scale), scale);
}

double plc_fixed_add_bits(double left, double right, int scale)
{
    long left_raw;
    long right_raw;
    long out_raw;

    left_raw = plc_fixed_raw_from_double(left, scale);
    right_raw = plc_fixed_raw_from_double(right, scale);
    out_raw = plc_fixed_raw_add(left_raw, right_raw);
    return plc_fixed_double_from_raw(out_raw, scale);
}

double plc_fixed_mul_bits(double left, double right, int scale)
{
    long left_raw;
    long right_raw;
    long out_raw;

    left_raw = plc_fixed_raw_from_double(left, scale);
    right_raw = plc_fixed_raw_from_double(right, scale);
    out_raw = plc_fixed_raw_mul(left_raw, right_raw, scale);
    return plc_fixed_double_from_raw(out_raw, scale);
}

int plc_fixed_div_bits(double left, double right, int scale, double *out)
{
    long left_raw;
    long right_raw;
    long out_raw;

    left_raw = plc_fixed_raw_from_double(left, scale);
    right_raw = plc_fixed_raw_from_double(right, scale);
    if (!plc_fixed_raw_div_checked(left_raw, right_raw, scale, &out_raw)) {
        if (out != 0) {
            *out = 0.0;
        }
        return 0;
    }
    if (out != 0) {
        *out = plc_fixed_double_from_raw(out_raw, scale);
    }
    return 1;
}
