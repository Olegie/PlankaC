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
    echo This target builds the full PlankaC DOS runner PLANKACD.EXE.
    exit /b 1
)

if not exist build mkdir build
if not exist build\dos mkdir build\dos

echo Building full PlankaC DOS runner...
pushd build\dos

set RSP=plankacd.rsp
> %RSP% echo -q
>> %RSP% echo -bt=dos
>> %RSP% echo -mh
>> %RSP% echo -i=..\..\c\include
>> %RSP% echo -i=..\..\c\internal
>> %RSP% echo -fe=PLANKACD.EXE
>> %RSP% echo ..\..\c\targets\dos\plankac_dos_runner.c
>> %RSP% echo ..\..\c\core\plankac_common.c
>> %RSP% echo ..\..\c\types\plankac_types.c
>> %RSP% echo ..\..\c\notation\plankac_2d.c
>> %RSP% echo ..\..\c\notation\plankac_document.c
>> %RSP% echo ..\..\c\notation\plankac_page.c
>> %RSP% echo ..\..\c\analyzer\plankac_analyzer.c
>> %RSP% echo ..\..\c\analyzer\plankac_schema.c
>> %RSP% echo ..\..\c\values\plankac_bits.c
>> %RSP% echo ..\..\c\values\plankac_value.c
>> %RSP% echo ..\..\c\models\plankac_chess_model.c
>> %RSP% echo ..\..\c\ir\plankac_ast.c
>> %RSP% echo ..\..\c\ir\plankac_ir.c
>> %RSP% echo ..\..\c\ir\plankac_evidence.c
>> %RSP% echo ..\..\c\backends\plankac_lowering.c
>> %RSP% echo ..\..\c\core\plankac_source.c
>> %RSP% echo ..\..\c\core\plankac_expr.c
>> %RSP% echo ..\..\c\backends\plankac_bytecode.c
>> %RSP% echo ..\..\c\backends\plankac_asm8086.c
>> %RSP% echo ..\..\c\backends\dos\plankac_doscom.c
>> %RSP% echo ..\..\c\core\plankac_runtime.c

wcl @%RSP%
if errorlevel 1 (
    popd
    exit /b 1
)
popd

echo Built build\dos\PLANKACD.EXE
echo Run it from the repository root in MS-DOS, FreeDOS, DOSBox, or Windows 9x DOS mode.
