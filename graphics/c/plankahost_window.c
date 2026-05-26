#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "plankahost.h"

typedef struct PHW_VIEW {
    int x;
    int y;
    int w;
    int h;
} PHW_VIEW;

static PLANKAHOST_APP g_app;
static PMG_IMAGE g_image;
static PMG_RENDER_STATE g_gui_state;
static double g_frame_value;
static int g_paused;
static int g_started_input;
static double g_accumulator;
static int g_has_pending;
static int g_after_operator;
static char g_pending_op[2];

static const char *const PHW_BUTTON_LABELS[] = {
    "7", "8", "9", "/", "ROOT",
    "4", "5", "6", "*", "X^2",
    "1", "2", "3", "-", "+/-",
    "0", ".", "BACK", "+", "="
};

static void phw_first_arg(const char *cmd_line, char *out, unsigned out_size)
{
    const char *p;
    unsigned n;
    char quote;

    if (out_size == 0) {
        return;
    }
    out[0] = '\0';
    if (cmd_line == 0) {
        return;
    }
    p = cmd_line;
    while (*p == ' ' || *p == '\t') {
        ++p;
    }
    if (*p == '\0') {
        return;
    }
    quote = 0;
    if (*p == '"' || *p == '\'') {
        quote = *p;
        ++p;
    }
    n = 0;
    while (*p != '\0' && n + 1 < out_size) {
        if ((quote != 0 && *p == quote)
                || (quote == 0 && (*p == ' ' || *p == '\t'))) {
            break;
        }
        out[n] = *p;
        ++n;
        ++p;
    }
    out[n] = '\0';
}

static int phw_render_current(char *err, unsigned err_size)
{
    return plankahost_render(&g_app, &g_image, &g_gui_state,
        g_frame_value, err, err_size);
}

static int phw_load_image(const char *source, char *err, unsigned err_size)
{
    g_image.width = 0;
    g_image.height = 0;
    g_image.pixels = 0;
    g_frame_value = 0.0;
    g_paused = 0;
    g_started_input = 0;
    g_accumulator = 0.0;
    g_has_pending = 0;
    g_after_operator = 0;
    g_pending_op[0] = '\0';
    pmg_render_state_default(&g_gui_state);

    if (!plankahost_open(&g_app, source, err, err_size)) {
        return 0;
    }
    if (!pmg_image_alloc(&g_image, g_app.width, g_app.height)) {
        plankahost_close(&g_app);
        strncpy(err, "image allocation failed", err_size - 1);
        err[err_size - 1] = '\0';
        return 0;
    }
    if (!phw_render_current(err, err_size)) {
        pmg_image_free(&g_image);
        plankahost_close(&g_app);
        return 0;
    }
    return 1;
}

