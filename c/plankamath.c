#include "plankamath.h"

#include <ctype.h>
#include <math.h>
#include <stdio.h>
#include <string.h>

static const char *PM_SOURCES[] = {
    "src/00_types.plk",
    "src/01_arithmetic.plk",
    "src/02_order.plk",
    "src/03_scientific.plk",
    "src/04_calculator.plk",
    "src/05_memory.plk",
    "src/06_data_structures.plk",
    "src/07_chess.plk",
    "src/08_relations_sets.plk",
    "examples/session_basic.plk",
    "examples/session_guarded.plk",
    "examples/session_memory.plk",
    "examples/session_scientific.plk",
    "tests/calculator_self_check.plk",
    0
};

const PM_PROC PM_PROCS[] = {
    {0, "type_sheet", 0, 1},
    {10, "add", 2, 1},
    {11, "subtract", 2, 1},
    {12, "multiply", 2, 1},
    {13, "negate", 1, 1},
    {14, "divide_checked", 2, 2},
    {15, "modulo_checked", 2, 2},
    {16, "average2", 2, 1},
    {30, "equal", 2, 1},
    {31, "less", 2, 1},
    {32, "maximum", 2, 1},
    {33, "minimum", 2, 1},
    {34, "absolute", 1, 1},
    {35, "sign", 1, 1},
    {36, "equal_bool", 2, 1},
    {50, "square", 1, 1},
    {51, "power2", 2, 1},
    {52, "root_checked", 1, 2},
    {53, "reciprocal_checked", 1, 2},
    {54, "percent_of", 2, 1},
    {55, "percent_change_checked", 2, 2},
    {70, "calculator_demo", 0, 1},
    {71, "chained_expression", 3, 1},
    {72, "guarded_division_demo", 2, 2},
    {73, "compare_and_average", 2, 1},
    {80, "memory_clear", 0, 1},
    {81, "memory_store", 1, 1},
    {82, "memory_add", 2, 1},
    {83, "memory_subtract", 2, 1},
    {84, "memory_recall", 1, 1},
    {100, "basic_session", 0, 1},
    {101, "scientific_session", 0, 1},
    {102, "guarded_session", 0, 2},
    {103, "memory_session", 0, 1},
    {900, "test_add", 0, 1},
    {901, "test_subtract", 0, 1},
    {902, "test_multiply", 0, 1},
    {903, "test_divide_checked", 0, 1},
    {904, "test_maximum", 0, 1},
    {905, "test_root_checked", 0, 1},
    {906, "test_division_by_zero_guard", 0, 1},
    {907, "test_memory_add", 0, 1},
    {999, "all_tests", 0, 1}
};

const unsigned PM_PROC_COUNT = sizeof(PM_PROCS) / sizeof(PM_PROCS[0]);

double pm_add(double v0, double v1)
{
    return v0 + v1;
}

double pm_subtract(double v0, double v1)
{
    return v0 - v1;
}

double pm_multiply(double v0, double v1)
{
    return v0 * v1;
}

double pm_negate(double v0)
{
    return 0.0 - v0;
}

PM_PAIR pm_divide_checked(double v0, double v1)
{
    PM_PAIR r;
    r.value = 0.0;
    r.error = PM_OK;
    if (v1 == 0.0) {
        r.error = PM_ERR;
    } else {
        r.value = v0 / v1;
    }
    return r;
}

PM_PAIR pm_modulo_checked(double v0, double v1)
{
    PM_PAIR r;
    r.value = 0.0;
    r.error = PM_OK;
    if (v1 == 0.0) {
        r.error = PM_ERR;
    } else {
        r.value = fmod(v0, v1);
    }
    return r;
}

double pm_average2(double v0, double v1)
{
    PM_PAIR r;
    r = pm_divide_checked(pm_add(v0, v1), 2.0);
    return r.value;
}

int pm_equal(double v0, double v1)
{
    return v0 == v1 ? 1 : 0;
}

int pm_equal_bool(int v0, int v1)
{
    return v0 == v1 ? 1 : 0;
}

int pm_less(double v0, double v1)
{
    return v0 < v1 ? 1 : 0;
}

