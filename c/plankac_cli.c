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

static void plc_print_usage(void)
{
    printf("PlankaC v0.1\n");
    printf("commands:\n");
    printf("  check | compile\n");
    printf("  list\n");
    printf("  run <procedure|Pnumber> [args...]\n");
    printf("  bytecode <output.pbc>\n");
    printf("  checkbc <input.pbc>\n");
    printf("  runbc <input.pbc> <procedure|Pnumber> [args...]\n");
    printf("  cgen <output.c>\n");
    printf("  asmgen <output.S>\n");
    printf("  asmimage <output.asm>\n");
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

    if (strcmp(argv[1], "check") == 0 || strcmp(argv[1], "compile") == 0) {
        printf("PlankaC OK: %d files, %d procedures\n",
            program.source_count, program.proc_count);
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

    if (strcmp(argv[1], "demo") == 0) {
        return plc_run_named(&program, "calculator_demo", 0, argv + argc);
    }

    if (strcmp(argv[1], "tests") == 0) {
        return plc_run_named(&program, "all_tests", 0, argv + argc);
    }

    printf("Unknown command: %s\n", argv[1]);
    return 1;
}
