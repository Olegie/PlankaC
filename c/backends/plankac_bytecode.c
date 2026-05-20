#include "plankac_internal.h"

static void plc_write_quoted(FILE *fp, const char *text)
{
    fputc('"', fp);
    while (*text != '\0') {
        if (*text == '"' || *text == '\\') {
            fputc('\\', fp);
        }
        fputc(*text, fp);
        ++text;
    }
    fputc('"', fp);
}

static void plc_write_c_quoted(FILE *fp, const char *text)
{
    fputc('"', fp);
    while (*text != '\0') {
        if (*text == '"' || *text == '\\') {
            fputc('\\', fp);
        }
        if (*text == '\n') {
            fputs("\\n", fp);
        } else if (*text == '\r') {
            fputs("\\r", fp);
        } else {
            fputc(*text, fp);
        }
        ++text;
    }
    fputc('"', fp);
}

static void plc_write_asm_quoted(FILE *fp, const char *text)
{
    fputc('"', fp);
    while (*text != '\0') {
        unsigned char c;

        c = (unsigned char)*text;
        if (c == '"' || c == '\\') {
            fputc('\\', fp);
            fputc(c, fp);
        } else if (c == '\n') {
            fputs("\\n", fp);
        } else if (c == '\r') {
            fputs("\\r", fp);
        } else if (c == '\t') {
            fputs("\\t", fp);
        } else if (c < 32 || c > 126) {
            fprintf(fp, "\\x%02X", (unsigned)c);
        } else {
            fputc(c, fp);
        }
        ++text;
    }
    fputc('"', fp);
}

int plc_emit_bytecode_stmt(FILE *fp, const PLC_PROC *proc,
    const PLC_STMT *stmt, char *err, unsigned err_size)
{
    char parts[5][PLC_MAX_LINE];
    char call_name[PLC_MAX_NAME];
    char call_args[PLC_MAX_LINE];
    int count;

    (void)err_size;

    if (plc_line_starts_with(stmt->text, "ASSERT")) {
        fprintf(fp, "  ASSERT ");
        plc_write_quoted(fp, plc_skip_space(plc_skip_space(stmt->text) + 6));
        fprintf(fp, "\n");
        return 1;
    }

    if (!plc_split_arrows(stmt->text, parts, &count)) {
        sprintf(err, "P%d %s line %d: too many arrows",
            proc->number, proc->name, stmt->line_no);
        return 0;
    }
    if (count == 2) {
        if (plc_is_top_call(parts[0], call_name, sizeof(call_name),
                call_args, sizeof(call_args))) {
            fprintf(fp, "  CALL %s ", call_name);
            plc_write_quoted(fp, call_args);
            fprintf(fp, " -> ");
            plc_write_quoted(fp, parts[1]);
            fprintf(fp, "\n");
        } else {
            fprintf(fp, "  EVAL ");
            plc_write_quoted(fp, parts[0]);
            fprintf(fp, " -> ");
            plc_write_quoted(fp, parts[1]);
            fprintf(fp, "\n");
        }
        return 1;
    }
    if (count == 3) {
        if (plc_line_starts_with(parts[0], "LOOP")) {
            fprintf(fp, "  LOOP ");
            plc_write_quoted(fp, plc_skip_space(plc_skip_space(parts[0]) + 4));
            fprintf(fp, " ");
            plc_write_quoted(fp, parts[1]);
            fprintf(fp, " -> ");
            plc_write_quoted(fp, parts[2]);
            fprintf(fp, "\n");
            return 1;
        }
        if (plc_is_top_call(parts[1], call_name, sizeof(call_name),
                call_args, sizeof(call_args))) {
            fprintf(fp, "  GCALL ");
            plc_write_quoted(fp, parts[0]);
            fprintf(fp, " %s ", call_name);
            plc_write_quoted(fp, call_args);
            fprintf(fp, " -> ");
            plc_write_quoted(fp, parts[2]);
            fprintf(fp, "\n");
        } else {
            fprintf(fp, "  GEVAL ");
            plc_write_quoted(fp, parts[0]);
            fprintf(fp, " ");
            plc_write_quoted(fp, parts[1]);
            fprintf(fp, " -> ");
            plc_write_quoted(fp, parts[2]);
            fprintf(fp, "\n");
        }
        return 1;
    }
    sprintf(err, "P%d %s line %d: expected assignment",
        proc->number, proc->name, stmt->line_no);
    return 0;
}

int plc_emit_bytecode(const PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size)
{
    FILE *fp;
    int i;
    int j;

    fp = fopen(path, "w");
    if (fp == 0) {
        sprintf(err, "cannot open bytecode output: %s", path);
        return 0;
    }
    fprintf(fp, "PLANKAC-BYTECODE 0.1\n");
    fprintf(fp, "SOURCES %d\n", program->source_count);
    fprintf(fp, "PROCEDURES %d\n", program->proc_count);
    for (i = 0; i < program->proc_count; ++i) {
        const PLC_PROC *proc;

        proc = &program->procs[i];
        fprintf(fp, "PROC P%d %s ARGC %d RESULTS %d\n",
            proc->number, proc->name, proc->argc, proc->results);
        for (j = 0; j < proc->stmt_count; ++j) {
            if (!plc_emit_bytecode_stmt(fp, proc, &proc->stmts[j],
                    err, err_size)) {
                fclose(fp);
                return 0;
            }
        }
        fprintf(fp, "END\n");
    }
    fclose(fp);
    return 1;
}

