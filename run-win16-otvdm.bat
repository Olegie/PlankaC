@echo off
setlocal

cd /d "%~dp0"

set "APP=build\win16\PlankaMath16.exe"
if not exist "%APP%" (
    echo %APP% was not found.
    echo Run build-win16.bat first.
    exit /b 1
)

set "OTVDM="
if exist "tools\otvdm\otvdmw.exe" set "OTVDM=tools\otvdm\otvdmw.exe"
if "%OTVDM%"=="" if exist "tools\otvdm\otvdm.exe" set "OTVDM=tools\otvdm\otvdm.exe"
if "%OTVDM%"=="" for /d %%D in ("tools\otvdm\*") do (
    if exist "%%~fD\otvdmw.exe" set "OTVDM=%%~fD\otvdmw.exe"
    if "%OTVDM%"=="" if exist "%%~fD\otvdm.exe" set "OTVDM=%%~fD\otvdm.exe"
)
if "%OTVDM%"=="" if exist "C:\OTVDM\otvdmw.exe" set "OTVDM=C:\OTVDM\otvdmw.exe"
if "%OTVDM%"=="" if exist "C:\OTVDM\otvdm.exe" set "OTVDM=C:\OTVDM\otvdm.exe"

if "%OTVDM%"=="" (
    for %%I in (otvdmw.exe otvdm.exe) do (
        if "%OTVDM%"=="" for %%P in (%%~$PATH:I) do set "OTVDM=%%P"
    )
)

if "%OTVDM%"=="" (
    echo otvdm/winevdm was not found.
    echo Download winevdm from https://github.com/otya128/winevdm/releases
    echo Extract it to tools\otvdm or install it to C:\OTVDM.
    echo Then run this script again.
    exit /b 1
)

echo Running %APP% through %OTVDM%
"%OTVDM%" "%CD%\%APP%"
