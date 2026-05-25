#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "plankagui.h"

typedef struct PGW_VIEW {
    int x;
    int y;
    int w;
    int h;
} PGW_VIEW;

static PMG_IMAGE g_image;
static PMG_SCENE g_scene;
static PMG_RENDER_STATE g_render_state;
static char g_status[128];
static int g_checksum;
static int g_started_input;
static double g_accumulator;
static int g_has_pending;
static int g_after_operator;
static char g_pending_op[2];

static const char *const PGW_BUTTON_LABELS[] = {
    "7", "8", "9", "/", "ROOT",
    "4", "5", "6", "*", "X^2",
    "1", "2", "3", "-", "+/-",
    "0", ".", "BACK", "+", "="
};

static int pgw_render_current(char *err, unsigned err_size)
{
    if (!pmg_render_scene_with_state(&g_scene, &g_image,
            &g_render_state, err, err_size)) {
        return 0;
    }
    return 1;
}

static int pgw_load_image(char *err, unsigned err_size)
{
    int width;
    int height;

    g_scene.ctx = 0;
    g_image.width = 0;
    g_image.height = 0;
    g_image.pixels = 0;
    g_checksum = 0;
    g_started_input = 0;
    g_accumulator = 0.0;
    g_has_pending = 0;
    g_after_operator = 0;
    g_pending_op[0] = '\0';
    g_status[0] = '\0';
    pmg_render_state_default(&g_render_state);

    if (!pmg_scene_open(&g_scene, err, err_size)) {
        return 0;
    }
    if (!pmg_scene_canvas(&g_scene, &width, &height, err, err_size)) {
        pmg_scene_close(&g_scene);
        return 0;
    }
    if (!pmg_image_alloc(&g_image, width, height)) {
        pmg_scene_close(&g_scene);
        strncpy(err, "image allocation failed", err_size - 1);
        err[err_size - 1] = '\0';
        return 0;
    }
    if (!pgw_render_current(err, err_size)) {
        pmg_image_free(&g_image);
        pmg_scene_close(&g_scene);
        return 0;
    }
    if (!pmg_scene_checksum(&g_scene, &g_checksum, err, err_size)) {
        pmg_image_free(&g_image);
        pmg_scene_close(&g_scene);
        return 0;
    }
    sprintf(g_status, "PlankaGUI - scene checksum %d", g_checksum);
    return 1;
}

static PGW_VIEW pgw_view_rect(HWND hwnd)
{
    RECT client;
    PGW_VIEW view;
    int client_w;
    int client_h;

    GetClientRect(hwnd, &client);
    client_w = client.right - client.left;
    client_h = client.bottom - client.top;
    view.x = 0;
    view.y = 0;
    view.w = client_w;
    view.h = client_h;
    if (g_image.width <= 0 || g_image.height <= 0
            || client_w <= 0 || client_h <= 0) {
        return view;
    }
    if ((long)client_w * (long)g_image.height
            <= (long)client_h * (long)g_image.width) {
        view.w = client_w;
        view.h = MulDiv(g_image.height, client_w, g_image.width);
    } else {
        view.h = client_h;
        view.w = MulDiv(g_image.width, client_h, g_image.height);
    }
    if (view.w < 1) {
        view.w = 1;
    }
    if (view.h < 1) {
        view.h = 1;
    }
    view.x = (client_w - view.w) / 2;
    view.y = (client_h - view.h) / 2;
    return view;
}

static unsigned char *pgw_make_bgr(unsigned long *out_size,
    unsigned long *out_stride)
{
    unsigned long stride;
    unsigned long size;
    unsigned char *dib;
    int x;
    int y;

    stride = ((unsigned long)g_image.width * 3UL + 3UL) & ~3UL;
    size = stride * (unsigned long)g_image.height;
    dib = (unsigned char *)malloc(size);
    if (dib == 0) {
        return 0;
    }
    memset(dib, 0, size);
    for (y = 0; y < g_image.height; ++y) {
        for (x = 0; x < g_image.width; ++x) {
            unsigned char *src;
            unsigned char *dst;

            src = g_image.pixels + ((y * g_image.width + x) * 3);
            dst = dib + (unsigned long)y * stride + (unsigned long)x * 3UL;
            dst[0] = src[2];
            dst[1] = src[1];
            dst[2] = src[0];
        }
    }
    *out_size = size;
    *out_stride = stride;
    return dib;
}

