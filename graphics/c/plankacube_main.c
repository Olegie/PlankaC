#include <stdio.h>
#include <stdlib.h>

#include "plankacube.h"

int main(int argc, char **argv)
{
    PMC_SCENE scene;
    PMG_IMAGE img;
    char err[256];
    const char *path;
    const char *source;
    double angle;
    int width;
    int height;

    path = argc > 1 ? argv[1] : "graphics/examples/plankacube.png";
    angle = argc > 2 ? atof(argv[2]) : 0.85;
    source = argc > 3 ? argv[3] : 0;
    scene.ctx = 0;
    img.width = 0;
    img.height = 0;
    img.pixels = 0;

    if (!pmc_scene_open_source(&scene, source, err, sizeof(err))) {
        fprintf(stderr, "PlankaCube scene error: %s\n", err);
        return 1;
    }
    if (!pmc_scene_canvas(&scene, &width, &height, err, sizeof(err))) {
        fprintf(stderr, "PlankaCube canvas error: %s\n", err);
        pmc_scene_close(&scene);
        return 1;
    }
    if (!pmg_image_alloc(&img, width, height)) {
        fprintf(stderr, "PlankaCube image allocation failed\n");
        pmc_scene_close(&scene);
        return 1;
    }
    if (!pmc_render_frame(&scene, &img, angle, err, sizeof(err))) {
        fprintf(stderr, "PlankaCube render error: %s\n", err);
        pmg_image_free(&img);
        pmc_scene_close(&scene);
        return 1;
    }
    if (!pmg_write_png(path, &img, err, sizeof(err))) {
        fprintf(stderr, "PlankaCube PNG error: %s\n", err);
        pmg_image_free(&img);
        pmc_scene_close(&scene);
        return 1;
    }
    printf("PlankaCube PNG written: %s\n", path);
    pmg_image_free(&img);
    pmc_scene_close(&scene);
    return 0;
}
