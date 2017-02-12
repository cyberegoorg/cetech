#ifndef CETECH_PLUGIN_API_H
#define CETECH_PLUGIN_API_H

#include <stdio.h>

#include "engine/input/types.h"
#include "engine/config/config_api.h"


enum {
    PLUGIN_API_ID = 1,
    KEYBOARD_API_ID,
    MOUSE_API_ID,
    GAMEPAD_API_ID,
    CONFIG_API_ID,
    TASK_API_ID,
    CONSOLE_SERVER_API_ID,
    DEVELOP_SERVER_API_ID,
    ENTCOM_API_ID,
    LUA_API_ID,
    MEMORY_API_ID,
    RENDERER_API_ID,
    MATERIAL_API_ID,
    MESH_API_ID,
    FILESYSTEM_API_ID,
    RESOURCE_API_ID,
    PACKAGE_API_ID,
    CAMERA_API_ID,
    LEVEL_API_ID,
    SCENEGRAPH_API_ID,
    TRANSFORM_API_ID,
    UNIT_API_ID,
    WORLD_API_ID,
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
