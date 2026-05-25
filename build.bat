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

set PLANKAC_INC=-Ic\include -Ic\internal
set PLANKAC_LIB_SOURCES=c\core\plankac_common.c c\types\plankac_types.c c\notation\plankac_2d.c c\analyzer\plankac_analyzer.c c\core\plankac_source.c c\core\plankac_expr.c c\backends\plankac_bytecode.c c\backends\plankac_asm8086.c c\core\plankac_runtime.c
set PLANKAC_NATIVE_SOURCE=c\backends\plankac_native_runtime.c
set PLANKAC_CLI_SOURCES=%PLANKAC_LIB_SOURCES% c\tools\plankac_cli.c
set PLANKAGUI_CORE=graphics\c\plankagui_scene.c graphics\c\plankagui_raster.c graphics\c\plankagui_font.c graphics\c\plankagui_png.c graphics\c\plankagui_render.c
set PLANKAGUI_EXPORT_SOURCES=graphics\c\plankagui_main.c %PLANKAGUI_CORE%
set PLANKAGUI_WINDOW_SOURCES=graphics\c\plankagui_window.c %PLANKAGUI_CORE%

echo Building console runner...
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% c\legacy\plankamath.c c\targets\plankamath_cli.c -o build\plankamath_cli.exe -lm
if errorlevel 1 exit /b 1

echo Building PlankaC...
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% %PLANKAC_CLI_SOURCES% -o build\plankac.exe -lm
if errorlevel 1 exit /b 1

echo Building PlankaC static library...
if exist build\libplankac.a del build\libplankac.a
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\core\plankac_common.c -o build\plankac_common.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\types\plankac_types.c -o build\plankac_types.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\notation\plankac_2d.c -o build\plankac_2d.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\analyzer\plankac_analyzer.c -o build\plankac_analyzer.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\core\plankac_source.c -o build\plankac_source.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\core\plankac_expr.c -o build\plankac_expr.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\backends\plankac_bytecode.c -o build\plankac_bytecode.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\backends\plankac_asm8086.c -o build\plankac_asm8086.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\core\plankac_runtime.c -o build\plankac_runtime.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c %PLANKAC_NATIVE_SOURCE% -o build\plankac_native_runtime.o
if errorlevel 1 exit /b 1
ar rcs build\libplankac.a build\plankac_common.o build\plankac_types.o build\plankac_2d.o build\plankac_analyzer.o build\plankac_source.o build\plankac_expr.o build\plankac_bytecode.o build\plankac_asm8086.o build\plankac_runtime.o build\plankac_native_runtime.o
if errorlevel 1 exit /b 1

echo Building PlankaC API demo...
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% examples\c_api_demo.c build\libplankac.a -o build\plankac_api_demo.exe -lm
if errorlevel 1 exit /b 1

echo Building PlankaC conformance tests...
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% tests\plankac_conformance.c build\libplankac.a -o build\plankac_conformance.exe -lm
if errorlevel 1 exit /b 1

echo Building PlankaGUI export tool...
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -Igraphics\c %PLANKAGUI_EXPORT_SOURCES% build\libplankac.a -o build\plankagui_export.exe -lm
if errorlevel 1 exit /b 1

echo Building PlankaGUI window...
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -Igraphics\c %PLANKAGUI_WINDOW_SOURCES% build\libplankac.a -o build\PlankaGUI.exe -mwindows -lgdi32 -lm
if errorlevel 1 exit /b 1

echo Building Windows GUI...
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\legacy\plankamath.c -o build\plankamath.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\targets\windows_gui.c -o build\windows_gui.o
if errorlevel 1 exit /b 1
gcc -mwindows build\plankac_common.o build\plankac_types.o build\plankac_2d.o build\plankac_analyzer.o build\plankac_source.o build\plankac_expr.o build\plankac_bytecode.o build\plankac_asm8086.o build\plankac_runtime.o build\plankamath.o build\windows_gui.o -o build\PlankaMath.exe -lm
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

build\plankac.exe asm8086 build\plankac_8086.asm
if errorlevel 1 exit /b 1

gcc -Wall -Wextra -std=c89 %PLANKAC_INC% build\plankac_generated.c build\libplankac.a -o build\plankac_generated.exe -lm
if errorlevel 1 exit /b 1

build\plankac_generated.exe set_session
if errorlevel 1 exit /b 1

build\plankac_generated.exe record_child_heap_session
if errorlevel 1 exit /b 1

build\plankac_generated.exe relation_compose_session
if errorlevel 1 exit /b 1

build\plankac_generated.exe chess_piece_struct_session
if errorlevel 1 exit /b 1

build\plankac_generated.exe two_dimensional_original_session
if errorlevel 1 exit /b 1

build\plankac_generated.exe three_d_pipeline_session
if errorlevel 1 exit /b 1

build\plankac_generated.exe vec3_child_heap_session
if errorlevel 1 exit /b 1

gcc -Wall -Wextra %PLANKAC_INC% build\plankac_asm_runtime.S build\libplankac.a -o build\plankac_asm_runtime.exe -lm
if errorlevel 1 exit /b 1

build\plankac_asm_runtime.exe set_session
if errorlevel 1 exit /b 1

build\plankac_asm_runtime.exe divide_checked 84 0
if errorlevel 1 exit /b 1

build\plankac_asm_runtime.exe loop_multiply 3 4
if errorlevel 1 exit /b 1

build\plankac_asm_runtime.exe pair_relation_session
if errorlevel 1 exit /b 1

build\plankac_asm_runtime.exe relation_child_heap_session
if errorlevel 1 exit /b 1

build\plankac_asm_runtime.exe record_child_heap_session
if errorlevel 1 exit /b 1

build\plankac_asm_runtime.exe relation_compose_session
if errorlevel 1 exit /b 1

build\plankac_asm_runtime.exe chess_piece_struct_session
if errorlevel 1 exit /b 1

build\plankac_asm_runtime.exe two_dimensional_original_session
if errorlevel 1 exit /b 1

build\plankac_asm_runtime.exe three_d_pipeline_session
if errorlevel 1 exit /b 1

build\plankac_asm_runtime.exe vec3_child_heap_session
if errorlevel 1 exit /b 1

build\plankac_asm_runtime.exe all_tests
if errorlevel 1 exit /b 1

build\plankac_api_demo.exe
if errorlevel 1 exit /b 1

build\plankac_conformance.exe
if errorlevel 1 exit /b 1

build\plankagui_export.exe graphics\examples\plankagui.png
if errorlevel 1 exit /b 1

echo Done.
