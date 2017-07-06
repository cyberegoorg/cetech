//! \addtogroup Plugin
//! \{
#ifndef CETECH_PLUGIN_API_H
#define CETECH_PLUGIN_API_H

#ifdef __cplusplus
extern "C" {
#endif

struct ct_api_v0;

//==============================================================================
// Typedefs
//==============================================================================

typedef void (*ct_load_module_t)(struct ct_api_v0 *api);
typedef void (*ct_unload_module_t)(struct ct_api_v0 *api);

//==============================================================================
// Interface
//==============================================================================

struct ct_module_api_v0 {
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