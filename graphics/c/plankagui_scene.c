#include <stdio.h>
#include <string.h>

#include "plankagui.h"

static const char *const PMG_SOURCES[] = {
    "graphics/src/plankagui.plk",
    0
};

static void pmg_set_error(char *err, unsigned err_size, const char *text)
{
    if (err != 0 && err_size > 0) {
        strncpy(err, text, err_size - 1);
        err[err_size - 1] = '\0';
    }
}

int pmg_scene_open(PMG_SCENE *scene, char *err, unsigned err_size)
{
    if (scene == 0) {
        pmg_set_error(err, err_size, "missing scene");
        return 0;
    }
    scene->ctx = plankac_create();
    if (scene->ctx == 0) {
        pmg_set_error(err, err_size, "cannot allocate PlankaC context");
        return 0;
    }
    if (!plankac_context_load_sources(scene->ctx, PMG_SOURCES,
            err, err_size)) {
        plankac_destroy(scene->ctx);
        scene->ctx = 0;
        return 0;
    }
    return 1;
}

void pmg_scene_close(PMG_SCENE *scene)
{
    if (scene != 0 && scene->ctx != 0) {
        plankac_destroy(scene->ctx);
        scene->ctx = 0;
    }
}

int pmg_scene_run(PMG_SCENE *scene, const char *name,
    const double *args, int argc, PLANKAC_RESULT *result,
    char *err, unsigned err_size)
{
    if (scene == 0 || scene->ctx == 0) {
        pmg_set_error(err, err_size, "scene is not loaded");
        return 0;
    }
    if (!plankac_context_run(scene->ctx, name, args, argc, result,
            err, err_size)) {
        return 0;
    }
    return 1;
}

int pmg_scene_canvas(PMG_SCENE *scene, int *width, int *height,
    char *err, unsigned err_size)
{
    PLANKAC_RESULT result;

    if (!pmg_scene_run(scene, "gui_canvas", 0, 0, &result,
            err, err_size)) {
        return 0;
    }
    if (result.count < 2) {
        pmg_set_error(err, err_size, "gui_canvas returned too few values");
        return 0;
    }
    *width = (int)result.value[0];
    *height = (int)result.value[1];
    return 1;
}

int pmg_scene_rect(PMG_SCENE *scene, const char *name,
    const double *args, int argc, PMG_RECT *rect,
    char *err, unsigned err_size)
{
    PLANKAC_RESULT result;

    if (!pmg_scene_run(scene, name, args, argc, &result, err, err_size)) {
        return 0;
    }
    if (result.count < 4) {
        pmg_set_error(err, err_size, "rectangle procedure returned too few values");
        return 0;
    }
    rect->x = (int)result.value[0];
    rect->y = (int)result.value[1];
    rect->w = (int)result.value[2];
    rect->h = (int)result.value[3];
    return 1;
}

int pmg_scene_style(PMG_SCENE *scene, int style, PMG_RGB *rgb,
    char *err, unsigned err_size)
{
    PLANKAC_RESULT result;
    double args[1];

    args[0] = (double)style;
    if (!pmg_scene_run(scene, "gui_style_rgb", args, 1, &result,
            err, err_size)) {
        return 0;
    }
    if (result.count < 3) {
        pmg_set_error(err, err_size, "style procedure returned too few values");
        return 0;
    }
    rgb->r = (unsigned char)((int)result.value[0]);
    rgb->g = (unsigned char)((int)result.value[1]);
    rgb->b = (unsigned char)((int)result.value[2]);
    return 1;
}

int pmg_scene_button_id(PMG_SCENE *scene, int row, int col,
    int *id, char *err, unsigned err_size)
{
    PLANKAC_RESULT result;
    double args[2];

    args[0] = (double)row;
    args[1] = (double)col;
    if (!pmg_scene_run(scene, "gui_button_id", args, 2, &result,
            err, err_size)) {
        return 0;
    }
    if (result.count < 1) {
        pmg_set_error(err, err_size, "button id procedure returned no value");
        return 0;
    }
    *id = (int)result.value[0];
    return 1;
}

int pmg_scene_checksum(PMG_SCENE *scene, int *checksum,
    char *err, unsigned err_size)
{
    PLANKAC_RESULT result;

    if (!pmg_scene_run(scene, "gui_scene_checksum", 0, 0, &result,
            err, err_size)) {
        return 0;
    }
    if (result.count < 1) {
        pmg_set_error(err, err_size, "checksum procedure returned no value");
        return 0;
    }
    *checksum = (int)result.value[0];
    return 1;
}
