#include "plankac_internal.h"

#define PLC_EVIDENCE_HASH_INIT 2166136261UL
#define PLC_EVIDENCE_HASH_PRIME 16777619UL

typedef struct PLC_EVIDENCE_STATS {
    int statement_count;
    int assignment_count;
    int guarded_assignment_count;
    int call_count;
    int contract_count;
    int loop_count;
    int assertion_count;
    int stop_count;
    int const_count;
    int bank_v_count;
    int bank_c_count;
    int bank_z_count;
    int bank_r_count;
    int family_counts[PLC_TYPE_FAMILY_UNKNOWN + 1];
    int max_args;
    int max_results;
    unsigned long fingerprint;
} PLC_EVIDENCE_STATS;

static void plc_evidence_hash_byte(unsigned long *hash, int value)
{
    *hash ^= (unsigned long)(value & 255);
    *hash *= PLC_EVIDENCE_HASH_PRIME;
    *hash &= 0xffffffffUL;
}

static void plc_evidence_hash_int(unsigned long *hash, int value)
{
    plc_evidence_hash_byte(hash, value);
    plc_evidence_hash_byte(hash, value >> 8);
    plc_evidence_hash_byte(hash, value >> 16);
    plc_evidence_hash_byte(hash, value >> 24);
}

static void plc_evidence_hash_text(unsigned long *hash, const char *text)
{
    if (text == 0) {
        plc_evidence_hash_byte(hash, 0);
        return;
    }
    while (*text != '\0') {
        plc_evidence_hash_byte(hash, (unsigned char)*text);
        ++text;
    }
    plc_evidence_hash_byte(hash, 0);
}

static void plc_evidence_json_text(FILE *out, const char *text)
{
    fputc('"', out);
    while (text != 0 && *text != '\0') {
        if (*text == '"' || *text == '\\') {
            fputc('\\', out);
            fputc(*text, out);
        } else if (*text == '\n') {
            fputs("\\n", out);
        } else if (*text == '\r') {
            fputs("\\r", out);
        } else if (*text == '\t') {
            fputs("\\t", out);
        } else if ((unsigned char)*text < 32) {
            fputc(' ', out);
        } else {
            fputc(*text, out);
        }
        ++text;
    }
    fputc('"', out);
}

static void plc_evidence_note_family(PLC_EVIDENCE_STATS *stats,
    const char *marker)
{
    PLC_TYPE_SPEC spec;
    char err[PLC_MAX_LINE];

    if (marker == 0 || marker[0] == '\0') {
        return;
    }
    err[0] = '\0';
    if (!plc_parse_type_marker_text(marker, &spec, err, sizeof(err))) {
        return;
    }
    if (spec.family < 0 || spec.family > PLC_TYPE_FAMILY_UNKNOWN) {
        stats->family_counts[PLC_TYPE_FAMILY_UNKNOWN]++;
        return;
    }
    stats->family_counts[spec.family]++;
}

static void plc_evidence_scan_type_markers(PLC_EVIDENCE_STATS *stats,
    const char *line)
{
    const char *p;
    const char *end;
    char marker[PLC_MAX_TYPE_TEXT];

    p = line;
    while (p != 0 && *p != '\0') {
        if (*p == '[') {
            end = strchr(p, ']');
            if (end == 0) {
                return;
            }
            plc_copy_range(marker, sizeof(marker), p, end + 1);
            plc_evidence_note_family(stats, marker);
            p = end + 1;
        } else {
            ++p;
        }
    }
}

static void plc_evidence_scan_banks(PLC_EVIDENCE_STATS *stats,
    const char *line)
{
    const char *p;

    for (p = line; p != 0 && *p != '\0'; ++p) {
        if ((p[0] == 'V' || p[0] == 'C' || p[0] == 'Z' || p[0] == 'R')
                && isdigit((unsigned char)p[1])) {
            if (p[0] == 'V') {
                stats->bank_v_count++;
            } else if (p[0] == 'C') {
                stats->bank_c_count++;
            } else if (p[0] == 'Z') {
                stats->bank_z_count++;
            } else {
                stats->bank_r_count++;
            }
        }
    }
}

static int plc_evidence_arrow_count(const char *line)
{
    int count;
    const char *p;

    count = 0;
    p = line;
    while (p != 0 && *p != '\0') {
        if (p[0] == '=' && p[1] == '>') {
            ++count;
            p += 2;
        } else {
            ++p;
        }
    }
    return count;
}