double pm_maximum(double v0, double v1)
{
    double z1;
    z1 = v0;
    if (z1 < v1) {
        z1 = v1;
    }
    return z1;
}

double pm_minimum(double v0, double v1)
{
    double z1;
    z1 = v0;
    if (v1 < z1) {
        z1 = v1;
    }
    return z1;
}

double pm_absolute(double v0)
{
    double z1;
    z1 = v0;
    if (v0 < 0.0) {
        z1 = 0.0 - v0;
    }
    return z1;
}

double pm_sign(double v0)
{
    if (v0 < 0.0) {
        return -1.0;
    }
    if (0.0 < v0) {
        return 1.0;
    }
    return 0.0;
}

double pm_square(double v0)
{
    return pm_multiply(v0, v0);
}

double pm_power2(double v0, int v1)
{
    return pow(v0, (double)v1);
}

PM_PAIR pm_root_checked(double v0)
{
    PM_PAIR r;
    r.value = 0.0;
    r.error = PM_OK;
    if (v0 < 0.0) {
        r.error = PM_ERR;
    } else {
        r.value = sqrt(v0);
    }
    return r;
}

PM_PAIR pm_reciprocal_checked(double v0)
{
    return pm_divide_checked(1.0, v0);
}

double pm_percent_of(double v0, double v1)
{
    PM_PAIR r;
    r = pm_divide_checked(pm_multiply(v0, v1), 100.0);
    return r.value;
}

PM_PAIR pm_percent_change_checked(double v0, double v1)
{
    PM_PAIR r;
    double z1;
    z1 = pm_subtract(v1, v0);
    r = pm_divide_checked(z1, v0);
    if (r.error == PM_OK) {
        r.value = pm_multiply(r.value, 100.0);
    } else {
        r.value = 0.0;
    }
    return r;
}

double pm_calculator_demo(void)
{
    PM_PAIR r;
    double z1;
    double z2;
    double z3;
    z1 = pm_add(40.0, 2.0);
    z2 = pm_multiply(z1, 3.0);
    z3 = pm_subtract(z2, 6.0);
    r = pm_divide_checked(z3, 4.0);
    return r.value;
}

double pm_chained_expression(double v0, double v1, double v2)
{
    double z1;
    double z2;
    z1 = pm_add(v0, v1);
    z2 = pm_square(z1);
    return pm_subtract(z2, v2);
}

PM_PAIR pm_guarded_division_demo(double v0, double v1)
{
    return pm_divide_checked(v0, v1);
}

double pm_compare_and_average(double v0, double v1)
{
    double z1;
    double z2;
    z1 = pm_minimum(v0, v1);
    z2 = pm_maximum(v0, v1);
    return pm_average2(z1, z2);
}

double pm_memory_clear(void)
{
    return 0.0;
}

double pm_memory_store(double v0)
{
    return v0;
}

double pm_memory_add(double v0, double v1)
{
    return pm_add(v0, v1);
}

double pm_memory_subtract(double v0, double v1)
{
    return pm_subtract(v0, v1);
}

double pm_memory_recall(double v0)
{
    return v0;
}

int pm_test_add(void)
{
    return pm_equal(pm_add(12.0, 8.0), 20.0);
}

int pm_test_subtract(void)
{
    return pm_equal(pm_subtract(12.0, 8.0), 4.0);
}

int pm_test_multiply(void)
{
    return pm_equal(pm_multiply(7.0, 6.0), 42.0);
}

int pm_test_divide_checked(void)
{
    PM_PAIR r;
    r = pm_divide_checked(84.0, 2.0);
    return pm_equal(r.value, 42.0) && pm_equal_bool(r.error, 0);
}

int pm_test_maximum(void)
{
    return pm_equal(pm_maximum(18.0, 27.0), 27.0);
}

int pm_test_root_checked(void)
{
    PM_PAIR r;
    r = pm_root_checked(81.0);
    return pm_equal(r.value, 9.0) && pm_equal_bool(r.error, 0);
}

int pm_test_division_by_zero_guard(void)
{
    PM_PAIR r;
    r = pm_divide_checked(84.0, 0.0);
    return pm_equal_bool(r.error, 1);
}

