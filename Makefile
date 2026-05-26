CC ?= gcc
AR ?= ar
CFLAGS ?= -Wall -Wextra -std=c89
CPPFLAGS += -Ic/include -Ic/internal
LDLIBS ?= -lm

BUILD := build

PLANKAC_LIB_SOURCES := \
	c/core/plankac_common.c \
	c/types/plankac_types.c \
	c/notation/plankac_2d.c \
	c/notation/plankac_document.c \
	c/notation/plankac_page.c \
	c/analyzer/plankac_analyzer.c \
	c/analyzer/plankac_schema.c \
	c/values/plankac_bits.c \
	c/values/plankac_value.c \
	c/models/plankac_chess_model.c \
	c/ir/plankac_ir.c \
	c/backends/plankac_lowering.c \
	c/core/plankac_source.c \
	c/core/plankac_expr.c \
	c/backends/plankac_bytecode.c \
	c/backends/plankac_asm8086.c \
	c/core/plankac_runtime.c

PLANKAC_NATIVE_SOURCE := c/backends/plankac_native_runtime.c
PLANKAC_OBJECTS := $(patsubst c/%.c,$(BUILD)/%.o,$(PLANKAC_LIB_SOURCES) $(PLANKAC_NATIVE_SOURCE))

PLANKAGUI_CORE := \
	graphics/c/plankagui_scene.c \
	graphics/c/plankagui_raster.c \
	graphics/c/plankagui_font.c \
	graphics/c/plankagui_png.c \
	graphics/c/plankagui_render.c

PLANKACUBE_CORE := \
	graphics/c/plankacube_scene.c \
	graphics/c/plankacube_render.c \
	graphics/c/plankagui_raster.c \
	graphics/c/plankagui_font.c

PLANKAHOST_CORE := \
	graphics/c/plankahost.c \
	graphics/c/plankagui_scene.c \
	graphics/c/plankagui_raster.c \
	graphics/c/plankagui_font.c \
	graphics/c/plankagui_render.c \
	graphics/c/plankacube_scene.c \
	graphics/c/plankacube_render.c

.PHONY: all ci check generated clean

all: \
	$(BUILD)/plankamath_cli \
	$(BUILD)/plankac \
	$(BUILD)/libplankac.a \
	$(BUILD)/plankac_api_demo \
	$(BUILD)/plankac_abi_demo \
	$(BUILD)/plankac_conformance \
	$(BUILD)/plankagui_export \
	$(BUILD)/plankacube_export \
	$(BUILD)/plankahost_demo

$(BUILD):
	mkdir -p $(BUILD)

$(BUILD)/%.o: c/%.c
	mkdir -p $(dir $@)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

$(BUILD)/libplankac.a: $(PLANKAC_OBJECTS)
	$(AR) rcs $@ $^

$(BUILD)/plankamath_cli: c/legacy/plankamath.c c/targets/plankamath_cli.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD)/plankac: $(PLANKAC_LIB_SOURCES) c/tools/plankac_cli.c | $(BUILD)
	$(CC) $(CPPFLAGS) $(CFLAGS) $^ -o $@ $(LDLIBS)

$(BUILD)/plankac_api_demo: examples/c_api_demo.c $(BUILD)/libplankac.a
	$(CC) $(CPPFLAGS) $(CFLAGS) examples/c_api_demo.c $(BUILD)/libplankac.a -o $@ $(LDLIBS)

$(BUILD)/plankac_abi_demo: examples/c_abi_demo.c $(BUILD)/libplankac.a
	$(CC) $(CPPFLAGS) $(CFLAGS) examples/c_abi_demo.c $(BUILD)/libplankac.a -o $@ $(LDLIBS)

$(BUILD)/plankac_conformance: tests/plankac_conformance.c $(BUILD)/libplankac.a
	$(CC) $(CPPFLAGS) $(CFLAGS) tests/plankac_conformance.c $(BUILD)/libplankac.a -o $@ $(LDLIBS)

