# PlankaGUI

PlankaGUI is a small graphics profile written in linear Plankalkuel notation.
The `.plk` source describes canvas size, window rectangles, button geometry,
procedure-list rows, argument fields, log area, and color palette entries.
New graphical surfaces can be added by writing more `.plk` procedures with
the same rectangle, style, and scene-contract shape.

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
| `c/plankagui_scene.c` | PlankaC loader and procedure-evaluation bridge |
| `c/plankagui_raster.c` | pixel image, filled rectangles, borders, panels |
| `c/plankagui_font.c` | compact bitmap text renderer |
| `c/plankagui_png.c` | dependency-free image export backend |
| `c/plankagui_render.c` | scene composition from evaluated `.plk` procedures |
| `c/plankagui_main.c` | command-line host |

Build from the repository root:

```bat
build.bat
```

Run only the renderer:

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
