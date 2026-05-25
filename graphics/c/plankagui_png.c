#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "plankagui.h"

static unsigned long pmg_crc_table[256];
static int pmg_crc_ready = 0;

static void pmg_png_error(char *err, unsigned err_size, const char *text)
{
    if (err != 0 && err_size > 0) {
        strncpy(err, text, err_size - 1);
        err[err_size - 1] = '\0';
    }
}

static void pmg_crc_init(void)
{
    unsigned long c;
    int n;
    int k;

    if (pmg_crc_ready) {
        return;
    }
    for (n = 0; n < 256; ++n) {
        c = (unsigned long)n;
        for (k = 0; k < 8; ++k) {
            if ((c & 1) != 0) {
                c = 0xedb88320UL ^ (c >> 1);
            } else {
                c = c >> 1;
            }
        }
        pmg_crc_table[n] = c;
    }
    pmg_crc_ready = 1;
}

static unsigned long pmg_crc_update(unsigned long crc,
    const unsigned char *buf, unsigned long len)
{
    unsigned long n;

    pmg_crc_init();
    for (n = 0; n < len; ++n) {
        crc = pmg_crc_table[(crc ^ buf[n]) & 0xff] ^ (crc >> 8);
    }
    return crc;
}

static void pmg_write_be32(FILE *fp, unsigned long v)
{
    fputc((int)((v >> 24) & 255), fp);
    fputc((int)((v >> 16) & 255), fp);
    fputc((int)((v >> 8) & 255), fp);
    fputc((int)(v & 255), fp);
}

static void pmg_write_chunk(FILE *fp, const char *type,
    const unsigned char *data, unsigned long len)
{
    unsigned long crc;

    pmg_write_be32(fp, len);
    fwrite(type, 1, 4, fp);
    if (len > 0) {
        fwrite(data, 1, len, fp);
    }
    crc = 0xffffffffUL;
    crc = pmg_crc_update(crc, (const unsigned char *)type, 4);
    if (len > 0) {
        crc = pmg_crc_update(crc, data, len);
    }
    crc ^= 0xffffffffUL;
    pmg_write_be32(fp, crc);
}

static unsigned long pmg_adler32(const unsigned char *data, unsigned long len)
{
    unsigned long a;
    unsigned long b;
    unsigned long i;

    a = 1;
    b = 0;
    for (i = 0; i < len; ++i) {
        a = (a + data[i]) % 65521UL;
        b = (b + a) % 65521UL;
    }
    return (b << 16) | a;
}

int pmg_write_png(const char *path, PMG_IMAGE *img,
    char *err, unsigned err_size)
{
    FILE *fp;
    unsigned char sig[8];
    unsigned char ihdr[13];
    unsigned char *raw;
    unsigned char *idat;
    unsigned long row_size;
    unsigned long raw_size;
    unsigned long max_blocks;
    unsigned long idat_size;
    unsigned long raw_pos;
    unsigned long idat_pos;
    unsigned long chunk;
    unsigned long remaining;
    unsigned long adler;
    int y;

    row_size = 1UL + (unsigned long)img->width * 3UL;
    raw_size = row_size * (unsigned long)img->height;
    raw = (unsigned char *)malloc(raw_size);
    if (raw == 0) {
        pmg_png_error(err, err_size, "png raw allocation failed");
        return 0;
    }
    for (y = 0; y < img->height; ++y) {
        raw[y * row_size] = 0;
        memcpy(raw + y * row_size + 1,
            img->pixels + (unsigned long)y * (unsigned long)img->width * 3UL,
            (unsigned long)img->width * 3UL);
    }

    max_blocks = raw_size / 65535UL + 1UL;
    idat_size = 2UL + raw_size + max_blocks * 5UL + 4UL;
    idat = (unsigned char *)malloc(idat_size);
    if (idat == 0) {
        free(raw);
        pmg_png_error(err, err_size, "png idat allocation failed");
        return 0;
    }

    idat_pos = 0;
    idat[idat_pos++] = 0x78;
    idat[idat_pos++] = 0x01;
    raw_pos = 0;
    remaining = raw_size;
    while (remaining > 0) {
        int final_block;

        chunk = remaining > 65535UL ? 65535UL : remaining;
        final_block = remaining <= 65535UL;
        idat[idat_pos++] = (unsigned char)(final_block ? 1 : 0);
        idat[idat_pos++] = (unsigned char)(chunk & 255);
        idat[idat_pos++] = (unsigned char)((chunk >> 8) & 255);
        idat[idat_pos++] = (unsigned char)((~chunk) & 255);
        idat[idat_pos++] = (unsigned char)(((~chunk) >> 8) & 255);
        memcpy(idat + idat_pos, raw + raw_pos, chunk);
        idat_pos += chunk;
        raw_pos += chunk;
        remaining -= chunk;
    }
    adler = pmg_adler32(raw, raw_size);
    idat[idat_pos++] = (unsigned char)((adler >> 24) & 255);
    idat[idat_pos++] = (unsigned char)((adler >> 16) & 255);
    idat[idat_pos++] = (unsigned char)((adler >> 8) & 255);
    idat[idat_pos++] = (unsigned char)(adler & 255);

    fp = fopen(path, "wb");
    if (fp == 0) {
        free(raw);
        free(idat);
        pmg_png_error(err, err_size, "cannot write png file");
        return 0;
    }

    sig[0] = 137;
    sig[1] = 80;
    sig[2] = 78;
    sig[3] = 71;
    sig[4] = 13;
    sig[5] = 10;
    sig[6] = 26;
    sig[7] = 10;
    fwrite(sig, 1, 8, fp);

    ihdr[0] = (unsigned char)((img->width >> 24) & 255);
    ihdr[1] = (unsigned char)((img->width >> 16) & 255);
    ihdr[2] = (unsigned char)((img->width >> 8) & 255);
    ihdr[3] = (unsigned char)(img->width & 255);
    ihdr[4] = (unsigned char)((img->height >> 24) & 255);
    ihdr[5] = (unsigned char)((img->height >> 16) & 255);
    ihdr[6] = (unsigned char)((img->height >> 8) & 255);
    ihdr[7] = (unsigned char)(img->height & 255);
    ihdr[8] = 8;
    ihdr[9] = 2;
    ihdr[10] = 0;
    ihdr[11] = 0;
    ihdr[12] = 0;
    pmg_write_chunk(fp, "IHDR", ihdr, 13);
    pmg_write_chunk(fp, "IDAT", idat, idat_pos);
    pmg_write_chunk(fp, "IEND", 0, 0);
    fclose(fp);
    free(raw);
    free(idat);
    return 1;
}
