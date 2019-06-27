#ifndef CE_PLUGIN_API_H
#define CE_PLUGIN_API_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib_types.h"

struct ce_api_a0;

#define CE_MODULE0_API \
    CE_ID64_0("ce_module_a0", 0x23a8afeb527d63dcULL)

//! Add static module and load it
//! \param name Module name
#define CE_ADD_STATIC_MODULE(name)                                     \
    extern void name ## _load_module(struct ce_api_a0*, int reload);               \
    extern void name ## _unload_module(struct ce_api_a0*, int reload);             \
    ce_module_a0->add_static(#name, name ## _load_module, name ## _unload_module)

//! Load static module
//! \param name Module name
#define CE_LOAD_STATIC_MODULE(api, name)                 \
    extern void name ## _load_module(struct ce_api_a0*, int reload); \
    name ## _load_module(api, 0)

//! Unload static module
//! \param name Module name
#define CE_UNLOAD_STATIC_MODULE(api, name)                     \
    extern void name ## _unload_module(struct ce_api_a0* api, int reload); \
    name ## _unload_module(api, 0)

#define CE_MODULE_LOAD(name) name##_load_module
#define CE_MODULE_UNLOAD(name) name##_unload_module

typedef void (ce_load_module_t0)(struct ce_api_a0 *api,
                                 int reload);

typedef void (ce_unload_module_t0)(struct ce_api_a0 *api,
                                   int reload);

struct ce_module_a0 {
    //! Add static modules
    //! \param fce get api module fce
    void (*add_static)(const char *name,
                       ce_load_module_t0 *load,
                       ce_unload_module_t0 *unload);

    //! Load module from path
    //! \param path Plugin path
    void (*load)(const char *path);

    //! Load module from dir
    //! \param path Plugin dir path
    void (*load_dirs)(const char *path);

    void (*unload_all)(void);

    //! Reload module by path
    void (*reload)(const char *path);

    //! Reload all loaded modules
    void (*reload_all)(void);

    void (*do_reload)();
};

CE_MODULE(ce_module_a0);

#ifdef __cplusplus
};
#endif

#endif //CE_PLUGIN_API_H
