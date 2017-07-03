#ifndef CETECH__MODULE_H
#define CETECH__MODULE_H

#include <cetech/kernel/module.h>


namespace module {
    void init(struct allocator *allocator,
              struct api_v0 *api_v0);

    void shutdown();

    //! Add static modules
    //! \param fce get api module fce
    void add_static(get_api_fce_t fce);

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

    //! Call init
    void call_init();

    //! Call shutdown
    void call_shutdown();

}

#endif //CETECH__MODULE_H
