#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "plankamath.h"
#include "plankac.h"

#define ID_COMPILE 101
#define ID_DEMO    102
#define ID_TESTS   103
#define ID_GUARD   104
#define ID_CLEAR   105
#define ID_RUN     106
#define ID_LIST    200
#define ID_CALC_0  300
#define ID_CALC_1  301
#define ID_CALC_2  302
#define ID_CALC_3  303
#define ID_CALC_4  304
#define ID_CALC_5  305
#define ID_CALC_6  306
#define ID_CALC_7  307
#define ID_CALC_8  308
#define ID_CALC_9  309
#define ID_CALC_DOT    310
#define ID_CALC_BACK   311
#define ID_CALC_ADD    312
#define ID_CALC_SUB    313
#define ID_CALC_MUL    314
#define ID_CALC_DIV    315
#define ID_CALC_EQ     316
#define ID_CALC_CLEAR  317
#define ID_CALC_SQRT   318
#define ID_CALC_SQUARE 319
#define ID_CALC_NEG    320

#define OP_NONE 0
#define OP_ADD  1
#define OP_SUB  2
#define OP_MUL  3
#define OP_DIV  4
#define GUI_MAX_PROCS 128
#define GUI_MAX_ARGS 16

static HWND g_display;
static HWND g_log;
static HWND g_status;
static HWND g_list;
static HWND g_arg0;
static HWND g_arg1;
static HWND g_arg2;
static HWND g_selected;
static double g_calc_accumulator = 0.0;
static int g_calc_has_accumulator = 0;
static int g_calc_pending_op = OP_NONE;
static int g_calc_start_new = 1;
static PLANKAC_PROC_INFO g_proc_infos[GUI_MAX_PROCS];
static int g_proc_count = 0;
static int g_plankac_ready = 0;

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
    SendMessage(g_log, EM_REPLACESEL, 0, (LPARAM)(LPCSTR)text);
    SendMessage(g_log, EM_REPLACESEL, 0, (LPARAM)(LPCSTR)"\r\n");
}

static void format_result(const PLANKAC_RESULT *result, char *out, unsigned out_size)
{
    char value[64];
    char line[256];
    int i;

    if (out_size == 0) {
        return;
    }
    out[0] = '\0';
    for (i = 0; i < result->count; ++i) {
        plankac_format(result->value[i], value, sizeof(value));
        if (i == 0) {
            sprintf(line, "R%d=%s", i, value);
        } else {
            sprintf(line, "%s R%d=%s", out, i, value);
        }
        strncpy(out, line, out_size - 1);
        out[out_size - 1] = '\0';
    }
}

static int legacy_run_proc(const char *name, const double *args,
    PLANKAC_RESULT *result)
{
    double r0;
    int r1;
    int count;

    if (pm_run_proc(name, args, &r0, &r1, &count) != PM_OK) {
        return 0;
    }
    result->count = count;
    result->value[0] = r0;
    if (count > 1) {
        result->value[1] = (double)r1;
    }
    return 1;
}

static int run_proc_gui(const char *name, const double *args, int argc,
    PLANKAC_RESULT *result, char *err, unsigned err_size)
{
    if (g_plankac_ready) {
        if (plankac_run(name, args, argc, result, err, err_size)) {
            return 1;
        }
        append_log(err);
    }
    if (legacy_run_proc(name, args, result)) {
        if (err_size > 0 && err != 0) {
            strcpy(err, "legacy runtime used");
        }
        return 1;
    }
    if (err_size > 0 && err != 0 && err[0] == '\0') {
        strcpy(err, "procedure failed");
    }
    return 0;
}

static int load_proc_infos_from_plankac(void)
{
    int count;
    int i;

    count = plankac_proc_count();
    if (count <= 0) {
        return 0;
    }
    if (count > GUI_MAX_PROCS) {
        count = GUI_MAX_PROCS;
    }
    for (i = 0; i < count; ++i) {
        if (!plankac_get_proc(i, &g_proc_infos[i])) {
            return 0;
        }
    }
    g_proc_count = count;
    g_plankac_ready = 1;
    return 1;
}

