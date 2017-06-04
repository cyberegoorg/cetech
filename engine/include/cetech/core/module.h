//! \addtogroup Plugin
//! \{
#ifndef CETECH_PLUGIN_API_H
#define CETECH_PLUGIN_API_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Defines
//==============================================================================

//! Add static module
//! \param name Plugin name
#define ADD_STATIC_PLUGIN(name)                 \
    extern void* name ## _get_module_api(int);  \
    module::add_static(name ## _get_module_api)

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
struct module_export_api_v0 {

    //! Init cvars
    //! \param Init cvars
    void (*init_cvar       )(struct config_api_v0);

    void (*init_api       )(struct api_v0 *);

    //! Init
    //! \param get_api_fce_t Get engine api
    void (*init            )(struct api_v0 *);

    //! Shutdown
    void (*shutdown        )(void);

    //! Reload begin
    //! \param get_api_fce_t Get engine api
    void *(*reload_begin   )(struct api_v0 *);

    //! Reload end
    //! \param get_api_fce_t Get engine api
    void (*reload_end      )(struct api_v0 *,
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

struct module_api_v0 {
    //! Reload module by path
    void (*module_reload)(const char *path);

    //! Reload all loaded modules
    void (*module_reload_all)();
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_PLUGIN_API_H
//! \}