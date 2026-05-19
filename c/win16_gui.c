#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "plankamath.h"

#ifndef CALLBACK
#define CALLBACK FAR PASCAL
#endif

#ifdef _WIN32
#define PM_CONTROL_ID(id) ((HMENU)(UINT_PTR)(id))
#else
#define PM_CONTROL_ID(id) ((HMENU)(id))
#endif

#define ID_COMPILE 101
#define ID_DEMO    102
#define ID_TESTS   103
#define ID_GUARD   104
#define ID_CLEAR   105
#define ID_RUN     106
#define ID_LIST    200

#define ID_CALC_0      300
#define ID_CALC_1      301
#define ID_CALC_2      302
#define ID_CALC_3      303
#define ID_CALC_4      304
#define ID_CALC_5      305
#define ID_CALC_6      306
#define ID_CALC_7      307
#define ID_CALC_8      308
#define ID_CALC_9      309
#define ID_CALC_DOT    310
#define ID_CALC_BACK   311
#define ID_CALC_ADD    312
#define ID_CALC_SUB    313
#define ID_CALC_MUL    314
#define ID_CALC_DIV    315
#define ID_CALC_EQ     316
#define ID_CALC_SQRT   317
#define ID_CALC_SQUARE 318
#define ID_CALC_NEG    319

#define OP_NONE 0
#define OP_ADD  1
#define OP_SUB  2
#define OP_MUL  3
#define OP_DIV  4

static HWND g_display;
static HWND g_status;
static HWND g_list;
static HWND g_selected;
static HWND g_arg0;
static HWND g_arg1;
static HWND g_arg2;
static HWND g_log;
static double g_accumulator = 0.0;
static int g_has_accumulator = 0;
static int g_pending_op = OP_NONE;
static int g_start_new = 1;

static void set_display(const char *text)
{
    SetWindowText(g_display, text);
}

static void set_status(const char *text)
{
    SetWindowText(g_status, text);
}

static void append_log(const char *text)
{
    int len;

    len = GetWindowTextLength(g_log);
    SendMessage(g_log, EM_SETSEL, len, len);
    SendMessage(g_log, EM_REPLACESEL, 0, (LPARAM)(LPSTR)text);
    SendMessage(g_log, EM_REPLACESEL, 0, (LPARAM)(LPSTR)"\r\n");
}

static HWND make_button(HWND parent, const char *text, int id,
    int x, int y, int w, int h)
{
    return CreateWindow("BUTTON", text, WS_CHILD | WS_VISIBLE,
        x, y, w, h, parent, PM_CONTROL_ID(id), 0, 0);
}

static void format_pair(double r0, int r1, int count,
    char *out, unsigned out_size)
{
    char a[64];
    char b[64];

    if (out_size == 0) {
        return;
    }
    out[0] = 0;
    pm_format(r0, a, sizeof(a));
    if (count > 1) {
        pm_format((double)r1, b, sizeof(b));
        sprintf(out, "R0=%s R1=%s", a, b);
    } else {
        sprintf(out, "R0=%s", a);
    }
}

static int selected_index(void)
{
    int idx;

    idx = (int)SendMessage(g_list, LB_GETCURSEL, 0, 0);
    if (idx < 0 || idx >= (int)PM_PROC_COUNT) {
        return 0;
    }
    return idx;
}

static void update_selected(void)
{
    int idx;
    char line[96];

    idx = selected_index();
    sprintf(line, "P%d %s  V=%d  R=%d",
        PM_PROCS[idx].number,
        PM_PROCS[idx].name,
        PM_PROCS[idx].argc,
        PM_PROCS[idx].results);
    SetWindowText(g_selected, line);
}

static void fill_proc_list(void)
{
    unsigned i;
    char line[96];

    SendMessage(g_list, LB_RESETCONTENT, 0, 0);
    for (i = 0; i < PM_PROC_COUNT; ++i) {
        sprintf(line, "P%d %s", PM_PROCS[i].number, PM_PROCS[i].name);
        SendMessage(g_list, LB_ADDSTRING, 0, (LPARAM)(LPSTR)line);
    }
    SendMessage(g_list, LB_SETCURSEL, 0, 0);
    update_selected();
}

static double read_number(HWND edit)
{
    char text[64];

    GetWindowText(edit, text, sizeof(text));
    return atof(text);
}

static double display_number(void)
{
    char text[64];

    GetWindowText(g_display, text, sizeof(text));
    return atof(text);
}

