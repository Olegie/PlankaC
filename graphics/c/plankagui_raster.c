#include <stdlib.h>
#include <string.h>

#include "plankagui.h"

int pmg_image_alloc(PMG_IMAGE *img, int width, int height)
{
    if (img == 0 || width <= 0 || height <= 0) {
        return 0;
    }
    img->width = width;
    img->height = height;
    img->pixels = (unsigned char *)malloc((unsigned long)width
        * (unsigned long)height * 3UL);
    if (img->pixels == 0) {
        img->width = 0;
        img->height = 0;
        return 0;
    }
    memset(img->pixels, 0, (unsigned long)width * (unsigned long)height * 3UL);
    return 1;
}

void pmg_image_free(PMG_IMAGE *img)
{
    if (img != 0) {
        free(img->pixels);
        img->pixels = 0;
        img->width = 0;
        img->height = 0;
    }
}

void pmg_put_pixel(PMG_IMAGE *img, int x, int y, PMG_RGB c)
{
    unsigned char *p;

    if (img == 0 || img->pixels == 0
            || x < 0 || y < 0 || x >= img->width || y >= img->height) {
        return;
    }
    p = img->pixels + ((y * img->width + x) * 3);
    p[0] = c.r;
    p[1] = c.g;
    p[2] = c.b;
}

void pmg_clear(PMG_IMAGE *img, PMG_RGB c)
{
    int x;
    int y;

    for (y = 0; y < img->height; ++y) {
        for (x = 0; x < img->width; ++x) {
            pmg_put_pixel(img, x, y, c);
        }
    }
}

void pmg_fill_rect(PMG_IMAGE *img, PMG_RECT r, PMG_RGB c)
{
    int x;
    int y;

    for (y = r.y; y < r.y + r.h; ++y) {
        for (x = r.x; x < r.x + r.w; ++x) {
            pmg_put_pixel(img, x, y, c);
        }
    }
}

void pmg_stroke_rect(PMG_IMAGE *img, PMG_RECT r, PMG_RGB c)
{
    int x;
    int y;

    for (x = r.x; x < r.x + r.w; ++x) {
        pmg_put_pixel(img, x, r.y, c);
        pmg_put_pixel(img, x, r.y + r.h - 1, c);
    }
    for (y = r.y; y < r.y + r.h; ++y) {
        pmg_put_pixel(img, r.x, y, c);
        pmg_put_pixel(img, r.x + r.w - 1, y, c);
    }
}

void pmg_panel(PMG_IMAGE *img, PMG_RECT r, PMG_RGB fill, PMG_RGB stroke)
{
    pmg_fill_rect(img, r, fill);
    pmg_stroke_rect(img, r, stroke);
}

void pmg_draw_line(PMG_IMAGE *img, int x0, int y0, int x1, int y1,
    PMG_RGB c)
{
    int dx;
    int dy;
    int sx;
    int sy;
    int err;

    dx = x1 > x0 ? x1 - x0 : x0 - x1;
    dy = y1 > y0 ? y1 - y0 : y0 - y1;
    sx = x0 < x1 ? 1 : -1;
    sy = y0 < y1 ? 1 : -1;
    err = dx - dy;
    for (;;) {
        int twice;

        pmg_put_pixel(img, x0, y0, c);
        if (x0 == x1 && y0 == y1) {
            break;
        }
        twice = err * 2;
        if (twice > -dy) {
            err -= dy;
            x0 += sx;
        }
        if (twice < dx) {
            err += dx;
            y0 += sy;
        }
    }
}
