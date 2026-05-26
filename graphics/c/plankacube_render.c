#include <stdio.h>

#include "plankacube.h"

static void pmc_draw_grid(PMG_IMAGE *img, PMG_RECT view,
    PMG_RGB grid, PMG_RGB border)
{
    int x;
    int y;

    for (x = view.x + 44; x < view.x + view.w; x += 44) {
        pmg_draw_line(img, x, view.y + 1, x, view.y + view.h - 2, grid);
    }
    for (y = view.y + 44; y < view.y + view.h; y += 44) {
        pmg_draw_line(img, view.x + 1, y, view.x + view.w - 2, y, grid);
    }
    pmg_draw_line(img, view.x + view.w / 2, view.y + 12,
        view.x + view.w / 2, view.y + view.h - 12, border);
    pmg_draw_line(img, view.x + 12, view.y + view.h / 2,
        view.x + view.w - 12, view.y + view.h / 2, border);
}

static int pmc_project_points(PMC_SCENE *scene, PMG_RECT view, double angle,
    PMC_POINT *points, char *err, unsigned err_size)
{
    double scale;
    int i;

    if (!pmc_scene_projection_scale(scene, &scale, err, err_size)) {
        return 0;
    }
    for (i = 0; i < 8; ++i) {
        double x;
        double y;
        double z;

        if (!pmc_scene_project_vertex(scene, i, angle, &x, &y, &z,
                err, err_size)) {
            return 0;
        }
        points[i].x = view.x + view.w / 2 + (int)(x * scale);
        points[i].y = view.y + view.h / 2 - (int)(y * scale);
        points[i].z = z;
    }
    return 1;
}

static int pmc_draw_cube(PMC_SCENE *scene, PMG_IMAGE *img, PMG_RECT view,
    double angle, PMG_RGB cube, PMG_RGB back, PMG_RGB accent,
    char *err, unsigned err_size)
{
    PMC_POINT points[8];
    int edge;

    if (!pmc_project_points(scene, view, angle, points, err, err_size)) {
        return 0;
    }
    for (edge = 0; edge < 12; ++edge) {
        int a;
        int b;
        PMG_RGB color;

        if (!pmc_scene_edge(scene, edge, &a, &b, err, err_size)) {
            return 0;
        }
        if (a < 0 || a >= 8 || b < 0 || b >= 8) {
            return 0;
        }
        color = (points[a].z + points[b].z) > 10.8 ? back : cube;
        pmg_draw_line(img, points[a].x, points[a].y,
            points[b].x, points[b].y, color);
        pmg_draw_line(img, points[a].x + 1, points[a].y,
            points[b].x + 1, points[b].y, color);
    }
    for (edge = 0; edge < 8; ++edge) {
        PMG_RECT marker;

        marker.x = points[edge].x - 3;
        marker.y = points[edge].y - 3;
        marker.w = 7;
        marker.h = 7;
        pmg_panel(img, marker, accent, accent);
    }
    return 1;
}