static void pgw_paint_image(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC dc;
    BITMAPINFO bmi;
    RECT client;
    PGW_VIEW view;
    unsigned char *dib;
    unsigned long dib_size;
    unsigned long dib_stride;

    dc = BeginPaint(hwnd, &ps);
    GetClientRect(hwnd, &client);
    FillRect(dc, &client, (HBRUSH)(COLOR_WINDOW + 1));
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = g_image.width;
    bmi.bmiHeader.biHeight = -g_image.height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;

    view = pgw_view_rect(hwnd);
    dib = pgw_make_bgr(&dib_size, &dib_stride);
    if (dib != 0) {
        (void)dib_size;
        (void)dib_stride;
        SetStretchBltMode(dc, COLORONCOLOR);
        StretchDIBits(dc, view.x, view.y, view.w, view.h,
            0, 0, g_image.width, g_image.height, dib,
            &bmi, DIB_RGB_COLORS, SRCCOPY);
        free(dib);
    }
    EndPaint(hwnd, &ps);
}

static int pgw_client_to_scene(HWND hwnd, int x, int y,
    int *scene_x, int *scene_y)
{
    PGW_VIEW view;

    view = pgw_view_rect(hwnd);
    if (x < view.x || y < view.y || x >= view.x + view.w
            || y >= view.y + view.h) {
        return 0;
    }
    *scene_x = MulDiv(x - view.x, g_image.width, view.w);
    *scene_y = MulDiv(y - view.y, g_image.height, view.h);
    return 1;
}

static void pgw_append_display(const char *text)
{
    if (!g_started_input || g_after_operator
            || strcmp(g_render_state.display, "42") == 0
            || strcmp(g_render_state.display, "ERR") == 0) {
        g_render_state.display[0] = '\0';
        g_started_input = 1;
        g_after_operator = 0;
    }
    if (strcmp(g_render_state.display, "0") == 0
            && strcmp(text, ".") != 0) {
        g_render_state.display[0] = '\0';
    }
    if (strlen(g_render_state.display) + strlen(text)
            < sizeof(g_render_state.display) - 1) {
        strcat(g_render_state.display, text);
    }
}

static double pgw_display_number(void)
{
    if (strcmp(g_render_state.display, "ERR") == 0
            || g_render_state.display[0] == '\0') {
        return 0.0;
    }
    return atof(g_render_state.display);
}

static void pgw_set_display_number(double value)
{
    plankac_format(value, g_render_state.display,
        (unsigned)sizeof(g_render_state.display));
}

static int pgw_run_unary(const char *proc, double value,
    double *out, char *err, unsigned err_size)
{
    PLANKAC_RESULT result;
    double args[1];

    args[0] = value;
    if (!pmg_scene_run(&g_scene, proc, args, 1, &result, err, err_size)) {
        return 0;
    }
    if (result.count < 1) {
        strncpy(err, "calculator procedure returned no result", err_size - 1);
        err[err_size - 1] = '\0';
        return 0;
    }
    if (result.count > 1 && (int)result.value[1] != 0) {
        strncpy(err, "calculator guard rejected input", err_size - 1);
        err[err_size - 1] = '\0';
        return 0;
    }
    *out = result.value[0];
    return 1;
}

static int pgw_run_binary(const char *proc, double left, double right,
    double *out, char *err, unsigned err_size)
{
    PLANKAC_RESULT result;
    double args[2];

    args[0] = left;
    args[1] = right;
    if (!pmg_scene_run(&g_scene, proc, args, 2, &result, err, err_size)) {
        return 0;
    }
    if (result.count < 1) {
        strncpy(err, "calculator procedure returned no result", err_size - 1);
        err[err_size - 1] = '\0';
        return 0;
    }
    if (result.count > 1 && (int)result.value[1] != 0) {
        strncpy(err, "calculator guard rejected input", err_size - 1);
        err[err_size - 1] = '\0';
        return 0;
    }
    *out = result.value[0];
    return 1;
}

static const char *pgw_op_proc(const char *op)
{
    if (strcmp(op, "+") == 0) {
        return "add";
    }
    if (strcmp(op, "-") == 0) {
        return "subtract";
    }
    if (strcmp(op, "*") == 0) {
        return "multiply";
    }
    if (strcmp(op, "/") == 0) {
        return "divide_checked";
    }
    return "";
}

