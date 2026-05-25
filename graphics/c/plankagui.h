#ifndef PLANKAGUI_H
#define PLANKAGUI_H

#include "plankac.h"

typedef struct PMG_RECT {
    int x;
    int y;
    int w;
    int h;
} PMG_RECT;

typedef struct PMG_RGB {
    unsigned char r;
    unsigned char g;
    unsigned char b;
} PMG_RGB;

typedef struct PMG_IMAGE {
    int width;
    int height;
    unsigned char *pixels;
} PMG_IMAGE;

typedef struct PMG_SCENE {
    PLANKAC_CONTEXT *ctx;
} PMG_SCENE;

int pmg_scene_open(PMG_SCENE *scene, char *err, unsigned err_size);
void pmg_scene_close(PMG_SCENE *scene);
int pmg_scene_run(PMG_SCENE *scene, const char *name,
    const double *args, int argc, PLANKAC_RESULT *result,
    char *err, unsigned err_size);
int pmg_scene_canvas(PMG_SCENE *scene, int *width, int *height,
    char *err, unsigned err_size);
int pmg_scene_rect(PMG_SCENE *scene, const char *name,
    const double *args, int argc, PMG_RECT *rect,
    char *err, unsigned err_size);
int pmg_scene_style(PMG_SCENE *scene, int style, PMG_RGB *rgb,
    char *err, unsigned err_size);
int pmg_scene_button_id(PMG_SCENE *scene, int row, int col,
    int *id, char *err, unsigned err_size);
int pmg_scene_checksum(PMG_SCENE *scene, int *checksum,
    char *err, unsigned err_size);

int pmg_image_alloc(PMG_IMAGE *img, int width, int height);
void pmg_image_free(PMG_IMAGE *img);
void pmg_put_pixel(PMG_IMAGE *img, int x, int y, PMG_RGB c);
void pmg_clear(PMG_IMAGE *img, PMG_RGB c);
void pmg_fill_rect(PMG_IMAGE *img, PMG_RECT r, PMG_RGB c);
void pmg_stroke_rect(PMG_IMAGE *img, PMG_RECT r, PMG_RGB c);
void pmg_panel(PMG_IMAGE *img, PMG_RECT r, PMG_RGB fill, PMG_RGB stroke);

int pmg_text_width(const char *text, int scale);
void pmg_draw_text(PMG_IMAGE *img, int x, int y, const char *text,
    int scale, PMG_RGB color);
void pmg_draw_text_center(PMG_IMAGE *img, PMG_RECT rect,
    const char *text, int scale, PMG_RGB color);

int pmg_render_scene(PMG_SCENE *scene, PMG_IMAGE *img,
    char *err, unsigned err_size);
int pmg_write_png(const char *path, PMG_IMAGE *img,
    char *err, unsigned err_size);

#endif
