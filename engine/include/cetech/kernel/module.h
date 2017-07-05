//! \addtogroup Plugin
//! \{
#ifndef CETECH_PLUGIN_API_H
#define CETECH_PLUGIN_API_H

#ifdef __cplusplus
extern "C" {
#endif

struct api_v0;

//==============================================================================
// Typedefs
//==============================================================================

typedef void (*load_module_t)(struct api_v0 *api);
typedef void (*unload_module_t)(struct api_v0 *api);

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