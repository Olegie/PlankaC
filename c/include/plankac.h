#ifndef PLANKAC_H
#define PLANKAC_H

#ifdef __cplusplus
extern "C" {
#endif

#define PLANKAC_OK 1
#define PLANKAC_ERR 0
#define PLANKAC_MAX_ARGS 8
#define PLANKAC_MAX_RESULTS 8
#define PLANKAC_MAX_NAME 64
#define PLANKAC_MAX_TYPE_TEXT 24
#define PLANKAC_MAX_NATIVE 32

typedef struct PLANKAC_RESULT {
    double value[PLANKAC_MAX_RESULTS];
    int count;
} PLANKAC_RESULT;

typedef int (*PLANKAC_NATIVE_FN)(void *user_data,
    const double *args, int argc, PLANKAC_RESULT *result,
    char *err, unsigned err_size);

typedef struct PLANKAC_PROC_INFO {
    int number;
    char name[PLANKAC_MAX_NAME];
    int argc;
    int results;
    int statements;
    char arg_types[PLANKAC_MAX_ARGS][PLANKAC_MAX_TYPE_TEXT];
    char result_types[PLANKAC_MAX_RESULTS][PLANKAC_MAX_TYPE_TEXT];
} PLANKAC_PROC_INFO;

typedef struct PLANKAC_NATIVE_INFO {
    char name[PLANKAC_MAX_NAME];
    int argc;
    int results;
    char arg_types[PLANKAC_MAX_ARGS][PLANKAC_MAX_TYPE_TEXT];
    char result_types[PLANKAC_MAX_RESULTS][PLANKAC_MAX_TYPE_TEXT];
} PLANKAC_NATIVE_INFO;

typedef struct PLANKAC_CONTEXT PLANKAC_CONTEXT;

PLANKAC_CONTEXT *plankac_create(void);
void plankac_destroy(PLANKAC_CONTEXT *ctx);
int plankac_context_load_default(PLANKAC_CONTEXT *ctx,
    char *err, unsigned err_size);
int plankac_context_load_sources(PLANKAC_CONTEXT *ctx,
    const char *const *sources, char *err, unsigned err_size);
int plankac_context_load_bytecode(PLANKAC_CONTEXT *ctx,
    const char *path, char *err, unsigned err_size);
int plankac_context_load_bytecode_text(PLANKAC_CONTEXT *ctx,
    const char *text, char *err, unsigned err_size);
int plankac_context_proc_count(PLANKAC_CONTEXT *ctx);
int plankac_context_get_proc(PLANKAC_CONTEXT *ctx, int index,
    PLANKAC_PROC_INFO *info);
int plankac_context_find_proc(PLANKAC_CONTEXT *ctx, const char *name,
    PLANKAC_PROC_INFO *info);
int plankac_context_register_native(PLANKAC_CONTEXT *ctx, const char *name,
    int argc, int results, const char *const *arg_types,
    const char *const *result_types, PLANKAC_NATIVE_FN fn, void *user_data,
    char *err, unsigned err_size);
int plankac_context_native_count(PLANKAC_CONTEXT *ctx);
int plankac_context_get_native(PLANKAC_CONTEXT *ctx, int index,
    PLANKAC_NATIVE_INFO *info);
int plankac_context_find_native(PLANKAC_CONTEXT *ctx, const char *name,
    PLANKAC_NATIVE_INFO *info);
int plankac_context_run(PLANKAC_CONTEXT *ctx, const char *name,
    const double *args, int argc, PLANKAC_RESULT *result,
    char *err, unsigned err_size);
int plankac_context_run_number(PLANKAC_CONTEXT *ctx, int number,
    const double *args, int argc, PLANKAC_RESULT *result,
    char *err, unsigned err_size);
int plankac_context_summary(PLANKAC_CONTEXT *ctx,
    char *out, unsigned out_size);
int plankac_context_write_bytecode(PLANKAC_CONTEXT *ctx,
    const char *path, char *err, unsigned err_size);
int plankac_context_write_c_backend(PLANKAC_CONTEXT *ctx,
    const char *path, char *err, unsigned err_size);
int plankac_context_write_asm_runtime(PLANKAC_CONTEXT *ctx,
    const char *path, char *err, unsigned err_size);
int plankac_context_write_asm_image(PLANKAC_CONTEXT *ctx,
    const char *path, char *err, unsigned err_size);
int plankac_context_write_asm8086_runtime(PLANKAC_CONTEXT *ctx,
    const char *path, char *err, unsigned err_size);
int plankac_context_write_ir(PLANKAC_CONTEXT *ctx,
    const char *path, char *err, unsigned err_size);
int plankac_context_write_lowering_report(PLANKAC_CONTEXT *ctx,
    const char *path, char *err, unsigned err_size);

int plankac_load(char *err, unsigned err_size);
int plankac_reload(char *err, unsigned err_size);
int plankac_compile_summary(char *out, unsigned out_size);
int plankac_proc_count(void);
int plankac_get_proc(int index, PLANKAC_PROC_INFO *info);
int plankac_find_proc(const char *name, PLANKAC_PROC_INFO *info);
int plankac_register_native(const char *name, int argc, int results,
    const char *const *arg_types, const char *const *result_types,
    PLANKAC_NATIVE_FN fn, void *user_data, char *err, unsigned err_size);
int plankac_native_count(void);
int plankac_get_native(int index, PLANKAC_NATIVE_INFO *info);
int plankac_find_native(const char *name, PLANKAC_NATIVE_INFO *info);
int plankac_run(const char *name, const double *args, int argc,
    PLANKAC_RESULT *result, char *err, unsigned err_size);
int plankac_run_number(int number, const double *args, int argc,
    PLANKAC_RESULT *result, char *err, unsigned err_size);
int plankac_write_bytecode(const char *path, char *err, unsigned err_size);
int plankac_write_c_backend(const char *path, char *err, unsigned err_size);
int plankac_write_asm_runtime(const char *path, char *err, unsigned err_size);
int plankac_write_asm_image(const char *path, char *err, unsigned err_size);
int plankac_write_asm8086_runtime(const char *path,
    char *err, unsigned err_size);
int plankac_write_ir(const char *path, char *err, unsigned err_size);
int plankac_write_lowering_report(const char *path,
    char *err, unsigned err_size);
void plankac_format(double value, char *out, unsigned out_size);

#ifdef __cplusplus
}
#endif

#endif