static void show_number(double value)
{
    char out[64];

    pm_format(value, out, sizeof(out));
    set_display(out);
    g_start_new = 1;
}

static void calc_error(const char *text)
{
    set_display("Error");
    set_status("Calc error");
    append_log(text);
    g_accumulator = 0.0;
    g_has_accumulator = 0;
    g_pending_op = OP_NONE;
    g_start_new = 1;
}

static void calc_digit(char digit)
{
    char text[64];
    int len;

    GetWindowText(g_display, text, sizeof(text));
    if (g_start_new || strcmp(text, "Error") == 0) {
        text[0] = digit;
        text[1] = 0;
        g_start_new = 0;
    } else {
        len = (int)strlen(text);
        if (strcmp(text, "0") == 0) {
            text[0] = digit;
            text[1] = 0;
        } else if (len < (int)sizeof(text) - 1) {
            text[len] = digit;
            text[len + 1] = 0;
        }
    }
    set_display(text);
}

static void calc_dot(void)
{
    char text[64];

    if (g_start_new) {
        set_display("0.");
        g_start_new = 0;
        return;
    }
    GetWindowText(g_display, text, sizeof(text));
    if (strchr(text, '.') == 0 && strlen(text) < sizeof(text) - 1) {
        strcat(text, ".");
        set_display(text);
    }
}

static void calc_back(void)
{
    char text[64];
    int len;

    GetWindowText(g_display, text, sizeof(text));
    len = (int)strlen(text);
    if (g_start_new || len <= 1 || strcmp(text, "Error") == 0) {
        set_display("0");
        g_start_new = 1;
        return;
    }
    text[len - 1] = 0;
    set_display(text);
}

static void calc_clear(void)
{
    set_display("0");
    set_status("Ready");
    g_accumulator = 0.0;
    g_has_accumulator = 0;
    g_pending_op = OP_NONE;
    g_start_new = 1;
}

static int calc_apply(double right)
{
    PM_PAIR pair;
    double result;

    if (!g_has_accumulator || g_pending_op == OP_NONE) {
        g_accumulator = right;
        g_has_accumulator = 1;
        return 1;
    }

    result = 0.0;
    if (g_pending_op == OP_ADD) {
        result = pm_add(g_accumulator, right);
    } else if (g_pending_op == OP_SUB) {
        result = pm_subtract(g_accumulator, right);
    } else if (g_pending_op == OP_MUL) {
        result = pm_multiply(g_accumulator, right);
    } else if (g_pending_op == OP_DIV) {
        pair = pm_divide_checked(g_accumulator, right);
        if (pair.error != PM_OK) {
            calc_error("Division by zero.");
            return 0;
        }
        result = pair.value;
    }

    g_accumulator = result;
    return 1;
}

static void calc_binary(int op)
{
    double right;

    right = display_number();
    if (!calc_apply(right)) {
        return;
    }
    g_pending_op = op;
    g_start_new = 1;
    show_number(g_accumulator);
}

static void calc_equal(void)
{
    double right;

    right = display_number();
    if (!calc_apply(right)) {
        return;
    }
    g_pending_op = OP_NONE;
    show_number(g_accumulator);
    append_log("calculator =>");
    append_log("operation completed");
}

static void calc_sqrt(void)
{
    PM_PAIR pair;

    pair = pm_root_checked(display_number());
    if (pair.error != PM_OK) {
        calc_error("Square root of a negative value.");
        return;
    }
    show_number(pair.value);
}

static void calc_square(void)
{
    show_number(pm_square(display_number()));
}

static void calc_negate(void)
{
    show_number(pm_negate(display_number()));
}

static void do_compile(void)
{
    char out[160];

    if (pm_compile_project(out, sizeof(out)) == PM_OK) {
        append_log(out);
        set_status("Source checked");
    } else {
        append_log(out);
        set_status("Source check failed");
    }
}

static void do_demo(void)
{
    double r;
    char out[64];

    r = pm_calculator_demo();
    pm_format(r, out, sizeof(out));
    set_display(out);
    append_log("calculator_demo() =>");
    append_log(out);
}

static void do_tests(void)
{
    char out[64];

    pm_format((double)pm_all_tests(), out, sizeof(out));
    set_display(out);
    append_log("all_tests() =>");
    append_log(out);
}

