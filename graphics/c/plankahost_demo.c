#include <stdio.h>

#include "plankahost.h"

int main(int argc, char **argv)
{
    const char *source;
    PLANKAHOST_APP app;
    PLANKAC_RESULT result;
    PLANKAC_PROC_INFO info;
    double args[2];
    char err[256];
    int procedures;

    source = argc > 1 ? argv[1] : "graphics/src/plankagui.plk";
    if (!plankahost_open(&app, source, err, sizeof(err))) {
        printf("PlankaHost load failed: %s\n", err);
        return 1;
    }

    procedures = plankahost_proc_count(&app);
    printf("source=%s\n", source);
    printf("kind=%d canvas=%dx%d checksum=%.0f procedures=%d\n",
        app.kind, app.width, app.height, app.checksum, procedures);

    if (!plankahost_find_proc(&app, "add", &info)) {
        printf("missing base procedure: add\n");
        plankahost_close(&app);
        return 1;
    }

    args[0] = 12.0;
    args[1] = 8.0;
    if (!plankahost_run(&app, "add", args, 2,
            &result, err, sizeof(err))) {
        printf("PlankaHost run failed: %s\n", err);
        plankahost_close(&app);
        return 1;
    }

    printf("add(12,8)=%.0f via P%d\n", result.value[0], info.number);

    if (!plankahost_run(&app, "app_kind", 0, 0,
            &result, err, sizeof(err))) {
        printf("PlankaHost app_kind failed: %s\n", err);
        plankahost_close(&app);
        return 1;
    }

    printf("app_kind=%.0f\n", result.value[0]);
    plankahost_close(&app);
    return 0;
}
