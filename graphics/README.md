# PlankaGUI

PlankaGUI is a small graphics profile written in linear Plankalkuel notation.
The `.plk` source describes canvas size, window rectangles, button geometry,
procedure-list rows, argument fields, log area, and color palette entries.
New graphical surfaces can be added by writing more `.plk` procedures with
the same rectangle, style, and scene-contract shape.

PlankaHost is the common application host. It loads the standard PlankaC
profile from `src/` and then a selected application file from `graphics/src/`.
That means calculator procedures, structured values, relations, chess
procedures, 3D helpers, and the selected GUI or cube profile are visible in one
procedure table.

PlankaCube extends that graphics profile with a 3D scene contract. Its `.plk`
source defines cube vertices, edge topology, model transforms, and perspective
projection. The C host supplies the timer and raster output; projected points
are evaluated through PlankaC for every rendered frame.

The C side is split into narrow modules. It loads
`graphics/src/plankagui.plk` through the PlankaC API, evaluates layout
procedures, rasterizes the scene, and writes an image file. Text labels are
mapped by the host because the current PlankaC value model is numeric and
handle-based, not string-based. PNG is only the current export backend for the
reference screenshot.

## Modules

| File | Role |
| --- | --- |
| `src/plankagui.plk` | Plankalkuel procedures for canvas, windows, buttons, rows, and palette |
| `src/plankacube.plk` | Plankalkuel procedures for cube geometry, transforms, projection, and palette |
| `c/plankahost.h` | shared host API for `.plk` applications |
| `c/plankahost.c` | common loader, procedure metadata, run, render, and timer bridge |
| `c/plankahost_window.c` | Windows host using the common PlankaHost API |
| `c/plankahost_demo.c` | console proof that the same host runs base and application procedures |
| `c/plankagui_scene.c` | PlankaC loader and procedure-evaluation bridge |
| `c/plankacube_scene.c` | PlankaC loader and procedure bridge for the cube scene |
| `c/plankagui_raster.c` | pixel image, filled rectangles, borders, panels |
| `c/plankagui_font.c` | compact bitmap text renderer |
| `c/plankagui_png.c` | dependency-free image export backend |
| `c/plankagui_render.c` | scene composition from evaluated `.plk` procedures |
| `c/plankacube_render.c` | cube frame composition from evaluated `.plk` procedures |
| `c/plankagui_main.c` | command-line host |
| `c/plankacube_main.c` | command-line host for the cube PNG export |
| `c/plankagui_window.c` | interactive calculator window |
| `c/plankacube_window.c` | animated cube window |

Build from the repository root:

```bat
build.bat
```

Run the shared host:

```bat
build\PlankaHost.exe graphics\src\plankagui.plk
build\PlankaHost.exe graphics\src\plankacube.plk
```

`PlankaHost.exe` is the main application entry point. `PlankaGUI.exe` and
`PlankaCube.exe` remain direct launchers for their specific profiles.

Check the host API without opening a window:

```bat
build\plankahost_demo.exe graphics\src\plankagui.plk
build\plankahost_demo.exe graphics\src\plankacube.plk
```

Run only the GUI renderer:

```bat
build\PlankaGUI.exe
```

The window is interactive. Keypad hit-testing uses the button rectangles
returned by `gui_button_rect`, and calculator operations call `.plk`
procedures through PlankaC. Resize keeps the scene aspect ratio and maps mouse
coordinates back into the original Plankalkuel-defined scene.

Export the reference image:

```bat
build\plankagui_export.exe graphics\examples\plankagui.png
```

Reference output:

![PlankaGUI reference render](examples/plankagui.png)

Run the 3D window:

```bat
build\PlankaCube.exe
```

Run an explicit `.plk` cube profile with the same host:

```bat
build\PlankaCube.exe graphics\src\plankacube.plk
```

Export the cube reference frame:

```bat
build\plankacube_export.exe graphics\examples\plankacube.png 0.85 graphics\src\plankacube.plk
```

PlankaCube reference output:

![PlankaCube reference render](examples/plankacube.png)