int plc_emit_c_backend(const PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size)
{
    FILE *out;
    FILE *tmp;
    char tmp_path[PLC_MAX_LINE];
    char line[PLC_MAX_LINE];

    if (strlen(path) + 5 >= sizeof(tmp_path)) {
        plc_set_error(err, err_size, "C backend output path too long");
        return 0;
    }
    strcpy(tmp_path, path);
    strcat(tmp_path, ".pbc");
    if (!plc_emit_bytecode(program, tmp_path, err, err_size)) {
        return 0;
    }
    out = fopen(path, "w");
    if (out == 0) {
        remove(tmp_path);
        sprintf(err, "cannot open C backend output: %s", path);
        return 0;
    }
    tmp = fopen(tmp_path, "r");
    if (tmp == 0) {
        fclose(out);
        remove(tmp_path);
        sprintf(err, "cannot read generated bytecode: %s", tmp_path);
        return 0;
    }

    fprintf(out, "/* Generated by PlankaC. */\n");
    fprintf(out, "#include <stdio.h>\n");
    fprintf(out, "#include <stdlib.h>\n");
    fprintf(out, "#include \"plankac.h\"\n\n");
    fprintf(out, "static const char *PLANKAC_EMBEDDED_BYTECODE[] = {\n");
    while (fgets(line, sizeof(line), tmp) != 0) {
        fprintf(out, "    ");
        plc_write_c_quoted(out, line);
        fprintf(out, ",\n");
    }
    fprintf(out, "    0\n");
    fprintf(out, "};\n\n");
    fprintf(out, "static int write_bytecode(const char *path)\n");
    fprintf(out, "{\n");
    fprintf(out, "    FILE *fp;\n");
    fprintf(out, "    int i;\n\n");
    fprintf(out, "    fp = fopen(path, \"w\");\n");
    fprintf(out, "    if (fp == 0) {\n");
    fprintf(out, "        return 0;\n");
    fprintf(out, "    }\n");
    fprintf(out, "    for (i = 0; PLANKAC_EMBEDDED_BYTECODE[i] != 0; ++i) {\n");
    fprintf(out, "        fputs(PLANKAC_EMBEDDED_BYTECODE[i], fp);\n");
    fprintf(out, "    }\n");
    fprintf(out, "    fclose(fp);\n");
    fprintf(out, "    return 1;\n");
    fprintf(out, "}\n\n");
    fprintf(out, "int main(int argc, char **argv)\n");
    fprintf(out, "{\n");
    fprintf(out, "    PLANKAC_CONTEXT *ctx;\n");
    fprintf(out, "    PLANKAC_RESULT result;\n");
    fprintf(out, "    double args[16];\n");
    fprintf(out, "    char err[256];\n");
    fprintf(out, "    const char *bytecode_path;\n");
    fprintf(out, "    int i;\n\n");
    fprintf(out, "    if (argc < 2) {\n");
    fprintf(out, "        printf(\"usage: %%s <procedure> [args...]\\n\", argv[0]);\n");
    fprintf(out, "        return 1;\n");
    fprintf(out, "    }\n");
    fprintf(out, "    if (argc - 2 > 16) {\n");
    fprintf(out, "        printf(\"too many arguments\\n\");\n");
    fprintf(out, "        return 1;\n");
    fprintf(out, "    }\n");
    fprintf(out, "    bytecode_path = \"plankac_embedded_runner.pbc\";\n");
    fprintf(out, "    if (!write_bytecode(bytecode_path)) {\n");
    fprintf(out, "        printf(\"cannot write embedded bytecode\\n\");\n");
    fprintf(out, "        return 1;\n");
    fprintf(out, "    }\n");
    fprintf(out, "    ctx = plankac_create();\n");
    fprintf(out, "    if (ctx == 0) {\n");
    fprintf(out, "        remove(bytecode_path);\n");
    fprintf(out, "        printf(\"cannot create PlankaC context\\n\");\n");
    fprintf(out, "        return 1;\n");
    fprintf(out, "    }\n");
    fprintf(out, "    err[0] = '\\0';\n");
    fprintf(out, "    if (!plankac_context_load_bytecode(ctx, bytecode_path, err, sizeof(err))) {\n");
    fprintf(out, "        plankac_destroy(ctx);\n");
    fprintf(out, "        remove(bytecode_path);\n");
    fprintf(out, "        printf(\"bytecode load failed: %%s\\n\", err);\n");
    fprintf(out, "        return 1;\n");
    fprintf(out, "    }\n");
    fprintf(out, "    remove(bytecode_path);\n");
    fprintf(out, "    for (i = 2; i < argc; ++i) {\n");
    fprintf(out, "        args[i - 2] = strtod(argv[i], 0);\n");
    fprintf(out, "    }\n");
    fprintf(out, "    if (!plankac_context_run(ctx, argv[1], args, argc - 2, &result, err, sizeof(err))) {\n");
    fprintf(out, "        plankac_destroy(ctx);\n");
    fprintf(out, "        printf(\"run failed: %%s\\n\", err);\n");
    fprintf(out, "        return 1;\n");
    fprintf(out, "    }\n");
    fprintf(out, "    for (i = 0; i < result.count; ++i) {\n");
    fprintf(out, "        char value[64];\n");
    fprintf(out, "        plankac_format(result.value[i], value, sizeof(value));\n");
    fprintf(out, "        if (i > 0) {\n");
    fprintf(out, "            printf(\" \");\n");
    fprintf(out, "        }\n");
    fprintf(out, "        printf(\"R%%d=%%s\", i, value);\n");
    fprintf(out, "    }\n");
    fprintf(out, "    printf(\"\\n\");\n");
    fprintf(out, "    plankac_destroy(ctx);\n");
    fprintf(out, "    return 0;\n");
    fprintf(out, "}\n");

    fclose(tmp);
    fclose(out);
    remove(tmp_path);
    return 1;
}

int plc_emit_asm_image(const PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size)
{
    FILE *out;
    FILE *tmp;
    char tmp_path[PLC_MAX_LINE];
    char line[PLC_MAX_LINE];
    int column;

    if (strlen(path) + 5 >= sizeof(tmp_path)) {
        plc_set_error(err, err_size, "ASM image output path too long");
        return 0;
    }
    strcpy(tmp_path, path);
    strcat(tmp_path, ".pbc");
    if (!plc_emit_bytecode(program, tmp_path, err, err_size)) {
        return 0;
    }
    out = fopen(path, "w");
    if (out == 0) {
        remove(tmp_path);
        sprintf(err, "cannot open ASM image output: %s", path);
        return 0;
    }
    tmp = fopen(tmp_path, "r");
    if (tmp == 0) {
        fclose(out);
        remove(tmp_path);
        sprintf(err, "cannot read generated bytecode: %s", tmp_path);
        return 0;
    }

    fprintf(out, "; Generated by PlankaC.\n");
    fprintf(out, "; This is an assembly bytecode image, not a standalone\n");
    fprintf(out, "; 8086 lowering of every Plankalkuel operation.\n");
    fprintf(out, "PUBLIC plankac_bytecode_image\n");
    fprintf(out, "PUBLIC plankac_bytecode_image_end\n");
    fprintf(out, ".DATA\n");
    fprintf(out, "plankac_bytecode_image LABEL BYTE\n");
    while (fgets(line, sizeof(line), tmp) != 0) {
        unsigned i;

        column = 0;
        fprintf(out, "    DB ");
        for (i = 0; line[i] != '\0'; ++i) {
            if (column > 0) {
                fprintf(out, ",");
            }
            fprintf(out, "%u", (unsigned char)line[i]);
            ++column;
            if (column >= 16 && line[i + 1] != '\0') {
                fprintf(out, "\n    DB ");
                column = 0;
            }
        }
        if (column > 0) {
            fprintf(out, "\n");
        }
    }
    fprintf(out, "    DB 0\n");
    fprintf(out, "plankac_bytecode_image_end LABEL BYTE\n");
    fprintf(out, "END\n");

    fclose(tmp);
    fclose(out);
    remove(tmp_path);
    return 1;
}

typedef struct PLC_ASM_REF {
    char bank;
    int index;
    int subscript;
    char field[PLC_MAX_FIELD_NAME];
} PLC_ASM_REF;

typedef struct PLC_ASM_COMPILER {
    FILE *fp;
    const PLC_PROGRAM *program;
    int label_id;
    int temp_id;
    char *err;
    unsigned err_size;
} PLC_ASM_COMPILER;

static const char *plc_asm_skip(const char *p)
{
    while (*p == ' ' || *p == '\t' || *p == '\r' || *p == '\n') {
        ++p;
    }
    return p;
}

