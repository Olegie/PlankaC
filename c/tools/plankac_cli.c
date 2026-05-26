#include "plankac_internal.h"

static int plc_run_named(const PLC_PROGRAM *program, const char *name,
    int argc, char **argv)
{
    const PLC_PROC *proc;
    double args[PLC_MAX_ARGS];
    PLC_VALUES out;
    char err[PLC_MAX_LINE];
    int i;

    proc = plc_find_proc(program, name);
    if (proc == 0) {
        printf("Unknown procedure: %s\n", name);
        return 1;
    }
    if (argc != proc->argc) {
        printf("%s expects %d argument(s), got %d\n",
            proc->name, proc->argc, argc);
        return 1;
    }
    for (i = 0; i < argc; ++i) {
        args[i] = strtod(argv[i], 0);
    }
    err[0] = '\0';
    if (!plc_execute_proc(program, proc, args, argc, &out,
            0, err, sizeof(err))) {
        printf("Run failed: %s\n", err);
        return 1;
    }
    for (i = 0; i < out.count; ++i) {
        char value[64];

        plc_format_value(out.value[i], value);
        if (i > 0) {
            printf(" ");
        }
        printf("R%d=%s", i, value);
    }
    printf("\n");
    if (strcmp(proc->name, "all_tests") == 0 && out.count > 0) {
        return out.value[0] != 0.0 ? 0 : 1;
    }
    return 0;
}

static int plc_load_program_with_file(PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size)
{
    const char *sources[64];
    int count;

    count = 0;
    while (PLC_SOURCES[count] != 0 && count < 62) {
        sources[count] = PLC_SOURCES[count];
        ++count;
    }
    if (count >= 62) {
        plc_set_error(err, err_size, "too many default sources");
        return 0;
    }
    sources[count] = path;
    ++count;
    sources[count] = 0;
    return plc_load_sources(program, sources, err, err_size);
}

typedef struct PLC_PIPELINE_PATHS {
    char bytecode[PLC_MAX_LINE];
    char c_source[PLC_MAX_LINE];
    char asm_source[PLC_MAX_LINE];
    char asm8086_source[PLC_MAX_LINE];
    char exe[PLC_MAX_LINE];
} PLC_PIPELINE_PATHS;

static int plc_make_output_path(const char *prefix, const char *suffix,
    char *out, unsigned out_size)
{
    if (prefix == 0 || prefix[0] == '\0') {
        return 0;
    }
    if (strlen(prefix) + strlen(suffix) + 1 > out_size) {
        return 0;
    }
    strcpy(out, prefix);
    strcat(out, suffix);
    return 1;
}

static int plc_pipeline_paths(const char *prefix, PLC_PIPELINE_PATHS *paths)
{
    if (paths == 0) {
        return 0;
    }
    memset(paths, 0, sizeof(*paths));
    return plc_make_output_path(prefix, ".pbc", paths->bytecode,
            sizeof(paths->bytecode))
        && plc_make_output_path(prefix, ".c", paths->c_source,
            sizeof(paths->c_source))
        && plc_make_output_path(prefix, ".S", paths->asm_source,
            sizeof(paths->asm_source))
        && plc_make_output_path(prefix, "_8086.asm", paths->asm8086_source,
            sizeof(paths->asm8086_source))
        && plc_make_output_path(prefix, ".exe", paths->exe,
            sizeof(paths->exe));
}

static void plc_quote_arg(char *out, unsigned out_size, const char *text)
{
    unsigned n;

    if (out_size == 0) {
        return;
    }
    n = 0;
    out[n++] = '"';
    while (text != 0 && *text != '\0' && n + 2 < out_size) {
        if (*text == '"') {
            if (n + 3 >= out_size) {
                break;
            }
            out[n++] = '\\';
        }
        out[n++] = *text;
        ++text;
    }
    if (n + 1 < out_size) {
        out[n++] = '"';
    }
    out[n] = '\0';
}

