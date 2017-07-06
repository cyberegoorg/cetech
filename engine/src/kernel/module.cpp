//==============================================================================
// Includes
//==============================================================================

#include <stdlib.h>
#include <memory.h>

#include <cetech/kernel/os.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/module.h>
#include <cetech/kernel/config.h>
#include <cetech/kernel/api_system.h>
#include <cetech/kernel/log.h>

#include "module_private.h"

//==============================================================================
// Defines
//==============================================================================

#define MAX_PLUGINS 256
#define MAX_PATH_LEN 256

#define PLUGIN_PREFIX "module_"
#define LOG_WHERE "module_system"


//==============================================================================
// Globals
//==============================================================================

static struct ModuleSystemGlobals {
    ct_load_module_t load_module[MAX_PLUGINS];
    ct_unload_module_t unload_module[MAX_PLUGINS];
    void *module_handler[MAX_PLUGINS];
    char used[MAX_PLUGINS];
    char path[MAX_PLUGINS][MAX_PATH_LEN];
} _G = {0};

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_api_a0);

//==============================================================================
// Private
//==============================================================================


void *load_object(const char *path);

void unload_object(void *so);

void *load_function(void *so,
                    const char *name);


void _add(const char path[11],
          ct_load_module_t load_fce,
          ct_unload_module_t unload_fce,
          void *handler) {

    for (size_t i = 0; i < MAX_PLUGINS; ++i) {
        if (_G.used[i]) {
            continue;
        }

        memcpy(_G.path[i], path, strlen(path));

        _G.used[i] = 1;

        _G.load_module[i] = load_fce;
        _G.unload_module[i] = unload_fce;
        _G.module_handler[i] = handler;

        break;
    }
}


//==============================================================================
// Interface
//==============================================================================

namespace module {

    void add_static(ct_load_module_t load,
                    ct_unload_module_t unload) {
        _add("__STATIC__", load, unload, NULL);
        load(&ct_api_a0);
    }

    void load(const char *path) {
        ct_log_a0.info(LOG_WHERE, "Loading module %s", path);

        void *obj = load_object(path);
        if (obj == NULL) {
            return;
        }

        ct_load_module_t load_fce = (ct_load_module_t) load_function(obj,
                                                               "load_module");
        if (load_fce == NULL) {
            return;
        }

        ct_unload_module_t unload_fce = (ct_unload_module_t) load_function(obj,
                                                                     "unload_module");
        if (unload_fce == NULL) {
            return;
        }

        load_fce(&ct_api_a0);

        _add(path, load_fce, unload_fce, nullptr);
    }

    void reload(const char *path) {
//        for (size_t i = 0; i < MAX_PLUGINS; ++i) {
//            if ((_G.module_handler[i] == NULL) ||
//                (strcmp(_G.path[i], path)) != 0) {
//                continue;
//            }
//
//            void *data = NULL;
//            struct module_export_a0 *api = _G.module_api[i];
//            if (api != NULL && api->reload_begin) {
//                data = api->reload_begin(&ct_api_a0);
//            }
//
//            unload_object(_G.module_handler[i]);
//
//            void *obj = load_object(path);
//            if (obj == NULL) {
//                return;
//            }
//
//            void *fce = load_function(obj, "load_module");
//            if (fce == NULL) {
//                return;
//            }
//
//            _G.module_api[i] = api = (module_export_a0 *) ((get_api_fce_t) fce)(
//                    PLUGIN_EXPORT_API_ID);
//            if (api != NULL && api->reload_end) {
//                api->reload_end(&ct_api_a0, data);
//            }
//        }
    }

    void reload_all() {
//        for (size_t i = 0; i < MAX_PLUGINS; ++i) {
//            if (_G.module_handler[i] == NULL) {
//                continue;
//            }
//
//            void *data = NULL;
//            struct module_export_a0 *api = _G.module_api[i];
//            if (api != NULL && api->reload_begin) {
//                data = api->reload_begin(&ct_api_a0);
//            }
//
//            unload_object(_G.module_handler[i]);
//
//            void *obj = load_object(_G.path[i]);
//            if (obj == NULL) {
//                return;
//            }
//
//            void *fce = load_function(obj, "load_module");
//            if (fce == NULL) {
//                return;
//            }
//
//            _G.module_api[i] = api = (module_export_a0 *) ((get_api_fce_t) fce)(
//                    PLUGIN_EXPORT_API_ID);
//            if (api != NULL && api->reload_end) {
//                api->reload_end(&ct_api_a0, data);
//            }
//        }
    }


    void load_dirs(const char *path) {
        char **files = nullptr;
        uint32_t files_count = 0;

        ct_path_a0.list(path, 1, &files, &files_count,
                        ct_memory_a0.main_allocator());

        for (int k = 0; k < files_count; ++k) {
            const char *filename = ct_path_a0.filename(files[k]);

            if (!strncmp(filename, PLUGIN_PREFIX, strlen(PLUGIN_PREFIX))) {
                load(files[k]);
            }
        }

        ct_path_a0.list_free(files, files_count,
                             ct_memory_a0.main_allocator());
    }

    void unload_all() {
        for (int i = MAX_PLUGINS - 1; i >= 0; --i) {
            if (!_G.used[i]) {
                continue;
            }

            _G.unload_module[i](&ct_api_a0);
        }
    }


    static ct_module_a0 module_api{
            .module_reload = reload,
            .module_reload_all = reload_all
    };

    void init(ct_allocator *allocator,
              struct ct_api_a0 *api) {
        CETECH_GET_API(api, ct_memory_a0);
        CETECH_GET_API(api, ct_path_a0);
        CETECH_GET_API(api, ct_log_a0);
        ct_api_a0 = *api;

        api->register_api("ct_module_a0", &module_api);

        _G = {0};

    }

    void shutdown() {
        _G = {0};
    }

}