#include <stdio.h>

#include "plankac.h"

static int host_mad(void *user_data, const double *args, int argc,
    PLANKAC_RESULT *result, char *err, unsigned err_size)
{
    double offset;

    if (argc != 2 || result == 0) {
        if (err != 0 && err_size > 0) {
            sprintf(err, "host_mad expects two arguments");
        }
        return PLANKAC_ERR;
    }
    offset = user_data != 0 ? *((double *)user_data) : 0.0;
    result->count = 2;
    result->value[0] = args[0] * args[1] + offset;
    result->value[1] = 0.0;
    return PLANKAC_OK;
}

int main(void)
{
    PLANKAC_CONTEXT *ctx;
    PLANKAC_RESULT result;
    PLANKAC_NATIVE_INFO native_info;
    const char *sources[2];
    const char *arg_types[2];
    const char *result_types[2];
    double offset;
    double args[2];
    char err[256];

    ctx = plankac_create();
    if (ctx == 0) {
        printf("cannot allocate PlankaC context\n");
        return 1;
    }

    arg_types[0] = "[:32.16]";
    arg_types[1] = "[:32.16]";
    result_types[0] = "[:32.16]";
    result_types[1] = "[:1.1]";
    offset = 6.0;

    if (!plankac_context_register_native(ctx, "host_mad", 2, 2,
            arg_types, result_types, host_mad, &offset,
            err, sizeof(err))) {
        printf("native registration failed: %s\n", err);
        plankac_destroy(ctx);
        return 1;
    }

    sources[0] = "examples/host_abi.plk";
    sources[1] = 0;
    if (!plankac_context_load_sources(ctx, sources, err, sizeof(err))) {
        printf("source load failed: %s\n", err);
        plankac_destroy(ctx);
        return 1;
    }

    if (!plankac_context_find_native(ctx, "host_mad", &native_info)) {
        printf("native metadata lookup failed\n");
        plankac_destroy(ctx);
        return 1;
    }

    args[0] = 6.0;
    args[1] = 6.0;
    if (!plankac_context_run(ctx, "host_bridge", args, 2,
            &result, err, sizeof(err))) {
        printf("run failed: %s\n", err);
        plankac_destroy(ctx);
        return 1;
    }

    printf("native functions: %d\n", plankac_context_native_count(ctx));
    printf("%s argc=%d results=%d\n",
        native_info.name, native_info.argc, native_info.results);
    printf("host_bridge(6, 6) -> R0=%.0f R1=%.0f\n",
        result.value[0], result.value[1]);

    plankac_destroy(ctx);
    return (result.count == 2 && result.value[0] == 42.0
        && result.value[1] == 0.0) ? 0 : 1;
}