static void plc_asm_label(char *out, unsigned out_size,
    const char *prefix, int id)
{
    (void)out_size;
    sprintf(out, ".L_%s_%d", prefix, id);
}

static int plc_asm_string(PLC_ASM_COMPILER *ctx, const char *text,
    char *out, unsigned out_size)
{
    int id;

    id = ctx->label_id++;
    plc_asm_label(out, out_size, "str", id);
    fprintf(ctx->fp, ".section .rdata,\"dr\"\n");
    fprintf(ctx->fp, "%s:\n", out);
    fprintf(ctx->fp, "    .asciz ");
    plc_write_asm_quoted(ctx->fp, text);
    fprintf(ctx->fp, "\n.text\n");
    return 1;
}

static void plc_asm_load_double(PLC_ASM_COMPILER *ctx, double value)
{
    union {
        double d;
        unsigned long long u;
    } bits;

    bits.d = value;
    fprintf(ctx->fp, "    movabs rax, 0x%016llX\n", bits.u);
    fprintf(ctx->fp, "    movq xmm0, rax\n");
}

static int plc_asm_parse_ref(const char **pp, PLC_ASM_REF *ref)
{
    const char *p;
    char *endptr;
    int n;

    p = plc_asm_skip(*pp);
    memset(ref, 0, sizeof(*ref));
    ref->subscript = -1;
    if (*p != 'V' && *p != 'Z' && *p != 'R') {
        return 0;
    }
    ref->bank = *p;
    ++p;
    if (!isdigit((unsigned char)*p)) {
        return 0;
    }
    ref->index = (int)strtol(p, &endptr, 10);
    p = endptr;
    p = plc_asm_skip(p);
    if (*p == '[' && p[1] != ':') {
        ++p;
        ref->subscript = (int)strtol(p, &endptr, 10);
        p = endptr;
        p = plc_asm_skip(p);
        if (*p != ']') {
            return 0;
        }
        ++p;
        p = plc_asm_skip(p);
    }
    n = 0;
    while (*p == '.') {
        if (n + 1 < (int)sizeof(ref->field)) {
            ref->field[n++] = *p;
        }
        ++p;
        if (!isalpha((unsigned char)*p) && *p != '_') {
            return 0;
        }
        while (isalnum((unsigned char)*p) || *p == '_') {
            if (n + 1 < (int)sizeof(ref->field)) {
                ref->field[n++] = *p;
            }
            ++p;
        }
        p = plc_asm_skip(p);
    }
    ref->field[n] = '\0';
    if (*p == '[') {
        p = plc_skip_type_marker(p);
    }
    *pp = p;
    return 1;
}

static int plc_asm_temp_alloc(PLC_ASM_COMPILER *ctx)
{
    int offset;

    offset = 800 + ctx->temp_id * 8;
    ++ctx->temp_id;
    return offset;
}

static void plc_asm_temp_free(PLC_ASM_COMPILER *ctx)
{
    if (ctx->temp_id > 0) {
        --ctx->temp_id;
    }
}

static void plc_asm_emit_get_ref(PLC_ASM_COMPILER *ctx,
    const PLC_ASM_REF *ref)
{
    char field_label[64];

    fprintf(ctx->fp, "    mov rcx, QWORD PTR [rbp-24]\n");
    fprintf(ctx->fp, "    mov edx, %d\n", (int)ref->bank);
    fprintf(ctx->fp, "    mov r8d, %d\n", ref->index);
    fprintf(ctx->fp, "    mov r9d, %d\n", ref->subscript);
    if (ref->field[0] != '\0') {
        plc_asm_string(ctx, ref->field, field_label, sizeof(field_label));
        fprintf(ctx->fp, "    lea rax, [rip+%s]\n", field_label);
    } else {
        fprintf(ctx->fp, "    xor eax, eax\n");
    }
    fprintf(ctx->fp, "    mov QWORD PTR [rsp+32], rax\n");
    fprintf(ctx->fp, "    call plc_native_get\n");
}

static void plc_asm_emit_set_ref(PLC_ASM_COMPILER *ctx,
    const PLC_ASM_REF *ref)
{
    char field_label[64];

    fprintf(ctx->fp, "    movsd QWORD PTR [rbp-32], xmm0\n");
    fprintf(ctx->fp, "    mov rcx, QWORD PTR [rbp-24]\n");
    fprintf(ctx->fp, "    mov edx, %d\n", (int)ref->bank);
    fprintf(ctx->fp, "    mov r8d, %d\n", ref->index);
    fprintf(ctx->fp, "    mov r9d, %d\n", ref->subscript);
    if (ref->field[0] != '\0') {
        plc_asm_string(ctx, ref->field, field_label, sizeof(field_label));
        fprintf(ctx->fp, "    lea rax, [rip+%s]\n", field_label);
    } else {
        fprintf(ctx->fp, "    xor eax, eax\n");
    }
    fprintf(ctx->fp, "    mov QWORD PTR [rsp+32], rax\n");
    fprintf(ctx->fp, "    lea rax, [rbp-32]\n");
    fprintf(ctx->fp, "    mov QWORD PTR [rsp+40], rax\n");
    fprintf(ctx->fp, "    call plc_native_set\n");
}

static int plc_asm_emit_expr(PLC_ASM_COMPILER *ctx, const char **pp);

static int plc_asm_emit_args(PLC_ASM_COMPILER *ctx, const char **pp,
    int base_offset, int *argc)
{
    const char *p;

    p = plc_asm_skip(*pp);
    if (*p != '(') {
        plc_set_error(ctx->err, ctx->err_size, "ASM expected '('");
        return 0;
    }
    ++p;
    *argc = 0;
    p = plc_asm_skip(p);
    if (*p == ')') {
        *pp = p + 1;
        return 1;
    }
    while (1) {
        if (*argc >= PLC_MAX_ARGS) {
            plc_set_error(ctx->err, ctx->err_size, "too many ASM call args");
            return 0;
        }
        if (!plc_asm_emit_expr(ctx, &p)) {
            return 0;
        }
        fprintf(ctx->fp, "    movsd QWORD PTR [rbp-%d], xmm0\n",
            base_offset - (*argc * 8));
        ++(*argc);
        p = plc_asm_skip(p);
        if (*p == ',') {
            ++p;
            continue;
        }
        if (*p == ')') {
            *pp = p + 1;
            return 1;
        }
        plc_set_error(ctx->err, ctx->err_size, "bad ASM call args");
        return 0;
    }
}

