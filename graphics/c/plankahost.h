#ifndef PLANKAHOST_H
#define PLANKAHOST_H

#include "plankacube.h"

#define PLANKAHOST_APP_GUI 1
#define PLANKAHOST_APP_CUBE 3

typedef struct PLANKAHOST_APP {
    PLANKAC_CONTEXT *ctx;
    int kind;
    int width;
    int height;
    double checksum;
    char source[512];
    char title[128];
} PLANKAHOST_APP;

int plankahost_open(PLANKAHOST_APP *app, const char *source,
    char *err, unsigned err_size);
void plankahost_close(PLANKAHOST_APP *app);
int plankahost_proc_count(PLANKAHOST_APP *app);
int plankahost_get_proc(PLANKAHOST_APP *app, int index,
    PLANKAC_PROC_INFO *info);
int plankahost_find_proc(PLANKAHOST_APP *app, const char *name,
    PLANKAC_PROC_INFO *info);
int plankahost_run(PLANKAHOST_APP *app, const char *name,
    const double *args, int argc, PLANKAC_RESULT *result,
    char *err, unsigned err_size);
int plankahost_render(PLANKAHOST_APP *app, PMG_IMAGE *img,
    const PMG_RENDER_STATE *gui_state, double frame_value,
    char *err, unsigned err_size);
int plankahost_button_at(PLANKAHOST_APP *app, int x, int y,
    int *id, char *err, unsigned err_size);
int plankahost_timer_step(PLANKAHOST_APP *app, double current,
    double *next, char *err, unsigned err_size);

#endif