static void plc_evidence_scan_statement(PLC_EVIDENCE_STATS *stats,
    const char *line)
{
    int arrows;

    arrows = plc_evidence_arrow_count(line);
    stats->statement_count++;
    if (arrows > 0) {
        stats->assignment_count++;
    }
    if (arrows > 1) {
        stats->guarded_assignment_count++;
    }
    if (strchr(line, '(') != 0 && arrows > 0) {
        stats->call_count++;
    }
    if (plc_line_starts_with(line, "REQUIRE")
            || plc_line_starts_with(line, "ENSURE")) {
        stats->contract_count++;
    }
    if (plc_line_starts_with(line, "ASSERT")) {
        stats->assertion_count++;
    }
    if (plc_line_starts_with(line, "LOOP")) {
        stats->loop_count++;
    }
    if (plc_line_starts_with(line, "STOPIF")) {
        stats->stop_count++;
    }
    if (plc_line_starts_with(line, "CONST")) {
        stats->const_count++;
    }
    plc_evidence_scan_type_markers(stats, line);
    plc_evidence_scan_banks(stats, line);
}

static void plc_evidence_collect(const PLC_PROGRAM *program,
    PLC_EVIDENCE_STATS *stats)
{
    int i;
    int j;
    int k;

    memset(stats, 0, sizeof(*stats));
    stats->fingerprint = PLC_EVIDENCE_HASH_INIT;
    plc_evidence_hash_text(&stats->fingerprint, PLANKAC_VERSION);
    plc_evidence_hash_int(&stats->fingerprint, program->source_count);
    plc_evidence_hash_int(&stats->fingerprint, program->proc_count);

    for (i = 0; i < program->proc_count; ++i) {
        const PLC_PROC *proc;

        proc = &program->procs[i];
        if (proc->argc > stats->max_args) {
            stats->max_args = proc->argc;
        }
        if (proc->results > stats->max_results) {
            stats->max_results = proc->results;
        }
        plc_evidence_hash_int(&stats->fingerprint, proc->number);
        plc_evidence_hash_text(&stats->fingerprint, proc->name);
        plc_evidence_hash_int(&stats->fingerprint, proc->argc);
        plc_evidence_hash_int(&stats->fingerprint, proc->results);
        plc_evidence_hash_int(&stats->fingerprint, proc->stmt_count);
        for (j = 0; j < proc->argc; ++j) {
            plc_evidence_note_family(stats, proc->arg_types[j]);
            plc_evidence_hash_text(&stats->fingerprint, proc->arg_types[j]);
        }
        for (j = 0; j < proc->results; ++j) {
            plc_evidence_note_family(stats, proc->result_types[j]);
            plc_evidence_hash_text(&stats->fingerprint, proc->result_types[j]);
        }
        for (k = 0; k < proc->stmt_count; ++k) {
            plc_evidence_hash_text(&stats->fingerprint, proc->stmts[k].text);
            plc_evidence_hash_int(&stats->fingerprint,
                proc->stmts[k].line_no);
            plc_evidence_scan_statement(stats, proc->stmts[k].text);
        }
    }
}

static void plc_evidence_write_family_counts(FILE *out,
    const PLC_EVIDENCE_STATS *stats)
{
    int i;

    fprintf(out, "    \"numeric\": %d,\n",
        stats->family_counts[PLC_TYPE_FAMILY_NUMERIC]);
    fprintf(out, "    \"boolean\": %d,\n",
        stats->family_counts[PLC_TYPE_FAMILY_BOOLEAN]);
    fprintf(out, "    \"complex\": %d,\n",
        stats->family_counts[PLC_TYPE_FAMILY_COMPLEX]);
    fprintf(out, "    \"list\": %d,\n",
        stats->family_counts[PLC_TYPE_FAMILY_LIST]);
    fprintf(out, "    \"set\": %d,\n",
        stats->family_counts[PLC_TYPE_FAMILY_SET]);
    fprintf(out, "    \"pair\": %d,\n",
        stats->family_counts[PLC_TYPE_FAMILY_PAIR]);
    fprintf(out, "    \"record\": %d,\n",
        stats->family_counts[PLC_TYPE_FAMILY_RECORD]);
    fprintf(out, "    \"vec3\": %d,\n",
        stats->family_counts[PLC_TYPE_FAMILY_VEC3]);
    fprintf(out, "    \"mat4\": %d,\n",
        stats->family_counts[PLC_TYPE_FAMILY_MAT4]);
    i = PLC_TYPE_FAMILY_UNKNOWN;
    fprintf(out, "    \"unknown\": %d\n", stats->family_counts[i]);
}

