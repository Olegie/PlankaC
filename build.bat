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
set PLANKAC_LIB_SOURCES=c\core\plankac_common.c c\types\plankac_types.c c\notation\plankac_2d.c c\notation\plankac_document.c c\notation\plankac_page.c c\analyzer\plankac_analyzer.c c\analyzer\plankac_schema.c c\values\plankac_bits.c c\values\plankac_value.c c\models\plankac_chess_model.c c\ir\plankac_ast.c c\ir\plankac_ir.c c\ir\plankac_evidence.c c\backends\plankac_lowering.c c\core\plankac_source.c c\core\plankac_expr.c c\backends\plankac_bytecode.c c\backends\plankac_asm8086.c c\core\plankac_runtime.c
set PLANKAC_NATIVE_SOURCE=c\backends\plankac_native_runtime.c
set PLANKAC_CLI_SOURCES=%PLANKAC_LIB_SOURCES% c\tools\plankac_cli.c
set PLANKAGUI_CORE=graphics\c\plankagui_scene.c graphics\c\plankagui_raster.c graphics\c\plankagui_font.c graphics\c\plankagui_png.c graphics\c\plankagui_render.c
set PLANKAGUI_EXPORT_SOURCES=graphics\c\plankagui_main.c %PLANKAGUI_CORE%
set PLANKAGUI_WINDOW_SOURCES=graphics\c\plankagui_window.c %PLANKAGUI_CORE%
set PLANKACUBE_CORE=graphics\c\plankacube_scene.c graphics\c\plankacube_render.c graphics\c\plankagui_raster.c graphics\c\plankagui_font.c
set PLANKACUBE_EXPORT_SOURCES=graphics\c\plankacube_main.c %PLANKACUBE_CORE% graphics\c\plankagui_png.c
set PLANKACUBE_WINDOW_SOURCES=graphics\c\plankacube_window.c %PLANKACUBE_CORE%
set PLANKAHOST_CORE=graphics\c\plankahost.c graphics\c\plankagui_scene.c graphics\c\plankagui_raster.c graphics\c\plankagui_font.c graphics\c\plankagui_render.c graphics\c\plankacube_scene.c graphics\c\plankacube_render.c
set PLANKAHOST_WINDOW_SOURCES=graphics\c\plankahost_window.c %PLANKAHOST_CORE%
set PLANKAHOST_DEMO_SOURCES=graphics\c\plankahost_demo.c %PLANKAHOST_CORE%

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
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\notation\plankac_document.c -o build\plankac_document.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\notation\plankac_page.c -o build\plankac_page.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\analyzer\plankac_analyzer.c -o build\plankac_analyzer.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\analyzer\plankac_schema.c -o build\plankac_schema.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\values\plankac_bits.c -o build\plankac_bits.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\values\plankac_value.c -o build\plankac_value.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\models\plankac_chess_model.c -o build\plankac_chess_model.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\ir\plankac_ast.c -o build\plankac_ast.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\ir\plankac_ir.c -o build\plankac_ir.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\ir\plankac_evidence.c -o build\plankac_evidence.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\backends\plankac_lowering.c -o build\plankac_lowering.o
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
ar rcs build\libplankac.a build\plankac_common.o build\plankac_types.o build\plankac_2d.o build\plankac_document.o build\plankac_page.o build\plankac_analyzer.o build\plankac_schema.o build\plankac_bits.o build\plankac_value.o build\plankac_chess_model.o build\plankac_ast.o build\plankac_ir.o build\plankac_evidence.o build\plankac_lowering.o build\plankac_source.o build\plankac_expr.o build\plankac_bytecode.o build\plankac_asm8086.o build\plankac_runtime.o build\plankac_native_runtime.o
if errorlevel 1 exit /b 1

echo Building PlankaC API demo...
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% examples\c_api_demo.c build\libplankac.a -o build\plankac_api_demo.exe -lm
if errorlevel 1 exit /b 1

echo Building PlankaC ABI demo...
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% examples\c_abi_demo.c build\libplankac.a -o build\plankac_abi_demo.exe -lm
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

echo Building PlankaCube export tool...
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -Igraphics\c %PLANKACUBE_EXPORT_SOURCES% build\libplankac.a -o build\plankacube_export.exe -lm
if errorlevel 1 exit /b 1

echo Building PlankaCube window...
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -Igraphics\c %PLANKACUBE_WINDOW_SOURCES% build\libplankac.a -o build\PlankaCube.exe -mwindows -lgdi32 -lm
if errorlevel 1 exit /b 1

echo Building PlankaHost window...
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -Igraphics\c %PLANKAHOST_WINDOW_SOURCES% build\libplankac.a -o build\PlankaHost.exe -mwindows -lgdi32 -lm
if errorlevel 1 exit /b 1