static int plc_compile_pipeline(const PLC_PROGRAM *source_program,
    const char *prefix, PLC_PIPELINE_PATHS *paths, char *err,
    unsigned err_size)
{
    static PLC_PROGRAM ir_program;

    if (!plc_pipeline_paths(prefix, paths)) {
        plc_set_error(err, err_size, "bad compiler output prefix");
        return 0;
    }
    if (!plc_emit_bytecode(source_program, paths->bytecode, err, err_size)) {
        return 0;
    }
    if (!plc_load_bytecode(&ir_program, paths->bytecode, err, err_size)) {
        plc_prefix_error(err, err_size, "IR reload failed: ");
        return 0;
    }
    if (!plc_emit_c_backend(&ir_program, paths->c_source, err, err_size)) {
        return 0;
    }
    if (!plc_emit_asm_runtime(&ir_program, paths->asm_source,
            err, err_size)) {
        return 0;
    }
    if (!plc_emit_asm8086_runtime(&ir_program, paths->asm8086_source,
            err, err_size)) {
        return 0;
    }
    printf("Compiler pipeline OK\n");
    printf("source: %d files, %d procedures\n",
        source_program->source_count, source_program->proc_count);
    printf("ir: %s\n", paths->bytecode);
    printf("ir procedures: %d\n", ir_program.proc_count);
    printf("c: %s\n", paths->c_source);
    printf("asm: %s\n", paths->asm_source);
    printf("asm8086: %s\n", paths->asm8086_source);
    return 1;
}

static int plc_run_shell_command(const char *command)
{
    int rc;

    rc = system(command);
    return rc == 0;
}

static int plc_compile_native_c(const PLC_PROGRAM *program,
    const char *prefix, char *err, unsigned err_size)
{
    PLC_PIPELINE_PATHS paths;
    char c_path[PLC_MAX_LINE + 4];
    char exe_path[PLC_MAX_LINE + 4];
    char command[PLC_MAX_LINE * 4];

    if (!plc_compile_pipeline(program, prefix, &paths, err, err_size)) {
        return 0;
    }
    plc_quote_arg(c_path, sizeof(c_path), paths.c_source);
    plc_quote_arg(exe_path, sizeof(exe_path), paths.exe);
    sprintf(command,
        "gcc -Wall -Wextra -std=c89 -Ic/include %s build/libplankac.a -o %s -lm",
        c_path, exe_path);
    if (!plc_run_shell_command(command)) {
        plc_set_error(err, err_size, "native C link failed");
        return 0;
    }
    printf("native-c: %s\n", paths.exe);
    return 1;
}

static int plc_compile_native_asm(const PLC_PROGRAM *program,
    const char *prefix, char *err, unsigned err_size)
{
    PLC_PIPELINE_PATHS paths;
    char asm_path[PLC_MAX_LINE + 4];
    char exe_path[PLC_MAX_LINE + 4];
    char command[PLC_MAX_LINE * 4];

    if (!plc_compile_pipeline(program, prefix, &paths, err, err_size)) {
        return 0;
    }
    plc_quote_arg(asm_path, sizeof(asm_path), paths.asm_source);
    plc_quote_arg(exe_path, sizeof(exe_path), paths.exe);
    sprintf(command,
        "gcc -Wall -Wextra -Ic/include %s build/libplankac.a -o %s -lm",
        asm_path, exe_path);
    if (!plc_run_shell_command(command)) {
        plc_set_error(err, err_size, "native ASM link failed");
        return 0;
    }
    printf("native-asm: %s\n", paths.exe);
    return 1;
}

