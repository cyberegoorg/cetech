#ifndef CETECH__MODULE_H
#define CETECH__MODULE_H

#include <cetech/kernel/module.h>

//! Add static module and load it
//! \param name Module name
#define ADD_STATIC_MODULE(name)                                     \
    extern void name ## _load_module(struct api_v0* api);           \
    extern void name ## _unload_module(struct api_v0* api);         \
    module::add_static(name ## _load_module, name ## _unload_module)


namespace module {
    void init(struct allocator *allocator,
              struct api_v0 *api_v0);

    void shutdown();

    //! Add static modules
    //! \param fce get api module fce
    void add_static(load_module_t load, unload_module_t unload);

    //! Load module from path
    //! \param path Plugin path
    void load(const char *path);

    //! Load module from dir
    //! \param path Plugin dir path
    void load_dirs(const char *path);

    //! Reload module by path
    void reload(const char *path);

    //! Reload all loaded modules
    void reload_all();

    void unload_all();
}

#endif //CETECH__MODULE_H