static int plc_asm_emit_call_value(PLC_ASM_COMPILER *ctx, const char *name,
    const char *args_text, int result_index)
{
    const PLC_PROC *proc;
    const char *p;
    char name_label[64];
    int argc;

    p = args_text;
    if (!plc_asm_emit_args(ctx, &p, 192, &argc)) {
        return 0;
    }
    proc = plc_find_proc(ctx->program, name);
    if (proc != 0) {
        fprintf(ctx->fp, "    lea rcx, [rbp-192]\n");
        fprintf(ctx->fp, "    lea rdx, [rbp-384]\n");
        fprintf(ctx->fp, "    mov r8, QWORD PTR [rbp-24]\n");
        fprintf(ctx->fp, "    call plc_native_p%d\n", proc->number);
        fprintf(ctx->fp, "    movsd xmm0, QWORD PTR [rbp-%d]\n",
            384 - result_index * 8);
        return 1;
    }
    plc_asm_string(ctx, name, name_label, sizeof(name_label));
    fprintf(ctx->fp, "    mov rcx, QWORD PTR [rbp-24]\n");
    fprintf(ctx->fp, "    lea rdx, [rip+%s]\n", name_label);
    fprintf(ctx->fp, "    lea r8, [rbp-192]\n");
    fprintf(ctx->fp, "    mov r9d, %d\n", argc);
    fprintf(ctx->fp, "    mov QWORD PTR [rsp+32], %d\n", result_index);
    fprintf(ctx->fp, "    call plc_native_builtin\n");
    return 1;
}

static int plc_asm_parse_identifier(const char **pp, char *name,
    unsigned name_size)
{
    const char *p;
    unsigned n;

    p = plc_asm_skip(*pp);
    if (!isalpha((unsigned char)*p) && *p != '_') {
        return 0;
    }
    n = 0;
    while (isalnum((unsigned char)*p) || *p == '_') {
        if (n + 1 < name_size) {
            name[n++] = *p;
        }
        ++p;
    }
    name[n] = '\0';
    *pp = p;
    return 1;
}

static int plc_asm_emit_primary(PLC_ASM_COMPILER *ctx, const char **pp)
{
    const char *p;
    PLC_ASM_REF ref;
    char name[PLC_MAX_NAME];
    char *endptr;
    double value;

    p = plc_asm_skip(*pp);
    if (*p == '(') {
        ++p;
        if (!plc_asm_emit_expr(ctx, &p)) {
            return 0;
        }
        p = plc_asm_skip(p);
        if (*p != ')') {
            plc_set_error(ctx->err, ctx->err_size, "ASM expected ')'");
            return 0;
        }
        *pp = p + 1;
        return 1;
    }
    if (*p == 'V' || *p == 'Z' || *p == 'R') {
        if (!plc_asm_parse_ref(&p, &ref)) {
            plc_set_error(ctx->err, ctx->err_size, "bad ASM ref");
            return 0;
        }
        plc_asm_emit_get_ref(ctx, &ref);
        *pp = p;
        return 1;
    }
    if (isdigit((unsigned char)*p) || *p == '.') {
        value = strtod(p, &endptr);
        if (endptr == p) {
            plc_set_error(ctx->err, ctx->err_size, "bad ASM number");
            return 0;
        }
        p = plc_skip_type_marker(endptr);
        plc_asm_load_double(ctx, value);
        *pp = p;
        return 1;
    }
    if (plc_asm_parse_identifier(&p, name, sizeof(name))) {
        const char *open;
        const char *close;
        char args_text[PLC_MAX_LINE];

        p = plc_asm_skip(p);
        if (*p != '(') {
            plc_set_error(ctx->err, ctx->err_size, "ASM expected call");
            return 0;
        }
        open = p;
        close = plc_matching_paren(open);
        if (close == 0) {
            plc_set_error(ctx->err, ctx->err_size, "ASM bad call");
            return 0;
        }
        plc_copy_range(args_text, sizeof(args_text), open, close + 1);
        if (!plc_asm_emit_call_value(ctx, name, args_text, 0)) {
            return 0;
        }
        *pp = close + 1;
        return 1;
    }
    plc_set_error(ctx->err, ctx->err_size, "ASM expected expression");
    return 0;
}

static int plc_asm_emit_unary(PLC_ASM_COMPILER *ctx, const char **pp)
{
    const char *p;

    p = plc_asm_skip(*pp);
    if (*p == '-') {
        ++p;
        if (!plc_asm_emit_unary(ctx, &p)) {
            return 0;
        }
        fprintf(ctx->fp, "    xorpd xmm1, xmm1\n");
        fprintf(ctx->fp, "    subsd xmm1, xmm0\n");
        fprintf(ctx->fp, "    movapd xmm0, xmm1\n");
        *pp = p;
        return 1;
    }
    if (*p == '!') {
        int ok_label;
        int end_label;

        ++p;
        if (!plc_asm_emit_unary(ctx, &p)) {
            return 0;
        }
        ok_label = ctx->label_id++;
        end_label = ctx->label_id++;
        fprintf(ctx->fp, "    xorpd xmm1, xmm1\n");
        fprintf(ctx->fp, "    ucomisd xmm0, xmm1\n");
        fprintf(ctx->fp, "    je .L_bool_true_%d\n", ok_label);
        plc_asm_load_double(ctx, 0.0);
        fprintf(ctx->fp, "    jmp .L_bool_end_%d\n", end_label);
        fprintf(ctx->fp, ".L_bool_true_%d:\n", ok_label);
        plc_asm_load_double(ctx, 1.0);
        fprintf(ctx->fp, ".L_bool_end_%d:\n", end_label);
        *pp = p;
        return 1;
    }
    return plc_asm_emit_primary(ctx, pp);
}

static int plc_asm_emit_power(PLC_ASM_COMPILER *ctx, const char **pp)
{
    const char *p;

    p = *pp;
    if (!plc_asm_emit_unary(ctx, &p)) {
        return 0;
    }
    p = plc_asm_skip(p);
    while (*p == '^') {
        int temp;

        ++p;
        temp = plc_asm_temp_alloc(ctx);
        fprintf(ctx->fp, "    movsd QWORD PTR [rbp-%d], xmm0\n", temp);
        if (!plc_asm_emit_unary(ctx, &p)) {
            return 0;
        }
        fprintf(ctx->fp, "    movapd xmm1, xmm0\n");
        fprintf(ctx->fp, "    movsd xmm0, QWORD PTR [rbp-%d]\n", temp);
        fprintf(ctx->fp, "    call pow\n");
        plc_asm_temp_free(ctx);
        p = plc_asm_skip(p);
    }
    *pp = p;
    return 1;
}

static int plc_asm_emit_mul(PLC_ASM_COMPILER *ctx, const char **pp)
{
    const char *p;

    p = *pp;
    if (!plc_asm_emit_power(ctx, &p)) {
        return 0;
    }
    p = plc_asm_skip(p);
    while (*p == '*' || *p == '/' || *p == '%') {
        char op;
        int temp;

        op = *p++;
        temp = plc_asm_temp_alloc(ctx);
        fprintf(ctx->fp, "    movsd QWORD PTR [rbp-%d], xmm0\n", temp);
        if (!plc_asm_emit_power(ctx, &p)) {
            return 0;
        }
        if (op == '%') {
            fprintf(ctx->fp, "    movapd xmm1, xmm0\n");
            fprintf(ctx->fp, "    movsd xmm0, QWORD PTR [rbp-%d]\n", temp);
            fprintf(ctx->fp, "    call fmod\n");
        } else {
            fprintf(ctx->fp, "    movsd xmm1, QWORD PTR [rbp-%d]\n", temp);
            if (op == '*') {
                fprintf(ctx->fp, "    mulsd xmm1, xmm0\n");
            } else {
                fprintf(ctx->fp, "    divsd xmm1, xmm0\n");
            }
            fprintf(ctx->fp, "    movapd xmm0, xmm1\n");
        }
        plc_asm_temp_free(ctx);
        p = plc_asm_skip(p);
    }
    *pp = p;
    return 1;
}