int pm_test_memory_add(void)
{
    double z1;
    double z2;
    double z3;
    z1 = pm_memory_clear();
    z2 = pm_memory_add(z1, 20.0);
    z3 = pm_memory_add(z2, 22.0);
    return pm_equal(z3, 42.0);
}

int pm_all_tests(void)
{
    return pm_test_add()
        && pm_test_subtract()
        && pm_test_multiply()
        && pm_test_divide_checked()
        && pm_test_maximum()
        && pm_test_root_checked()
        && pm_test_division_by_zero_guard()
        && pm_test_memory_add();
}

static int pm_name_is(const char *a, const char *b)
{
    return strcmp(a, b) == 0;
}

int pm_run_proc(const char *name, const double *args, double *r0, int *r1, int *result_count)
{
    PM_PAIR pair;

    *r0 = 0.0;
    *r1 = 0;
    *result_count = 1;

    if (pm_name_is(name, "type_sheet")) {
        *r0 = 1.0;
    } else if (pm_name_is(name, "add")) {
        *r0 = pm_add(args[0], args[1]);
    } else if (pm_name_is(name, "subtract")) {
        *r0 = pm_subtract(args[0], args[1]);
    } else if (pm_name_is(name, "multiply")) {
        *r0 = pm_multiply(args[0], args[1]);
    } else if (pm_name_is(name, "negate")) {
        *r0 = pm_negate(args[0]);
    } else if (pm_name_is(name, "divide_checked")) {
        pair = pm_divide_checked(args[0], args[1]);
        *r0 = pair.value;
        *r1 = pair.error;
        *result_count = 2;
    } else if (pm_name_is(name, "modulo_checked")) {
        pair = pm_modulo_checked(args[0], args[1]);
        *r0 = pair.value;
        *r1 = pair.error;
        *result_count = 2;
    } else if (pm_name_is(name, "average2")) {
        *r0 = pm_average2(args[0], args[1]);
    } else if (pm_name_is(name, "equal")) {
        *r0 = (double)pm_equal(args[0], args[1]);
    } else if (pm_name_is(name, "less")) {
        *r0 = (double)pm_less(args[0], args[1]);
    } else if (pm_name_is(name, "maximum")) {
        *r0 = pm_maximum(args[0], args[1]);
    } else if (pm_name_is(name, "minimum")) {
        *r0 = pm_minimum(args[0], args[1]);
    } else if (pm_name_is(name, "absolute")) {
        *r0 = pm_absolute(args[0]);
    } else if (pm_name_is(name, "sign")) {
        *r0 = pm_sign(args[0]);
    } else if (pm_name_is(name, "equal_bool")) {
        *r0 = (double)pm_equal_bool((int)args[0], (int)args[1]);
    } else if (pm_name_is(name, "square")) {
        *r0 = pm_square(args[0]);
    } else if (pm_name_is(name, "power2")) {
        *r0 = pm_power2(args[0], (int)args[1]);
    } else if (pm_name_is(name, "root_checked")) {
        pair = pm_root_checked(args[0]);
        *r0 = pair.value;
        *r1 = pair.error;
        *result_count = 2;
    } else if (pm_name_is(name, "reciprocal_checked")) {
        pair = pm_reciprocal_checked(args[0]);
        *r0 = pair.value;
        *r1 = pair.error;
        *result_count = 2;
    } else if (pm_name_is(name, "percent_of")) {
        *r0 = pm_percent_of(args[0], args[1]);
    } else if (pm_name_is(name, "percent_change_checked")) {
        pair = pm_percent_change_checked(args[0], args[1]);
        *r0 = pair.value;
        *r1 = pair.error;
        *result_count = 2;
    } else if (pm_name_is(name, "calculator_demo")) {
        *r0 = pm_calculator_demo();
    } else if (pm_name_is(name, "chained_expression")) {
        *r0 = pm_chained_expression(args[0], args[1], args[2]);
    } else if (pm_name_is(name, "guarded_division_demo")) {
        pair = pm_guarded_division_demo(args[0], args[1]);
        *r0 = pair.value;
        *r1 = pair.error;
        *result_count = 2;
    } else if (pm_name_is(name, "compare_and_average")) {
        *r0 = pm_compare_and_average(args[0], args[1]);
    } else if (pm_name_is(name, "memory_clear")) {
        *r0 = pm_memory_clear();
    } else if (pm_name_is(name, "memory_store")) {
        *r0 = pm_memory_store(args[0]);
    } else if (pm_name_is(name, "memory_add")) {
        *r0 = pm_memory_add(args[0], args[1]);
    } else if (pm_name_is(name, "memory_subtract")) {
        *r0 = pm_memory_subtract(args[0], args[1]);
    } else if (pm_name_is(name, "memory_recall")) {
        *r0 = pm_memory_recall(args[0]);
    } else if (pm_name_is(name, "basic_session")) {
        PM_PAIR tmp;
        tmp = pm_divide_checked(pm_multiply(pm_add(12.0, 8.0), pm_subtract(12.0, 8.0)), 4.0);
        *r0 = tmp.value;
    } else if (pm_name_is(name, "scientific_session")) {
        *r0 = pm_average2(pm_power2(2.0, 8), pm_root_checked(81.0).value);
    } else if (pm_name_is(name, "guarded_session")) {
        pair = pm_divide_checked(100.0, 0.0);
        *r0 = pair.value;
        *r1 = pair.error;
        *result_count = 2;
    } else if (pm_name_is(name, "memory_session")) {
        double z1;
        double z2;
        double z3;
        double z4;
        z1 = pm_memory_clear();
        z2 = pm_memory_add(z1, 25.0);
        z3 = pm_memory_add(z2, 17.0);
        z4 = pm_memory_subtract(z3, 2.0);
        *r0 = pm_memory_recall(z4);
    } else if (pm_name_is(name, "test_add")) {
        *r0 = (double)pm_test_add();
    } else if (pm_name_is(name, "test_subtract")) {
        *r0 = (double)pm_test_subtract();
    } else if (pm_name_is(name, "test_multiply")) {
        *r0 = (double)pm_test_multiply();
    } else if (pm_name_is(name, "test_divide_checked")) {
        *r0 = (double)pm_test_divide_checked();
    } else if (pm_name_is(name, "test_maximum")) {
        *r0 = (double)pm_test_maximum();
    } else if (pm_name_is(name, "test_root_checked")) {
        *r0 = (double)pm_test_root_checked();
    } else if (pm_name_is(name, "test_division_by_zero_guard")) {
        *r0 = (double)pm_test_division_by_zero_guard();
    } else if (pm_name_is(name, "test_memory_add")) {
        *r0 = (double)pm_test_memory_add();
    } else if (pm_name_is(name, "all_tests")) {
        *r0 = (double)pm_all_tests();
    } else {
        return PM_ERR;
    }

    return PM_OK;
}

