@echo off
setlocal

cd /d "%~dp0"

if "%WATCOM%"=="" (
    if exist "%~dp0tools\ow19\binnt\wcl.exe" set "WATCOM=%~dp0tools\ow19"
)

if not "%WATCOM%"=="" (
    if exist "%WATCOM%\binnt64" set "PATH=%WATCOM%\binnt64;%PATH%"
    set "PATH=%WATCOM%\binnt;%WATCOM%\binw;%PATH%"
    set "INCLUDE=%WATCOM%\h;%WATCOM%\h\win;%INCLUDE%"
    set "LIB=%WATCOM%\lib286;%WATCOM%\lib286\win;%LIB%"
)

where wcl >nul 2>nul
if errorlevel 1 (
    echo Open Watcom wcl was not found.
    echo Install Open Watcom 1.9 or Open Watcom V2 and run this script from its environment.
    echo This target is the real Win16 Windows 3.x build, not the modern MinGW build.
    exit /b 1
)

if not exist build mkdir build
if not exist build\win16 mkdir build\win16

echo Building real Win16 PlankaMath shell...
pushd build\win16
wcl -q -bt=windows -l=windows -ml -I=..\..\c\include -fe=PlankaMath16.exe ..\..\c\targets\win16_gui.c ..\..\c\legacy\plankamath.c
if errorlevel 1 (
    popd
    exit /b 1
)
popd

echo Built build\win16\PlankaMath16.exe
echo This executable is intended for Windows 3.x / Win16 environments.
