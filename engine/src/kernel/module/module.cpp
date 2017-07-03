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
    get_api_fce_t load_module[MAX_PLUGINS];
    struct module_export_api_v0 *module_api[MAX_PLUGINS];
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


void _add(const char *path,
          get_api_fce_t fce,
          void *handler) {

    for (size_t i = 0; i < MAX_PLUGINS; ++i) {
        if (_G.used[i]) {
            continue;
        }

        memcpy(_G.path[i], path, strlen(path));

        module_export_api_v0 *api = (module_export_api_v0 *) fce(
                PLUGIN_EXPORT_API_ID);

        _G.module_api[i] = api;
        _G.load_module[i] = fce;
        _G.module_handler[i] = handler;
        _G.used[i] = 1;

        break;
    }
}


//==============================================================================
// Interface
//==============================================================================

namespace module {

    void add_static(get_api_fce_t fce) {
        _add("__STATIC__", fce, NULL);
//    _callm_init(fce);
    }

    void load(const char *path) {
        log_api_v0.info(LOG_WHERE, "Loading module %s", path);

        void *obj = load_object(path);
        if (obj == NULL) {
            return;
        }

        void *fce = load_function(obj, "load_module");
        if (fce == NULL) {
            return;
        }

        _add(path, (get_api_fce_t) fce, obj);
//    _callm_init(fce);
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

    void call_init() {
        for (size_t i = 0; i < MAX_PLUGINS; ++i) {
            if (!_G.used[i] || !_G.module_api[i]->init) {
                continue;
            }

            _G.module_api[i]->init(&api_v0);
        }
    }


    void call_shutdown() {
        for (int i = MAX_PLUGINS - 1; i >= 0; --i) {
            if (!_G.used[i] || !_G.module_api[i]->shutdown) {
                continue;
            }

            _G.module_api[i]->shutdown();
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