static void load_proc_infos_from_legacy(void)
{
    int count;
    int i;

    count = (int)PM_PROC_COUNT;
    if (count > GUI_MAX_PROCS) {
        count = GUI_MAX_PROCS;
    }
    for (i = 0; i < count; ++i) {
        g_proc_infos[i].number = PM_PROCS[i].number;
        strncpy(g_proc_infos[i].name, PM_PROCS[i].name,
            sizeof(g_proc_infos[i].name) - 1);
        g_proc_infos[i].name[sizeof(g_proc_infos[i].name) - 1] = '\0';
        g_proc_infos[i].argc = PM_PROCS[i].argc;
        g_proc_infos[i].results = PM_PROCS[i].results;
        g_proc_infos[i].statements = 0;
    }
    g_proc_count = count;
    g_plankac_ready = 0;
}

static void populate_function_list(void)
{
    char line[96];
    int i;

    SendMessage(g_list, LB_RESETCONTENT, 0, 0);
    for (i = 0; i < g_proc_count; ++i) {
        sprintf(line, "P%d  %s", g_proc_infos[i].number, g_proc_infos[i].name);
        SendMessage(g_list, LB_ADDSTRING, 0, (LPARAM)(LPCSTR)line);
    }
    SendMessage(g_list, LB_SETCURSEL, 0, 0);
    if (g_proc_count > 0) {
        SetWindowText(g_selected, g_proc_infos[0].name);
    }
}

static HWND make_button(HWND hwnd, const char *text, int id, int x, int y, int w, int h)
{
    return CreateWindow("BUTTON", text, WS_CHILD | WS_VISIBLE,
        x, y, w, h, hwnd, (HMENU)(INT_PTR)id, 0, 0);
}

static void do_compile(void)
{
    char out[160];
    if (plankac_compile_summary(out, sizeof(out))) {
        load_proc_infos_from_plankac();
        populate_function_list();
        set_status("PlankaC loaded");
    } else {
        char legacy[160];
        if (pm_compile_project(legacy, sizeof(legacy)) == PM_OK) {
            load_proc_infos_from_legacy();
            populate_function_list();
            set_status("Legacy loaded");
            append_log(out);
            append_log(legacy);
            return;
        }
        set_status("Load failed");
    }
    append_log(out);
}

static void do_demo(void)
{
    PLANKAC_RESULT result;
    double args[1];
    char err[160];
    char out[64];
    args[0] = 0.0;
    err[0] = '\0';
    if (!run_proc_gui("calculator_demo", args, 0, &result, err, sizeof(err))) {
        set_status("Demo failed");
        append_log(err);
        return;
    }
    plankac_format(result.value[0], out, sizeof(out));
    set_display(out);
    append_log("calculator_demo() =>");
    append_log(out);
}

static void do_tests(void)
{
    PLANKAC_RESULT result;
    double args[1];
    char err[160];
    char out[64];

    args[0] = 0.0;
    err[0] = '\0';
    if (!run_proc_gui("all_tests", args, 0, &result, err, sizeof(err))) {
        set_status("Tests failed");
        append_log(err);
        return;
    }
    plankac_format(result.value[0], out, sizeof(out));
    set_display(out);
    append_log("all_tests() =>");
    append_log(out);
}

static void do_guard(void)
{
    PLANKAC_RESULT result;
    double args[2];
    char err[160];
    char out[64];

    args[0] = 84.0;
    args[1] = 0.0;
    err[0] = '\0';
    if (!run_proc_gui("guarded_division_demo", args, 2,
            &result, err, sizeof(err))) {
        set_status("Guard failed");
        append_log(err);
        return;
    }
    format_result(&result, out, sizeof(out));
    set_display(out);
    append_log("guarded_division_demo(84, 0) =>");
    append_log(out);
}

static void fill_function_list(void)
{
    char err[160];

    err[0] = '\0';
    if (plankac_reload(err, sizeof(err))) {
        load_proc_infos_from_plankac();
        set_status("PlankaC loaded");
    } else {
        load_proc_infos_from_legacy();
        set_status("Legacy loaded");
        append_log(err);
    }
    populate_function_list();
}