static int pgw_apply_pending(double right, double *out,
    char *err, unsigned err_size)
{
    const char *proc;

    if (!g_has_pending) {
        *out = right;
        return 1;
    }
    proc = pgw_op_proc(g_pending_op);
    if (proc[0] == '\0') {
        strncpy(err, "unknown pending operator", err_size - 1);
        err[err_size - 1] = '\0';
        return 0;
    }
    return pgw_run_binary(proc, g_accumulator, right, out, err, err_size);
}

static void pgw_fail_calculator(HWND hwnd, const char *message)
{
    char err[256];

    strcpy(g_render_state.display, "ERR");
    strcpy(g_render_state.status, "STATUS: CALCULATOR GUARD");
    strcpy(g_render_state.log1, "PLANKAC PROCEDURE REJECTED INPUT");
    strncpy(g_render_state.log2, message, sizeof(g_render_state.log2) - 1);
    g_render_state.log2[sizeof(g_render_state.log2) - 1] = '\0';
    strcpy(g_render_state.log3, "STATE RESET");
    g_has_pending = 0;
    g_after_operator = 0;
    g_started_input = 0;
    pgw_render_current(err, sizeof(err));
    InvalidateRect(hwnd, 0, TRUE);
}

static void pgw_arm_operator(const char *op, HWND hwnd)
{
    char err[256];
    double current;
    double out;

    current = pgw_display_number();
    if (g_has_pending && !g_after_operator) {
        if (!pgw_apply_pending(current, &out, err, sizeof(err))) {
            pgw_fail_calculator(hwnd, err);
            return;
        }
        g_accumulator = out;
        pgw_set_display_number(out);
    } else {
        g_accumulator = current;
    }
    strcpy(g_pending_op, op);
    g_has_pending = 1;
    g_after_operator = 1;
    g_started_input = 0;
    sprintf(g_render_state.status, "STATUS: OPERATOR %s ARMED", op);
    sprintf(g_render_state.log1, "PENDING %s VIA PLANKAC", op);
    sprintf(g_render_state.log2, "ACCUMULATOR READY");
    sprintf(g_render_state.log3, "NEXT NUMBER WILL BE RIGHT ARGUMENT");
}

static void pgw_handle_equal(HWND hwnd)
{
    char err[256];
    double current;
    double out;

    current = pgw_display_number();
    if (!g_has_pending) {
        return;
    }
    if (!pgw_apply_pending(current, &out, err, sizeof(err))) {
        pgw_fail_calculator(hwnd, err);
        return;
    }
    pgw_set_display_number(out);
    sprintf(g_render_state.status, "STATUS: RESULT FROM PLANKAC");
    sprintf(g_render_state.log1, "CALLED %s", pgw_op_proc(g_pending_op));
    sprintf(g_render_state.log2, "LEFT AND RIGHT VALUES EVALUATED");
    sprintf(g_render_state.log3, "RESULT WRITTEN TO DISPLAY");
    g_accumulator = out;
    g_has_pending = 0;
    g_after_operator = 1;
    g_started_input = 0;
}

