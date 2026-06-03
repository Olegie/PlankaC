#include "plankac_internal.h"

static int plc_doscom_write_byte(FILE *out, unsigned value,
    char *err, unsigned err_size)
{
    if (fputc((int)(value & 0xffu), out) == EOF) {
        plc_set_error(err, err_size, "cannot write DOS COM output");
        return 0;
    }
    return 1;
}

static int plc_doscom_write_word(FILE *out, unsigned value,
    char *err, unsigned err_size)
{
    return plc_doscom_write_byte(out, value & 0xffu, err, err_size)
        && plc_doscom_write_byte(out, (value >> 8) & 0xffu, err, err_size);
}

static int plc_doscom_write_text(FILE *out, const char *text,
    char *err, unsigned err_size)
{
    while (text != 0 && *text != '\0') {
        if (!plc_doscom_write_byte(out, (unsigned char)*text,
                err, err_size)) {
            return 0;
        }
        ++text;
    }
    return 1;
}

static int plc_doscom_payload_path(const char *path, char *out,
    unsigned out_size, char *err, unsigned err_size)
{
    if (strlen(path) + 5 >= out_size) {
        plc_set_error(err, err_size, "DOS COM path is too long");
        return 0;
    }
    strcpy(out, path);
    strcat(out, ".pbc");
    return 1;
}

static int plc_doscom_read_payload(const char *path, unsigned char **data,
    long *size, char *err, unsigned err_size)
{
    FILE *fp;
    long n;
    unsigned char *bytes;

    *data = 0;
    *size = 0;
    fp = fopen(path, "rb");
    if (fp == 0) {
        sprintf(err, "cannot open DOS COM bytecode image: %s", path);
        return 0;
    }
    if (fseek(fp, 0L, SEEK_END) != 0) {
        fclose(fp);
        plc_set_error(err, err_size, "cannot seek DOS COM bytecode image");
        return 0;
    }
    n = ftell(fp);
    if (n < 0) {
        fclose(fp);
        plc_set_error(err, err_size, "cannot size DOS COM bytecode image");
        return 0;
    }
    if (fseek(fp, 0L, SEEK_SET) != 0) {
        fclose(fp);
        plc_set_error(err, err_size, "cannot rewind DOS COM bytecode image");
        return 0;
    }
    bytes = (unsigned char *)malloc((size_t)n);
    if (bytes == 0 && n > 0) {
        fclose(fp);
        plc_set_error(err, err_size, "out of memory reading bytecode image");
        return 0;
    }
    if (n > 0 && fread(bytes, 1, (size_t)n, fp) != (size_t)n) {
        free(bytes);
        fclose(fp);
        plc_set_error(err, err_size, "cannot read DOS COM bytecode image");
        return 0;
    }
    fclose(fp);
    *data = bytes;
    *size = n;
    return 1;
}

int plc_emit_doscom(const PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size)
{
    FILE *out;
    char message[PLC_MAX_LINE];
    char payload_path[PLC_MAX_LINE];
    unsigned char *payload;
    long payload_size;
    long total_size;
    unsigned message_offset;

    if (program == 0) {
        plc_set_error(err, err_size, "missing program for DOS COM output");
        return 0;
    }
    if (path == 0 || path[0] == '\0') {
        plc_set_error(err, err_size, "missing DOS COM output path");
        return 0;
    }
    payload = 0;
    payload_size = 0;
    if (!plc_doscom_payload_path(path, payload_path, sizeof(payload_path),
            err, err_size)) {
        return 0;
    }
    if (!plc_emit_bytecode(program, payload_path, err, err_size)) {
        return 0;
    }
    if (!plc_doscom_read_payload(payload_path, &payload, &payload_size,
            err, err_size)) {
        remove(payload_path);
        return 0;
    }
    remove(payload_path);

    sprintf(message,
        "PlankaC DOS COM bootstrap\r\nprofile: PLANKAC-DOSCOM-8086\r\nprocedures: %d\r\nbytecode: embedded %ld bytes\r\nrunner: PLANKACD.EXE\r\n$",
        program->proc_count, payload_size);
    total_size = 12L + (long)strlen(message)
        + 31L + payload_size;
    if (total_size > 60000L) {
        free(payload);
        sprintf(err, "DOS COM image too large: %ld bytes", total_size);
        return 0;
    }

    out = fopen(path, "wb");
    if (out == 0) {
        free(payload);
        sprintf(err, "cannot open DOS COM output: %s", path);
        return 0;
    }

    message_offset = 0x100u + 12u;
    if (!plc_doscom_write_byte(out, 0xBAu, err, err_size)
            || !plc_doscom_write_word(out, message_offset, err, err_size)
            || !plc_doscom_write_byte(out, 0xB4u, err, err_size)
            || !plc_doscom_write_byte(out, 0x09u, err, err_size)
            || !plc_doscom_write_byte(out, 0xCDu, err, err_size)
            || !plc_doscom_write_byte(out, 0x21u, err, err_size)
            || !plc_doscom_write_byte(out, 0xB8u, err, err_size)
            || !plc_doscom_write_word(out, 0x4C00u, err, err_size)
            || !plc_doscom_write_byte(out, 0xCDu, err, err_size)
            || !plc_doscom_write_byte(out, 0x21u, err, err_size)
            || !plc_doscom_write_text(out, message, err, err_size)) {
        free(payload);
        fclose(out);
        return 0;
    }
    if (!plc_doscom_write_text(out, "\r\n--PLANKAC-BYTECODE-IMAGE--\r\n",
            err, err_size)) {
        free(payload);
        fclose(out);
        return 0;
    }
    if (payload_size > 0
            && fwrite(payload, 1, (size_t)payload_size, out)
                != (size_t)payload_size) {
        free(payload);
        fclose(out);
        plc_set_error(err, err_size, "cannot write DOS COM bytecode payload");
        return 0;
    }
    free(payload);
    if (fclose(out) != 0) {
        plc_set_error(err, err_size, "cannot close DOS COM output");
        return 0;
    }
    return 1;
}
