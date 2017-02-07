#ifndef CETECH_PLUGIN_API_H
#define CETECH_PLUGIN_API_H

#include <stdio.h>

#include "engine/input/types.h"
#include "engine/task/types.h"
#include "engine/config/config_api.h"


enum {
    PLUGIN_API_ID = 1,
    KEYBOARD_API_ID,
    MOUSE_API_ID,
    GAMEPAD_API_ID,
    CONFIG_API_ID,
    TASK_API_ID
};

typedef void *(*get_api_fce_t)(int api,
                               int version);

struct plugin_api_v0 {
    void (*init_cvar       )(struct ConfigApiV1);

    void (*init            )(get_api_fce_t);

    void (*shutdown        )(void);

    void *(*reload_begin    )(get_api_fce_t);

    void (*reload_end      )(get_api_fce_t,
                             void *);

    void (*update          )();

    void (*after_update    )(float dt);
};

#endif //CETECH_PLUGIN_API_H