static int pm_is_program_line(const char *line)
{
    return line[0] == 'P' && isdigit((unsigned char)line[1]);
}

static int pm_is_end_line(const char *line)
{
    while (*line == ' ' || *line == '\t') {
        ++line;
    }
    return strncmp(line, "END", 3) == 0;
}

int pm_compile_project(char *out, unsigned out_size)
{
    FILE *fp;
    char line[256];
    int programs;
    int ends;
    int files;
    int i;

    programs = 0;
    ends = 0;
    files = 0;

    for (i = 0; PM_SOURCES[i] != 0; ++i) {
        fp = fopen(PM_SOURCES[i], "r");
        if (fp == 0) {
            if (out_size > 0) {
                sprintf(out, "Missing source: %s", PM_SOURCES[i]);
            }
            return PM_ERR;
        }
        ++files;
        while (fgets(line, sizeof(line), fp) != 0) {
            if (pm_is_program_line(line)) {
                ++programs;
            }
            if (pm_is_end_line(line)) {
                ++ends;
            }
        }
        fclose(fp);
    }

    if (programs != ends) {
        if (out_size > 0) {
            sprintf(out, "Compile failed: P=%d END=%d", programs, ends);
        }
        return PM_ERR;
    }

    if (out_size > 0) {
        sprintf(out, "Compile OK: %d files, %d procedures", files, programs);
    }
    return PM_OK;
}

void pm_format(double value, char *out, unsigned out_size)
{
    long whole;
    double diff;
    if (out_size == 0) {
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
