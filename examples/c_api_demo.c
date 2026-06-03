#include <stdio.h>

#include "plankac.h"

int main(void)
{
    PLANKAC_CONTEXT *ctx;
    PLANKAC_RESULT result;
    PLANKAC_TYPED_RESULT typed_result;
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
    if (plankac_context_find_proc(ctx, "complex_norm_session", &info)) {
        printf("found P%d %s args=%d results=%d\n",
            info.number, info.name, info.argc, info.results);
    }
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

    if (!plankac_context_run_typed(ctx, "divide_checked", args, 2,
            &typed_result, err, sizeof(err))) {
        printf("typed run failed: %s\n", err);
        plankac_destroy(ctx);
        return 1;
    }
    printf("typed divide status -> tag=%d raw=%lld type=%s\n",
        typed_result.value[1].tag, typed_result.value[1].raw,
        typed_result.value[1].type_text);

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

    if (!plankac_context_run_number(ctx, 140, 0, 0,
            &result, err, sizeof(err))) {
        printf("run failed: %s\n", err);
        plankac_destroy(ctx);
        return 1;
    }
    plankac_format(result.value[0], text, sizeof(text));
    printf("P140 complex_norm_session() -> R0=%s\n", text);

    if (!plankac_context_run(ctx, "three_d_pipeline_session", 0, 0,
            &result, err, sizeof(err))) {
        printf("run failed: %s\n", err);
        plankac_destroy(ctx);
        return 1;
    }
    plankac_format(result.value[0], text, sizeof(text));
    printf("three_d_pipeline_session() -> R0=%s\n", text);

    if (!plankac_context_write_asm_runtime(ctx,
            "build/plankac_api_demo_runtime.S", err, sizeof(err))) {
        printf("asm backend failed: %s\n", err);
        plankac_destroy(ctx);
        return 1;
    }
    printf("wrote build/plankac_api_demo_runtime.S\n");

    if (!plankac_context_write_doscom(ctx,
            "build/plankac_api_demo_dos.com", err, sizeof(err))) {
        printf("doscom backend failed: %s\n", err);
        plankac_destroy(ctx);
        return 1;
    }
    printf("wrote build/plankac_api_demo_dos.com\n");

    plankac_destroy(ctx);
    return 0;
}
