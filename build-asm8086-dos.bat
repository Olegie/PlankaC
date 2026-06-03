@echo off
setlocal

cd /d "%~dp0"

if not exist build mkdir build
if not exist build\dos mkdir build\dos

if not exist build\plankac.exe (
    echo build\plankac.exe was not found.
    echo Run build.bat first so the PlankaC compiler is available.
    exit /b 1
)

set "ASM=build\dos\PLANKAC86.ASM"
set "OBJ=PLANKAC86.OBJ"
set "EXE=PLANKAC86.EXE"

echo Generating 8086/DOS assembly from PlankaC...
build\plankac.exe asm8086 "%ASM%"
if errorlevel 1 exit /b 1

set "ASMTOOL="
for %%I in (tasm.exe masm.exe wasm.exe) do (
    if "%ASMTOOL%"=="" for %%P in (%%~$PATH:I) do set "ASMTOOL=%%P"
)

set "LINKTOOL="
for %%I in (tlink.exe link.exe wlink.exe) do (
    if "%LINKTOOL%"=="" for %%P in (%%~$PATH:I) do set "LINKTOOL=%%P"
)

if "%ASMTOOL%"=="" (
    echo MASM, TASM, or Open Watcom WASM was not found.
    echo Generated source is ready at %ASM%.
    echo Install a DOS-capable assembler/linker toolchain or assemble it inside DOSBox.
    exit /b 1
)

if "%LINKTOOL%"=="" (
    echo LINK, TLINK, or Open Watcom WLINK was not found.
    echo Generated source is ready at %ASM%.
    exit /b 1
)
for %%A in ("%ASMTOOL%") do set "ASMNAME=%%~nxA"
for %%L in ("%LINKTOOL%") do set "LINKNAME=%%~nxL"

pushd build\dos
echo Assembling %ASMTOOL%...
if /i "%ASMNAME%"=="wasm.exe" (
    "%ASMTOOL%" -q PLANKAC86.ASM -fo=PLANKAC86.OBJ
) else (
    "%ASMTOOL%" PLANKAC86.ASM
)
if errorlevel 1 (
    popd
    exit /b 1
)

echo Linking %LINKTOOL%...
if /i "%LINKNAME%"=="tlink.exe" (
    "%LINKTOOL%" %OBJ%,%EXE%
) else if /i "%LINKNAME%"=="wlink.exe" (
    "%LINKTOOL%" system dos file %OBJ% name %EXE%
) else (
    "%LINKTOOL%" %OBJ%;
)
if errorlevel 1 (
    popd
    exit /b 1
)
popd

echo Built build\dos\%EXE%
echo This is the generated PlankaC 8086/DOS runner profile.
