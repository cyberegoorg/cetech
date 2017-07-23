//! \addtogroup Plugin
//! \{
#ifndef CETECH_PLUGIN_API_H
#define CETECH_PLUGIN_API_H

#ifdef __cplusplus
extern "C" {
#endif


//==============================================================================
// Interface
//==============================================================================

struct ct_module_a0 {
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