static int plc_asm_emit_add(PLC_ASM_COMPILER *ctx, const char **pp)
{
    const char *p;

    p = *pp;
    if (!plc_asm_emit_mul(ctx, &p)) {
        return 0;
    }
    p = plc_asm_skip(p);
    while (*p == '+' || *p == '-') {
        char op;
        int temp;

        op = *p++;
        temp = plc_asm_temp_alloc(ctx);
        fprintf(ctx->fp, "    movsd QWORD PTR [rbp-%d], xmm0\n", temp);
        if (!plc_asm_emit_mul(ctx, &p)) {
            return 0;
        }
        fprintf(ctx->fp, "    movsd xmm1, QWORD PTR [rbp-%d]\n", temp);
        if (op == '+') {
            fprintf(ctx->fp, "    addsd xmm1, xmm0\n");
        } else {
            fprintf(ctx->fp, "    subsd xmm1, xmm0\n");
        }
        fprintf(ctx->fp, "    movapd xmm0, xmm1\n");
        plc_asm_temp_free(ctx);
        p = plc_asm_skip(p);
    }
    *pp = p;
    return 1;
}

static int plc_asm_match_compare(const char **pp, char *op)
{
    const char *p;

    p = plc_asm_skip(*pp);
    if (p[0] == '!' && p[1] == '=') {
        *op = 'n';
        *pp = p + 2;
        return 1;
    }
    if (p[0] == '<' && p[1] == '=') {
        *op = 'l';
        *pp = p + 2;
        return 1;
    }
    if (p[0] == '>' && p[1] == '=') {
        *op = 'g';
        *pp = p + 2;
        return 1;
    }
    if (*p == '=' || *p == '<' || *p == '>') {
        *op = *p;
        *pp = p + 1;
        return 1;
    }
    return 0;
}

static int plc_asm_emit_compare_expr(PLC_ASM_COMPILER *ctx, const char **pp)
{
    const char *p;
    char op;

    p = *pp;
    if (!plc_asm_emit_add(ctx, &p)) {
        return 0;
    }
    while (plc_asm_match_compare(&p, &op)) {
        int temp;
        int true_label;
        int end_label;

        temp = plc_asm_temp_alloc(ctx);
        fprintf(ctx->fp, "    movsd QWORD PTR [rbp-%d], xmm0\n", temp);
        if (!plc_asm_emit_add(ctx, &p)) {
            return 0;
        }
        true_label = ctx->label_id++;
        end_label = ctx->label_id++;
        fprintf(ctx->fp, "    movsd xmm1, QWORD PTR [rbp-%d]\n", temp);
        fprintf(ctx->fp, "    ucomisd xmm1, xmm0\n");
        if (op == '=') {
            fprintf(ctx->fp, "    je .L_cmp_true_%d\n", true_label);
        } else if (op == 'n') {
            fprintf(ctx->fp, "    jne .L_cmp_true_%d\n", true_label);
        } else if (op == '<') {
            fprintf(ctx->fp, "    jb .L_cmp_true_%d\n", true_label);
        } else if (op == 'l') {
            fprintf(ctx->fp, "    jbe .L_cmp_true_%d\n", true_label);
        } else if (op == '>') {
            fprintf(ctx->fp, "    ja .L_cmp_true_%d\n", true_label);
        } else {
            fprintf(ctx->fp, "    jae .L_cmp_true_%d\n", true_label);
        }
        plc_asm_load_double(ctx, 0.0);
        fprintf(ctx->fp, "    jmp .L_cmp_end_%d\n", end_label);
        fprintf(ctx->fp, ".L_cmp_true_%d:\n", true_label);
        plc_asm_load_double(ctx, 1.0);
        fprintf(ctx->fp, ".L_cmp_end_%d:\n", end_label);
        plc_asm_temp_free(ctx);
        p = plc_asm_skip(p);
    }
    *pp = p;
    return 1;
}

static int plc_asm_emit_and(PLC_ASM_COMPILER *ctx, const char **pp)
{
    const char *p;

    p = *pp;
    if (!plc_asm_emit_compare_expr(ctx, &p)) {
        return 0;
    }
    p = plc_asm_skip(p);
    while (*p == '&') {
        int temp;
        int false_label;
        int end_label;

        ++p;
        temp = plc_asm_temp_alloc(ctx);
        fprintf(ctx->fp, "    movsd QWORD PTR [rbp-%d], xmm0\n", temp);
        if (!plc_asm_emit_compare_expr(ctx, &p)) {
            return 0;
        }
        false_label = ctx->label_id++;
        end_label = ctx->label_id++;
        fprintf(ctx->fp, "    xorpd xmm2, xmm2\n");
        fprintf(ctx->fp, "    ucomisd xmm0, xmm2\n");
        fprintf(ctx->fp, "    je .L_and_false_%d\n", false_label);
        fprintf(ctx->fp, "    movsd xmm1, QWORD PTR [rbp-%d]\n", temp);
        fprintf(ctx->fp, "    ucomisd xmm1, xmm2\n");
        fprintf(ctx->fp, "    je .L_and_false_%d\n", false_label);
        plc_asm_load_double(ctx, 1.0);
        fprintf(ctx->fp, "    jmp .L_and_end_%d\n", end_label);
        fprintf(ctx->fp, ".L_and_false_%d:\n", false_label);
        plc_asm_load_double(ctx, 0.0);
        fprintf(ctx->fp, ".L_and_end_%d:\n", end_label);
        plc_asm_temp_free(ctx);
        p = plc_asm_skip(p);
    }
    *pp = p;
    return 1;
}

static int plc_asm_emit_expr(PLC_ASM_COMPILER *ctx, const char **pp)
{
    const char *p;

    p = *pp;
    if (!plc_asm_emit_and(ctx, &p)) {
        return 0;
    }
    p = plc_asm_skip(p);
    while (*p == '|') {
        int temp;
        int true_label;
        int end_label;

        ++p;
        temp = plc_asm_temp_alloc(ctx);
        fprintf(ctx->fp, "    movsd QWORD PTR [rbp-%d], xmm0\n", temp);
        if (!plc_asm_emit_and(ctx, &p)) {
            return 0;
        }
        true_label = ctx->label_id++;
        end_label = ctx->label_id++;
        fprintf(ctx->fp, "    xorpd xmm2, xmm2\n");
        fprintf(ctx->fp, "    ucomisd xmm0, xmm2\n");
        fprintf(ctx->fp, "    jne .L_or_true_%d\n", true_label);
        fprintf(ctx->fp, "    movsd xmm1, QWORD PTR [rbp-%d]\n", temp);
        fprintf(ctx->fp, "    ucomisd xmm1, xmm2\n");
        fprintf(ctx->fp, "    jne .L_or_true_%d\n", true_label);
        plc_asm_load_double(ctx, 0.0);
        fprintf(ctx->fp, "    jmp .L_or_end_%d\n", end_label);
        fprintf(ctx->fp, ".L_or_true_%d:\n", true_label);
        plc_asm_load_double(ctx, 1.0);
        fprintf(ctx->fp, ".L_or_end_%d:\n", end_label);
        plc_asm_temp_free(ctx);
        p = plc_asm_skip(p);
    }
    *pp = p;
    return 1;
}

