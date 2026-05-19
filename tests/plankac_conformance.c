#include <math.h>
#include <stdio.h>
#include <string.h>

#include "../c/plankac.h"

static int nearly(double a, double b)
{
    double d;

    d = a - b;
    if (d < 0.0) {
        d = 0.0 - d;
    }
    return d < 0.000001;
}

static int expect_load_ok(const char *name, const char *path)
{
    PLANKAC_CONTEXT *ctx;
    const char *sources[2];
    char err[256];
    int ok;

    ctx = plankac_create();
    if (ctx == 0) {
        printf("FAIL %s: no context\n", name);
        return 0;
    }
    sources[0] = path;
    sources[1] = 0;
    ok = plankac_context_load_sources(ctx, sources, err, sizeof(err));
    plankac_destroy(ctx);
    if (!ok) {
        printf("FAIL %s: %s\n", name, err);
        return 0;
    }
    printf("OK   %s\n", name);
    return 1;
}

static int expect_load_fail(const char *name, const char *path,
    const char *needle)
{
    PLANKAC_CONTEXT *ctx;
    const char *sources[2];
    char err[256];
    int ok;

    ctx = plankac_create();
    if (ctx == 0) {
        printf("FAIL %s: no context\n", name);
        return 0;
    }
    sources[0] = path;
    sources[1] = 0;
    err[0] = '\0';
    ok = plankac_context_load_sources(ctx, sources, err, sizeof(err));
    plankac_destroy(ctx);
    if (ok) {
        printf("FAIL %s: load unexpectedly passed\n", name);
        return 0;
    }
    if (strstr(err, needle) == 0) {
        printf("FAIL %s: expected '%s', got '%s'\n", name, needle, err);
        return 0;
    }
    printf("OK   %s\n", name);
    return 1;
}

static int expect_run_edge(void)
{
    PLANKAC_CONTEXT *ctx;
    PLANKAC_RESULT result;
    const char *sources[2];
    double args[1];
    char err[256];
    int ok;

    ctx = plankac_create();
    if (ctx == 0) {
        printf("FAIL run_edge: no context\n");
        return 0;
    }
    sources[0] = "tests/conformance/valid_edge.plk";
    sources[1] = 0;
    if (!plankac_context_load_sources(ctx, sources, err, sizeof(err))) {
        printf("FAIL run_edge: %s\n", err);
        plankac_destroy(ctx);
        return 0;
    }

    args[0] = 5.0;
    ok = plankac_context_run(ctx, "edge_guard", args, 1,
        &result, err, sizeof(err));
    if (!ok || result.count != 2 || !nearly(result.value[0], 6.0)
            || !nearly(result.value[1], 0.0)) {
        printf("FAIL run_edge_positive\n");
        plankac_destroy(ctx);
        return 0;
    }

    args[0] = -5.0;
    ok = plankac_context_run(ctx, "edge_guard", args, 1,
        &result, err, sizeof(err));
    if (!ok || result.count != 2 || !nearly(result.value[0], 0.0)
            || !nearly(result.value[1], 1.0)) {
        printf("FAIL run_edge_negative\n");
        plankac_destroy(ctx);
        return 0;
    }

    ok = plankac_context_run(ctx, "edge_guard", args, 0,
        &result, err, sizeof(err));
    if (ok || strstr(err, "expects 1 argument") == 0) {
        printf("FAIL run_arg_count: %s\n", err);
        plankac_destroy(ctx);
        return 0;
    }

    printf("OK   run_edge_cases\n");
    plankac_destroy(ctx);
    return 1;
}

