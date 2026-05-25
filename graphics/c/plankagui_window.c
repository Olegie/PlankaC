#include <stdio.h>
#include <string.h>
#include <windows.h>

#include "plankagui.h"

static PMG_IMAGE g_image;
static char g_status[128];

static int pgw_load_image(char *err, unsigned err_size)
{
    PMG_SCENE scene;
    int width;
    int height;
    int checksum;

    scene.ctx = 0;
    g_image.width = 0;
    g_image.height = 0;
    g_image.pixels = 0;
    g_status[0] = '\0';

    if (!pmg_scene_open(&scene, err, err_size)) {
        return 0;
    }
    if (!pmg_scene_canvas(&scene, &width, &height, err, err_size)) {
        pmg_scene_close(&scene);
        return 0;
    }
    if (!pmg_image_alloc(&g_image, width, height)) {
        pmg_scene_close(&scene);
        strncpy(err, "image allocation failed", err_size - 1);
        err[err_size - 1] = '\0';
        return 0;
    }
    if (!pmg_render_scene(&scene, &g_image, err, err_size)) {
        pmg_image_free(&g_image);
        pmg_scene_close(&scene);
        return 0;
    }
    if (!pmg_scene_checksum(&scene, &checksum, err, err_size)) {
        pmg_image_free(&g_image);
        pmg_scene_close(&scene);
        return 0;
    }
    sprintf(g_status, "PlankaGUI - scene checksum %d", checksum);
    pmg_scene_close(&scene);
    return 1;
}

static void pgw_paint_image(HWND hwnd)
{
    PAINTSTRUCT ps;
    HDC dc;
    BITMAPINFO bmi;
    RECT client;
    int x;
    int y;

    dc = BeginPaint(hwnd, &ps);
    GetClientRect(hwnd, &client);
    memset(&bmi, 0, sizeof(bmi));
    bmi.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    bmi.bmiHeader.biWidth = g_image.width;
    bmi.bmiHeader.biHeight = -g_image.height;
    bmi.bmiHeader.biPlanes = 1;
    bmi.bmiHeader.biBitCount = 24;
    bmi.bmiHeader.biCompression = BI_RGB;

    x = (client.right - client.left - g_image.width) / 2;
    y = (client.bottom - client.top - g_image.height) / 2;
    if (x < 0) {
        x = 0;
    }
    if (y < 0) {
        y = 0;
    }
    StretchDIBits(dc, x, y, g_image.width, g_image.height,
        0, 0, g_image.width, g_image.height, g_image.pixels,
        &bmi, DIB_RGB_COLORS, SRCCOPY);
    EndPaint(hwnd, &ps);
}

static LRESULT CALLBACK pgw_proc(HWND hwnd, UINT msg,
    WPARAM wparam, LPARAM lparam)
{
    switch (msg) {
    case WM_PAINT:
        pgw_paint_image(hwnd);
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
    return (int)msg.wParam;
}