int plc_emit_evidence(const PLC_PROGRAM *program, const char *path,
    char *err, unsigned err_size)
{
    PLC_EVIDENCE_STATS stats;
    FILE *out;
    int i;

    if (program == 0) {
        plc_set_error(err, err_size, "missing program");
        return 0;
    }
    out = fopen(path, "w");
    if (out == 0) {
        plc_set_error(err, err_size, "cannot write evidence file");
        return 0;
    }
    plc_evidence_collect(program, &stats);

    fprintf(out, "{\n");
    fprintf(out, "  \"format\": \"plankac-evidence-v1\",\n");
    fprintf(out, "  \"version\": ");
    plc_evidence_json_text(out, PLANKAC_VERSION);
    fprintf(out, ",\n");
    fprintf(out, "  \"fingerprint\": \"plc-%08lx\",\n",
        stats.fingerprint);
    fprintf(out, "  \"source_files\": %d,\n", program->source_count);
    fprintf(out, "  \"procedures\": %d,\n", program->proc_count);
    fprintf(out, "  \"statements\": %d,\n", stats.statement_count);
    fprintf(out, "  \"max_args\": %d,\n", stats.max_args);
    fprintf(out, "  \"max_results\": %d,\n", stats.max_results);
    fprintf(out, "  \"statement_kinds\": {\n");
    fprintf(out, "    \"assignments\": %d,\n", stats.assignment_count);
    fprintf(out, "    \"guarded_assignments\": %d,\n",
        stats.guarded_assignment_count);
    fprintf(out, "    \"calls\": %d,\n", stats.call_count);
    fprintf(out, "    \"contracts\": %d,\n", stats.contract_count);
    fprintf(out, "    \"assertions\": %d,\n", stats.assertion_count);
    fprintf(out, "    \"loops\": %d,\n", stats.loop_count);
    fprintf(out, "    \"stop_criteria\": %d,\n", stats.stop_count);
    fprintf(out, "    \"constants\": %d\n", stats.const_count);
    fprintf(out, "  },\n");
    fprintf(out, "  \"banks\": {\n");
    fprintf(out, "    \"V\": %d,\n", stats.bank_v_count);
    fprintf(out, "    \"C\": %d,\n", stats.bank_c_count);
    fprintf(out, "    \"Z\": %d,\n", stats.bank_z_count);
    fprintf(out, "    \"R\": %d\n", stats.bank_r_count);
    fprintf(out, "  },\n");
    fprintf(out, "  \"type_families\": {\n");
    plc_evidence_write_family_counts(out, &stats);
    fprintf(out, "  },\n");
    fprintf(out, "  \"backend_contract\": {\n");
    fprintf(out, "    \"bytecode\": \"emitted from loaded source profile\",\n");
    fprintf(out, "    \"ir\": \"typed statement stream with family labels\",\n");
    fprintf(out, "    \"c\": \"generated runner using public runtime ABI\",\n");
    fprintf(out, "    \"x86_64_asm\": \"generated native runner plus runtime ABI\",\n");
    fprintf(out, "    \"asm8086\": \"generated 16-bit assembly image\"\n");
    fprintf(out, "  },\n");
    fprintf(out, "  \"catalog\": [\n");
    for (i = 0; i < program->proc_count; ++i) {
        const PLC_PROC *proc;

        proc = &program->procs[i];
        fprintf(out, "    {\"p\": %d, \"name\": ",
            proc->number);
        plc_evidence_json_text(out, proc->name);
        fprintf(out, ", \"args\": %d, \"results\": %d, \"statements\": %d}",
            proc->argc, proc->results, proc->stmt_count);
        if (i + 1 < program->proc_count) {
            fprintf(out, ",");
        }
        fprintf(out, "\n");
    }
    fprintf(out, "  ]\n");
    fprintf(out, "}\n");
    fclose(out);
    return 1;
}
