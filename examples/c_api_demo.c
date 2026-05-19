#include <stdio.h>

#include "../c/plankac.h"

int main(void)
{
    PLANKAC_CONTEXT *ctx;
    PLANKAC_RESULT result;
    PLANKAC_PROC_INFO info;
    double args[2];
    char err[256];
    char text[64];
    int i;

    ctx = plankac_create();
    if (ctx == 0) {
        printf("cannot allocate PlankaC context\n");
        return 1;
    }

    if (!plankac_context_load_default(ctx, err, sizeof(err))) {
        printf("load failed: %s\n", err);
        plankac_destroy(ctx);
        return 1;
    }

    printf("procedures: %d\n", plankac_context_proc_count(ctx));
    for (i = 0; i < 5; ++i) {
        if (plankac_context_get_proc(ctx, i, &info)) {
            printf("P%d %s args=%d results=%d\n",
                info.number, info.name, info.argc, info.results);
            if (info.argc > 0 || info.results > 0) {
                printf("  first types: V0=%s R0=%s\n",
                    info.argc > 0 ? info.arg_types[0] : "-",
                    info.results > 0 ? info.result_types[0] : "-");
            }
        }
    }

    args[0] = 84.0;
    args[1] = 0.0;
    if (!plankac_context_run(ctx, "divide_checked", args, 2,
            &result, err, sizeof(err))) {
        printf("run failed: %s\n", err);
        plankac_destroy(ctx);
        return 1;
    }

    plankac_format(result.value[0], text, sizeof(text));
    printf("divide_checked(84, 0) -> R0=%s R1=%.0f\n",
        text, result.value[1]);

    args[0] = 12.0;
    args[1] = 8.0;
    if (!plankac_context_run(ctx, "add", args, 2,
            &result, err, sizeof(err))) {
        printf("run failed: %s\n", err);
        plankac_destroy(ctx);
        return 1;
    }

    plankac_format(result.value[0], text, sizeof(text));
    printf("add(12, 8) -> R0=%s\n", text);

    plankac_destroy(ctx);
    return 0;
}
