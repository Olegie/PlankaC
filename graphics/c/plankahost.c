#include <stdio.h>
#include <string.h>

#include "plankahost.h"

static const char *const PH_BASE_ROOT[] = {
    "src/00_types.plk",
    "src/01_arithmetic.plk",
    "src/02_order.plk",
    "src/03_scientific.plk",
    "src/04_calculator.plk",
    "src/05_memory.plk",
    "src/06_data_structures.plk",
    "src/07_chess.plk",
    "src/08_relations_sets.plk",
    "src/09_complex.plk",
    "src/10_relation_algebra.plk",
    "src/11_structured_values.plk",
    "src/12_relation_composition.plk",
    "src/13_chess_board.plk",
    "src/14_two_dimensional_tables.plk",
    "src/15_3d_geometry.plk",
    "src/16_value_algebra.plk",
    "src/17_chess_model.plk",
    "src/18_two_dimensional_general.plk",
    "examples/session_basic.plk",
    "examples/session_guarded.plk",
    "examples/session_memory.plk",
    "examples/session_scientific.plk",
    "tests/calculator_self_check.plk",
    0
};

static const char *const PH_BASE_BUILD[] = {
    "../src/00_types.plk",
    "../src/01_arithmetic.plk",
    "../src/02_order.plk",
    "../src/03_scientific.plk",
    "../src/04_calculator.plk",
    "../src/05_memory.plk",
    "../src/06_data_structures.plk",
    "../src/07_chess.plk",
    "../src/08_relations_sets.plk",
    "../src/09_complex.plk",
    "../src/10_relation_algebra.plk",
    "../src/11_structured_values.plk",
    "../src/12_relation_composition.plk",
    "../src/13_chess_board.plk",
    "../src/14_two_dimensional_tables.plk",
    "../src/15_3d_geometry.plk",
    "../src/16_value_algebra.plk",
    "../src/17_chess_model.plk",
    "../src/18_two_dimensional_general.plk",
    "../examples/session_basic.plk",
    "../examples/session_guarded.plk",
    "../examples/session_memory.plk",
    "../examples/session_scientific.plk",
    "../tests/calculator_self_check.plk",
    0
};

static void ph_set_error(char *err, unsigned err_size, const char *text)
{
    if (err != 0 && err_size > 0) {
        strncpy(err, text, err_size - 1);
        err[err_size - 1] = '\0';
    }
}

static int ph_is_absolute_path(const char *path)
{
    if (path == 0 || path[0] == '\0') {
        return 0;
    }
    if (path[0] == '/' || path[0] == '\\') {
        return 1;
    }
    return path[1] == ':';
}

static void ph_build_source_path(const char *source, char *out,
    unsigned out_size)
{
    if (out_size == 0) {
        return;
    }
    out[0] = '\0';
    if (source == 0 || source[0] == '\0') {
        strncpy(out, "../graphics/src/plankagui.plk", out_size - 1);
        out[out_size - 1] = '\0';
        return;
    }
    if (ph_is_absolute_path(source)
            || source[0] == '.'
            || strncmp(source, "../", 3) == 0
            || strncmp(source, "..\\", 3) == 0) {
        strncpy(out, source, out_size - 1);
        out[out_size - 1] = '\0';
        return;
    }
    strncpy(out, "../", out_size - 1);
    out[out_size - 1] = '\0';
    if (strlen(out) + strlen(source) < out_size) {
        strcat(out, source);
    }
}

static int ph_make_sources(const char *const *base, const char *app_source,
    const char **sources, const char *resolved_source)
{
    int count;

    count = 0;
    while (base[count] != 0) {
        sources[count] = base[count];
        ++count;
    }
    sources[count] = resolved_source != 0 && resolved_source[0] != '\0'
        ? resolved_source : app_source;
    ++count;
    sources[count] = 0;
    return count;
}

static int ph_load_sources(PLANKAHOST_APP *app, const char *source,
    char *err, unsigned err_size)
{
    const char *root_sources[40];
    const char *build_sources[40];
    char root_app[512];
    char build_app[512];
    char first_err[256];

    root_app[0] = '\0';
    build_app[0] = '\0';
    if (source == 0 || source[0] == '\0') {
        strcpy(root_app, "graphics/src/plankagui.plk");
    } else {
        strncpy(root_app, source, sizeof(root_app) - 1);
        root_app[sizeof(root_app) - 1] = '\0';
    }
    ph_build_source_path(source, build_app, sizeof(build_app));
    ph_make_sources(PH_BASE_ROOT, source, root_sources, root_app);
    ph_make_sources(PH_BASE_BUILD, source, build_sources, build_app);

    first_err[0] = '\0';
    if (plankac_context_load_sources(app->ctx, root_sources,
            first_err, sizeof(first_err))) {
        return 1;
    }
    if (plankac_context_load_sources(app->ctx, build_sources,
            err, err_size)) {
        return 1;
    }
    if (err != 0 && err_size > 0 && err[0] == '\0') {
        strncpy(err, first_err, err_size - 1);
        err[err_size - 1] = '\0';
    }
    return 0;
}

static int ph_run_optional(PLANKAHOST_APP *app, const char *name,
    const double *args, int argc, PLANKAC_RESULT *result)
{
    char err[256];

    return plankac_context_run(app->ctx, name, args, argc, result,
        err, sizeof(err));
}

