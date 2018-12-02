#ifndef CE_PLUGIN_API_H
#define CE_PLUGIN_API_H

#include <celib/module.inl>

struct ce_api_a0;

//! Add static module and load it
//! \param name Module name
#define CE_ADD_STATIC_MODULE(name)                                     \
    extern void name ## _initapi_module(struct ce_api_a0*);               \
    extern void name ## _load_module(struct ce_api_a0*, int reload);               \
    extern void name ## _unload_module(struct ce_api_a0*, int reload);             \
    ce_module_a0->add_static(name ## _load_module, name ## _unload_module, name ## _initapi_module)

//! Load static module
//! \param name Module name
#define CE_LOAD_STATIC_MODULE(api, name)                 \
    extern void name ## _load_module(struct ce_api_a0*, int reload); \
    extern void name ## _initapi_module(struct ce_api_a0*); \
    name ## _initapi_module(api); \
    name ## _load_module(api, 0)

//! Unload static module
//! \param name Module name
#define CE_UNLOAD_STATIC_MODULE(api, name)                     \
    extern void name ## _unload_module(struct ce_api_a0* api, int reload); \
    name ## _unload_module(api, 0)

#define CE_MODULE_INITAPI(name) name##_initapi_module
#define CE_MODULE_LOAD(name) name##_load_module
#define CE_MODULE_UNLOAD(name) name##_unload_module

#ifdef __cplusplus
#define CE_MODULE_DEF(name, initapi, load, unload) \
    extern "C" void name##_initapi_module(struct ce_api_a0 *api) initapi \
    extern "C" void name##_load_module(struct ce_api_a0 *api, int reload) load \
    extern "C" void name##_unload_module(struct ce_api_a0 *api, int reload) unload

#else
#define CE_MODULE_DEF(name, initapi, load, unload) \
    void name##_initapi_module(struct ce_api_a0 *api) initapi \
    void name##_load_module(struct ce_api_a0 *api, int reload) load \
    void name##_unload_module(struct ce_api_a0 *api, int reload) unload
#endif

typedef void (*ce_load_module_t)(struct ce_api_a0 *api,
                                 int reload);

typedef void (*ce_unload_module_t)(struct ce_api_a0 *api,
                                   int reload);

typedef void (*ce_initapi_module_t)(struct ce_api_a0 *api);

struct ce_module_a0 {
    //! Add static modules
    //! \param fce get api module fce
    void (*add_static)(ce_load_module_t load,
                       ce_unload_module_t unload,
                       ce_initapi_module_t initapi);

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

CE_MODULE(ce_module_a0);

#endif //CE_PLUGIN_API_H
