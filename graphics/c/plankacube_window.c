#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

#include "plankacube.h"

typedef struct PCW_VIEW {
    int x;
    int y;
    int w;
    int h;
} PCW_VIEW;

static PMC_SCENE g_scene;
static PMG_IMAGE g_image;
static double g_angle;
static int g_paused;
static char g_title[128];

static int pcw_render_current(char *err, unsigned err_size)
{
    return pmc_render_frame(&g_scene, &g_image, g_angle, err, err_size);
}

static int pcw_load_image(const char *source, char *err, unsigned err_size)
{
    int width;
    int height;
    double checksum;

    g_scene.ctx = 0;
    g_image.width = 0;
    g_image.height = 0;
    g_image.pixels = 0;
    g_angle = 0.0;
    g_paused = 0;
    g_title[0] = '\0';

    if (!pmc_scene_open_source(&g_scene, source, err, err_size)) {
        return 0;
    }
    if (!pmc_scene_canvas(&g_scene, &width, &height, err, err_size)) {
        pmc_scene_close(&g_scene);
        return 0;
    }
    if (!pmg_image_alloc(&g_image, width, height)) {
        pmc_scene_close(&g_scene);
        strncpy(err, "image allocation failed", err_size - 1);
        err[err_size - 1] = '\0';
        return 0;
    }
    if (!pcw_render_current(err, err_size)) {
        pmg_image_free(&g_image);
        pmc_scene_close(&g_scene);
        return 0;
    }
    if (!pmc_scene_checksum(&g_scene, &checksum, err, err_size)) {
        pmg_image_free(&g_image);
        pmc_scene_close(&g_scene);
        return 0;
    }
    sprintf(g_title, "PlankaCube - scene checksum %.0f", checksum);
    return 1;
}

static void pcw_first_arg(const char *cmd_line, char *out, unsigned out_size)
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

static PCW_VIEW pcw_view_rect(HWND hwnd)
{
    RECT client;
    PCW_VIEW view;
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

static unsigned char *pcw_make_bgr(unsigned long *out_size,
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

static void pcw_paint_image(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC dc;
    BITMAPINFO bmi;
    RECT client;
    PCW_VIEW view;
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

    view = pcw_view_rect(hwnd);
    dib = pcw_make_bgr(&dib_size, &dib_stride);
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

static void pcw_step(HWND hwnd)
{
    char err[256];

    if (g_paused) {
        return;
    }
    g_angle += 0.045;
    if (g_angle > 6.283185307179586 * 32.0) {
        g_angle = 0.0;
    }
    if (!pcw_render_current(err, sizeof(err))) {
        MessageBoxA(hwnd, err, "PlankaCube", MB_ICONERROR | MB_OK);
        g_paused = 1;
        return;
    }
    InvalidateRect(hwnd, 0, FALSE);
}

static LRESULT CALLBACK pcw_proc(HWND hwnd, UINT msg,
    WPARAM wparam, LPARAM lparam)
{
    (void)lparam;

    switch (msg) {
    case WM_ERASEBKGND:
        return 1;
    case WM_PAINT:
        pcw_paint_image(hwnd);
        return 0;
    case WM_SIZE:
        InvalidateRect(hwnd, 0, TRUE);
        return 0;
    case WM_TIMER:
        pcw_step(hwnd);
        return 0;
    case WM_LBUTTONDOWN:
        g_paused = !g_paused;
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

    pcw_first_arg(cmd_line, source, sizeof(source));
    if (!pcw_load_image(source[0] != '\0' ? source : 0, err, sizeof(err))) {
        MessageBoxA(0, err, "PlankaCube", MB_ICONERROR | MB_OK);
        return 1;
    }

    memset(&wc, 0, sizeof(wc));
    wc.lpfnWndProc = pcw_proc;
    wc.hInstance = instance;
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
    wc.lpszClassName = "PlankaCubeWindow";
    if (!RegisterClassA(&wc)) {
        MessageBoxA(0, "window class registration failed",
            "PlankaCube", MB_ICONERROR | MB_OK);
        pmg_image_free(&g_image);
        pmc_scene_close(&g_scene);
        return 1;
    }

    rect.left = 0;
    rect.top = 0;
    rect.right = g_image.width;
    rect.bottom = g_image.height;
    AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);
    hwnd = CreateWindowA("PlankaCubeWindow",
        g_title[0] != '\0' ? g_title : "PlankaCube",
        WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        rect.right - rect.left, rect.bottom - rect.top,
        0, 0, instance, 0);
    if (hwnd == 0) {
        MessageBoxA(0, "window creation failed",
            "PlankaCube", MB_ICONERROR | MB_OK);
        pmg_image_free(&g_image);
        pmc_scene_close(&g_scene);
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
    pmc_scene_close(&g_scene);
    return (int)msg.wParam;
}