static int expect_extended_features(void)
{
    PLANKAC_CONTEXT *ctx;
    PLANKAC_RESULT result;
    char err[256];
    int ok;

    ctx = plankac_create();
    if (ctx == 0) {
        printf("FAIL extended_features: no context\n");
        return 0;
    }
    if (!plankac_context_load_default(ctx, err, sizeof(err))) {
        printf("FAIL extended_features load: %s\n", err);
        plankac_destroy(ctx);
        return 0;
    }

    ok = plankac_context_run(ctx, "indexed_sum", 0, 0,
        &result, err, sizeof(err));
    if (!ok || !nearly(result.value[0], 30.0)) {
        printf("FAIL indexed_sum\n");
        plankac_destroy(ctx);
        return 0;
    }

    ok = plankac_context_run(ctx, "record_sum", 0, 0,
        &result, err, sizeof(err));
    if (!ok || !nearly(result.value[0], 7.0)) {
        printf("FAIL record_sum\n");
        plankac_destroy(ctx);
        return 0;
    }

    ok = plankac_context_run(ctx, "list_session", 0, 0,
        &result, err, sizeof(err));
    if (!ok || !nearly(result.value[0], 9.0)) {
        printf("FAIL list_session\n");
        plankac_destroy(ctx);
        return 0;
    }

    ok = plankac_context_run(ctx, "two_dimensional_add", 0, 0,
        &result, err, sizeof(err));
    if (!ok || !nearly(result.value[0], 5.0)) {
        printf("FAIL two_dimensional_add\n");
        plankac_destroy(ctx);
        return 0;
    }

    ok = plankac_context_run(ctx, "type_model_session", 0, 0,
        &result, err, sizeof(err));
    if (!ok || !nearly(result.value[0], 9.5)) {
        printf("FAIL type_model_session\n");
        plankac_destroy(ctx);
        return 0;
    }

    {
        double args[2];

        args[0] = 3.0;
        args[1] = 4.0;
        ok = plankac_context_run(ctx, "loop_multiply", args, 2,
            &result, err, sizeof(err));
        if (!ok || !nearly(result.value[0], 12.0)) {
            printf("FAIL loop_multiply\n");
            plankac_destroy(ctx);
            return 0;
        }
    }

    {
        double args[4];

        args[0] = 1.0;
        args[1] = 1.0;
        args[2] = 1.0;
        args[3] = 8.0;
        ok = plankac_context_run(ctx, "chess_rook_move", args, 4,
            &result, err, sizeof(err));
        if (!ok || !nearly(result.value[0], 1.0)) {
            printf("FAIL chess_rook_move\n");
            plankac_destroy(ctx);
            return 0;
        }
        args[2] = 2.0;
        args[3] = 3.0;
        ok = plankac_context_run(ctx, "chess_knight_move", args, 4,
            &result, err, sizeof(err));
        if (!ok || !nearly(result.value[0], 1.0)) {
            printf("FAIL chess_knight_move\n");
            plankac_destroy(ctx);
            return 0;
        }
    }

    ok = plankac_context_run(ctx, "nested_record_session", 0, 0,
        &result, err, sizeof(err));
    if (!ok || !nearly(result.value[0], 25.0)) {
        printf("FAIL nested_record_session\n");
        plankac_destroy(ctx);
        return 0;
    }

    ok = plankac_context_run(ctx, "set_session", 0, 0,
        &result, err, sizeof(err));
    if (!ok || !nearly(result.value[0], 2.0)) {
        printf("FAIL set_session\n");
        plankac_destroy(ctx);
        return 0;
    }

    ok = plankac_context_run(ctx, "set_logic_session", 0, 0,
        &result, err, sizeof(err));
    if (!ok || !nearly(result.value[0], 1.0)) {
        printf("FAIL set_logic_session\n");
        plankac_destroy(ctx);
        return 0;
    }

    ok = plankac_context_run(ctx, "pair_relation_session", 0, 0,
        &result, err, sizeof(err));
    if (!ok || !nearly(result.value[0], 2.0)) {
        printf("FAIL pair_relation_session\n");
        plankac_destroy(ctx);
        return 0;
    }

    ok = plankac_context_run(ctx, "two_dimensional_fuller", 0, 0,
        &result, err, sizeof(err));
    if (!ok || !nearly(result.value[0], 1.0)) {
        printf("FAIL two_dimensional_fuller\n");
        plankac_destroy(ctx);
        return 0;
    }

    ok = plankac_context_run(ctx, "chess_piece_record", 0, 0,
        &result, err, sizeof(err));
    if (!ok || !nearly(result.value[0], 41.0)) {
        printf("FAIL chess_piece_record\n");
        plankac_destroy(ctx);
        return 0;
    }

    ok = plankac_context_run(ctx, "chess_attack_relation", 0, 0,
        &result, err, sizeof(err));
    if (!ok || !nearly(result.value[0], 1.0)) {
        printf("FAIL chess_attack_relation\n");
        plankac_destroy(ctx);
        return 0;
    }

    ok = plankac_context_run(ctx, "logic_not_session", 0, 0,
        &result, err, sizeof(err));
    if (!ok || !nearly(result.value[0], 1.0)) {
        printf("FAIL logic_not_session\n");
        plankac_destroy(ctx);
        return 0;
    }

    printf("OK   extended_features\n");
    plankac_destroy(ctx);
    return 1;
}

