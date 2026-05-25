#include <string.h>

#include "plankagui.h"

static const char *const PMG_BUTTON_LABELS[] = {
    "7", "8", "9", "/", "SQRT",
    "4", "5", "6", "*", "X^2",
    "1", "2", "3", "-", "+/-",
    "0", ".", "BACK", "+", "="
};

static const char *const PMG_PROC_LABELS[] = {
    "P3000 GUI_CANVAS",
    "P3001 GUI_WINDOW_RECT",
    "P3010 GUI_BUTTON_RECT",
    "P3020 GUI_FUNCTION_ROW_RECT",
    "P3040 GUI_STYLE_RGB",
    "P3050 GUI_SCENE_CHECKSUM"
};

int pmg_render_scene(PMG_SCENE *scene, PMG_IMAGE *img,
    char *err, unsigned err_size)
{
    PMG_RECT rect;
    PMG_RECT row_rect;
    PMG_RGB bg;
    PMG_RGB title;
    PMG_RGB white;
    PMG_RGB border;
    PMG_RGB button;
    PMG_RGB accent;
    PMG_RGB text;
    PMG_RGB ok;
    double args[2];
    int row;
    int col;
    int i;

    if (!pmg_scene_style(scene, 0, &bg, err, err_size)
            || !pmg_scene_style(scene, 1, &title, err, err_size)
            || !pmg_scene_style(scene, 2, &white, err, err_size)
            || !pmg_scene_style(scene, 3, &border, err, err_size)
            || !pmg_scene_style(scene, 4, &button, err, err_size)
            || !pmg_scene_style(scene, 5, &accent, err, err_size)
            || !pmg_scene_style(scene, 6, &text, err, err_size)
            || !pmg_scene_style(scene, 7, &ok, err, err_size)) {
        return 0;
    }

    pmg_clear(img, bg);

    if (!pmg_scene_rect(scene, "gui_window_rect", 0, 0, &rect,
            err, err_size)) {
        return 0;
    }
    pmg_panel(img, rect, bg, border);

    if (!pmg_scene_rect(scene, "gui_title_rect", 0, 0, &rect,
            err, err_size)) {
        return 0;
    }
    pmg_panel(img, rect, title, title);
    pmg_draw_text(img, 24, 21, "PLANKAGUI", 2, white);
    pmg_draw_text(img, img->width - 120, 22, "P3050 OK", 1, white);

    if (!pmg_scene_rect(scene, "gui_status_rect", 0, 0, &rect,
            err, err_size)) {
        return 0;
    }
    pmg_panel(img, rect, white, border);
    pmg_draw_text(img, rect.x + 8, rect.y + 10,
        "STATUS: SCENE PROCEDURES LOADED", 1, ok);

    if (!pmg_scene_rect(scene, "gui_display_rect", 0, 0, &rect,
            err, err_size)) {
        return 0;
    }
    pmg_panel(img, rect, white, border);
    pmg_draw_text(img, rect.x + rect.w - 42, rect.y + 8, "42", 2, text);

    if (!pmg_scene_rect(scene, "gui_procedure_list_rect", 0, 0, &rect,
            err, err_size)) {
        return 0;
    }
    pmg_panel(img, rect, white, border);
    pmg_draw_text(img, rect.x + 8, rect.y + 10, "PROCEDURES", 1, text);

    for (i = 0; i < 6; ++i) {
        args[0] = (double)i;
        if (!pmg_scene_rect(scene, "gui_function_row_rect", args, 1,
                &row_rect, err, err_size)) {
            return 0;
        }
        if (i == 0) {
            pmg_panel(img, row_rect, accent, accent);
            pmg_draw_text(img, row_rect.x + 6, row_rect.y + 6,
                PMG_PROC_LABELS[i], 1, white);
        } else {
            pmg_draw_text(img, row_rect.x + 6, row_rect.y + 6,
                PMG_PROC_LABELS[i], 1, text);
        }
    }

    if (!pmg_scene_rect(scene, "gui_argument_panel_rect", 0, 0, &rect,
            err, err_size)) {
        return 0;
    }
    pmg_panel(img, rect, bg, border);
    pmg_draw_text(img, rect.x + 10, rect.y + 10, "ARGUMENTS", 1, text);
    for (i = 0; i < 2; ++i) {
        args[0] = (double)i;
        if (!pmg_scene_rect(scene, "gui_argument_row_rect", args, 1,
                &row_rect, err, err_size)) {
            return 0;
        }
        pmg_draw_text(img, row_rect.x, row_rect.y + 7,
            i == 0 ? "V0" : "V1", 1, text);
        row_rect.x += 28;
        row_rect.w -= 28;
        pmg_panel(img, row_rect, white, border);
        pmg_draw_text(img, row_rect.x + 6, row_rect.y + 7,
            i == 0 ? "1" : "0", 1, text);
    }

    if (!pmg_scene_rect(scene, "gui_keypad_panel_rect", 0, 0, &rect,
            err, err_size)) {
        return 0;
    }
    pmg_panel(img, rect, bg, border);
    pmg_draw_text(img, rect.x + 12, rect.y + 10, "KEYPAD", 1, text);

    for (row = 0; row < 4; ++row) {
        for (col = 0; col < 5; ++col) {
            int id;
            int scale;

            args[0] = (double)row;
            args[1] = (double)col;
            if (!pmg_scene_rect(scene, "gui_button_rect", args, 2,
                    &rect, err, err_size)) {
                return 0;
            }
            if (!pmg_scene_button_id(scene, row, col, &id, err, err_size)) {
                return 0;
            }
            if (id < 0 || id >= 20) {
                return 0;
            }
            pmg_panel(img, rect, button, border);
            scale = strlen(PMG_BUTTON_LABELS[id]) > 2 ? 1 : 2;
            pmg_draw_text_center(img, rect, PMG_BUTTON_LABELS[id],
                scale, text);
        }
    }

    if (!pmg_scene_rect(scene, "gui_log_rect", 0, 0, &rect,
            err, err_size)) {
        return 0;
    }
    pmg_panel(img, rect, white, border);
    pmg_draw_text(img, rect.x + 8, rect.y + 12,
        "PLANKAGUI GRAPHICS RUNTIME", 1, text);
    pmg_draw_text(img, rect.x + 8, rect.y + 31,
        "SOURCE: PLANKAGUI.PLK", 1, text);
    pmg_draw_text(img, rect.x + 8, rect.y + 50,
        "HOST: EVALUATED PROCEDURES", 1, text);
    return 1;
}
