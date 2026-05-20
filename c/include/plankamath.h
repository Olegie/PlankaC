#ifndef PLANKAMATH_H
#define PLANKAMATH_H

#ifdef __cplusplus
extern "C" {
#endif

#define PM_OK 0
#define PM_ERR 1

typedef struct PM_PAIR {
    double value;
    int error;
} PM_PAIR;

typedef struct PM_PROC {
    int number;
    const char *name;
    int argc;
    int results;
} PM_PROC;

extern const PM_PROC PM_PROCS[];
extern const unsigned PM_PROC_COUNT;

double pm_add(double v0, double v1);
double pm_subtract(double v0, double v1);
double pm_multiply(double v0, double v1);
double pm_negate(double v0);
PM_PAIR pm_divide_checked(double v0, double v1);
PM_PAIR pm_modulo_checked(double v0, double v1);
double pm_average2(double v0, double v1);

int pm_equal(double v0, double v1);
int pm_equal_bool(int v0, int v1);
int pm_less(double v0, double v1);
double pm_maximum(double v0, double v1);
double pm_minimum(double v0, double v1);
double pm_absolute(double v0);
double pm_sign(double v0);

double pm_square(double v0);
double pm_power2(double v0, int v1);
PM_PAIR pm_root_checked(double v0);
PM_PAIR pm_reciprocal_checked(double v0);
double pm_percent_of(double v0, double v1);
PM_PAIR pm_percent_change_checked(double v0, double v1);

double pm_calculator_demo(void);
double pm_chained_expression(double v0, double v1, double v2);
PM_PAIR pm_guarded_division_demo(double v0, double v1);
double pm_compare_and_average(double v0, double v1);

double pm_memory_clear(void);
double pm_memory_store(double v0);
double pm_memory_add(double v0, double v1);
double pm_memory_subtract(double v0, double v1);
double pm_memory_recall(double v0);

int pm_test_add(void);
int pm_test_subtract(void);
int pm_test_multiply(void);
int pm_test_divide_checked(void);
int pm_test_maximum(void);
int pm_test_root_checked(void);
int pm_test_division_by_zero_guard(void);
int pm_test_memory_add(void);
int pm_all_tests(void);

int pm_run_proc(const char *name, const double *args, double *r0, int *r1, int *result_count);
int pm_compile_project(char *out, unsigned out_size);
void pm_format(double value, char *out, unsigned out_size);

#ifdef __cplusplus
}
#endif

#endif