static void do_guard(void)
{
    PM_PAIR pair;
    char out[96];

    pair = pm_guarded_division_demo(84.0, 0.0);
    format_pair(pair.value, pair.error, 2, out, sizeof(out));
    set_display(out);
    append_log("guarded_division_demo(84, 0) =>");
    append_log(out);
}

static void do_run_selected(void)
{
    int idx;
    double args[3];
    double r0;
    int r1;
    int count;
    char out[128];

    idx = selected_index();
    args[0] = read_number(g_arg0);
    args[1] = read_number(g_arg1);
    args[2] = read_number(g_arg2);

    if (pm_run_proc(PM_PROCS[idx].name, args, &r0, &r1, &count) != PM_OK) {
        set_status("Run failed");
        append_log("Procedure failed.");
        return;
    }

    format_pair(r0, r1, count, out, sizeof(out));
    set_display(out);
    sprintf(out, "%s(...) =>", PM_PROCS[idx].name);
    append_log(out);
    format_pair(r0, r1, count, out, sizeof(out));
    append_log(out);
    set_status("Ran function");
}

static void create_calculator(HWND hwnd)
{
    int x;
    int y;
    int w;
    int h;
    int gap;

    x = 8;
    y = 88;
    w = 72;
    h = 22;
    gap = 4;

    make_button(hwnd, "7", ID_CALC_7, x, y, w, h);
    make_button(hwnd, "8", ID_CALC_8, x + 76, y, w, h);
    make_button(hwnd, "9", ID_CALC_9, x + 152, y, w, h);
    make_button(hwnd, "/", ID_CALC_DIV, x + 228, y, w, h);
    make_button(hwnd, "sqrt", ID_CALC_SQRT, x + 304, y, w, h);

    y += h + gap;
    make_button(hwnd, "4", ID_CALC_4, x, y, w, h);
    make_button(hwnd, "5", ID_CALC_5, x + 76, y, w, h);
    make_button(hwnd, "6", ID_CALC_6, x + 152, y, w, h);
    make_button(hwnd, "*", ID_CALC_MUL, x + 228, y, w, h);
    make_button(hwnd, "x^2", ID_CALC_SQUARE, x + 304, y, w, h);

    y += h + gap;
    make_button(hwnd, "1", ID_CALC_1, x, y, w, h);
    make_button(hwnd, "2", ID_CALC_2, x + 76, y, w, h);
    make_button(hwnd, "3", ID_CALC_3, x + 152, y, w, h);
    make_button(hwnd, "-", ID_CALC_SUB, x + 228, y, w, h);
    make_button(hwnd, "+/-", ID_CALC_NEG, x + 304, y, w, h);

    y += h + gap;
    make_button(hwnd, "0", ID_CALC_0, x, y, w, h);
    make_button(hwnd, ".", ID_CALC_DOT, x + 76, y, w, h);
    make_button(hwnd, "Back", ID_CALC_BACK, x + 152, y, w, h);
    make_button(hwnd, "+", ID_CALC_ADD, x + 228, y, w, h);
    make_button(hwnd, "=", ID_CALC_EQ, x + 304, y, w, h);
}

static void create_controls(HWND hwnd)
{
    CreateWindow("STATIC", "PlankaMath16", WS_CHILD | WS_VISIBLE,
        8, 8, 110, 16, hwnd, 0, 0, 0);
    g_status = CreateWindow("STATIC", "Ready", WS_CHILD | WS_VISIBLE,
        244, 8, 150, 16, hwnd, 0, 0, 0);

    g_display = CreateWindow("EDIT", "0",
        WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT,
        8, 28, 386, 22, hwnd, 0, 0, 0);

    make_button(hwnd, "Compile", ID_COMPILE, 8, 58, 72, 22);
    make_button(hwnd, "Demo", ID_DEMO, 84, 58, 72, 22);
    make_button(hwnd, "Tests", ID_TESTS, 160, 58, 72, 22);
    make_button(hwnd, "Guard", ID_GUARD, 236, 58, 72, 22);
    make_button(hwnd, "Clear", ID_CLEAR, 312, 58, 72, 22);

    create_calculator(hwnd);

    g_list = CreateWindow("LISTBOX", "",
        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
        8, 202, 160, 166, hwnd, PM_CONTROL_ID(ID_LIST), 0, 0);

    g_selected = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE,
        178, 202, 210, 18, hwnd, 0, 0, 0);

    CreateWindow("STATIC", "V0", WS_CHILD | WS_VISIBLE,
        178, 230, 24, 18, hwnd, 0, 0, 0);
    g_arg0 = CreateWindow("EDIT", "1", WS_CHILD | WS_VISIBLE | WS_BORDER,
        206, 228, 128, 22, hwnd, 0, 0, 0);

    CreateWindow("STATIC", "V1", WS_CHILD | WS_VISIBLE,
        178, 256, 24, 18, hwnd, 0, 0, 0);
    g_arg1 = CreateWindow("EDIT", "2", WS_CHILD | WS_VISIBLE | WS_BORDER,
        206, 254, 128, 22, hwnd, 0, 0, 0);

    CreateWindow("STATIC", "V2", WS_CHILD | WS_VISIBLE,
        178, 282, 24, 18, hwnd, 0, 0, 0);
    g_arg2 = CreateWindow("EDIT", "0", WS_CHILD | WS_VISIBLE | WS_BORDER,
        206, 280, 128, 22, hwnd, 0, 0, 0);

    make_button(hwnd, "Run Selected", ID_RUN, 206, 308, 128, 24);

    g_log = CreateWindow("EDIT", "",
        WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL
        | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
        178, 338, 210, 60, hwnd, 0, 0, 0);

    fill_proc_list();
    append_log("PlankaMath16 Win16 shell loaded.");
}