static int selected_index(void)
{
    int idx;
    idx = (int)SendMessage(g_list, LB_GETCURSEL, 0, 0);
    if (idx < 0 || idx >= g_proc_count) {
        return 0;
    }
    return idx;
}

static void update_selected(void)
{
    int idx;
    char line[128];
    idx = selected_index();
    sprintf(line, "P%d %s  args=%d  results=%d",
        g_proc_infos[idx].number,
        g_proc_infos[idx].name,
        g_proc_infos[idx].argc,
        g_proc_infos[idx].results);
    SetWindowText(g_selected, line);
}

static double read_arg(HWND edit)
{
    char text[64];
    GetWindowText(edit, text, sizeof(text));
    return atof(text);
}

static void do_run_selected(void)
{
    int idx;
    double args[GUI_MAX_ARGS];
    PLANKAC_RESULT result;
    char err[160];
    char out[160];
    int i;

    idx = selected_index();
    for (i = 0; i < GUI_MAX_ARGS; ++i) {
        args[i] = 0.0;
    }
    args[0] = read_arg(g_arg0);
    args[1] = read_arg(g_arg1);
    args[2] = read_arg(g_arg2);
    err[0] = '\0';

    if (!run_proc_gui(g_proc_infos[idx].name, args, g_proc_infos[idx].argc,
            &result, err, sizeof(err))) {
        set_status("Run failed");
        append_log(err);
        return;
    }

    format_result(&result, out, sizeof(out));
    set_display(out);
    sprintf(out, "%s(...) =>", g_proc_infos[idx].name);
    append_log(out);
    format_result(&result, out, sizeof(out));
    append_log(out);
    set_status("Ran function");
}

static double display_number(void)
{
    char text[64];
    GetWindowText(g_display, text, sizeof(text));
    return atof(text);
}

static void calc_show_number(double value)
{
    char out[64];
    plankac_format(value, out, sizeof(out));
    set_display(out);
    g_calc_start_new = 1;
}

static void calc_error(const char *message)
{
    set_display("Error");
    set_status("Calc error");
    append_log(message);
    g_calc_accumulator = 0.0;
    g_calc_has_accumulator = 0;
    g_calc_pending_op = OP_NONE;
    g_calc_start_new = 1;
}