$(BUILD)/plankagui_export: graphics/c/plankagui_main.c $(PLANKAGUI_CORE) $(BUILD)/libplankac.a
	$(CC) $(CPPFLAGS) -Igraphics/c $(CFLAGS) graphics/c/plankagui_main.c $(PLANKAGUI_CORE) $(BUILD)/libplankac.a -o $@ $(LDLIBS)

$(BUILD)/plankacube_export: graphics/c/plankacube_main.c $(PLANKACUBE_CORE) graphics/c/plankagui_png.c $(BUILD)/libplankac.a
	$(CC) $(CPPFLAGS) -Igraphics/c $(CFLAGS) graphics/c/plankacube_main.c $(PLANKACUBE_CORE) graphics/c/plankagui_png.c $(BUILD)/libplankac.a -o $@ $(LDLIBS)

$(BUILD)/plankahost_demo: graphics/c/plankahost_demo.c $(PLANKAHOST_CORE) $(BUILD)/libplankac.a
	$(CC) $(CPPFLAGS) -Igraphics/c $(CFLAGS) graphics/c/plankahost_demo.c $(PLANKAHOST_CORE) $(BUILD)/libplankac.a -o $@ $(LDLIBS)

generated: all
	$(BUILD)/plankac bytecode $(BUILD)/plankamath.pbc
	$(BUILD)/plankac ir $(BUILD)/plankac.ir
	$(BUILD)/plankac lowering $(BUILD)/plankac.lowering
	$(BUILD)/plankac cgen $(BUILD)/plankac_generated.c
	$(BUILD)/plankac asmgen $(BUILD)/plankac_asm_runtime.S
	$(BUILD)/plankac asm8086 $(BUILD)/plankac_8086.asm

check: generated
	$(BUILD)/plankamath_cli compile
	$(BUILD)/plankamath_cli tests
	$(BUILD)/plankac check
	$(BUILD)/plankac tests
	$(BUILD)/plankac checkbc $(BUILD)/plankamath.pbc
	$(BUILD)/plankac runbc $(BUILD)/plankamath.pbc set_session
	$(BUILD)/plankac runfile graphics/src/plankagui.plk app_kind
	$(BUILD)/plankac runfile graphics/src/plankacube.plk app_kind
	$(BUILD)/plankac runfile examples/max3.plk max3_demo
	$(BUILD)/plankahost_demo graphics/src/plankagui.plk
	$(BUILD)/plankahost_demo graphics/src/plankacube.plk
	$(BUILD)/plankac compile $(BUILD)/plankac_pipeline
	$(BUILD)/plankac native-c $(BUILD)/plankac_native_c
	$(BUILD)/plankac_native_c.exe set_session
	$(CC) $(CPPFLAGS) $(CFLAGS) $(BUILD)/plankac_generated.c $(BUILD)/libplankac.a -o $(BUILD)/plankac_generated $(LDLIBS)
	$(BUILD)/plankac_generated set_session
	$(BUILD)/plankac_generated record_child_heap_session
	$(BUILD)/plankac_generated relation_compose_session
	$(BUILD)/plankac_generated chess_piece_struct_session
	$(BUILD)/plankac_generated two_dimensional_original_session
	$(BUILD)/plankac_generated three_d_pipeline_session
	test -s $(BUILD)/plankac_asm_runtime.S
	grep -q "plc_native_p10" $(BUILD)/plankac_asm_runtime.S
	grep -q ".globl main" $(BUILD)/plankac_asm_runtime.S
	test -s $(BUILD)/plankac_8086.asm
	$(BUILD)/plankac_api_demo
	$(BUILD)/plankac_abi_demo
	$(BUILD)/plankac_conformance
	$(BUILD)/plankagui_export graphics/examples/plankagui.png
	$(BUILD)/plankacube_export graphics/examples/plankacube.png 0.85 graphics/src/plankacube.plk

ci: check

clean:
	rm -rf $(BUILD)