static int plc_asm_emit_expr_text(PLC_ASM_COMPILER *ctx, const char *text)
{
    const char *p;

    p = text;
    if (!plc_asm_emit_expr(ctx, &p)) {
        return 0;
    }
    p = plc_asm_skip(p);
    if (*p != '\0') {
        plc_set_error(ctx->err, ctx->err_size,
            "unexpected text after ASM expression");
        return 0;
    }
    return 1;
}

static int plc_asm_assign_one(PLC_ASM_COMPILER *ctx, const char *target)
{
    PLC_ASM_REF ref;
    const char *p;

    p = target;
    if (!plc_asm_parse_ref(&p, &ref)) {
        plc_set_error(ctx->err, ctx->err_size, "bad ASM target");
        return 0;
    }
    plc_asm_emit_set_ref(ctx, &ref);
    return 1;
}

static int plc_asm_assign_result_index(PLC_ASM_COMPILER *ctx,
    const char *target, int result_index)
{
    fprintf(ctx->fp, "    movsd xmm0, QWORD PTR [rbp-%d]\n",
        384 - result_index * 8);
    return plc_asm_assign_one(ctx, target);
}

static int plc_asm_emit_call_stmt(PLC_ASM_COMPILER *ctx, const char *name,
    const char *args_text, const char *targets)
{
    const PLC_PROC *proc;
    const char *p;
    char target[PLC_MAX_LINE];
    int argc;
    int result_index;
    char name_label[64];

    p = args_text;
    if (!plc_asm_emit_args(ctx, &p, 192, &argc)) {
        return 0;
    }
    proc = plc_find_proc(ctx->program, name);
    if (proc != 0) {
        fprintf(ctx->fp, "    lea rcx, [rbp-192]\n");
        fprintf(ctx->fp, "    lea rdx, [rbp-384]\n");
        fprintf(ctx->fp, "    mov r8, QWORD PTR [rbp-24]\n");
        fprintf(ctx->fp, "    call plc_native_p%d\n", proc->number);
    } else {
        plc_asm_string(ctx, name, name_label, sizeof(name_label));
        fprintf(ctx->fp, "    mov rcx, QWORD PTR [rbp-24]\n");
        fprintf(ctx->fp, "    lea rdx, [rip+%s]\n", name_label);
        fprintf(ctx->fp, "    lea r8, [rbp-192]\n");
        fprintf(ctx->fp, "    mov r9d, %d\n", argc);
        fprintf(ctx->fp, "    mov QWORD PTR [rsp+32], 0\n");
        fprintf(ctx->fp, "    call plc_native_builtin\n");
        fprintf(ctx->fp, "    movsd QWORD PTR [rbp-384], xmm0\n");
    }
    p = targets;
    result_index = 0;
    while (*p != '\0') {
        const char *comma;

        comma = strchr(p, ',');
        if (comma == 0) {
            plc_copy_range(target, sizeof(target), p, p + strlen(p));
            p += strlen(p);
        } else {
            plc_copy_range(target, sizeof(target), p, comma);
            p = comma + 1;
        }
        plc_trim_in_place(target);
        if (!plc_asm_assign_result_index(ctx, target, result_index)) {
            return 0;
        }
        ++result_index;
        p = plc_asm_skip(p);
    }
    return 1;
}

static int plc_asm_emit_value_to_targets(PLC_ASM_COMPILER *ctx,
    const char *value_text, const char *targets)
{
    char call_name[PLC_MAX_NAME];
    char call_args[PLC_MAX_LINE];

    if (plc_is_top_call(value_text, call_name, sizeof(call_name),
            call_args, sizeof(call_args))) {
        char wrapped_args[PLC_MAX_LINE];

        if (strlen(call_args) + 3 >= sizeof(wrapped_args)) {
            plc_set_error(ctx->err, ctx->err_size, "ASM call args too long");
            return 0;
        }
        strcpy(wrapped_args, "(");
        strcat(wrapped_args, call_args);
        strcat(wrapped_args, ")");
        return plc_asm_emit_call_stmt(ctx, call_name, wrapped_args, targets);
    }
    if (!plc_asm_emit_expr_text(ctx, value_text)) {
        return 0;
    }
    return plc_asm_assign_one(ctx, targets);
}

static int plc_asm_emit_statement(PLC_ASM_COMPILER *ctx,
    const PLC_STMT *stmt)
{
    char parts[5][PLC_MAX_LINE];
    int count;

    if (plc_line_starts_with(stmt->text, "ASSERT")) {
        const char *expr;
        int ok_label;

        expr = plc_skip_space(plc_skip_space(stmt->text) + 6);
        if (!plc_asm_emit_expr_text(ctx, expr)) {
            return 0;
        }
        ok_label = ctx->label_id++;
        fprintf(ctx->fp, "    xorpd xmm1, xmm1\n");
        fprintf(ctx->fp, "    ucomisd xmm0, xmm1\n");
        fprintf(ctx->fp, "    jne .L_assert_ok_%d\n", ok_label);
        fprintf(ctx->fp, "    int3\n");
        fprintf(ctx->fp, ".L_assert_ok_%d:\n", ok_label);
        return 1;
    }
    if (!plc_split_arrows(stmt->text, parts, &count)) {
        plc_set_error(ctx->err, ctx->err_size, "bad ASM arrow chain");
        return 0;
    }
    if (count == 2) {
        return plc_asm_emit_value_to_targets(ctx, parts[0], parts[1]);
    }
    if (count == 3 && plc_line_starts_with(parts[0], "LOOP")) {
        const char *expr;
        int loop_label;
        int end_label;

        expr = plc_skip_space(plc_skip_space(parts[0]) + 4);
        if (!plc_asm_emit_expr_text(ctx, expr)) {
            return 0;
        }
        fprintf(ctx->fp, "    cvttsd2si eax, xmm0\n");
        fprintf(ctx->fp, "    mov DWORD PTR [rbp-40], eax\n");
        loop_label = ctx->label_id++;
        end_label = ctx->label_id++;
        fprintf(ctx->fp, ".L_loop_%d:\n", loop_label);
        fprintf(ctx->fp, "    cmp DWORD PTR [rbp-40], 0\n");
        fprintf(ctx->fp, "    jle .L_loop_end_%d\n", end_label);
        if (!plc_asm_emit_value_to_targets(ctx, parts[1], parts[2])) {
            return 0;
        }
        fprintf(ctx->fp, "    sub DWORD PTR [rbp-40], 1\n");
        fprintf(ctx->fp, "    jmp .L_loop_%d\n", loop_label);
        fprintf(ctx->fp, ".L_loop_end_%d:\n", end_label);
        return 1;
    }
    if (count == 3) {
        int end_label;

        if (!plc_asm_emit_expr_text(ctx, parts[0])) {
            return 0;
        }
        end_label = ctx->label_id++;
        fprintf(ctx->fp, "    xorpd xmm1, xmm1\n");
        fprintf(ctx->fp, "    ucomisd xmm0, xmm1\n");
        fprintf(ctx->fp, "    je .L_guard_end_%d\n", end_label);
        if (!plc_asm_emit_value_to_targets(ctx, parts[1], parts[2])) {
            return 0;
        }
        fprintf(ctx->fp, ".L_guard_end_%d:\n", end_label);
        return 1;
    }
    plc_set_error(ctx->err, ctx->err_size, "bad ASM statement");
    return 0;
}

