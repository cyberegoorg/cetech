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

//! Load static module
//! \param name Module name
#define LOAD_STATIC_MODULE(api, name)                       \
    extern void* name ## _load_module(struct api_v0* api);  \
    name ## _load_module(api)

//! Unload static module
//! \param name Module name
#define UNLOAD_STATIC_MODULE(api, name)                      \
    extern void name ## _unload_module(struct api_v0* api);  \
    name ## _unload_module(api)

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

    //! Init
    //! \param get_api_fce_t Get engine api
    void (*init            )(struct api_v0 *);

    //! Shutdown
    void (*shutdown        )(void);
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