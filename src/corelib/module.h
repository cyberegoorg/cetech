#ifndef CETECH_PLUGIN_API_H
#define CETECH_PLUGIN_API_H

#include <corelib/module.inl>

struct ct_api_a0;

//==============================================================================
// Defines
//==============================================================================

//! Add static module and load it
//! \param name Module name
#define CETECH_ADD_STATIC_MODULE(name)                                     \
    extern void name ## _initapi_module(struct ct_api_a0*);               \
    extern void name ## _load_module(struct ct_api_a0*, int reload);               \
    extern void name ## _unload_module(struct ct_api_a0*, int reload);             \
    ct_module_a0->add_static(name ## _load_module, name ## _unload_module, name ## _initapi_module)

//! Load static module
//! \param name Module name
#define CETECH_LOAD_STATIC_MODULE(api, name)                 \
    extern void name ## _load_module(struct ct_api_a0*, int reload); \
    extern void name ## _initapi_module(struct ct_api_a0*); \
    name ## _initapi_module(api); \
    name ## _load_module(api, 0)

//! Unload static module
//! \param name Module name
#define CETECH_UNLOAD_STATIC_MODULE(api, name)                     \
    extern void name ## _unload_module(struct ct_api_a0* api, int reload); \
    name ## _unload_module(api, 0)

#define CETECH_MODULE_INITAPI(name) name##_initapi_module
#define CETECH_MODULE_LOAD(name) name##_load_module
#define CETECH_MODULE_UNLOAD(name) name##_unload_module

#ifdef __cplusplus
#define CETECH_MODULE_DEF(name, initapi, load, unload) \
    extern "C" void name##_initapi_module(struct ct_api_a0 *api) initapi \
    extern "C" void name##_load_module(struct ct_api_a0 *api, int reload) load \
    extern "C" void name##_unload_module(struct ct_api_a0 *api, int reload) unload

#else
#define CETECH_MODULE_DEF(name, initapi, load, unload) \
    void name##_initapi_module(struct ct_api_a0 *api) initapi \
    void name##_load_module(struct ct_api_a0 *api, int reload) load \
    void name##_unload_module(struct ct_api_a0 *api, int reload) unload
#endif

//==============================================================================
// Typedefs
//==============================================================================

typedef void (*ct_load_module_t)(struct ct_api_a0 *api,
                                 int reload);

typedef void (*ct_unload_module_t)(struct ct_api_a0 *api,
                                   int reload);

typedef void (*ct_initapi_module_t)(struct ct_api_a0 *api);

//==============================================================================
// Interface
//==============================================================================

struct ct_module_a0 {
    //! Add static modules
    //! \param fce get api module fce
    void (*add_static)(ct_load_module_t load,
                       ct_unload_module_t unload,
                       ct_initapi_module_t initapi);

    //! Load module from path
    //! \param path Plugin path
    void (*load)(const char *path);

    //! Load module from dir
    //! \param path Plugin dir path
    void (*load_dirs)(const char* path);

    void (*unload_all)();

    //! Reload module by path
    void (*reload)(const char *path);

    //! Reload all loaded modules
    void (*reload_all)();
};

CT_MODULE(ct_module_a0);

#endif //CETECH_PLUGIN_API_H