static void plc_print_usage(void)
{
    printf("PlankaC %s\n", PLANKAC_VERSION);
    printf("commands:\n");
    printf("  check\n");
    printf("  compile <output-prefix>\n");
    printf("  native-c <output-prefix>\n");
    printf("  native-asm <output-prefix>\n");
    printf("  list\n");
    printf("  run <procedure|Pnumber> [args...]\n");
    printf("  checkfile <extra.plk>\n");
    printf("  runfile <extra.plk> <procedure|Pnumber> [args...]\n");
    printf("  bytecode <output.pbc>\n");
    printf("  checkbc <input.pbc>\n");
    printf("  runbc <input.pbc> <procedure|Pnumber> [args...]\n");
    printf("  ir <output.ir>\n");
    printf("  lowering <output.txt>\n");
    printf("  cgen <output.c>\n");
    printf("  asmgen <output.S>\n");
    printf("  asmimage <output.asm>\n");
    printf("  asm8086 <output.asm>\n");
    printf("  demo\n");
    printf("  tests\n");
}

int main(int argc, char **argv)
{
    static PLC_PROGRAM program;
    char err[PLC_MAX_LINE];
    int i;

    err[0] = '\0';
    if (!plc_load_program(&program, err, sizeof(err))) {
        printf("PlankaC load failed: %s\n", err);
        return 1;
    }

    if (argc < 2) {
        plc_print_usage();
        return 0;
    }

    if (strcmp(argv[1], "check") == 0) {
        printf("PlankaC OK: %d files, %d procedures\n",
            program.source_count, program.proc_count);
        return 0;
    }

    if (strcmp(argv[1], "compile") == 0) {
        PLC_PIPELINE_PATHS paths;

        if (argc < 3) {
            printf("Usage: compile <output-prefix>\n");
            return 1;
        }
        if (!plc_compile_pipeline(&program, argv[2], &paths,
                err, sizeof(err))) {
            printf("Compile failed: %s\n", err);
            return 1;
        }
        return 0;
    }

    if (strcmp(argv[1], "native-c") == 0) {
        if (argc < 3) {
            printf("Usage: native-c <output-prefix>\n");
            return 1;
        }
        if (!plc_compile_native_c(&program, argv[2], err, sizeof(err))) {
            printf("Native C failed: %s\n", err);
            return 1;
        }
        return 0;
    }

    if (strcmp(argv[1], "native-asm") == 0) {
        if (argc < 3) {
            printf("Usage: native-asm <output-prefix>\n");
            return 1;
        }
        if (!plc_compile_native_asm(&program, argv[2], err, sizeof(err))) {
            printf("Native ASM failed: %s\n", err);
            return 1;
        }
        return 0;
    }

    if (strcmp(argv[1], "list") == 0) {
        for (i = 0; i < program.proc_count; ++i) {
            printf("P%d %s argc=%d results=%d statements=%d\n",
                program.procs[i].number,
                program.procs[i].name,
                program.procs[i].argc,
                program.procs[i].results,
                program.procs[i].stmt_count);
        }
        return 0;
    }

    if (strcmp(argv[1], "run") == 0) {
        if (argc < 3) {
            printf("Missing procedure name.\n");
            return 1;
        }
        return plc_run_named(&program, argv[2], argc - 3, argv + 3);
    }

    if (strcmp(argv[1], "checkfile") == 0) {
        if (argc < 3) {
            printf("Missing .plk input path.\n");
            return 1;
        }
        if (!plc_load_program_with_file(&program, argv[2],
                err, sizeof(err))) {
            printf("PlankaC file load failed: %s\n", err);
            return 1;
        }
        printf("PlankaC file OK: %d files, %d procedures\n",
            program.source_count, program.proc_count);
        return 0;
    }

    if (strcmp(argv[1], "runfile") == 0) {
        if (argc < 4) {
            printf("Usage: runfile <extra.plk> <procedure|Pnumber> [args...]\n");
            return 1;
        }
        if (!plc_load_program_with_file(&program, argv[2],
                err, sizeof(err))) {
            printf("PlankaC file load failed: %s\n", err);
            return 1;
        }
        return plc_run_named(&program, argv[3], argc - 4, argv + 4);
    }

    if (strcmp(argv[1], "bytecode") == 0) {
        if (argc < 3) {
            printf("Missing bytecode output path.\n");
            return 1;
        }
        if (!plc_emit_bytecode(&program, argv[2], err, sizeof(err))) {
            printf("Bytecode failed: %s\n", err);
            return 1;
        }
        printf("Bytecode written: %s\n", argv[2]);
        return 0;
    }

    if (strcmp(argv[1], "checkbc") == 0) {
        static PLC_PROGRAM bytecode_program;

        if (argc < 3) {
            printf("Missing bytecode input path.\n");
            return 1;
        }
        if (!plc_load_bytecode(&bytecode_program, argv[2],
                err, sizeof(err))) {
            printf("Bytecode load failed: %s\n", err);
            return 1;
        }
        printf("Bytecode OK: %d procedures\n",
            bytecode_program.proc_count);
        return 0;
    }

    if (strcmp(argv[1], "runbc") == 0) {
        static PLC_PROGRAM bytecode_program;

        if (argc < 4) {
            printf("Usage: runbc <input.pbc> <procedure|Pnumber> [args...]\n");
            return 1;
        }
        if (!plc_load_bytecode(&bytecode_program, argv[2],
                err, sizeof(err))) {
            printf("Bytecode load failed: %s\n", err);
            return 1;
        }
        return plc_run_named(&bytecode_program, argv[3],
            argc - 4, argv + 4);
    }

    if (strcmp(argv[1], "ir") == 0) {
        if (argc < 3) {
            printf("Missing IR output path.\n");
            return 1;
        }
        if (!plc_emit_ir(&program, argv[2], err, sizeof(err))) {
            printf("IR failed: %s\n", err);
            return 1;
        }
        printf("IR written: %s\n", argv[2]);
        return 0;
    }

    if (strcmp(argv[1], "lowering") == 0) {
        if (argc < 3) {
            printf("Missing lowering output path.\n");
            return 1;
        }
        if (!plc_emit_lowering_report(&program, argv[2],
                err, sizeof(err))) {
            printf("Lowering failed: %s\n", err);
            return 1;
        }
        printf("Lowering written: %s\n", argv[2]);
        return 0;
    }

    if (strcmp(argv[1], "cgen") == 0) {
        if (argc < 3) {
            printf("Missing C backend output path.\n");
            return 1;
        }
        if (!plc_emit_c_backend(&program, argv[2], err, sizeof(err))) {
            printf("C backend failed: %s\n", err);
            return 1;
        }
        printf("C backend written: %s\n", argv[2]);
        return 0;
    }

    if (strcmp(argv[1], "asmgen") == 0) {
        if (argc < 3) {
            printf("Missing ASM runtime output path.\n");
            return 1;
        }
        if (!plc_emit_asm_runtime(&program, argv[2], err, sizeof(err))) {
            printf("ASM runtime failed: %s\n", err);
            return 1;
        }
        printf("ASM runtime written: %s\n", argv[2]);
        return 0;
    }

    if (strcmp(argv[1], "asmimage") == 0) {
        if (argc < 3) {
            printf("Missing ASM image output path.\n");
            return 1;
        }
        if (!plc_emit_asm_image(&program, argv[2], err, sizeof(err))) {
            printf("ASM image failed: %s\n", err);
            return 1;
        }
        printf("ASM image written: %s\n", argv[2]);
        return 0;
    }

    if (strcmp(argv[1], "asm8086") == 0) {
        if (argc < 3) {
            printf("Missing 8086 ASM output path.\n");
            return 1;
        }
        if (!plc_emit_asm8086_runtime(&program, argv[2], err, sizeof(err))) {
            printf("8086 ASM failed: %s\n", err);
            return 1;
        }
        printf("8086 ASM written: %s\n", argv[2]);
        return 0;
    }

    if (strcmp(argv[1], "demo") == 0) {
        return plc_run_named(&program, "calculator_demo", 0, argv + argc);
    }

    if (strcmp(argv[1], "tests") == 0) {
        return plc_run_named(&program, "all_tests", 0, argv + argc);
    }

    printf("Unknown command: %s\n", argv[1]);
    return 1;
}