echo Building PlankaHost API demo...
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -Igraphics\c %PLANKAHOST_DEMO_SOURCES% build\libplankac.a -o build\plankahost_demo.exe -lm
if errorlevel 1 exit /b 1

echo Building Windows GUI...
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\legacy\plankamath.c -o build\plankamath.o
if errorlevel 1 exit /b 1
gcc -Wall -Wextra -std=c89 %PLANKAC_INC% -c c\targets\windows_gui.c -o build\windows_gui.o
if errorlevel 1 exit /b 1
gcc -mwindows build\plankac_common.o build\plankac_types.o build\plankac_2d.o build\plankac_document.o build\plankac_page.o build\plankac_analyzer.o build\plankac_schema.o build\plankac_bits.o build\plankac_value.o build\plankac_chess_model.o build\plankac_ast.o build\plankac_ir.o build\plankac_evidence.o build\plankac_lowering.o build\plankac_source.o build\plankac_expr.o build\plankac_bytecode.o build\plankac_asm8086.o build\plankac_runtime.o build\plankamath.o build\windows_gui.o -o build\PlankaMath.exe -lm
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

build\plankac.exe ast build\plankac.ast
if errorlevel 1 exit /b 1

build\plankac.exe ir build\plankac.ir
if errorlevel 1 exit /b 1

build\plankac.exe evidence build\plankac.evidence.json
if errorlevel 1 exit /b 1

build\plankac.exe lowering build\plankac.lowering
if errorlevel 1 exit /b 1

build\plankac.exe checkbc build\plankamath.pbc
if errorlevel 1 exit /b 1

build\plankac.exe runbc build\plankamath.pbc set_session
if errorlevel 1 exit /b 1

build\plankac.exe runfile graphics\src\plankacube.plk cube_scene_checksum
if errorlevel 1 exit /b 1

build\plankac.exe runfile graphics\src\plankagui.plk app_kind
if errorlevel 1 exit /b 1

build\plankac.exe runfile graphics\src\plankacube.plk app_kind
if errorlevel 1 exit /b 1

build\plankac.exe checkfile examples\max3.plk
if errorlevel 1 exit /b 1

build\plankac.exe runfile examples\max3.plk max3_demo
if errorlevel 1 exit /b 1

build\plankac.exe astfile examples\max3.plk build\max3.ast
if errorlevel 1 exit /b 1

build\plankac.exe irfile examples\max3.plk build\max3.ir
if errorlevel 1 exit /b 1

build\plankac.exe evidencefile examples\max3.plk build\max3.evidence.json
if errorlevel 1 exit /b 1

findstr /c:"plankac-evidence-v1" build\plankac.evidence.json >nul
if errorlevel 1 exit /b 1

findstr /c:"fingerprint" build\max3.evidence.json >nul
if errorlevel 1 exit /b 1

findstr /c:"PLANKAC-AST 1" build\plankac.ast >nul
if errorlevel 1 exit /b 1

findstr /c:"expression_nodes" build\plankac.ast >nul
if errorlevel 1 exit /b 1

findstr /c:"op=CALL" build\max3.ast >nul
if errorlevel 1 exit /b 1

findstr /c:"expr_nodes" build\max3.ir >nul
if errorlevel 1 exit /b 1

build\plankahost_demo.exe graphics\src\plankagui.plk
if errorlevel 1 exit /b 1

build\plankahost_demo.exe graphics\src\plankacube.plk
if errorlevel 1 exit /b 1

build\plankac.exe cgen build\plankac_generated.c
if errorlevel 1 exit /b 1

build\plankac.exe asmgen build\plankac_asm_runtime.S
if errorlevel 1 exit /b 1

build\plankac.exe asm8086 build\plankac_8086.asm
if errorlevel 1 exit /b 1

findstr /c:"plankac_8086_smoke PROC NEAR" build\plankac_8086.asm >nul
if errorlevel 1 exit /b 1

build\plankac.exe compile build\plankac_pipeline
if errorlevel 1 exit /b 1

build\plankac.exe native-c build\plankac_native_c
if errorlevel 1 exit /b 1

build\plankac_native_c.exe set_session
if errorlevel 1 exit /b 1

build\plankac.exe native-asm build\plankac_native_asm
if errorlevel 1 exit /b 1

build\plankac_native_asm.exe add 12 8
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

build\plankac_abi_demo.exe
if errorlevel 1 exit /b 1

build\plankac_conformance.exe
if errorlevel 1 exit /b 1

build\plankagui_export.exe graphics\examples\plankagui.png
if errorlevel 1 exit /b 1

build\plankacube_export.exe graphics\examples\plankacube.png 0.85 graphics\src\plankacube.plk
if errorlevel 1 exit /b 1

echo Done.
