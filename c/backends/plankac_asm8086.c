#include "plankac_internal.h"

static void plc_asm8086_label_name(const PLC_PROC *proc,
    char *out, unsigned out_size)
{
    (void)out_size;
    sprintf(out, "plc8086_%s", proc->name);
}

static void plc_asm8086_emit_public(FILE *out, const PLC_PROC *proc)
{
    char label[PLC_MAX_NAME + 16];

    plc_asm8086_label_name(proc, label, sizeof(label));
    fprintf(out, "PUBLIC plc8086_p%d\n", proc->number);
    fprintf(out, "PUBLIC %s\n", label);
}

static void plc_asm8086_emit_proc_header(FILE *out, const PLC_PROC *proc)
{
    char label[PLC_MAX_NAME + 16];

    plc_asm8086_label_name(proc, label, sizeof(label));
    fprintf(out, "plc8086_p%d PROC NEAR\n", proc->number);
    fprintf(out, "%s LABEL NEAR\n", label);
    fprintf(out, "    push bp\n");
    fprintf(out, "    mov bp, sp\n");
}

static void plc_asm8086_emit_proc_footer(FILE *out, const PLC_PROC *proc)
{
    fprintf(out, "    pop bp\n");
    if (proc->argc > 0) {
        fprintf(out, "    ret %d\n", proc->argc * 2);
    } else {
        fprintf(out, "    ret\n");
    }
    fprintf(out, "plc8086_p%d ENDP\n\n", proc->number);
}

static int plc_asm8086_emit_compare(FILE *out, const PLC_PROC *proc,
    const char *jump)
{
    plc_asm8086_emit_proc_header(out, proc);
    fprintf(out, "    xor dx, dx\n");
    fprintf(out, "    mov ax, [bp+4]\n");
    fprintf(out, "    cmp ax, [bp+6]\n");
    fprintf(out, "    %s plc8086_true_%d\n", jump, proc->number);
    fprintf(out, "    xor ax, ax\n");
    fprintf(out, "    jmp plc8086_done_%d\n", proc->number);
    fprintf(out, "plc8086_true_%d:\n", proc->number);
    fprintf(out, "    mov ax, 1\n");
    fprintf(out, "plc8086_done_%d:\n", proc->number);
    plc_asm8086_emit_proc_footer(out, proc);
    return 1;
}

