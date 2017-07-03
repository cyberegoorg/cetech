//==============================================================================
// Includes
//==============================================================================

#include <stdlib.h>
#include <memory.h>

#include <cetech/kernel/path.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/module.h>
#include <cetech/kernel/config.h>
#include <cetech/kernel/api.h>
#include <cetech/kernel/log.h>


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
    load_module_t load_module[MAX_PLUGINS];
    unload_module_t unload_module[MAX_PLUGINS];
    void *module_handler[MAX_PLUGINS];
    char used[MAX_PLUGINS];
    char path[MAX_PLUGINS][MAX_PATH_LEN];
} _G = {0};

CETECH_DECL_API(memory_api_v0);
CETECH_DECL_API(path_v0);
CETECH_DECL_API(log_api_v0);
CETECH_DECL_API(api_v0);

//==============================================================================
// Private
//==============================================================================


void *load_object(const char *path);

void unload_object(void *so);

void *load_function(void *so,
                    const char *name);


void _add(const char path[11],
          load_module_t load_fce,
          unload_module_t unload_fce,
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

    void add_static(load_module_t load,
                    unload_module_t unload) {
        _add("__STATIC__", load, unload, NULL);
        load(&api_v0);
    }

    void load(const char *path) {
        log_api_v0.info(LOG_WHERE, "Loading module %s", path);

        void *obj = load_object(path);
        if (obj == NULL) {
            return;
        }

        load_module_t load_fce = (load_module_t) load_function(obj, "load_module");
        if (load_fce == NULL) {
            return;
        }

        unload_module_t unload_fce = (unload_module_t) load_function(obj, "unload_module");
        if (unload_fce == NULL) {
            return;
        }

        load_fce(&api_v0);

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
//            struct module_export_api_v0 *api = _G.module_api[i];
//            if (api != NULL && api->reload_begin) {
//                data = api->reload_begin(&api_v0);
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
//            _G.module_api[i] = api = (module_export_api_v0 *) ((get_api_fce_t) fce)(
//                    PLUGIN_EXPORT_API_ID);
//            if (api != NULL && api->reload_end) {
//                api->reload_end(&api_v0, data);
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
//            struct module_export_api_v0 *api = _G.module_api[i];
//            if (api != NULL && api->reload_begin) {
//                data = api->reload_begin(&api_v0);
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
//            _G.module_api[i] = api = (module_export_api_v0 *) ((get_api_fce_t) fce)(
//                    PLUGIN_EXPORT_API_ID);
//            if (api != NULL && api->reload_end) {
//                api->reload_end(&api_v0, data);
//            }
//        }
    }


    void load_dirs(const char *path) {
        char **files = nullptr;
        uint32_t files_count = 0;

        path_v0.list(path, 1, &files, &files_count,
                     memory_api_v0.main_scratch_allocator());

        for (int k = 0; k < files_count; ++k) {
            const char *filename = path_v0.filename(files[k]);

            if (!strncmp(filename, PLUGIN_PREFIX, strlen(PLUGIN_PREFIX))) {
                load(files[k]);
            }
        }

        path_v0.list_free(files, files_count,
                          memory_api_v0.main_scratch_allocator());
    }

    void unload_all() {
        for (int i = MAX_PLUGINS - 1; i >= 0; --i) {
            if (!_G.used[i]) {
                continue;
            }

            _G.unload_module[i](&api_v0);
        }
    }


    static module_api_v0 module_api{
            .module_reload = reload,
            .module_reload_all = reload_all
    };

    void init(struct allocator *allocator,
              struct api_v0 *api) {
        CETECH_GET_API(api, memory_api_v0);
        CETECH_GET_API(api, path_v0);
        CETECH_GET_API(api, log_api_v0);
        api_v0 = *api;

        api->register_api("module_api_v0", &module_api);

        _G = {0};

    }

    void shutdown() {
        _G = {0};
    }

}