#include <stdio.h>

#include "plankagui.h"

int main(int argc, char **argv)
{
    PMG_SCENE scene;
    PMG_IMAGE img;
    const char *out_path;
    char err[256];
    int width;
    int height;
    int checksum;

    out_path = argc > 1 ? argv[1] : "graphics/examples/plankagui.png";
    scene.ctx = 0;
    img.width = 0;
    img.height = 0;
    img.pixels = 0;

    if (!pmg_scene_open(&scene, err, sizeof(err))) {
        fprintf(stderr, "load failed: %s\n", err);
        return 1;
    }
    if (!pmg_scene_canvas(&scene, &width, &height, err, sizeof(err))) {
        fprintf(stderr, "canvas failed: %s\n", err);
        pmg_scene_close(&scene);
        return 1;
    }
    if (!pmg_image_alloc(&img, width, height)) {
        fprintf(stderr, "image allocation failed\n");
        pmg_scene_close(&scene);
        return 1;
    }
    if (!pmg_render_scene(&scene, &img, err, sizeof(err))) {
        fprintf(stderr, "render failed: %s\n", err);
        pmg_image_free(&img);
        pmg_scene_close(&scene);
        return 1;
    }
    if (!pmg_write_png(out_path, &img, err, sizeof(err))) {
        fprintf(stderr, "png failed: %s\n", err);
        pmg_image_free(&img);
        pmg_scene_close(&scene);
        return 1;
    }
    if (!pmg_scene_checksum(&scene, &checksum, err, sizeof(err))) {
        fprintf(stderr, "checksum failed: %s\n", err);
        pmg_image_free(&img);
        pmg_scene_close(&scene);
        return 1;
    }
    printf("PlankaGUI image written: %s\n", out_path);
    printf("scene checksum: %d\n", checksum);
    pmg_image_free(&img);
    pmg_scene_close(&scene);
    return 0;
}