static int plc_asm8086_emit_direct_proc(FILE *out, const PLC_PROC *proc)
{
    if (strcmp(proc->name, "type_sheet") == 0) {
        plc_asm8086_emit_proc_header(out, proc);
        fprintf(out, "    mov ax, 1\n");
        fprintf(out, "    xor dx, dx\n");
        plc_asm8086_emit_proc_footer(out, proc);
        return 1;
    }
    if (strcmp(proc->name, "add") == 0) {
        plc_asm8086_emit_proc_header(out, proc);
        fprintf(out, "    xor dx, dx\n");
        fprintf(out, "    mov ax, [bp+4]\n");
        fprintf(out, "    add ax, [bp+6]\n");
        plc_asm8086_emit_proc_footer(out, proc);
        return 1;
    }
    if (strcmp(proc->name, "subtract") == 0) {
        plc_asm8086_emit_proc_header(out, proc);
        fprintf(out, "    xor dx, dx\n");
        fprintf(out, "    mov ax, [bp+4]\n");
        fprintf(out, "    sub ax, [bp+6]\n");
        plc_asm8086_emit_proc_footer(out, proc);
        return 1;
    }
    if (strcmp(proc->name, "multiply") == 0) {
        plc_asm8086_emit_proc_header(out, proc);
        fprintf(out, "    mov ax, [bp+4]\n");
        fprintf(out, "    imul WORD PTR [bp+6]\n");
        plc_asm8086_emit_proc_footer(out, proc);
        return 1;
    }
    if (strcmp(proc->name, "negate") == 0) {
        plc_asm8086_emit_proc_header(out, proc);
        fprintf(out, "    xor dx, dx\n");
        fprintf(out, "    mov ax, [bp+4]\n");
        fprintf(out, "    neg ax\n");
        plc_asm8086_emit_proc_footer(out, proc);
        return 1;
    }
    if (strcmp(proc->name, "divide_checked") == 0) {
        plc_asm8086_emit_proc_header(out, proc);
        fprintf(out, "    mov bx, [bp+6]\n");
        fprintf(out, "    cmp bx, 0\n");
        fprintf(out, "    jne plc8086_div_ok_%d\n", proc->number);
        fprintf(out, "    xor ax, ax\n");
        fprintf(out, "    mov dx, 1\n");
        fprintf(out, "    jmp plc8086_div_done_%d\n", proc->number);
        fprintf(out, "plc8086_div_ok_%d:\n", proc->number);
        fprintf(out, "    mov ax, [bp+4]\n");
        fprintf(out, "    cwd\n");
        fprintf(out, "    idiv bx\n");
        fprintf(out, "    xor dx, dx\n");
        fprintf(out, "plc8086_div_done_%d:\n", proc->number);
        plc_asm8086_emit_proc_footer(out, proc);
        return 1;
    }
    if (strcmp(proc->name, "modulo_checked") == 0) {
        plc_asm8086_emit_proc_header(out, proc);
        fprintf(out, "    mov bx, [bp+6]\n");
        fprintf(out, "    cmp bx, 0\n");
        fprintf(out, "    jne plc8086_mod_ok_%d\n", proc->number);
        fprintf(out, "    xor ax, ax\n");
        fprintf(out, "    mov dx, 1\n");
        fprintf(out, "    jmp plc8086_mod_done_%d\n", proc->number);
        fprintf(out, "plc8086_mod_ok_%d:\n", proc->number);
        fprintf(out, "    mov ax, [bp+4]\n");
        fprintf(out, "    cwd\n");
        fprintf(out, "    idiv bx\n");
        fprintf(out, "    mov ax, dx\n");
        fprintf(out, "    xor dx, dx\n");
        fprintf(out, "plc8086_mod_done_%d:\n", proc->number);
        plc_asm8086_emit_proc_footer(out, proc);
        return 1;
    }
    if (strcmp(proc->name, "average2") == 0) {
        plc_asm8086_emit_proc_header(out, proc);
        fprintf(out, "    xor dx, dx\n");
        fprintf(out, "    mov ax, [bp+4]\n");
        fprintf(out, "    add ax, [bp+6]\n");
        fprintf(out, "    sar ax, 1\n");
        plc_asm8086_emit_proc_footer(out, proc);
        return 1;
    }
    if (strcmp(proc->name, "equal") == 0
            || strcmp(proc->name, "equal_bool") == 0) {
        return plc_asm8086_emit_compare(out, proc, "je");
    }
    if (strcmp(proc->name, "less") == 0) {
        return plc_asm8086_emit_compare(out, proc, "jl");
    }
    if (strcmp(proc->name, "maximum") == 0) {
        plc_asm8086_emit_proc_header(out, proc);
        fprintf(out, "    xor dx, dx\n");
        fprintf(out, "    mov ax, [bp+4]\n");
        fprintf(out, "    cmp ax, [bp+6]\n");
        fprintf(out, "    jge plc8086_max_done_%d\n", proc->number);
        fprintf(out, "    mov ax, [bp+6]\n");
        fprintf(out, "plc8086_max_done_%d:\n", proc->number);
        plc_asm8086_emit_proc_footer(out, proc);
        return 1;
    }
    if (strcmp(proc->name, "minimum") == 0) {
        plc_asm8086_emit_proc_header(out, proc);
        fprintf(out, "    xor dx, dx\n");
        fprintf(out, "    mov ax, [bp+4]\n");
        fprintf(out, "    cmp ax, [bp+6]\n");
        fprintf(out, "    jle plc8086_min_done_%d\n", proc->number);
        fprintf(out, "    mov ax, [bp+6]\n");
        fprintf(out, "plc8086_min_done_%d:\n", proc->number);
        plc_asm8086_emit_proc_footer(out, proc);
        return 1;
    }
    if (strcmp(proc->name, "absolute") == 0) {
        plc_asm8086_emit_proc_header(out, proc);
        fprintf(out, "    xor dx, dx\n");
        fprintf(out, "    mov ax, [bp+4]\n");
        fprintf(out, "    cmp ax, 0\n");
        fprintf(out, "    jge plc8086_abs_done_%d\n", proc->number);
        fprintf(out, "    neg ax\n");
        fprintf(out, "plc8086_abs_done_%d:\n", proc->number);
        plc_asm8086_emit_proc_footer(out, proc);
        return 1;
    }
    if (strcmp(proc->name, "sign") == 0) {
        plc_asm8086_emit_proc_header(out, proc);
        fprintf(out, "    xor dx, dx\n");
        fprintf(out, "    mov ax, [bp+4]\n");
        fprintf(out, "    cmp ax, 0\n");
        fprintf(out, "    jg plc8086_sign_pos_%d\n", proc->number);
        fprintf(out, "    jl plc8086_sign_neg_%d\n", proc->number);
        fprintf(out, "    jmp plc8086_sign_done_%d\n", proc->number);
        fprintf(out, "plc8086_sign_pos_%d:\n", proc->number);
        fprintf(out, "    mov ax, 1\n");
        fprintf(out, "    jmp plc8086_sign_done_%d\n", proc->number);
        fprintf(out, "plc8086_sign_neg_%d:\n", proc->number);
        fprintf(out, "    mov ax, -1\n");
        fprintf(out, "plc8086_sign_done_%d:\n", proc->number);
        plc_asm8086_emit_proc_footer(out, proc);
        return 1;
    }
    if (strcmp(proc->name, "square") == 0
            || strcmp(proc->name, "power2") == 0) {
        plc_asm8086_emit_proc_header(out, proc);
        fprintf(out, "    mov ax, [bp+4]\n");
        fprintf(out, "    imul ax\n");
        plc_asm8086_emit_proc_footer(out, proc);
        return 1;
    }
    return 0;
}

