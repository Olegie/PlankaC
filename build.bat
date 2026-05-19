@echo off
setlocal

cd /d "%~dp0"

where gcc >nul 2>nul
if errorlevel 1 (
    echo gcc was not found in PATH.
    echo Install GCC/MinGW or add the compiler bin directory to PATH.
    exit /b 1
)

where ar >nul 2>nul
if errorlevel 1 (
    echo ar was not found in PATH.
    echo Install GCC/MinGW or add the compiler bin directory to PATH.
    exit /b 1
)

if not exist build mkdir build

set PLANKAC_LIB_SOURCES=c\plankac_common.c c\plankac_source.c c\plankac_expr.c c\plankac_bytecode.c c\plankac_runtime.c
set PLANKAC_CLI_SOURCES=%PLANKAC_LIB_SOURCES% c\plankac_cli.c

echo Building console runner...
gcc -Wall -Wextra -std=c89 c\plankamath.c c\plankamath_cli.c -o build\plankamath_cli.exe -lm
if errorlevel 1 exit /b 1

echo Building PlankaC...
gcc -Wall -Wextra -std=c89 %PLANKAC_CLI_SOURCES% -o build\plankac.exe -lm
if errorlevel 1 exit /b 1

echo Building PlankaC static library...
if exist build\libplankac.a del build\libplankac.a
gcc -Wall -Wextra -std=c89 -c c\plankac_common.c -o build\plankac_common.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 -c c\plankac_source.c -o build\plankac_source.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 -c c\plankac_expr.c -o build\plankac_expr.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 -c c\plankac_bytecode.c -o build\plankac_bytecode.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 -c c\plankac_runtime.c -o build\plankac_runtime.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 -c c\plankac_native_runtime.c -o build\plankac_native_runtime.o
if errorlevel 1 exit /b 1
ar rcs build\libplankac.a build\plankac_common.o build\plankac_source.o build\plankac_expr.o build\plankac_bytecode.o build\plankac_runtime.o build\plankac_native_runtime.o
if errorlevel 1 exit /b 1

echo Building PlankaC API demo...
gcc -Wall -Wextra -std=c89 examples\c_api_demo.c build\libplankac.a -o build\plankac_api_demo.exe -lm
if errorlevel 1 exit /b 1

echo Building PlankaC conformance tests...
gcc -Wall -Wextra -std=c89 tests\plankac_conformance.c build\libplankac.a -o build\plankac_conformance.exe -lm
if errorlevel 1 exit /b 1

echo Building Windows GUI...
gcc -Wall -Wextra -std=c89 -c c\plankamath.c -o build\plankamath.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 -c c\windows_gui.c -o build\windows_gui.o
if errorlevel 1 exit /b 1
gcc -mwindows build\plankac_common.o build\plankac_source.o build\plankac_expr.o build\plankac_bytecode.o build\plankac_runtime.o build\plankamath.o build\windows_gui.o -o build\PlankaMath.exe -lm
if errorlevel 1 exit /b 1

echo Running checks...
build\plankamath_cli.exe compile
if errorlevel 1 exit /b 1

build\plankamath_cli.exe tests
if errorlevel 1 exit /b 1

build\plankac.exe check
if errorlevel 1 exit /b 1

build\plankac.exe tests
if errorlevel 1 exit /b 1

build\plankac.exe bytecode build\plankamath.pbc
if errorlevel 1 exit /b 1

build\plankac.exe checkbc build\plankamath.pbc
if errorlevel 1 exit /b 1

build\plankac.exe runbc build\plankamath.pbc set_session
if errorlevel 1 exit /b 1

build\plankac.exe cgen build\plankac_generated.c
if errorlevel 1 exit /b 1

build\plankac.exe asmgen build\plankac_asm_runtime.S
if errorlevel 1 exit /b 1

gcc -Wall -Wextra -std=c89 -Ic build\plankac_generated.c build\libplankac.a -o build\plankac_generated.exe -lm
if errorlevel 1 exit /b 1

build\plankac_generated.exe set_session
if errorlevel 1 exit /b 1

gcc -Wall -Wextra -Ic build\plankac_asm_runtime.S build\libplankac.a -o build\plankac_asm_runtime.exe -lm
if errorlevel 1 exit /b 1

build\plankac_asm_runtime.exe set_session
if errorlevel 1 exit /b 1

build\plankac_asm_runtime.exe divide_checked 84 0
if errorlevel 1 exit /b 1

build\plankac_asm_runtime.exe loop_multiply 3 4
if errorlevel 1 exit /b 1

build\plankac_asm_runtime.exe pair_relation_session
if errorlevel 1 exit /b 1

build\plankac_asm_runtime.exe all_tests
if errorlevel 1 exit /b 1

build\plankac_api_demo.exe
if errorlevel 1 exit /b 1

build\plankac_conformance.exe
if errorlevel 1 exit /b 1

echo Done.
