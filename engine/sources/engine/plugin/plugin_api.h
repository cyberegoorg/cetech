//! \addtogroup Plugin
//! \{
#ifndef CETECH_PLUGIN_API_H
#define CETECH_PLUGIN_API_H

#include <stdio.h>
#include "engine/config/cvar.h"

enum {
    PLUGIN_EXPORT_API_ID = 1,
    APPLICATION_API_ID,
    KEYBOARD_API_ID,
    MOUSE_API_ID,
    GAMEPAD_API_ID,
    MACHINE_API_ID,
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

//! Plugin expot api struct V0
struct plugin_api_v0 {

    //! Init cvars
    //! \param Init cvars
    void (*init_cvar       )(struct ConfigApiV0);

    //! Init
    //! \param get_api_fce_t Get engine api
    void (*init            )(get_api_fce_t);

    //! Shutdown
    void (*shutdown        )(void);

    //! Reload begin
    //! \param get_api_fce_t Get engine api
    void *(*reload_begin   )(get_api_fce_t);

    //! Reload end
    //! \param get_api_fce_t Get engine api
    void (*reload_end      )(get_api_fce_t,
                             void *);

    //! Call update
    void (*update          )();

    //! After update
    //! \param dt Deltatime
    void (*after_update    )(float dt);
};

#endif //CETECH_PLUGIN_API_H
//! \}