static void plc_asm8086_emit_stub(FILE *out, const PLC_PROC *proc)
{
    plc_asm8086_emit_proc_header(out, proc);
    fprintf(out, "    ; Procedure kept in the bytecode image.\n");
    fprintf(out, "    ; Full 16-bit lowering is not available for this value family yet.\n");
    fprintf(out, "    xor ax, ax\n");
    fprintf(out, "    mov dx, 1\n");
    plc_asm8086_emit_proc_footer(out, proc);
}

static void plc_asm8086_emit_byte(FILE *out, unsigned char value,
    int *column)
{
    if (*column == 0) {
        fprintf(out, "    DB ");
    } else {
        fprintf(out, ",");
    }
    fprintf(out, "%u", (unsigned)value);
    ++(*column);
    if (*column >= 16) {
        fprintf(out, "\n");
        *column = 0;
    }
}

static int plc_asm8086_emit_bytecode_image(const PLC_PROGRAM *program,
    FILE *out, const char *tmp_path, char *err, unsigned err_size)
{
    FILE *tmp;
    int c;
    int column;

    if (!plc_emit_bytecode(program, tmp_path, err, err_size)) {
        return 0;
    }
    tmp = fopen(tmp_path, "rb");
    if (tmp == 0) {
        remove(tmp_path);
        sprintf(err, "cannot read generated bytecode: %s", tmp_path);
        return 0;
    }
    fprintf(out, "plankac_8086_bytecode_image LABEL BYTE\n");
    column = 0;
    while ((c = fgetc(tmp)) != EOF) {
        plc_asm8086_emit_byte(out, (unsigned char)c, &column);
    }
    plc_asm8086_emit_byte(out, 0, &column);
    if (column != 0) {
        fprintf(out, "\n");
    }
    fprintf(out, "plankac_8086_bytecode_image_end LABEL BYTE\n\n");
    fclose(tmp);
    remove(tmp_path);
    return 1;
}

int plc_emit_asm8086_runtime(const PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size)
{
    FILE *out;
    char tmp_path[PLC_MAX_LINE];
    int i;

    if (strlen(path) + 5 >= sizeof(tmp_path)) {
        plc_set_error(err, err_size, "8086 ASM output path too long");
        return 0;
    }
    strcpy(tmp_path, path);
    strcat(tmp_path, ".pbc");
    out = fopen(path, "w");
    if (out == 0) {
        sprintf(err, "cannot open 8086 ASM output: %s", path);
        return 0;
    }

    fprintf(out, "; Generated by PlankaC 8086/DOS backend.\n");
    fprintf(out, "; MASM/TASM style 16-bit source.\n");
    fprintf(out, "; AX holds R0 for direct primitive procedures.\n");
    fprintf(out, "; DX is zero on success, non-zero when a stub or guard is hit.\n\n");
    fprintf(out, ".MODEL SMALL\n");
    fprintf(out, ".STACK 100h\n\n");
    fprintf(out, ".DATA\n");
    fprintf(out, "plankac_8086_source_count DW %d\n", program->source_count);
    fprintf(out, "plankac_8086_proc_count DW %d\n\n", program->proc_count);
    if (!plc_asm8086_emit_bytecode_image(program, out, tmp_path,
            err, err_size)) {
        fclose(out);
        return 0;
    }
    fprintf(out, ".CODE\n");
    fprintf(out, "PUBLIC plankac_8086_start\n");
    fprintf(out, "PUBLIC plankac_8086_proc_count\n");
    fprintf(out, "PUBLIC plankac_8086_bytecode_image\n");
    fprintf(out, "PUBLIC plankac_8086_bytecode_image_end\n");
    for (i = 0; i < program->proc_count; ++i) {
        plc_asm8086_emit_public(out, &program->procs[i]);
    }
    fprintf(out, "\nplankac_8086_start PROC FAR\n");
    fprintf(out, "    mov ax, @data\n");
    fprintf(out, "    mov ds, ax\n");
    fprintf(out, "    mov ax, 4C00h\n");
    fprintf(out, "    int 21h\n");
    fprintf(out, "plankac_8086_start ENDP\n\n");

    for (i = 0; i < program->proc_count; ++i) {
        if (!plc_asm8086_emit_direct_proc(out, &program->procs[i])) {
            plc_asm8086_emit_stub(out, &program->procs[i]);
        }
    }
    fprintf(out, "END plankac_8086_start\n");
    fclose(out);
    plc_copy_error(err, err_size, "");
    return 1;
}
