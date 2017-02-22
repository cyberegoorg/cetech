//! \defgroup Plugin
//! Plugin system
//! \{

#ifndef CETECH_PLUGIN_SYSTEM_H
#define CETECH_PLUGIN_SYSTEM_H

//==============================================================================
// Includes
//==============================================================================

#include "plugin_api.h"

//==============================================================================
// Defines
//==============================================================================
//! Add static plugin
//! \param name Plugin name
#define ADD_STATIC_PLUGIN(name)                \
    void* name ## _get_plugin_api(int,int);    \
    plugin_add_static(name ## _get_plugin_api)

//==============================================================================
// Interface
//==============================================================================

//! Add static plugins
//! \param fce get api plugin fce
void plugin_add_static(get_api_fce_t fce);

//! Load plugin from path
//! \param path Plugin path
void plugin_load(const char *path);

//! Load plugin from dir
//! \param path Plugin dir path
void plugin_load_dirs(const char *path);

//! Reload plugin by path
void plugin_reload(const char *path);

//! Reload all loaded plugins
void plugin_reload_all();

//! Call update
void plugin_call_update();

//! Call init cvar
void plugin_call_init_cvar();

//! Call init
void plugin_call_init();

//! Call shutdown
void plugin_call_shutdown();

//! Call after update
void plugin_call_after_update(float dt);

//! Plugin get engine api
void *plugin_get_engine_api(int api,
                            int version);


#endif //CETECH_PLUGIN_SYSTEM_H

//! \}