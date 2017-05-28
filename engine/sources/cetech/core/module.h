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
struct api_v0;
struct allocator;

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

    void (*init_api       )(struct api_v0*);

    //! Init
    //! \param get_api_fce_t Get engine api
    void (*init            )(struct api_v0*);

    //! Shutdown
    void (*shutdown        )(void);

    //! Reload begin
    //! \param get_api_fce_t Get engine api
    void *(*reload_begin   )(struct api_v0*);

    //! Reload end
    //! \param get_api_fce_t Get engine api
    void (*reload_end      )(struct api_v0*,
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

void module_init(struct allocator *allocator, struct api_v0* api_v0);
void module_shutdown();

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

//! Call init api
void module_call_init_api();

//! Call init
void module_call_init();

//! Call shutdown
void module_call_shutdown();

//! Call after update
void module_call_after_update(float dt);


#endif //CETECH_PLUGIN_API_H
//! \}