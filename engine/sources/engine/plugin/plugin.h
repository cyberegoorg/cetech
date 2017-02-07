#ifndef CETECH_PLUGIN_SYSTEM_H
#define CETECH_PLUGIN_SYSTEM_H

//==============================================================================
// Includes
//==============================================================================

#include "plugin_api.h"

//==============================================================================
// Defines
//==============================================================================
#define ADD_STATIC_PLUGIN(name)                \
    void* name ## _get_plugin_api(int,int);   \
    plugin_add_static(name ## _get_plugin_api)

//==============================================================================
// Interface
//==============================================================================

void plugin_add_static(get_api_fce_t fce);

void plugin_load(const char *path);

void plugin_load_dirs(const char *path);

void plugin_reload(const char *path);

void plugin_call_update();

void plugin_call_init_cvar();

void plugin_call_init();

void plugin_call_update();

void plugin_call_shutdown();

void plugin_call_after_update(float dt);

void *plugin_get_engine_api(int api,
                            int version);


#endif //CETECH_PLUGIN_SYSTEM_H