static int plc_asm_emit_proc(PLC_ASM_COMPILER *ctx, const PLC_PROC *proc)
{
    int i;
    int end_label;

    end_label = ctx->label_id++;
    ctx->temp_id = 0;
    fprintf(ctx->fp, ".globl plc_native_p%d\n", proc->number);
    fprintf(ctx->fp, "plc_native_p%d:\n", proc->number);
    fprintf(ctx->fp, "    push rbp\n");
    fprintf(ctx->fp, "    mov rbp, rsp\n");
    fprintf(ctx->fp, "    sub rsp, 4096\n");
    fprintf(ctx->fp, "    mov QWORD PTR [rbp-8], rcx\n");
    fprintf(ctx->fp, "    mov QWORD PTR [rbp-16], rdx\n");
    fprintf(ctx->fp, "    mov rcx, r8\n");
    fprintf(ctx->fp, "    call plc_native_frame_create_child\n");
    fprintf(ctx->fp, "    mov QWORD PTR [rbp-24], rax\n");
    for (i = 0; i < proc->argc; ++i) {
        PLC_ASM_REF ref;

        memset(&ref, 0, sizeof(ref));
        ref.bank = 'V';
        ref.index = i;
        ref.subscript = -1;
        fprintf(ctx->fp, "    mov rax, QWORD PTR [rbp-8]\n");
        fprintf(ctx->fp, "    movsd xmm0, QWORD PTR [rax+%d]\n", i * 8);
        plc_asm_emit_set_ref(ctx, &ref);
    }
    for (i = 0; i < proc->stmt_count; ++i) {
        if (!plc_asm_emit_statement(ctx, &proc->stmts[i])) {
            char prefix[160];

            sprintf(prefix, "P%d %s line %d: ",
                proc->number, proc->name, proc->stmts[i].line_no);
            plc_prefix_error(ctx->err, ctx->err_size, prefix);
            return 0;
        }
    }
    fprintf(ctx->fp, ".L_proc_end_%d:\n", end_label);
    for (i = 0; i < proc->results; ++i) {
        PLC_ASM_REF ref;

        memset(&ref, 0, sizeof(ref));
        ref.bank = 'R';
        ref.index = i;
        ref.subscript = -1;
        plc_asm_emit_get_ref(ctx, &ref);
        fprintf(ctx->fp, "    mov rax, QWORD PTR [rbp-16]\n");
        fprintf(ctx->fp, "    movsd QWORD PTR [rax+%d], xmm0\n", i * 8);
    }
    fprintf(ctx->fp, "    mov rcx, QWORD PTR [rbp-24]\n");
    fprintf(ctx->fp, "    call plc_native_frame_destroy\n");
    fprintf(ctx->fp, "    add rsp, 4096\n");
    fprintf(ctx->fp, "    pop rbp\n");
    fprintf(ctx->fp, "    ret\n");
    return 1;
}

