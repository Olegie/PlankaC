#include <stdio.h>
#include <string.h>

#include "plankacube.h"

static const char *const PMC_SOURCES_ROOT[] = {
    "src/00_types.plk",
    "src/01_arithmetic.plk",
    "src/03_scientific.plk",
    "src/15_3d_geometry.plk",
    "graphics/src/plankacube.plk",
    0
};

static const char *const PMC_SOURCES_BUILD[] = {
    "../src/00_types.plk",
    "../src/01_arithmetic.plk",
    "../src/03_scientific.plk",
    "../src/15_3d_geometry.plk",
    "../graphics/src/plankacube.plk",
    0
};

static const char *const PMC_BASE_ROOT[] = {
    "src/00_types.plk",
    "src/01_arithmetic.plk",
    "src/03_scientific.plk",
    "src/15_3d_geometry.plk",
    0
};

static const char *const PMC_BASE_BUILD[] = {
    "../src/00_types.plk",
    "../src/01_arithmetic.plk",
    "../src/03_scientific.plk",
    "../src/15_3d_geometry.plk",
    0
};

static void pmc_set_error(char *err, unsigned err_size, const char *text)
{
    if (err != 0 && err_size > 0) {
        strncpy(err, text, err_size - 1);
        err[err_size - 1] = '\0';
    }
}

static int pmc_scene_open_sources(PMC_SCENE *scene,
    const char *const *root_sources, const char *const *build_sources,
    const char *missing_text, char *err, unsigned err_size)
{
    char first_err[256];

    if (scene == 0) {
        pmc_set_error(err, err_size, "missing cube scene");
        return 0;
    }
    if (err != 0 && err_size > 0) {
        err[0] = '\0';
    }
    first_err[0] = '\0';
    scene->ctx = plankac_create();
    if (scene->ctx == 0) {
        pmc_set_error(err, err_size, "cannot allocate PlankaC context");
        return 0;
    }
    if (plankac_context_load_sources(scene->ctx, root_sources,
            first_err, sizeof(first_err))) {
        return 1;
    }
    if (plankac_context_load_sources(scene->ctx, build_sources,
            err, err_size)) {
        return 1;
    }
    if (err != 0 && err_size > 0 && err[0] == '\0') {
        strncpy(err, first_err, err_size - 1);
        err[err_size - 1] = '\0';
    }
    if (err != 0 && err_size > 0 && err[0] == '\0') {
        pmc_set_error(err, err_size, missing_text);
    }
    if (scene->ctx != 0) {
        plankac_destroy(scene->ctx);
        scene->ctx = 0;
    }
    return 0;
}

int pmc_scene_open(PMC_SCENE *scene, char *err, unsigned err_size)
{
    return pmc_scene_open_sources(scene, PMC_SOURCES_ROOT, PMC_SOURCES_BUILD,
        "missing graphics/src/plankacube.plk", err, err_size);
}

int pmc_scene_open_source(PMC_SCENE *scene, const char *source,
    char *err, unsigned err_size)
{
    const char *root_sources[8];
    const char *build_sources[8];
    int count;

    if (source == 0 || source[0] == '\0') {
        return pmc_scene_open(scene, err, err_size);
    }
    count = 0;
    while (PMC_BASE_ROOT[count] != 0) {
        root_sources[count] = PMC_BASE_ROOT[count];
        build_sources[count] = PMC_BASE_BUILD[count];
        ++count;
    }
    root_sources[count] = source;
    build_sources[count] = source;
    ++count;
    root_sources[count] = 0;
    build_sources[count] = 0;
    return pmc_scene_open_sources(scene, root_sources, build_sources,
        "missing PlankaCube source profile", err, err_size);
}

void pmc_scene_close(PMC_SCENE *scene)
{
    if (scene != 0 && scene->ctx != 0) {
        plankac_destroy(scene->ctx);
        scene->ctx = 0;
    }
}

int pmc_scene_run(PMC_SCENE *scene, const char *name,
    const double *args, int argc, PLANKAC_RESULT *result,
    char *err, unsigned err_size)
{
    if (scene == 0 || scene->ctx == 0) {
        pmc_set_error(err, err_size, "cube scene is not loaded");
        return 0;
    }
    return plankac_context_run(scene->ctx, name, args, argc, result,
        err, err_size);
}