int pmc_render_frame(PMC_SCENE *scene, PMG_IMAGE *img, double angle,
    char *err, unsigned err_size)
{
    PMG_RECT rect;
    PMG_RECT view;
    PMG_RECT row;
    PMG_RGB bg;
    PMG_RGB title;
    PMG_RGB white;
    PMG_RGB border;
    PMG_RGB grid;
    PMG_RGB cube;
    PMG_RGB accent;
    PMG_RGB text;
    PMG_RGB ok;
    PMG_RGB muted;
    char angle_text[64];
    double args[1];
    int i;

    if (!pmc_scene_style(scene, 0, &bg, err, err_size)
            || !pmc_scene_style(scene, 1, &title, err, err_size)
            || !pmc_scene_style(scene, 2, &white, err, err_size)
            || !pmc_scene_style(scene, 3, &border, err, err_size)
            || !pmc_scene_style(scene, 4, &grid, err, err_size)
            || !pmc_scene_style(scene, 5, &cube, err, err_size)
            || !pmc_scene_style(scene, 6, &accent, err, err_size)
            || !pmc_scene_style(scene, 7, &text, err, err_size)
            || !pmc_scene_style(scene, 8, &ok, err, err_size)
            || !pmc_scene_style(scene, 9, &muted, err, err_size)) {
        return 0;
    }

    pmg_clear(img, bg);
    if (!pmc_scene_rect(scene, "cube_window_rect", 0, 0, &rect,
            err, err_size)) {
        return 0;
    }
    pmg_panel(img, rect, bg, border);

    if (!pmc_scene_rect(scene, "cube_title_rect", 0, 0, &rect,
            err, err_size)) {
        return 0;
    }
    pmg_panel(img, rect, title, title);
    pmg_draw_text(img, rect.x + 16, rect.y + 9, "PLANKACUBE", 2, white);
    pmg_draw_text(img, rect.x + rect.w - 150, rect.y + 10,
        "P3240 SCENE", 1, white);

    if (!pmc_scene_rect(scene, "cube_status_rect", 0, 0, &rect,
            err, err_size)) {
        return 0;
    }
    pmg_panel(img, rect, white, border);
    pmg_draw_text(img, rect.x + 8, rect.y + 10,
        "STATUS: 3D PROCEDURES RUNNING", 1, ok);

    if (!pmc_scene_rect(scene, "cube_viewport_rect", 0, 0, &view,
            err, err_size)) {
        return 0;
    }
    pmg_panel(img, view, white, border);
    pmc_draw_grid(img, view, grid, muted);
    if (!pmc_draw_cube(scene, img, view, angle, cube, muted, accent,
            err, err_size)) {
        return 0;
    }

    if (!pmc_scene_rect(scene, "cube_info_rect", 0, 0, &rect,
            err, err_size)) {
        return 0;
    }
    pmg_panel(img, rect, white, border);
    pmg_draw_text(img, rect.x + 12, rect.y + 14,
        "PLANKACUBE 3D API", 1, text);
    pmg_draw_text(img, rect.x + 12, rect.y + 44,
        "MODEL: CUBE_VERTEX", 1, text);
    pmg_draw_text(img, rect.x + 12, rect.y + 66,
        "TOPOLOGY: CUBE_EDGE", 1, text);
    pmg_draw_text(img, rect.x + 12, rect.y + 88,
        "MATRIX: MAT4_ROTATE", 1, text);
    pmg_draw_text(img, rect.x + 12, rect.y + 110,
        "CAMERA: PERSPECTIVE", 1, text);
    pmg_draw_text(img, rect.x + 12, rect.y + 148,
        "HOST: TIMER + RASTER", 1, muted);

    if (!pmc_scene_rect(scene, "cube_metric_rect", 0, 0, &rect,
            err, err_size)) {
        return 0;
    }
    pmg_panel(img, rect, white, border);
    sprintf(angle_text, "ANGLE %.2f RAD", angle);
    pmg_draw_text(img, rect.x + 14, rect.y + 14, angle_text, 1, text);
    pmg_draw_text(img, rect.x + 14, rect.y + 42,
        "PROCEDURE PROFILE", 1, text);

    for (i = 0; i < 5; ++i) {
        const char *label;

        args[0] = (double)i;
        if (!pmc_scene_rect(scene, "cube_function_row_rect", args, 1,
                &row, err, err_size)) {
            return 0;
        }
        label = "P3231 PROJECT_VERTEX";
        if (i == 0) {
            label = "P3200 CUBE_CANVAS";
        } else if (i == 1) {
            label = "P3221 CUBE_VERTEX";
        } else if (i == 2) {
            label = "P3222 CUBE_EDGE";
        } else if (i == 3) {
            label = "P3230 MODEL_TRANSFORM";
        }
        if (i == 3) {
            pmg_panel(img, row, accent, accent);
            pmg_draw_text(img, row.x + 6, row.y + 6, label, 1, white);
        } else {
            pmg_draw_text(img, row.x + 6, row.y + 6, label, 1, text);
        }
    }
    return 1;
}