static void pgw_handle_button(HWND hwnd, int id)
{
    char err[256];
    double value;
    double out;

    g_render_state.active_button = id;
    sprintf(g_render_state.status, "STATUS: BUTTON %s SELECTED",
        PGW_BUTTON_LABELS[id]);
    sprintf(g_render_state.log1, "CLICK HIT PLK BUTTON RECT");
    sprintf(g_render_state.log2, "BUTTON ID %d FROM GUI_BUTTON_ID", id);
    sprintf(g_render_state.log3, "READY");

    if ((id >= 0 && id <= 2) || (id >= 5 && id <= 7)
            || (id >= 10 && id <= 12) || id == 15) {
        pgw_append_display(PGW_BUTTON_LABELS[id]);
    } else if (id == 16) {
        if (strchr(g_render_state.display, '.') == 0) {
            pgw_append_display(".");
        }
    } else if (id == 17) {
        size_t len;

        len = strlen(g_render_state.display);
        if (len > 0) {
            g_render_state.display[len - 1] = '\0';
        }
        if (g_render_state.display[0] == '\0') {
            strcpy(g_render_state.display, "0");
            g_started_input = 0;
        }
    } else if (id == 14) {
        if (g_render_state.display[0] == '-') {
            memmove(g_render_state.display, g_render_state.display + 1,
                strlen(g_render_state.display));
        } else if (strlen(g_render_state.display)
                < sizeof(g_render_state.display) - 1) {
            memmove(g_render_state.display + 1, g_render_state.display,
                strlen(g_render_state.display) + 1);
            g_render_state.display[0] = '-';
        }
    } else if (id == 3) {
        pgw_arm_operator("/", hwnd);
    } else if (id == 8) {
        pgw_arm_operator("*", hwnd);
    } else if (id == 13) {
        pgw_arm_operator("-", hwnd);
    } else if (id == 18) {
        pgw_arm_operator("+", hwnd);
    } else if (id == 19) {
        pgw_handle_equal(hwnd);
    } else if (id == 4 || id == 9) {
        value = pgw_display_number();
        if (!pgw_run_unary(id == 4 ? "root_checked" : "square",
                value, &out, err, sizeof(err))) {
            pgw_fail_calculator(hwnd, err);
            return;
        }
        pgw_set_display_number(out);
        sprintf(g_render_state.status, "STATUS: RESULT FROM PLANKAC");
        sprintf(g_render_state.log1, "CALLED %s",
            id == 4 ? "ROOT_CHECKED" : "SQUARE");
        sprintf(g_render_state.log2, "UNARY PROCEDURE RETURNED R0");
        sprintf(g_render_state.log3, "DISPLAY UPDATED");
        g_after_operator = 1;
        g_started_input = 0;
    }
    if (!pgw_render_current(err, sizeof(err))) {
        MessageBoxA(hwnd, err, "PlankaGUI", MB_ICONERROR | MB_OK);
        return;
    }
    InvalidateRect(hwnd, 0, TRUE);
}

static LRESULT CALLBACK pgw_proc(HWND hwnd, UINT msg,
    WPARAM wparam, LPARAM lparam)
{
    char err[256];
    int x;
    int y;
    int scene_x;
    int scene_y;
    int id;

    switch (msg) {
    case WM_PAINT:
        pgw_paint_image(hwnd);
        return 0;
    case WM_SIZE:
        InvalidateRect(hwnd, 0, TRUE);
        return 0;
    case WM_LBUTTONDOWN:
        x = LOWORD(lparam);
        y = HIWORD(lparam);
        if (pgw_client_to_scene(hwnd, x, y, &scene_x, &scene_y)) {
            if (pmg_scene_button_at(&g_scene, scene_x, scene_y, &id,
                    err, sizeof(err)) && id >= 0) {
                pgw_handle_button(hwnd, id);
            }
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    default:
        return DefWindowProc(hwnd, msg, wparam, lparam);
    }
}

int PASCAL WinMain(HINSTANCE instance, HINSTANCE prev_instance,
    LPSTR cmd_line, int show_cmd)
{
    WNDCLASSA wc;
    HWND hwnd;
    MSG msg;
    RECT rect;
    char err[256];
    const char *title;

    (void)prev_instance;
    (void)cmd_line;

    if (!pgw_load_image(err, sizeof(err))) {
        MessageBoxA(0, err, "PlankaGUI", MB_ICONERROR | MB_OK);
        return 1;
    }

    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = pgw_proc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "PlankaGUIWindow";
    if (!RegisterClassA(&wc)) {
        MessageBoxA(0, "window class registration failed",
            "PlankaGUI", MB_ICONERROR | MB_OK);
        pmg_image_free(&g_image);
        return 1;
    }

    rect.left = 0;
    rect.top = 0;
    rect.right = g_image.width;
    rect.bottom = g_image.height;
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    title = g_status[0] != '\0' ? g_status : "PlankaGUI";
    hwnd = CreateWindowA("PlankaGUIWindow", title,
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        0, 0, instance, 0);
    if (hwnd == 0) {
        MessageBoxA(0, "window creation failed",
            "PlankaGUI", MB_ICONERROR | MB_OK);
        pmg_image_free(&g_image);
        return 1;
    }

    ShowWindow(hwnd, show_cmd);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, 0, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    pmg_image_free(&g_image);
    pmg_scene_close(&g_scene);
    return (int)msg.wParam;
}