static int expect_bytecode_runner(void)
{
    PLANKAC_CONTEXT *ctx;
    PLANKAC_RESULT result;
    char err[256];
    int ok;

    ctx = plankac_create();
    if (ctx == 0) {
        printf("FAIL bytecode_runner: no context\n");
        return 0;
    }
    if (!plankac_context_load_bytecode(ctx, "build/plankamath.pbc",
            err, sizeof(err))) {
        printf("FAIL bytecode_runner load: %s\n", err);
        plankac_destroy(ctx);
        return 0;
    }
    ok = plankac_context_run(ctx, "set_session", 0, 0,
        &result, err, sizeof(err));
    if (!ok || !nearly(result.value[0], 2.0)) {
        printf("FAIL bytecode_runner run: %s\n", err);
        plankac_destroy(ctx);
        return 0;
    }
    printf("OK   bytecode_runner\n");
    plankac_destroy(ctx);
    return 1;
}

static int expect_run_fail(const char *name, const char *path,
    const char *proc, const char *needle)
{
    PLANKAC_CONTEXT *ctx;
    PLANKAC_RESULT result;
    const char *sources[2];
    char err[256];
    int ok;

    ctx = plankac_create();
    if (ctx == 0) {
        printf("FAIL %s: no context\n", name);
        return 0;
    }
    sources[0] = path;
    sources[1] = 0;
    if (!plankac_context_load_sources(ctx, sources, err, sizeof(err))) {
        printf("FAIL %s load: %s\n", name, err);
        plankac_destroy(ctx);
        return 0;
    }
    err[0] = '\0';
    ok = plankac_context_run(ctx, proc, 0, 0, &result, err, sizeof(err));
    plankac_destroy(ctx);
    if (ok) {
        printf("FAIL %s: run unexpectedly passed\n", name);
        return 0;
    }
    if (strstr(err, needle) == 0) {
        printf("FAIL %s: expected '%s', got '%s'\n", name, needle, err);
        return 0;
    }
    printf("OK   %s\n", name);
    return 1;
}

int main(void)
{
    int ok;

    ok = 1;
    ok = expect_load_ok("load_valid_edge",
            "tests/conformance/valid_edge.plk") && ok;
    ok = expect_run_edge() && ok;
    ok = expect_load_fail("missing_end",
            "tests/conformance/bad_missing_end.plk",
            "source ended inside procedure") && ok;
    ok = expect_load_fail("bad_header",
            "tests/conformance/bad_header.plk",
            "missing parameter list") && ok;
    ok = expect_load_fail("stray_end",
            "tests/conformance/bad_stray_end.plk",
            "stray END") && ok;
    ok = expect_load_fail("duplicate_proc",
            "tests/conformance/bad_duplicate_proc.plk",
            "duplicate procedure") && ok;
    ok = expect_load_fail("bad_type_marker",
            "tests/conformance/bad_type_marker.plk",
            "bad type marker") && ok;
    ok = expect_load_fail("bad_type_mismatch",
            "tests/conformance/bad_type_mismatch.plk",
            "type mismatch") && ok;
    ok = expect_load_fail("bad_recursion",
            "tests/conformance/bad_recursion.plk",
            "recursive procedure rejected") && ok;
    ok = expect_load_fail("bad_pnumber_recursion",
            "tests/conformance/bad_pnumber_recursion.plk",
            "recursive procedure rejected") && ok;
    ok = expect_load_fail("bad_interproc_type",
            "tests/conformance/bad_interproc_type.plk",
            "procedure argument type mismatch") && ok;
    ok = expect_run_fail("bad_statement",
            "tests/conformance/bad_statement.plk",
            "bad_statement",
            "too many arrows") && ok;
    ok = expect_run_fail("unknown_call",
            "tests/conformance/bad_unknown_call.plk",
            "bad_call",
            "unknown procedure") && ok;
    ok = expect_run_fail("bad_assertion",
            "tests/conformance/bad_assertion.plk",
            "bad_assertion",
            "assertion failed") && ok;
    ok = expect_extended_features() && ok;
    ok = expect_bytecode_runner() && ok;

    if (!ok) {
        printf("CONFORMANCE FAILED\n");
        return 1;
    }
    printf("CONFORMANCE OK\n");
    return 0;
}
