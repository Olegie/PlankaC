# PlankaHost API

PlankaHost is the shared application host for graphical `.plk` programs. It
loads the standard PlankaC source profile first, then one application file.
The resulting context contains the calculator procedures, structured-data
procedures, relation helpers, chess examples, 3D helpers, and the application
procedures from the selected file.

Public header:

```text
graphics/c/plankahost.h
```

Minimal command-line demonstration:

```text
graphics/c/plankahost_demo.c
```

The host layer is intentionally small. It owns the operating-system boundary:
window creation, timers, mouse and keyboard input, pixel buffers, and drawing
to the screen. The application layer stays in `.plk`.

## Application Contract

A PlankaHost application should expose these procedures:

```text
app_kind() => R0
app_canvas() => R0, R1
app_checksum() => R0
app_timer_step(V0) => R0
```

`app_kind` selects the renderer family:

```text
1 = PlankaGUI-style 2D interface
3 = PlankaCube-style 3D scene
```

`app_canvas` returns the base scene size. `app_checksum` is a deterministic
summary used by tests and window titles. `app_timer_step` advances an
application-defined frame value; for static interfaces it can return the input
unchanged.

Kind-specific procedures remain available. A GUI profile can expose
`gui_canvas`, `gui_button_rect`, `gui_function_row_rect`, `gui_style_rgb`, and
`gui_scene_checksum`. A cube profile can expose `cube_canvas`, `cube_vertex`,
`cube_edge`, `cube_model_transform`, `cube_project_vertex`, and
`cube_scene_checksum`.

## C Usage

```c
#include "plankahost.h"

int main(void)
{
    PLANKAHOST_APP app;
    PLANKAC_RESULT result;
    PLANKAC_PROC_INFO info;
    double args[2];
    char err[256];
    int count;

    if (!plankahost_open(&app, "graphics/src/plankagui.plk",
            err, sizeof(err))) {
        return 1;
    }

    count = plankahost_proc_count(&app);
    if (count <= 0) {
        plankahost_close(&app);
        return 1;
    }

    if (!plankahost_find_proc(&app, "add", &info)) {
        plankahost_close(&app);
        return 1;
    }

    args[0] = 12.0;
    args[1] = 8.0;
    if (!plankahost_run(&app, "add", args, 2,
            &result, err, sizeof(err))) {
        plankahost_close(&app);
        return 1;
    }

    plankahost_close(&app);
    return result.value[0] == 20.0 ? 0 : 1;
}
```

The same loaded host can also run application procedures:

```c
plankahost_run(&app, "app_canvas", 0, 0, &result, err, sizeof(err));
plankahost_run(&app, "gui_button_rect", args, 2, &result, err, sizeof(err));
```

For a 3D profile:

```c
plankahost_open(&app, "graphics/src/plankacube.plk", err, sizeof(err));
plankahost_run(&app, "cube_project_vertex", args, 2, &result, err, sizeof(err));
```

## API Functions

```text
plankahost_open()
plankahost_close()
plankahost_proc_count()
plankahost_get_proc()
plankahost_find_proc()
plankahost_run()
plankahost_render()
plankahost_button_at()
plankahost_timer_step()
```

`plankahost_open` loads the full source set. `plankahost_proc_count`,
`plankahost_get_proc`, and `plankahost_find_proc` expose procedure metadata.
`plankahost_run` executes any visible procedure by name, including procedures
from the base profile and procedures from the application file.

`plankahost_render`, `plankahost_button_at`, and `plankahost_timer_step` are
used by the Windows host to connect a loaded `.plk` application to pixels and
input events. A different platform host can reuse the same functions and
replace only the windowing backend.

## Command-Line Use

Run a procedure from an application file without writing a new C host:

```text
build\plankac.exe runfile graphics\src\plankagui.plk app_canvas
build\plankac.exe runfile graphics\src\plankagui.plk add 12 8
build\plankac.exe runfile graphics\src\plankacube.plk cube_scene_checksum
build\plankac.exe runfile graphics\src\plankacube.plk app_timer_step 0.5
```

Open the shared graphical host:

```text
build\PlankaHost.exe graphics\src\plankagui.plk
build\PlankaHost.exe graphics\src\plankacube.plk
```

Check the same API without opening a window:

```text
build\plankahost_demo.exe graphics\src\plankagui.plk
build\plankahost_demo.exe graphics\src\plankacube.plk
```