static PHW_VIEW phw_view_rect(HWND hwnd)
{
    RECT client;
    PHW_VIEW view;
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

static unsigned char *phw_make_bgr(unsigned long *out_size,
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

static void phw_paint_image(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC dc;
    BITMAPINFO bmi;
    RECT client;
    PHW_VIEW view;
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

    view = phw_view_rect(hwnd);
    dib = phw_make_bgr(&dib_size, &dib_stride);
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

static int phw_client_to_scene(HWND hwnd, int x, int y,
    int *scene_x, int *scene_y)
{
    PHW_VIEW view;

    view = phw_view_rect(hwnd);
    if (x < view.x || y < view.y || x >= view.x + view.w
            || y >= view.y + view.h) {
        return 0;
    }
    *scene_x = MulDiv(x - view.x, g_image.width, view.w);
    *scene_y = MulDiv(y - view.y, g_image.height, view.h);
    return 1;
}

static void phw_append_display(const char *text)
{
    if (!g_started_input || g_after_operator
            || strcmp(g_gui_state.display, "42") == 0
            || strcmp(g_gui_state.display, "ERR") == 0) {
        g_gui_state.display[0] = '\0';
        g_started_input = 1;
        g_after_operator = 0;
    }
    if (strcmp(g_gui_state.display, "0") == 0 && strcmp(text, ".") != 0) {
        g_gui_state.display[0] = '\0';
    }
    if (strlen(g_gui_state.display) + strlen(text)
            < sizeof(g_gui_state.display) - 1) {
        strcat(g_gui_state.display, text);
    }
}

static double phw_display_number(void)
{
    if (strcmp(g_gui_state.display, "ERR") == 0
            || g_gui_state.display[0] == '\0') {
        return 0.0;
    }
    return atof(g_gui_state.display);
}

static void phw_set_display_number(double value)
{
    plankac_format(value, g_gui_state.display,
        (unsigned)sizeof(g_gui_state.display));
}

static int phw_run_unary(const char *proc, double value,
    double *out, char *err, unsigned err_size)
{
    PLANKAC_RESULT result;
    double args[1];

    args[0] = value;
    if (!plankahost_run(&g_app, proc, args, 1, &result, err, err_size)) {
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

static int phw_run_binary(const char *proc, double left, double right,
    double *out, char *err, unsigned err_size)
{
    PLANKAC_RESULT result;
    double args[2];

    args[0] = left;
    args[1] = right;
    if (!plankahost_run(&g_app, proc, args, 2, &result, err, err_size)) {
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

static const char *phw_op_proc(const char *op)
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

static int phw_apply_pending(double right, double *out,
    char *err, unsigned err_size)
{
    const char *proc;

    if (!g_has_pending) {
        *out = right;
        return 1;
    }
    proc = phw_op_proc(g_pending_op);
    if (proc[0] == '\0') {
        strncpy(err, "unknown pending operator", err_size - 1);
        err[err_size - 1] = '\0';
        return 0;
    }
    return phw_run_binary(proc, g_accumulator, right, out,
        err, err_size);
}

static void phw_fail_calculator(HWND hwnd, const char *message)
{
    char err[256];

    strcpy(g_gui_state.display, "ERR");
    strcpy(g_gui_state.status, "STATUS: CALCULATOR GUARD");
    strcpy(g_gui_state.log1, "PLANKAC PROCEDURE REJECTED INPUT");
    strncpy(g_gui_state.log2, message, sizeof(g_gui_state.log2) - 1);
    g_gui_state.log2[sizeof(g_gui_state.log2) - 1] = '\0';
    strcpy(g_gui_state.log3, "STATE RESET");
    g_has_pending = 0;
    g_after_operator = 0;
    g_started_input = 0;
    phw_render_current(err, sizeof(err));
    InvalidateRect(hwnd, 0, TRUE);
}

static void phw_arm_operator(const char *op, HWND hwnd)
{
    char err[256];
    double current;
    double out;

    current = phw_display_number();
    if (g_has_pending && !g_after_operator) {
        if (!phw_apply_pending(current, &out, err, sizeof(err))) {
            phw_fail_calculator(hwnd, err);
            return;
        }
        g_accumulator = out;
        phw_set_display_number(out);
    } else {
        g_accumulator = current;
    }
    strcpy(g_pending_op, op);
    g_has_pending = 1;
    g_after_operator = 1;
    g_started_input = 0;
    sprintf(g_gui_state.status, "STATUS: OPERATOR %s ARMED", op);
    sprintf(g_gui_state.log1, "PENDING %s VIA PLANKAHOST", op);
    sprintf(g_gui_state.log2, "ACCUMULATOR READY");
    sprintf(g_gui_state.log3, "NEXT NUMBER WILL BE RIGHT ARGUMENT");
}

static void phw_handle_equal(HWND hwnd)
{
    char err[256];
    double current;
    double out;

    current = phw_display_number();
    if (!g_has_pending) {
        return;
    }
    if (!phw_apply_pending(current, &out, err, sizeof(err))) {
        phw_fail_calculator(hwnd, err);
        return;
    }
    phw_set_display_number(out);
    sprintf(g_gui_state.status, "STATUS: RESULT FROM PLANKAHOST");
    sprintf(g_gui_state.log1, "CALLED %s", phw_op_proc(g_pending_op));
    sprintf(g_gui_state.log2, "LEFT AND RIGHT VALUES EVALUATED");
    sprintf(g_gui_state.log3, "RESULT WRITTEN TO DISPLAY");
    g_accumulator = out;
    g_has_pending = 0;
    g_after_operator = 1;
    g_started_input = 0;
}

static void phw_handle_gui_button(HWND hwnd, int id)
{
    char err[256];
    double value;
    double out;

    if (id < 0 || id >= 20) {
        return;
    }
    g_gui_state.active_button = id;
    sprintf(g_gui_state.status, "STATUS: BUTTON %s SELECTED",
        PHW_BUTTON_LABELS[id]);
    sprintf(g_gui_state.log1, "CLICK HIT PLK BUTTON RECT");
    sprintf(g_gui_state.log2, "BUTTON ID %d FROM GUI_BUTTON_ID", id);
    sprintf(g_gui_state.log3, "READY");

    if ((id >= 0 && id <= 2) || (id >= 5 && id <= 7)
            || (id >= 10 && id <= 12) || id == 15) {
        phw_append_display(PHW_BUTTON_LABELS[id]);
    } else if (id == 16) {
        if (strchr(g_gui_state.display, '.') == 0) {
            phw_append_display(".");
        }
    } else if (id == 17) {
        size_t len;

        len = strlen(g_gui_state.display);
        if (len > 0) {
            g_gui_state.display[len - 1] = '\0';
        }
        if (g_gui_state.display[0] == '\0') {
            strcpy(g_gui_state.display, "0");
            g_started_input = 0;
        }
    } else if (id == 14) {
        if (g_gui_state.display[0] == '-') {
            memmove(g_gui_state.display, g_gui_state.display + 1,
                strlen(g_gui_state.display));
        } else if (strlen(g_gui_state.display)
                < sizeof(g_gui_state.display) - 1) {
            memmove(g_gui_state.display + 1, g_gui_state.display,
                strlen(g_gui_state.display) + 1);
            g_gui_state.display[0] = '-';
        }
    } else if (id == 3) {
        phw_arm_operator("/", hwnd);
    } else if (id == 8) {
        phw_arm_operator("*", hwnd);
    } else if (id == 13) {
        phw_arm_operator("-", hwnd);
    } else if (id == 18) {
        phw_arm_operator("+", hwnd);
    } else if (id == 19) {
        phw_handle_equal(hwnd);
    } else if (id == 4 || id == 9) {
        value = phw_display_number();
        if (!phw_run_unary(id == 4 ? "root_checked" : "square",
                value, &out, err, sizeof(err))) {
            phw_fail_calculator(hwnd, err);
            return;
        }
        phw_set_display_number(out);
        sprintf(g_gui_state.status, "STATUS: RESULT FROM PLANKAHOST");
        sprintf(g_gui_state.log1, "CALLED %s",
            id == 4 ? "ROOT_CHECKED" : "SQUARE");
        sprintf(g_gui_state.log2, "UNARY PROCEDURE RETURNED R0");
        sprintf(g_gui_state.log3, "DISPLAY UPDATED");
        g_after_operator = 1;
        g_started_input = 0;
    }
    if (!phw_render_current(err, sizeof(err))) {
        MessageBoxA(hwnd, err, "PlankaHost", MB_ICONERROR | MB_OK);
        return;
    }
    InvalidateRect(hwnd, 0, TRUE);
}

static void phw_step(HWND hwnd)
{
    char err[256];
    double next_value;

    if (g_paused || g_app.kind != PLANKAHOST_APP_CUBE) {
        return;
    }
    if (!plankahost_timer_step(&g_app, g_frame_value, &next_value,
            err, sizeof(err))) {
        MessageBoxA(hwnd, err, "PlankaHost", MB_ICONERROR | MB_OK);
        g_paused = 1;
        return;
    }
    g_frame_value = next_value;
    if (!phw_render_current(err, sizeof(err))) {
        MessageBoxA(hwnd, err, "PlankaHost", MB_ICONERROR | MB_OK);
        g_paused = 1;
        return;
    }
    InvalidateRect(hwnd, 0, FALSE);
}

static LRESULT CALLBACK phw_proc(HWND hwnd, UINT msg,
    WPARAM wparam, LPARAM lparam)
{
    char err[256];
    int x;
    int y;
    int scene_x;
    int scene_y;
    int id;

    switch (msg) {
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT:
        phw_paint_image(hwnd);
        return 0;
    case WM_SIZE:
        InvalidateRect(hwnd, 0, TRUE);
        return 0;
    case WM_TIMER:
        phw_step(hwnd);
        return 0;
    case WM_LBUTTONDOWN:
        if (g_app.kind == PLANKAHOST_APP_CUBE) {
            g_paused = !g_paused;
            return 0;
        }
        x = LOWORD(lparam);
        y = HIWORD(lparam);
        if (phw_client_to_scene(hwnd, x, y, &scene_x, &scene_y)) {
            if (plankahost_button_at(&g_app, scene_x, scene_y, &id,
                    err, sizeof(err)) && id >= 0) {
                phw_handle_gui_button(hwnd, id);
            }
        }
        return 0;
    case WM_KEYDOWN:
        if (wparam == VK_SPACE) {
            g_paused = !g_paused;
            return 0;
        }
        return 0;
    case WM_DESTROY:
        KillTimer(hwnd, 1);
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
    char source[512];

    (void)prev_instance;

    phw_first_arg(cmd_line, source, sizeof(source));
    if (!phw_load_image(source[0] != '\0' ? source : 0,
            err, sizeof(err))) {
        MessageBoxA(0, err, "PlankaHost", MB_ICONERROR | MB_OK);
        return 1;
    }

    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = phw_proc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "PlankaHostWindow";
    if (!RegisterClassA(&wc)) {
        MessageBoxA(0, "window class registration failed",
            "PlankaHost", MB_ICONERROR | MB_OK);
        pmg_image_free(&g_image);
        plankahost_close(&g_app);
        return 1;
    }

    rect.left = 0;
    rect.top = 0;
    rect.right = g_image.width;
    rect.bottom = g_image.height;
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    hwnd = CreateWindowA("PlankaHostWindow",
        g_app.title[0] != '\0' ? g_app.title : "PlankaHost",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        0, 0, instance, 0);
    if (hwnd == 0) {
        MessageBoxA(0, "window creation failed",
            "PlankaHost", MB_ICONERROR | MB_OK);
        pmg_image_free(&g_image);
        plankahost_close(&g_app);
        return 1;
    }

    SetTimer(hwnd, 1, 33, 0);
    ShowWindow(hwnd, show_cmd);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, 0, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    pmg_image_free(&g_image);
    plankahost_close(&g_app);
    return (int)msg.wParam;
}