static void plc_asm_emit_main(PLC_ASM_COMPILER *ctx)
{
    int i;

    fprintf(ctx->fp, ".globl main\n");
    fprintf(ctx->fp, "main:\n");
    fprintf(ctx->fp, "    push rbp\n");
    fprintf(ctx->fp, "    mov rbp, rsp\n");
    fprintf(ctx->fp, "    sub rsp, 2048\n");
    fprintf(ctx->fp, "    mov DWORD PTR [rbp-4], ecx\n");
    fprintf(ctx->fp, "    mov QWORD PTR [rbp-16], rdx\n");
    fprintf(ctx->fp, "    cmp ecx, 2\n");
    fprintf(ctx->fp, "    jge .L_main_args_ok\n");
    fprintf(ctx->fp, "    mov rax, QWORD PTR [rbp-16]\n");
    fprintf(ctx->fp, "    lea rcx, [rip+usage_msg]\n");
    fprintf(ctx->fp, "    mov rdx, QWORD PTR [rax]\n");
    fprintf(ctx->fp, "    call printf\n");
    fprintf(ctx->fp, "    mov eax, 1\n");
    fprintf(ctx->fp, "    jmp .L_main_return\n");
    fprintf(ctx->fp, ".L_main_args_ok:\n");
    fprintf(ctx->fp, "    mov eax, DWORD PTR [rbp-4]\n");
    fprintf(ctx->fp, "    sub eax, 2\n");
    fprintf(ctx->fp, "    cmp eax, 16\n");
    fprintf(ctx->fp, "    jle .L_parse_args\n");
    fprintf(ctx->fp, "    lea rcx, [rip+too_many_msg]\n");
    fprintf(ctx->fp, "    call printf\n");
    fprintf(ctx->fp, "    mov eax, 1\n");
    fprintf(ctx->fp, "    jmp .L_main_return\n");
    fprintf(ctx->fp, ".L_parse_args:\n");
    fprintf(ctx->fp, "    mov DWORD PTR [rbp-32], 2\n");
    fprintf(ctx->fp, ".L_parse_loop:\n");
    fprintf(ctx->fp, "    mov eax, DWORD PTR [rbp-32]\n");
    fprintf(ctx->fp, "    cmp eax, DWORD PTR [rbp-4]\n");
    fprintf(ctx->fp, "    jge .L_dispatch\n");
    fprintf(ctx->fp, "    mov rdx, QWORD PTR [rbp-16]\n");
    fprintf(ctx->fp, "    mov rcx, QWORD PTR [rdx+rax*8]\n");
    fprintf(ctx->fp, "    xor edx, edx\n");
    fprintf(ctx->fp, "    call strtod\n");
    fprintf(ctx->fp, "    mov eax, DWORD PTR [rbp-32]\n");
    fprintf(ctx->fp, "    sub eax, 2\n");
    fprintf(ctx->fp, "    lea rdx, [rbp-512]\n");
    fprintf(ctx->fp, "    movsd QWORD PTR [rdx+rax*8], xmm0\n");
    fprintf(ctx->fp, "    add DWORD PTR [rbp-32], 1\n");
    fprintf(ctx->fp, "    jmp .L_parse_loop\n");
    fprintf(ctx->fp, ".L_dispatch:\n");
    for (i = 0; i < ctx->program->proc_count; ++i) {
        const PLC_PROC *proc;
        char name_label[64];
        char p_label[64];

        proc = &ctx->program->procs[i];
        plc_asm_string(ctx, proc->name, name_label, sizeof(name_label));
        fprintf(ctx->fp, "    mov rax, QWORD PTR [rbp-16]\n");
        fprintf(ctx->fp, "    mov rcx, QWORD PTR [rax+8]\n");
        fprintf(ctx->fp, "    lea rdx, [rip+%s]\n", name_label);
        fprintf(ctx->fp, "    call strcmp\n");
        fprintf(ctx->fp, "    test eax, eax\n");
        fprintf(ctx->fp, "    je .L_call_proc_%d\n", proc->number);
        sprintf(p_label, "P%d", proc->number);
        plc_asm_string(ctx, p_label, name_label, sizeof(name_label));
        fprintf(ctx->fp, "    mov rax, QWORD PTR [rbp-16]\n");
        fprintf(ctx->fp, "    mov rcx, QWORD PTR [rax+8]\n");
        fprintf(ctx->fp, "    lea rdx, [rip+%s]\n", name_label);
        fprintf(ctx->fp, "    call strcmp\n");
        fprintf(ctx->fp, "    test eax, eax\n");
        fprintf(ctx->fp, "    je .L_call_proc_%d\n", proc->number);
    }
    fprintf(ctx->fp, "    lea rcx, [rip+unknown_msg]\n");
    fprintf(ctx->fp, "    mov rax, QWORD PTR [rbp-16]\n");
    fprintf(ctx->fp, "    mov rdx, QWORD PTR [rax+8]\n");
    fprintf(ctx->fp, "    call printf\n");
    fprintf(ctx->fp, "    mov eax, 1\n");
    fprintf(ctx->fp, "    jmp .L_main_return\n");
    for (i = 0; i < ctx->program->proc_count; ++i) {
        const PLC_PROC *proc;

        proc = &ctx->program->procs[i];
        fprintf(ctx->fp, ".L_call_proc_%d:\n", proc->number);
        fprintf(ctx->fp, "    lea rcx, [rbp-512]\n");
        fprintf(ctx->fp, "    lea rdx, [rbp-384]\n");
        fprintf(ctx->fp, "    xor r8d, r8d\n");
        fprintf(ctx->fp, "    call plc_native_p%d\n", proc->number);
        fprintf(ctx->fp, "    mov DWORD PTR [rbp-36], %d\n", proc->results);
        fprintf(ctx->fp, "    jmp .L_print_results\n");
    }
    fprintf(ctx->fp, ".L_print_results:\n");
    fprintf(ctx->fp, "    mov DWORD PTR [rbp-40], 0\n");
    fprintf(ctx->fp, ".L_print_loop:\n");
    fprintf(ctx->fp, "    mov eax, DWORD PTR [rbp-40]\n");
    fprintf(ctx->fp, "    cmp eax, DWORD PTR [rbp-36]\n");
    fprintf(ctx->fp, "    jge .L_print_done\n");
    fprintf(ctx->fp, "    cmp eax, 0\n");
    fprintf(ctx->fp, "    je .L_print_no_space\n");
    fprintf(ctx->fp, "    lea rcx, [rip+space_msg]\n");
    fprintf(ctx->fp, "    call printf\n");
    fprintf(ctx->fp, ".L_print_no_space:\n");
    fprintf(ctx->fp, "    mov eax, DWORD PTR [rbp-40]\n");
    fprintf(ctx->fp, "    lea rdx, [rbp-384]\n");
    fprintf(ctx->fp, "    movsd xmm0, QWORD PTR [rdx+rax*8]\n");
    fprintf(ctx->fp, "    lea rdx, [rbp-768]\n");
    fprintf(ctx->fp, "    mov r8d, 64\n");
    fprintf(ctx->fp, "    call plc_native_format\n");
    fprintf(ctx->fp, "    lea rcx, [rip+result_msg]\n");
    fprintf(ctx->fp, "    mov edx, DWORD PTR [rbp-40]\n");
    fprintf(ctx->fp, "    lea r8, [rbp-768]\n");
    fprintf(ctx->fp, "    call printf\n");
    fprintf(ctx->fp, "    add DWORD PTR [rbp-40], 1\n");
    fprintf(ctx->fp, "    jmp .L_print_loop\n");
    fprintf(ctx->fp, ".L_print_done:\n");
    fprintf(ctx->fp, "    lea rcx, [rip+newline_msg]\n");
    fprintf(ctx->fp, "    call printf\n");
    fprintf(ctx->fp, "    xor eax, eax\n");
    fprintf(ctx->fp, ".L_main_return:\n");
    fprintf(ctx->fp, "    add rsp, 2048\n");
    fprintf(ctx->fp, "    pop rbp\n");
    fprintf(ctx->fp, "    ret\n");
}

int plc_emit_asm_runtime(const PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size)
{
    FILE *out;
    PLC_ASM_COMPILER ctx;
    int i;

    out = fopen(path, "w");
    if (out == 0) {
        sprintf(err, "cannot open ASM runtime output: %s", path);
        return 0;
    }

    memset(&ctx, 0, sizeof(ctx));
    ctx.fp = out;
    ctx.program = program;
    ctx.err = err;
    ctx.err_size = err_size;

    fprintf(out, "/* Generated by PlankaC native ASM backend. */\n");
    fprintf(out, ".intel_syntax noprefix\n");
    fprintf(out, ".extern printf\n");
    fprintf(out, ".extern strcmp\n");
    fprintf(out, ".extern strtod\n");
    fprintf(out, ".extern pow\n");
    fprintf(out, ".extern fmod\n");
    fprintf(out, ".extern plc_native_frame_create\n");
    fprintf(out, ".extern plc_native_frame_create_child\n");
    fprintf(out, ".extern plc_native_frame_destroy\n");
    fprintf(out, ".extern plc_native_get\n");
    fprintf(out, ".extern plc_native_set\n");
    fprintf(out, ".extern plc_native_builtin\n");
    fprintf(out, ".extern plc_native_format\n");
    fprintf(out, ".section .rdata,\"dr\"\n");
    fprintf(out, "usage_msg:\n");
    fprintf(out, "    .asciz \"usage: %%s <procedure> [args...]\\n\"\n");
    fprintf(out, "too_many_msg:\n");
    fprintf(out, "    .asciz \"too many arguments\\n\"\n");
    fprintf(out, "unknown_msg:\n");
    fprintf(out, "    .asciz \"unknown procedure: %%s\\n\"\n");
    fprintf(out, "space_msg:\n");
    fprintf(out, "    .asciz \" \"\n");
    fprintf(out, "result_msg:\n");
    fprintf(out, "    .asciz \"R%%d=%%s\"\n");
    fprintf(out, "newline_msg:\n");
    fprintf(out, "    .asciz \"\\n\"\n");
    fprintf(out, ".text\n");

    for (i = 0; i < program->proc_count; ++i) {
        if (!plc_asm_emit_proc(&ctx, &program->procs[i])) {
            fclose(out);
            return 0;
        }
    }
    plc_asm_emit_main(&ctx);
    fclose(out);
    return 1;
}