static void calc_digit(char digit)
{
    char text[64];
    int len;
    GetWindowText(g_display, text, sizeof(text));
    if (g_calc_start_new || strcmp(text, "Error") == 0) {
        text[0] = digit;
        text[1] = 0;
        g_calc_start_new = 0;
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
    if (g_calc_start_new) {
        set_display("0.");
        g_calc_start_new = 0;
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
    if (g_calc_start_new || len <= 1 || strcmp(text, "Error") == 0) {
        set_display("0");
        g_calc_start_new = 1;
        return;
    }
    text[len - 1] = 0;
    set_display(text);
}

static void calc_clear(void)
{
    set_display("0");
    set_status("Ready");
    g_calc_accumulator = 0.0;
    g_calc_has_accumulator = 0;
    g_calc_pending_op = OP_NONE;
    g_calc_start_new = 1;
}

static int calc_apply(double right)
{
    PLANKAC_RESULT result_values;
    double args[2];
    char err[160];
    double result;
    result = 0.0;
    if (!g_calc_has_accumulator || g_calc_pending_op == OP_NONE) {
        g_calc_accumulator = right;
        g_calc_has_accumulator = 1;
        return 1;
    }
    args[0] = g_calc_accumulator;
    args[1] = right;
    err[0] = '\0';
    if (g_calc_pending_op == OP_ADD) {
        if (!run_proc_gui("add", args, 2, &result_values, err, sizeof(err))) {
            calc_error(err);
            return 0;
        }
        result = result_values.value[0];
    } else if (g_calc_pending_op == OP_SUB) {
        if (!run_proc_gui("subtract", args, 2, &result_values, err, sizeof(err))) {
            calc_error(err);
            return 0;
        }
        result = result_values.value[0];
    } else if (g_calc_pending_op == OP_MUL) {
        if (!run_proc_gui("multiply", args, 2, &result_values, err, sizeof(err))) {
            calc_error(err);
            return 0;
        }
        result = result_values.value[0];
    } else if (g_calc_pending_op == OP_DIV) {
        if (!run_proc_gui("divide_checked", args, 2,
                &result_values, err, sizeof(err))) {
            calc_error(err);
            return 0;
        }
        if (result_values.count > 1 && result_values.value[1] != 0.0) {
            calc_error("Division by zero.");
            return 0;
        }
        result = result_values.value[0];
    }
    g_calc_accumulator = result;
    g_calc_has_accumulator = 1;
    calc_show_number(result);
    return 1;
}

static void calc_set_operation(int op)
{
    double value;
    value = display_number();
    if (g_calc_pending_op != OP_NONE && !g_calc_start_new) {
        if (!calc_apply(value)) {
            return;
        }
    } else {
        g_calc_accumulator = value;
        g_calc_has_accumulator = 1;
    }
    g_calc_pending_op = op;
    g_calc_start_new = 1;
    set_status("Calc op");
}

static void calc_equals(void)
{
    double value;
    value = display_number();
    if (g_calc_pending_op == OP_NONE) {
        calc_show_number(value);
        return;
    }
    if (calc_apply(value)) {
        g_calc_pending_op = OP_NONE;
        set_status("Calc result");
    }
}

static void calc_sqrt(void)
{
    PLANKAC_RESULT result;
    double args[1];
    char err[160];

    args[0] = display_number();
    err[0] = '\0';
    if (!run_proc_gui("root_checked", args, 1, &result, err, sizeof(err))) {
        calc_error(err);
        return;
    }
    if (result.count > 1 && result.value[1] != 0.0) {
        calc_error("Root of negative value.");
        return;
    }
    calc_show_number(result.value[0]);
    set_status("sqrt");
}

static void calc_square(void)
{
    PLANKAC_RESULT result;
    double args[1];
    char err[160];

    args[0] = display_number();
    err[0] = '\0';
    if (!run_proc_gui("square", args, 1, &result, err, sizeof(err))) {
        calc_error(err);
        return;
    }
    calc_show_number(result.value[0]);
    set_status("x^2");
}

static void calc_negate(void)
{
    PLANKAC_RESULT result;
    double args[1];
    char err[160];

    args[0] = display_number();
    err[0] = '\0';
    if (!run_proc_gui("negate", args, 1, &result, err, sizeof(err))) {
        calc_error(err);
        return;
    }
    calc_show_number(result.value[0]);
    set_status("+/-");
}

static void create_calculator(HWND hwnd)
{
    int x;
    int y;
    int w;
    int h;
    int dx;
    int dy;
    x = 494;
    y = 104;
    w = 42;
    h = 28;
    dx = 46;
    dy = 34;

    CreateWindow("STATIC", "Calculator", WS_CHILD | WS_VISIBLE,
        x, y - 20, 120, 18, hwnd, 0, 0, 0);

    make_button(hwnd, "7", ID_CALC_7, x, y, w, h);
    make_button(hwnd, "8", ID_CALC_8, x + dx, y, w, h);
    make_button(hwnd, "9", ID_CALC_9, x + dx * 2, y, w, h);
    make_button(hwnd, "/", ID_CALC_DIV, x + dx * 3, y, w, h);

    make_button(hwnd, "4", ID_CALC_4, x, y + dy, w, h);
    make_button(hwnd, "5", ID_CALC_5, x + dx, y + dy, w, h);
    make_button(hwnd, "6", ID_CALC_6, x + dx * 2, y + dy, w, h);
    make_button(hwnd, "*", ID_CALC_MUL, x + dx * 3, y + dy, w, h);

    make_button(hwnd, "1", ID_CALC_1, x, y + dy * 2, w, h);
    make_button(hwnd, "2", ID_CALC_2, x + dx, y + dy * 2, w, h);
    make_button(hwnd, "3", ID_CALC_3, x + dx * 2, y + dy * 2, w, h);
    make_button(hwnd, "-", ID_CALC_SUB, x + dx * 3, y + dy * 2, w, h);

    make_button(hwnd, "0", ID_CALC_0, x, y + dy * 3, w, h);
    make_button(hwnd, ".", ID_CALC_DOT, x + dx, y + dy * 3, w, h);
    make_button(hwnd, "Back", ID_CALC_BACK, x + dx * 2, y + dy * 3, w, h);
    make_button(hwnd, "+", ID_CALC_ADD, x + dx * 3, y + dy * 3, w, h);

    make_button(hwnd, "sqrt", ID_CALC_SQRT, x, y + dy * 4, w, h);
    make_button(hwnd, "x^2", ID_CALC_SQUARE, x + dx, y + dy * 4, w, h);
    make_button(hwnd, "+/-", ID_CALC_NEG, x + dx * 2, y + dy * 4, w, h);
    make_button(hwnd, "=", ID_CALC_EQ, x + dx * 3, y + dy * 4, w, h);

    make_button(hwnd, "C", ID_CALC_CLEAR, x, y + dy * 5, w + dx * 3, h);
}

#ifdef _WIN32
#define CMD_ID(wp, lp) LOWORD(wp)
#define CMD_EVENT(wp, lp) HIWORD(wp)
#else
#define CMD_ID(wp, lp) (wp)
#define CMD_EVENT(wp, lp) HIWORD(lParam)
#endif

LRESULT CALLBACK MainWndProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
    switch (msg) {
    case WM_CREATE:
        CreateWindow("STATIC", "PlankaMath", WS_CHILD | WS_VISIBLE,
            10, 8, 180, 18, hwnd, 0, 0, 0);
        g_status = CreateWindow("STATIC", "Ready", WS_CHILD | WS_VISIBLE,
            510, 8, 100, 18, hwnd, 0, 0, 0);
        g_display = CreateWindow("EDIT", "0",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_RIGHT | ES_READONLY,
            10, 32, 600, 30, hwnd, 0, 0, 0);
        CreateWindow("BUTTON", "Compile", WS_CHILD | WS_VISIBLE,
            10, 70, 95, 24, hwnd, (HMENU)ID_COMPILE, 0, 0);
        CreateWindow("BUTTON", "Demo", WS_CHILD | WS_VISIBLE,
            112, 70, 95, 24, hwnd, (HMENU)ID_DEMO, 0, 0);
        CreateWindow("BUTTON", "Tests", WS_CHILD | WS_VISIBLE,
            214, 70, 95, 24, hwnd, (HMENU)ID_TESTS, 0, 0);
        CreateWindow("BUTTON", "Guard", WS_CHILD | WS_VISIBLE,
            316, 70, 95, 24, hwnd, (HMENU)ID_GUARD, 0, 0);
        CreateWindow("BUTTON", "Clear", WS_CHILD | WS_VISIBLE,
            418, 70, 95, 24, hwnd, (HMENU)ID_CLEAR, 0, 0);
        g_list = CreateWindow("LISTBOX", "",
            WS_CHILD | WS_VISIBLE | WS_BORDER | WS_VSCROLL | LBS_NOTIFY,
            10, 104, 250, 300, hwnd, (HMENU)ID_LIST, 0, 0);
        g_selected = CreateWindow("STATIC", "", WS_CHILD | WS_VISIBLE,
            278, 106, 325, 20, hwnd, 0, 0, 0);
        CreateWindow("STATIC", "V0", WS_CHILD | WS_VISIBLE,
            278, 138, 24, 20, hwnd, 0, 0, 0);
        g_arg0 = CreateWindow("EDIT", "0", WS_CHILD | WS_VISIBLE | WS_BORDER,
            308, 136, 120, 24, hwnd, 0, 0, 0);
        CreateWindow("STATIC", "V1", WS_CHILD | WS_VISIBLE,
            278, 168, 24, 20, hwnd, 0, 0, 0);
        g_arg1 = CreateWindow("EDIT", "0", WS_CHILD | WS_VISIBLE | WS_BORDER,
            308, 166, 120, 24, hwnd, 0, 0, 0);
        CreateWindow("STATIC", "V2", WS_CHILD | WS_VISIBLE,
            278, 198, 24, 20, hwnd, 0, 0, 0);
        g_arg2 = CreateWindow("EDIT", "0", WS_CHILD | WS_VISIBLE | WS_BORDER,
            308, 196, 120, 24, hwnd, 0, 0, 0);
        CreateWindow("BUTTON", "Run Selected", WS_CHILD | WS_VISIBLE,
            278, 232, 150, 26, hwnd, (HMENU)ID_RUN, 0, 0);
        g_log = CreateWindow("EDIT", "",
            WS_CHILD | WS_VISIBLE | WS_BORDER | ES_MULTILINE | ES_AUTOVSCROLL | WS_VSCROLL | ES_READONLY,
            278, 270, 204, 134, hwnd, 0, 0, 0);
        create_calculator(hwnd);
        fill_function_list();
        append_log("PlankaMath GUI loaded.");
        if (g_plankac_ready) {
            append_log("Execution path: PlankaC reads and runs .plk.");
        } else {
            append_log("Execution path: legacy C runtime fallback.");
        }
        break;
    case WM_COMMAND:
        if (CMD_ID(wParam, lParam) == ID_LIST && CMD_EVENT(wParam, lParam) == LBN_SELCHANGE) {
            update_selected();
            break;
        }
        switch (CMD_ID(wParam, lParam)) {
        case ID_COMPILE:
            do_compile();
            break;
        case ID_DEMO:
            do_demo();
            break;
        case ID_TESTS:
            do_tests();
            break;
        case ID_GUARD:
            do_guard();
            break;
        case ID_CLEAR:
            SetWindowText(g_log, "");
            set_display("0");
            set_status("Ready");
            break;
        case ID_RUN:
            do_run_selected();
            break;
        case ID_CALC_0:
            calc_digit('0');
            break;
        case ID_CALC_1:
            calc_digit('1');
            break;
        case ID_CALC_2:
            calc_digit('2');
            break;
        case ID_CALC_3:
            calc_digit('3');
            break;
        case ID_CALC_4:
            calc_digit('4');
            break;
        case ID_CALC_5:
            calc_digit('5');
            break;
        case ID_CALC_6:
            calc_digit('6');
            break;
        case ID_CALC_7:
            calc_digit('7');
            break;
        case ID_CALC_8:
            calc_digit('8');
            break;
        case ID_CALC_9:
            calc_digit('9');
            break;
        case ID_CALC_DOT:
            calc_dot();
            break;
        case ID_CALC_BACK:
            calc_back();
            break;
        case ID_CALC_CLEAR:
            calc_clear();
            break;
        case ID_CALC_ADD:
            calc_set_operation(OP_ADD);
            break;
        case ID_CALC_SUB:
            calc_set_operation(OP_SUB);
            break;
        case ID_CALC_MUL:
            calc_set_operation(OP_MUL);
            break;
        case ID_CALC_DIV:
            calc_set_operation(OP_DIV);
            break;
        case ID_CALC_EQ:
            calc_equals();
            break;
        case ID_CALC_SQRT:
            calc_sqrt();
            break;
        case ID_CALC_SQUARE:
            calc_square();
            break;
        case ID_CALC_NEG:
            calc_negate();
            break;
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return 0;
}

int PASCAL WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    WNDCLASS wc;
    HWND hwnd;
    MSG msg;

    (void)lpCmdLine;

    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = MainWndProc;
    wc.cbClsExtra = 0;
    wc.cbWndExtra = 0;
    wc.hInstance = hInstance;
    wc.hIcon = LoadIcon(0, IDI_APPLICATION);
    wc.hCursor = LoadCursor(0, IDC_ARROW);
    wc.hbrBackground = (HBRUSH)(COLOR_BTNFACE + 1);
    wc.lpszMenuName = 0;
    wc.lpszClassName = "PlankaMathWindow";

    if (!hPrevInstance) {
        if (!RegisterClass(&wc)) {
            return 0;
        }
    }

    hwnd = CreateWindow("PlankaMathWindow", "PlankaMath",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 760, 480,
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

    return (int)msg.wParam;
}
