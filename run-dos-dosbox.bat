@echo off
setlocal

cd /d "%~dp0"

set "APP=build\dos\PMDOS.EXE"
if not exist "%APP%" (
    echo %APP% was not found.
    echo Run build-dos.bat first.
    exit /b 1
)

set "DOSBOX="
for %%I in (dosbox-x.exe dosbox.exe) do (
    if "%DOSBOX%"=="" for %%P in (%%~$PATH:I) do set "DOSBOX=%%P"
)

if "%DOSBOX%"=="" if exist "C:\Program Files\DOSBox-X\dosbox-x.exe" set "DOSBOX=C:\Program Files\DOSBox-X\dosbox-x.exe"
if "%DOSBOX%"=="" if exist "C:\Program Files (x86)\DOSBox-0.74-3\DOSBox.exe" set "DOSBOX=C:\Program Files (x86)\DOSBox-0.74-3\DOSBox.exe"

if "%DOSBOX%"=="" (
    echo DOSBox or DOSBox-X was not found.
    echo Install DOSBox, DOSBox Staging, or DOSBox-X, then run this script again.
    exit /b 1
)

set "DOSCMD=PMDOS %*"
if "%*"=="" set "DOSCMD=PMDOS"

echo Running %DOSCMD% through %DOSBOX%
"%DOSBOX%" -c "mount c %CD%\build\dos" -c "c:" -c "%DOSCMD%"
