//! \defgroup Plugin
//! Plugin system
//! \{

#ifndef CETECH_PLUGIN_SYSTEM_H
#define CETECH_PLUGIN_SYSTEM_H

//==============================================================================
// Includes
//==============================================================================

#include <cetech/module/module.h>

//==============================================================================
// Defines
//==============================================================================
//! Add static module
//! \param name Plugin name
#define ADD_STATIC_PLUGIN(name)                \
    void* name ## _get_module_api(int);        \
    module_add_static(name ## _get_module_api)

//==============================================================================
// Interface
//==============================================================================

//! Add static modules
//! \param fce get api module fce
void module_add_static(get_api_fce_t fce);

//! Load module from path
//! \param path Plugin path
void module_load(const char *path);

//! Load module from dir
//! \param path Plugin dir path
void module_load_dirs(const char *path);

//! Reload module by path
void module_reload(const char *path);

//! Reload all loaded modules
void module_reload_all();

//! Call update
void module_call_update();

//! Call init cvar
void module_call_init_cvar();

//! Call init
void module_call_init();

//! Call shutdown
void module_call_shutdown();

//! Call after update
void module_call_after_update(float dt);

//! Plugin get engine api
void *module_get_engine_api(int api);


#endif //CETECH_PLUGIN_SYSTEM_H

//! \}