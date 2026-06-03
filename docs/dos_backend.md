# DOS Backend

PlankaC now has three DOS-oriented outputs. They are intentionally separate,
because DOS `.COM` and DOS `.EXE` programs have different memory limits.

## Full PlankaC DOS Runner

The full DOS route is `PLANKACD.EXE`:

```bat
build-dos-plankac.bat
```

That script builds `build\dos\PLANKACD.EXE` with Open Watcom in huge memory
model. The target is a real 16-bit DOS MZ executable and uses the PlankaC API,
not the old compact calculator mirror.

Supported commands:

```text
PLANKACD check
PLANKACD list
PLANKACD run add 12 8
PLANKACD tests
PLANKACD bytecode PLANKAC.PBC
PLANKACD checkbc PLANKAC.PBC
PLANKACD runbc PLANKAC.PBC set_session
```

The normal Windows build also compiles `build\plankacd_host.exe` from the same
source file (`c/targets/dos/plankac_dos_runner.c`) and runs those commands as a
host-side proof before the 16-bit toolchain is involved.

## 8086 Assembly Source

Readable 8086 assembly is emitted with:

```bat
build\plankac.exe asm8086 build\plankac_8086.asm
```

That file is intended for MASM, TASM, or Open Watcom. It carries the bytecode
image, direct arithmetic procedures, a DOS start entry, and the 16-bit compound
dispatch ABI.

`build-asm8086-dos.bat` writes `build\dos\PLANKAC86.ASM` and assembles it when
MASM/TASM with LINK/TLINK or Open Watcom `wasm` with `wlink` are available.

## Assembler-Free COM Bootstrap

The `.COM` path is still useful, but it is not the full runner:

```bat
build\plankac.exe doscom build\plankac_dos.com
```

`c/backends/dos/plankac_doscom.c` writes the first 8086 machine bytes directly,
prints a DOS profile banner through `int 21h`, and embeds the generated
PlankaC bytecode image after the bootstrap message. No MASM, TASM, Open
Watcom, or NASM is needed for this artifact.

Expected banner:

```text
PlankaC DOS COM bootstrap
profile: PLANKAC-DOSCOM-8086
procedures: <loaded procedure count>
bytecode: embedded <byte count> bytes
runner: PLANKACD.EXE
```

The full execution path is `PLANKACD.EXE`. The `.COM` file is a direct binary
bootstrap and bytecode container inside the 64K `.COM` model.
