# PlankaMath GUI Host

Source file:

```text
c/targets/windows_gui.c
c/targets/win16_gui.c
```

PlankaMath has two Windows GUI hosts.

`c/targets/windows_gui.c` is the modern Win32/Win64 host. It loads the procedure table
through PlankaC and runs selected procedures from the `.plk` source plans.
Calculator keys call the same `.plk` procedures used by the console
interpreter, with the legacy C runtime kept as a fallback.

The shared application host is:

```text
graphics/c/plankahost_window.c
graphics/c/plankahost.c
```

`PlankaHost.exe` loads the standard PlankaC profile plus one `.plk`
application file. It can start the calculator-style PlankaGUI profile or the
3D cube profile without compiling a new C host for each profile:

```text
build\PlankaHost.exe graphics\src\plankagui.plk
build\PlankaHost.exe graphics\src\plankacube.plk
```

`c/targets/win16_gui.c` is the real Win16 host for Windows 3.x. Build it with Open
Watcom by running:

```text
build-win16.bat
```

The output is `build\win16\PlankaMath16.exe`, a 16-bit Windows NE executable.
The target is meant for Windows 3.x and compatible Win16 environments. The
Win16 host uses the compact PlankaMath C runtime; the full parser, bytecode
runner, C backend, and ASM backend stay on the modern PlankaC toolchain path.

64-bit Windows does not include the Win16 subsystem. Use a real Windows 3.x
environment, a virtual machine, DOSBox-X with Windows 3.x, or `otvdm/winevdm`.
The helper script:

```text
run-win16-otvdm.bat
```

starts `build\win16\PlankaMath16.exe` through `otvdm` when the wrapper is
available under `tools\otvdm`, `C:\OTVDM`, or `PATH`.

The DOS target is not a GUI host. It is a separate text-mode runner:

```text
build-dos.bat
run-dos-dosbox.bat demo
```

The output executable is `build\dos\PMDOS.EXE`, a real 16-bit DOS MZ program.
