# PlankaMath GUI Host

Source file:

```text
c/windows_gui.c
c/win16_gui.c
```

PlankaMath has two Windows GUI hosts.

`c/windows_gui.c` is the modern Win32/Win64 host. It loads the procedure table
through PlankaC and runs selected procedures from the `.plk` source plans.
Calculator keys call the same `.plk` procedures used by the console
interpreter, with the older C runtime kept as a fallback.

`c/win16_gui.c` is the real Win16 host for Windows 3.x. Build it with Open
Watcom by running:

```text
build-win16.bat
```

The output is `build\win16\PlankaMath16.exe`. This is not a modern executable
with an old look; it is a Win16 target. The Win16 host uses the compact
PlankaMath C mapping of the calculator procedures because the full PlankaC
parser tables and backend modules are too large for a practical Windows 3.x
program.

On Windows 10/11 x64 this executable cannot be launched directly. Use a real
Windows 3.x environment, a virtual machine, DOSBox-X with Windows 3.x, or
`otvdm/winevdm`. The helper script:

```text
run-win16-otvdm.bat
```

starts `build\win16\PlankaMath16.exe` through `otvdm` when `otvdm` is available
in `tools\otvdm`, `C:\OTVDM`, or `PATH`.