int plankahost_open(PLANKAHOST_APP *app, const char *source,
    char *err, unsigned err_size)
{
    PLANKAC_RESULT result;
    PLANKAC_PROC_INFO info;

    if (app == 0) {
        ph_set_error(err, err_size, "missing PlankaHost app");
        return 0;
    }
    memset(app, 0, sizeof(*app));
    app->ctx = plankac_create();
    if (app->ctx == 0) {
        ph_set_error(err, err_size, "cannot allocate PlankaC context");
        return 0;
    }
    if (!ph_load_sources(app, source, err, err_size)) {
        plankahost_close(app);
        return 0;
    }
    if (source != 0 && source[0] != '\0') {
        strncpy(app->source, source, sizeof(app->source) - 1);
    } else {
        strcpy(app->source, "graphics/src/plankagui.plk");
    }

    if (ph_run_optional(app, "app_kind", 0, 0, &result)
            && result.count > 0) {
        app->kind = (int)result.value[0];
    } else if (plankac_context_find_proc(app->ctx, "cube_project_vertex",
            &info)) {
        app->kind = PLANKAHOST_APP_CUBE;
    } else {
        app->kind = PLANKAHOST_APP_GUI;
    }

    if (ph_run_optional(app, "app_canvas", 0, 0, &result)
            && result.count >= 2) {
        app->width = (int)result.value[0];
        app->height = (int)result.value[1];
    } else if (app->kind == PLANKAHOST_APP_CUBE
            && ph_run_optional(app, "cube_canvas", 0, 0, &result)
            && result.count >= 2) {
        app->width = (int)result.value[0];
        app->height = (int)result.value[1];
    } else if (ph_run_optional(app, "gui_canvas", 0, 0, &result)
            && result.count >= 2) {
        app->width = (int)result.value[0];
        app->height = (int)result.value[1];
    } else {
        ph_set_error(err, err_size, "app does not expose a canvas");
        plankahost_close(app);
        return 0;
    }

    if (ph_run_optional(app, "app_checksum", 0, 0, &result)
            && result.count > 0) {
        app->checksum = result.value[0];
    } else if (app->kind == PLANKAHOST_APP_CUBE
            && ph_run_optional(app, "cube_scene_checksum", 0, 0, &result)
            && result.count > 0) {
        app->checksum = result.value[0];
    } else if (ph_run_optional(app, "gui_scene_checksum", 0, 0, &result)
            && result.count > 0) {
        app->checksum = result.value[0];
    }

    sprintf(app->title, "PlankaHost - kind %d checksum %.0f",
        app->kind, app->checksum);
    return 1;
}

void plankahost_close(PLANKAHOST_APP *app)
{
    if (app != 0 && app->ctx != 0) {
        plankac_destroy(app->ctx);
        app->ctx = 0;
    }
}

int plankahost_proc_count(PLANKAHOST_APP *app)
{
    if (app == 0 || app->ctx == 0) {
        return 0;
    }
    return plankac_context_proc_count(app->ctx);
}

int plankahost_get_proc(PLANKAHOST_APP *app, int index,
    PLANKAC_PROC_INFO *info)
{
    if (app == 0 || app->ctx == 0) {
        return 0;
    }
    return plankac_context_get_proc(app->ctx, index, info);
}

int plankahost_find_proc(PLANKAHOST_APP *app, const char *name,
    PLANKAC_PROC_INFO *info)
{
    if (app == 0 || app->ctx == 0) {
        return 0;
    }
    return plankac_context_find_proc(app->ctx, name, info);
}

int plankahost_run(PLANKAHOST_APP *app, const char *name,
    const double *args, int argc, PLANKAC_RESULT *result,
    char *err, unsigned err_size)
{
    if (app == 0 || app->ctx == 0) {
        ph_set_error(err, err_size, "PlankaHost app is not loaded");
        return 0;
    }
    return plankac_context_run(app->ctx, name, args, argc, result,
        err, err_size);
}

int plankahost_render(PLANKAHOST_APP *app, PMG_IMAGE *img,
    const PMG_RENDER_STATE *gui_state, double frame_value,
    char *err, unsigned err_size)
{
    if (app == 0 || app->ctx == 0) {
        ph_set_error(err, err_size, "PlankaHost app is not loaded");
        return 0;
    }
    if (app->kind == PLANKAHOST_APP_CUBE) {
        PMC_SCENE cube_scene;

        cube_scene.ctx = app->ctx;
        return pmc_render_frame(&cube_scene, img, frame_value,
            err, err_size);
    }
    {
        PMG_SCENE gui_scene;

        gui_scene.ctx = app->ctx;
        return pmg_render_scene_with_state(&gui_scene, img, gui_state,
            err, err_size);
    }
}

int plankahost_button_at(PLANKAHOST_APP *app, int x, int y,
    int *id, char *err, unsigned err_size)
{
    PMG_SCENE gui_scene;

    if (app == 0 || app->ctx == 0) {
        ph_set_error(err, err_size, "PlankaHost app is not loaded");
        return 0;
    }
    if (app->kind != PLANKAHOST_APP_GUI) {
        *id = -1;
        return 1;
    }
    gui_scene.ctx = app->ctx;
    return pmg_scene_button_at(&gui_scene, x, y, id, err, err_size);
}

int plankahost_timer_step(PLANKAHOST_APP *app, double current,
    double *next, char *err, unsigned err_size)
{
    PLANKAC_RESULT result;
    double args[1];

    if (app == 0 || app->ctx == 0) {
        ph_set_error(err, err_size, "PlankaHost app is not loaded");
        return 0;
    }
    args[0] = current;
    if (plankac_context_run(app->ctx, "app_timer_step", args, 1,
            &result, err, err_size) && result.count > 0) {
        *next = result.value[0];
        return 1;
    }
    if (app->kind == PLANKAHOST_APP_CUBE) {
        *next = current + 0.045;
    } else {
        *next = current;
    }
    return 1;
}
