#include "plankac.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DOS_RUNNER_MAX_ARGS PLANKAC_MAX_ARGS

static void dos_print_help(void)
{
    printf("PlankaC DOS runner %s\n", PLANKAC_VERSION);
    printf("commands:\n");
    printf("  check\n");
    printf("  list\n");
    printf("  run <procedure|Pnumber> [args...]\n");
    printf("  tests\n");
    printf("  bytecode <output.pbc>\n");
    printf("  checkbc <input.pbc>\n");
    printf("  runbc <input.pbc> <procedure|Pnumber> [args...]\n");
    printf("\n");
    printf("examples:\n");
    printf("  PLANKACD check\n");
    printf("  PLANKACD run add 12 8\n");
    printf("  PLANKACD bytecode PLANKAC.PBC\n");
    printf("  PLANKACD runbc PLANKAC.PBC set_session\n");
}

static int dos_load_default(PLANKAC_CONTEXT *ctx)
{
    char err[256];

    err[0] = '\0';
    if (!plankac_context_load_default(ctx, err, sizeof(err))) {
        printf("load failed: %s\n", err);
        return 0;
    }
    return 1;
}

static int dos_load_bytecode(PLANKAC_CONTEXT *ctx, const char *path)
{
    char err[256];

    err[0] = '\0';
    if (!plankac_context_load_bytecode(ctx, path, err, sizeof(err))) {
        printf("bytecode load failed: %s\n", err);
        return 0;
    }
    return 1;
}

static void dos_print_summary(PLANKAC_CONTEXT *ctx)
{
    char summary[256];

    summary[0] = '\0';
    if (plankac_context_summary(ctx, summary, sizeof(summary))) {
        printf("%s\n", summary);
    }
}

static int dos_list(PLANKAC_CONTEXT *ctx)
{
    PLANKAC_PROC_INFO info;
    int count;
    int i;

    count = plankac_context_proc_count(ctx);
    for (i = 0; i < count; ++i) {
        if (plankac_context_get_proc(ctx, i, &info)) {
            printf("P%d %s argc=%d results=%d statements=%d\n",
                info.number, info.name, info.argc, info.results,
                info.statements);
        }
    }
    return 0;
}

static int dos_parse_args(int argc, char **argv, double *args)
{
    int i;

    if (argc > DOS_RUNNER_MAX_ARGS) {
        printf("too many arguments: %d\n", argc);
        return 0;
    }
    for (i = 0; i < DOS_RUNNER_MAX_ARGS; ++i) {
        args[i] = 0.0;
    }
    for (i = 0; i < argc; ++i) {
        args[i] = atof(argv[i]);
    }
    return 1;
}

static int dos_run(PLANKAC_CONTEXT *ctx, const char *name,
    int argc, char **argv)
{
    PLANKAC_PROC_INFO info;
    PLANKAC_RESULT result;
    double args[DOS_RUNNER_MAX_ARGS];
    char err[256];
    char text[64];
    int i;

    if (!plankac_context_find_proc(ctx, name, &info)) {
        printf("unknown procedure: %s\n", name);
        return 1;
    }
    if (argc != info.argc) {
        printf("%s expects %d argument(s), got %d\n",
            info.name, info.argc, argc);
        return 1;
    }
    if (!dos_parse_args(argc, argv, args)) {
        return 1;
    }
    memset(&result, 0, sizeof(result));
    err[0] = '\0';
    if (!plankac_context_run(ctx, name, args, argc,
            &result, err, sizeof(err))) {
        printf("run failed: %s\n", err);
        return 1;
    }
    printf("%s ->", info.name);
    for (i = 0; i < result.count; ++i) {
        text[0] = '\0';
        plankac_format(result.value[i], text, sizeof(text));
        printf(" R%d=%s", i, text);
    }
    printf("\n");
    if (strcmp(info.name, "all_tests") == 0 && result.count > 0) {
        return result.value[0] != 0.0 ? 0 : 1;
    }
    return 0;
}

static int dos_write_bytecode(PLANKAC_CONTEXT *ctx, const char *path)
{
    char err[256];

    err[0] = '\0';
    if (!plankac_context_write_bytecode(ctx, path, err, sizeof(err))) {
        printf("bytecode failed: %s\n", err);
        return 1;
    }
    printf("bytecode written: %s\n", path);
    return 0;
}

int main(int argc, char **argv)
{
    PLANKAC_CONTEXT *ctx;
    int rc;

    if (argc < 2) {
        dos_print_help();
        return 0;
    }
    ctx = plankac_create();
    if (ctx == 0) {
        printf("out of memory\n");
        return 1;
    }
    rc = 1;
    if (strcmp(argv[1], "check") == 0) {
        if (dos_load_default(ctx)) {
            dos_print_summary(ctx);
            rc = 0;
        }
    } else if (strcmp(argv[1], "list") == 0) {
        if (dos_load_default(ctx)) {
            rc = dos_list(ctx);
        }
    } else if (strcmp(argv[1], "run") == 0) {
        if (argc < 3) {
            printf("missing procedure name\n");
        } else if (dos_load_default(ctx)) {
            rc = dos_run(ctx, argv[2], argc - 3, argv + 3);
        }
    } else if (strcmp(argv[1], "tests") == 0) {
        if (dos_load_default(ctx)) {
            rc = dos_run(ctx, "all_tests", 0, argv + argc);
        }
    } else if (strcmp(argv[1], "bytecode") == 0) {
        if (argc < 3) {
            printf("missing bytecode output path\n");
        } else if (dos_load_default(ctx)) {
            rc = dos_write_bytecode(ctx, argv[2]);
        }
    } else if (strcmp(argv[1], "checkbc") == 0) {
        if (argc < 3) {
            printf("missing bytecode input path\n");
        } else if (dos_load_bytecode(ctx, argv[2])) {
            dos_print_summary(ctx);
            rc = 0;
        }
    } else if (strcmp(argv[1], "runbc") == 0) {
        if (argc < 4) {
            printf("usage: runbc <input.pbc> <procedure|Pnumber> [args...]\n");
        } else if (dos_load_bytecode(ctx, argv[2])) {
            rc = dos_run(ctx, argv[3], argc - 4, argv + 4);
        }
    } else {
        printf("unknown command: %s\n", argv[1]);
        dos_print_help();
    }
    plankac_destroy(ctx);
    return rc;
}
