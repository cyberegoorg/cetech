//! \addtogroup Plugin
//! \{
#ifndef CETECH_PLUGIN_API_H
#define CETECH_PLUGIN_API_H

//==============================================================================
// Includes
//==============================================================================

//==============================================================================
// Defines
//==============================================================================

#define IMPORT_API(name) static struct name name;
#define INIT_API(get_api, name, api_id) name = *(struct name*) get_api(api_id)

//! Add static module
//! \param name Plugin name
#define ADD_STATIC_PLUGIN(name)                \
    void* name ## _get_module_api(int);        \
    module_add_static(name ## _get_module_api)


//==============================================================================
// Enums
//==============================================================================

enum {
    PLUGIN_EXPORT_API_ID = 1,
};

struct config_api_v0;

//==============================================================================
// Typedefs
//==============================================================================

typedef void *(*get_api_fce_t)(int api);


//==============================================================================
// Api
//==============================================================================

//! Plugin expot api struct V0
struct module_api_v0 {

    //! Init cvars
    //! \param Init cvars
    void (*init_cvar       )(struct config_api_v0);

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

#endif //CETECH_PLUGIN_API_H
//! \}