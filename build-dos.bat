@echo off
setlocal

cd /d "%~dp0"

if "%WATCOM%"=="" (
    if exist "%~dp0tools\ow19\binnt\wcl.exe" set "WATCOM=%~dp0tools\ow19"
)

if not "%WATCOM%"=="" (
    if exist "%WATCOM%\binnt64" set "PATH=%WATCOM%\binnt64;%PATH%"
    set "PATH=%WATCOM%\binnt;%WATCOM%\binw;%PATH%"
    set "INCLUDE=%WATCOM%\h;%INCLUDE%"
    set "LIB=%WATCOM%\lib286;%LIB%"
)

where wcl >nul 2>nul
if errorlevel 1 (
    echo Open Watcom wcl was not found.
    echo Install Open Watcom 1.9 or Open Watcom V2 and run this script from its environment.
    echo This target is the real 16-bit DOS build, not the modern Windows console build.
    exit /b 1
)

if not exist build mkdir build
if not exist build\dos mkdir build\dos

echo Building real 16-bit DOS PlankaMath runner...
pushd build\dos
wcl -q -bt=dos -ml -I=..\..\c -fe=PMDOS.EXE ..\..\c\dos_cli.c ..\..\c\plankamath.c
if errorlevel 1 (
    popd
    exit /b 1
)
popd

echo Built build\dos\PMDOS.EXE
echo This executable is intended for MS-DOS, FreeDOS, DOSBox, or Windows 9x DOS mode.
