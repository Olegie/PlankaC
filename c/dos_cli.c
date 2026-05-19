#include "plankamath.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DOS_MAX_ARGS 8

static void print_value(double value)
{
    char out[64];

    pm_format(value, out, sizeof(out));
    printf("%s", out);
}

static void print_result(double r0, int r1, int count)
{
    printf("R0=");
    print_value(r0);
    if (count > 1) {
        printf(" R1=");
        print_value((double)r1);
    }
    printf("\n");
}

static void print_help(void)
{
    printf("PlankaMath DOS runner\n");
    printf("Commands:\n");
    printf("  demo\n");
    printf("  tests\n");
    printf("  guarded\n");
    printf("  list\n");
    printf("  run <procedure> [V0] [V1] [V2]\n");
    printf("\n");
    printf("Examples:\n");
    printf("  PMDOS demo\n");
    printf("  PMDOS run add 12 8\n");
    printf("  PMDOS run divide_checked 84 0\n");
}

static int list_procs(void)
{
    unsigned i;

    for (i = 0; i < PM_PROC_COUNT; ++i) {
        printf("P%d %s V=%d R=%d\n",
            PM_PROCS[i].number,
            PM_PROCS[i].name,
            PM_PROCS[i].argc,
            PM_PROCS[i].results);
    }
    return 0;
}

static int run_named(int argc, char **argv)
{
    double args[DOS_MAX_ARGS];
    double r0;
    int r1;
    int count;
    int i;

    if (argc < 3) {
        printf("Missing procedure name.\n");
        return 1;
    }

    for (i = 0; i < DOS_MAX_ARGS; ++i) {
        args[i] = 0.0;
    }
    for (i = 3; i < argc && i - 3 < DOS_MAX_ARGS; ++i) {
        args[i - 3] = atof(argv[i]);
    }

    r0 = 0.0;
    r1 = 0;
    count = 0;
    if (pm_run_proc(argv[2], args, &r0, &r1, &count) != PM_OK) {
        printf("Unknown or failed procedure: %s\n", argv[2]);
        return 1;
    }

    print_result(r0, r1, count);
    return 0;
}

int main(int argc, char **argv)
{
    PM_PAIR pair;

    if (argc < 2) {
        print_help();
        return 0;
    }

    if (strcmp(argv[1], "demo") == 0) {
        print_value(pm_calculator_demo());
        printf("\n");
        return 0;
    }

    if (strcmp(argv[1], "tests") == 0) {
        printf("%d\n", pm_all_tests());
        return pm_all_tests() ? 0 : 1;
    }

    if (strcmp(argv[1], "guarded") == 0) {
        pair = pm_guarded_division_demo(84.0, 0.0);
        print_result(pair.value, pair.error, 2);
        return 0;
    }

    if (strcmp(argv[1], "list") == 0) {
        return list_procs();
    }

    if (strcmp(argv[1], "run") == 0) {
        return run_named(argc, argv);
    }

    print_help();
    return 1;
}