int pmc_scene_canvas(PMC_SCENE *scene, int *width, int *height,
    char *err, unsigned err_size)
{
    PLANKAC_RESULT result;

    if (!pmc_scene_run(scene, "cube_canvas", 0, 0, &result,
            err, err_size)) {
        return 0;
    }
    if (result.count < 2) {
        pmc_set_error(err, err_size, "cube_canvas returned too few values");
        return 0;
    }
    *width = (int)result.value[0];
    *height = (int)result.value[1];
    return 1;
}

int pmc_scene_rect(PMC_SCENE *scene, const char *name,
    const double *args, int argc, PMG_RECT *rect,
    char *err, unsigned err_size)
{
    PLANKAC_RESULT result;

    if (!pmc_scene_run(scene, name, args, argc, &result, err, err_size)) {
        return 0;
    }
    if (result.count < 4) {
        pmc_set_error(err, err_size,
            "cube rectangle procedure returned too few values");
        return 0;
    }
    rect->x = (int)result.value[0];
    rect->y = (int)result.value[1];
    rect->w = (int)result.value[2];
    rect->h = (int)result.value[3];
    return 1;
}

int pmc_scene_style(PMC_SCENE *scene, int style, PMG_RGB *rgb,
    char *err, unsigned err_size)
{
    PLANKAC_RESULT result;
    double args[1];

    args[0] = (double)style;
    if (!pmc_scene_run(scene, "cube_style_rgb", args, 1, &result,
            err, err_size)) {
        return 0;
    }
    if (result.count < 3) {
        pmc_set_error(err, err_size,
            "cube style procedure returned too few values");
        return 0;
    }
    rgb->r = (unsigned char)((int)result.value[0]);
    rgb->g = (unsigned char)((int)result.value[1]);
    rgb->b = (unsigned char)((int)result.value[2]);
    return 1;
}

int pmc_scene_projection_scale(PMC_SCENE *scene, double *scale,
    char *err, unsigned err_size)
{
    PLANKAC_RESULT result;

    if (!pmc_scene_run(scene, "cube_projection_scale", 0, 0, &result,
            err, err_size)) {
        return 0;
    }
    if (result.count < 1) {
        pmc_set_error(err, err_size,
            "cube_projection_scale returned no value");
        return 0;
    }
    *scale = result.value[0];
    return 1;
}

int pmc_scene_edge(PMC_SCENE *scene, int index, int *a, int *b,
    char *err, unsigned err_size)
{
    PLANKAC_RESULT result;
    double args[1];

    args[0] = (double)index;
    if (!pmc_scene_run(scene, "cube_edge", args, 1, &result,
            err, err_size)) {
        return 0;
    }
    if (result.count < 2) {
        pmc_set_error(err, err_size, "cube_edge returned too few values");
        return 0;
    }
    *a = (int)result.value[0];
    *b = (int)result.value[1];
    return 1;
}

int pmc_scene_project_vertex(PMC_SCENE *scene, int index, double angle,
    double *x, double *y, double *z, char *err, unsigned err_size)
{
    PLANKAC_RESULT result;
    double args[2];

    args[0] = (double)index;
    args[1] = angle;
    if (!pmc_scene_run(scene, "cube_project_vertex", args, 2, &result,
            err, err_size)) {
        return 0;
    }
    if (result.count < 3) {
        pmc_set_error(err, err_size,
            "cube_project_vertex returned too few values");
        return 0;
    }
    *x = result.value[0];
    *y = result.value[1];
    *z = result.value[2];
    return 1;
}

int pmc_scene_checksum(PMC_SCENE *scene, double *checksum,
    char *err, unsigned err_size)
{
    PLANKAC_RESULT result;

    if (!pmc_scene_run(scene, "cube_scene_checksum", 0, 0, &result,
            err, err_size)) {
        return 0;
    }
    if (result.count < 1) {
        pmc_set_error(err, err_size,
            "cube_scene_checksum returned no value");
        return 0;
    }
    *checksum = result.value[0];
    return 1;
}
