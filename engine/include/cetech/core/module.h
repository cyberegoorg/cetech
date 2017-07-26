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

//! Add static module and load it
//! \param name Module name
#define ADD_STATIC_MODULE(name)                                     \
    extern void name ## _load_module(struct ct_api_a0*);               \
    extern void name ## _unload_module(struct ct_api_a0*);             \
    ct_module_a0.add_static(name ## _load_module, name ## _unload_module)

//! Load static module
//! \param name Module name
#define LOAD_STATIC_MODULE(api, name)                 \
    extern void name ## _load_module(struct ct_api_a0*); \
    name ## _load_module(api)

//! Unload static module
//! \param name Module name
#define UNLOAD_STATIC_MODULE(api, name)                     \
    extern void name ## _unload_module(struct ct_api_a0* api); \
    name ## _unload_module(api)

//==============================================================================
// Typedefs
//==============================================================================

typedef void (*ct_load_module_t)(struct ct_api_a0 *api);

typedef void (*ct_unload_module_t)(struct ct_api_a0 *api);


//==============================================================================
// Interface
//==============================================================================

struct ct_module_a0 {
    //! Add static modules
    //! \param fce get api module fce
    void (*add_static)(ct_load_module_t load,
                       ct_unload_module_t unload);

    //! Load module from path
    //! \param path Plugin path
    void (*load)(const char *path);

    //! Load module from dir
    //! \param path Plugin dir path
    void (*load_dirs)(const char *path);

    void (*unload_all)();

    //! Reload module by path
    void (*reload)(const char *path);

    //! Reload all loaded modules
    void (*reload_all)();
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_PLUGIN_API_H
//! \}