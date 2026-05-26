#ifndef PLANKACUBE_H
#define PLANKACUBE_H

#include "plankagui.h"

typedef struct PMC_SCENE {
    PLANKAC_CONTEXT *ctx;
} PMC_SCENE;

typedef struct PMC_POINT {
    int x;
    int y;
    double z;
} PMC_POINT;

int pmc_scene_open(PMC_SCENE *scene, char *err, unsigned err_size);
int pmc_scene_open_source(PMC_SCENE *scene, const char *source,
    char *err, unsigned err_size);
void pmc_scene_close(PMC_SCENE *scene);
int pmc_scene_run(PMC_SCENE *scene, const char *name,
    const double *args, int argc, PLANKAC_RESULT *result,
    char *err, unsigned err_size);
int pmc_scene_canvas(PMC_SCENE *scene, int *width, int *height,
    char *err, unsigned err_size);
int pmc_scene_rect(PMC_SCENE *scene, const char *name,
    const double *args, int argc, PMG_RECT *rect,
    char *err, unsigned err_size);
int pmc_scene_style(PMC_SCENE *scene, int style, PMG_RGB *rgb,
    char *err, unsigned err_size);
int pmc_scene_projection_scale(PMC_SCENE *scene, double *scale,
    char *err, unsigned err_size);
int pmc_scene_edge(PMC_SCENE *scene, int index, int *a, int *b,
    char *err, unsigned err_size);
int pmc_scene_project_vertex(PMC_SCENE *scene, int index, double angle,
    double *x, double *y, double *z, char *err, unsigned err_size);
int pmc_scene_checksum(PMC_SCENE *scene, double *checksum,
    char *err, unsigned err_size);

int pmc_render_frame(PMC_SCENE *scene, PMG_IMAGE *img, double angle,
    char *err, unsigned err_size);

#endif