static int command_id(WPARAM wParam)
{
#ifdef _WIN32
    return LOWORD(wParam);
#else
    return (int)wParam;
#endif
}

#ifdef _WIN32
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
#else
long FAR PASCAL MainWndProc(HWND hwnd, WORD msg, WORD wParam, LONG lParam)
#endif
{
    int id;

    switch (msg) {
    case WM_CREATE:
        create_controls(hwnd);
        return 0;
    case WM_COMMAND:
        id = command_id(wParam);
        if (id == ID_COMPILE) {
            do_compile();
        } else if (id == ID_DEMO) {
            do_demo();
        } else if (id == ID_TESTS) {
            do_tests();
        } else if (id == ID_GUARD) {
            do_guard();
        } else if (id == ID_CLEAR) {
            calc_clear();
            SetWindowText(g_log, "");
        } else if (id == ID_RUN) {
            do_run_selected();
        } else if (id == ID_LIST) {
            update_selected();
        } else if (id >= ID_CALC_0 && id <= ID_CALC_9) {
            calc_digit((char)('0' + id - ID_CALC_0));
        } else if (id == ID_CALC_DOT) {
            calc_dot();
        } else if (id == ID_CALC_BACK) {
            calc_back();
        } else if (id == ID_CALC_ADD) {
            calc_binary(OP_ADD);
        } else if (id == ID_CALC_SUB) {
            calc_binary(OP_SUB);
        } else if (id == ID_CALC_MUL) {
            calc_binary(OP_MUL);
        } else if (id == ID_CALC_DIV) {
            calc_binary(OP_DIV);
        } else if (id == ID_CALC_EQ) {
            calc_equal();
        } else if (id == ID_CALC_SQRT) {
            calc_sqrt();
        } else if (id == ID_CALC_SQUARE) {
            calc_square();
        } else if (id == ID_CALC_NEG) {
            calc_negate();
        }
        return 0;
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, msg, wParam, lParam);
}

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc;
    HWND hwnd;
    MSG msg;
#ifndef _WIN32
    FARPROC proc_instance;
#endif

    lpCmdLine = lpCmdLine;
    if (!hPrevInstance) {
        memset(&wc, 0, sizeof(wc));
        wc.style = CS_HREDRAW | CS_VREDRAW;
#ifdef _WIN32
        wc.lpfnWndProc = MainWndProc;
#else
        proc_instance = MakeProcInstance((FARPROC)MainWndProc, hInstance);
        wc.lpfnWndProc = (WNDPROC)proc_instance;
#endif
        wc.hInstance = hInstance;
        wc.hIcon = LoadIcon(0, IDI_APPLICATION);
        wc.hCursor = LoadCursor(0, IDC_ARROW);
        wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
        wc.lpszClassName = "PlankaMath16Window";
        if (!RegisterClass(&wc)) {
            return 0;
        }
    }

    hwnd = CreateWindow("PlankaMath16Window", "PlankaMath16",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 430, 450,
        0, 0, hInstance, 0);

    if (!hwnd) {
        return 0;
    }

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);

    while (GetMessage(&msg, 0, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

#ifndef _WIN32
    FreeProcInstance(proc_instance);
#endif

    return msg.wParam;
}
