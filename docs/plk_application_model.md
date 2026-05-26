# PLK Application Model

PlankaC separates the application layer from the host layer.

`.plk` files are the source layer. They define procedures, value flow, guards,
data structures, geometry, layout contracts, and callable behavior. For normal
computation, a user should not need to write C:

```bat
build\plankac.exe run add 12 8
build\plankac.exe runfile graphics\src\plankagui.plk app_canvas
build\plankac.exe runfile graphics\src\plankacube.plk app_timer_step 0.5
```

`runfile` loads the standard PlankaC profile and then one additional `.plk`
file. The additional file acts as an application module on top of the existing
library profile, so it can call or coexist with the older arithmetic,
calculator, data-structure, relation, chess, and 3D procedures.

C is the host layer. It provides the operating-system boundary: executable
startup, files, windows, timers, mouse and keyboard input, pixel buffers, and
platform-specific build targets. This is the same split used by many language
runtimes: the language owns the program model; the host owns the machine
interface.

## PlankaHost Contract

`PlankaHost.exe` is the shared graphical launcher. It accepts a `.plk`
application file:

```bat
build\PlankaHost.exe graphics\src\plankagui.plk
build\PlankaHost.exe graphics\src\plankacube.plk
```

The loaded application should provide:

```text
app_kind
app_canvas
app_checksum
app_timer_step
```

`app_kind` selects the host family. Current values:

```text
1 = PlankaGUI-style 2D interface
3 = PlankaCube-style 3D scene
```

`app_canvas` returns the logical scene size. `app_checksum` gives a stable
summary value for verification. `app_timer_step` moves the application frame
state forward; static applications can simply return the input value.

## Kind-Specific Contracts

The GUI profile in `graphics/src/plankagui.plk` provides procedures such as:

```text
gui_canvas
gui_window_rect
gui_button_rect
gui_function_row_rect
gui_style_rgb
gui_scene_checksum
```

The cube profile in `graphics/src/plankacube.plk` provides procedures such as:

```text
cube_canvas
cube_style_rgb
cube_viewport_rect
cube_vertex
cube_edge
cube_model_transform
cube_project_vertex
cube_scene_checksum
```

These names are part of the boundary between `.plk` and the host. A new
application can be another `.plk` file as long as it follows one of the known
contracts. A new machine capability, such as sound output, filesystem access,
or a different windowing backend, still belongs in C because it is outside the
numeric/value model of the current language profile.

The engineering direction is to keep moving project-specific behavior into
`.plk` procedures while keeping C compact, generic, and reusable.
