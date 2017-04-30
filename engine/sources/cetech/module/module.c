//==============================================================================
// Includes
//==============================================================================

#include <stdlib.h>
#include <memory.h>
#include <cetech/object.h>
#include <cetech/fs.h>
#include <cetech/path.h>
#include <cetech/memory.h>

#include <cetech/application/module.h>
#include <cetech/string.h>

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

#define _G PluginSystemGlobals

static struct G {
    get_api_fce_t get_module_api[MAX_PLUGINS];
    struct module_api_v0 *module_api[MAX_PLUGINS];
    void *module_handler[MAX_PLUGINS];
    char used[MAX_PLUGINS];
    char path[MAX_PLUGINS][MAX_PATH_LEN];
} PluginSystemGlobals = {0};

//==============================================================================
// Private
//==============================================================================


void *load_object(const char *path);
void unload_object(void *so);
void *load_function(void *so,
                        void *name);

void _callm_init(get_api_fce_t fce) {
    struct module_api_v0 *api = fce(PLUGIN_EXPORT_API_ID);

    if (api) {
        CETECH_ASSERT("module", api->init != NULL);
        api->init(module_get_engine_api);
    }
}

void _callm_shutdown(get_api_fce_t fce) {
    struct module_api_v0 *api = fce(PLUGIN_EXPORT_API_ID);

    if (api) {
        CETECH_ASSERT("module", api->shutdown != NULL);
        api->shutdown();
    }
}

void _add(const char *path,
          get_api_fce_t fce,
          void *handler) {

    for (size_t i = 0; i < MAX_PLUGINS; ++i) {
        if (_G.used[i]) {
            continue;
        }

        str_set(_G.path[i], path);
        _G.module_api[i] = fce(PLUGIN_EXPORT_API_ID);
        _G.get_module_api[i] = fce;
        _G.module_handler[i] = handler;
        _G.used[i] = 1;

        break;
    }
}


//==============================================================================
// Interface
//==============================================================================

void module_add_static(get_api_fce_t fce) {
    _add("__STATIC__", fce, NULL);
//    _callm_init(fce);
}

void module_load(const char *path) {
    log_info(LOG_WHERE, "Loading module %s", path);

    void *obj = load_object(path);
    if (obj == NULL) {
        return;
    }

    void *fce = load_function(obj, "get_module_api");
    if (fce == NULL) {
        return;
    }

    _add(path, fce, obj);
//    _callm_init(fce);
}

void module_reload(const char *path) {
    for (size_t i = 0; i < MAX_PLUGINS; ++i) {
        if ((_G.module_handler[i] == NULL) ||
            (strcmp(_G.path[i], path)) != 0) {
            continue;
        }

        void *data = NULL;
        struct module_api_v0 *api = _G.module_api[i];
        if (api != NULL && api->reload_begin) {
            data = api->reload_begin(module_get_engine_api);
        }

        unload_object(_G.module_handler[i]);

        void *obj = load_object(path);
        if (obj == NULL) {
            return;
        }

        void *fce = load_function(obj, "get_module_api");
        if (fce == NULL) {
            return;
        }

        _G.module_api[i] = api = ((get_api_fce_t) fce)(PLUGIN_EXPORT_API_ID);
        if (api != NULL && api->reload_end) {
            api->reload_end(module_get_engine_api, data);
        }
    }
}

void module_reload_all() {
    for (size_t i = 0; i < MAX_PLUGINS; ++i) {
        if (_G.module_handler[i] == NULL) {
            continue;
        }

        void *data = NULL;
        struct module_api_v0 *api = _G.module_api[i];
        if (api != NULL && api->reload_begin) {
            data = api->reload_begin(module_get_engine_api);
        }

        unload_object(_G.module_handler[i]);

        void *obj = load_object(_G.path[i]);
        if (obj == NULL) {
            return;
        }

        void *fce = load_function(obj, "get_module_api");
        if (fce == NULL) {
            return;
        }

        _G.module_api[i] = api = ((get_api_fce_t) fce)(PLUGIN_EXPORT_API_ID);
        if (api != NULL && api->reload_end) {
            api->reload_end(module_get_engine_api, data);
        }
    }
}

void *module_get_engine_api(int api) {
    for (size_t i = 0; i < MAX_PLUGINS; ++i) {
        if (!_G.used[i]) {
            continue;
        }

        void *p_api = _G.get_module_api[i](api);
        if (p_api != NULL) {
            return p_api;
        }
    }

    return NULL;
}

void module_load_dirs(const char *path) {
    ARRAY_T(pchar) files;
    ARRAY_INIT(pchar, &files, _memsys_main_scratch_allocator());

    dir_list(path, 1, &files, _memsys_main_scratch_allocator());

    for (int k = 0; k < ARRAY_SIZE(&files); ++k) {
        const char *filename = path_filename(ARRAY_AT(&files, k));

        if (str_startswith(filename, PLUGIN_PREFIX)) {
            module_load(ARRAY_AT(&files, k));
        }
    }

    dir_list_free(&files, _memsys_main_scratch_allocator());
    ARRAY_DESTROY(pchar, &files);
}

void module_call_init() {
    for (size_t i = 0; i < MAX_PLUGINS; ++i) {
        if (!_G.used[i] || !_G.module_api[i]->init) {
            continue;
        }

        _G.module_api[i]->init(module_get_engine_api);
    }
}

void module_call_init_cvar() {
    struct config_api_v0 ConfigApiV0 = *(struct config_api_v0 *) module_get_engine_api(CONFIG_API_ID);

    for (size_t i = 0; i < MAX_PLUGINS; ++i) {
        if (!_G.used[i] || !_G.module_api[i]->init_cvar) {
            continue;
        }

        _G.module_api[i]->init_cvar(ConfigApiV0);
    }
}

void module_call_shutdown() {
    for (int i = MAX_PLUGINS - 1; i >= 0; --i) {
        if (!_G.used[i] || !_G.module_api[i]->shutdown) {
            continue;
        }

        _G.module_api[i]->shutdown();
    }
}

void module_call_update() {
    for (size_t i = 0; i < MAX_PLUGINS; ++i) {
        if (!_G.used[i] || !_G.module_api[i]->update) {
            continue;
        }

        _G.module_api[i]->update();
    }
}

void module_call_after_update(float dt) {
    for (size_t i = 0; i < MAX_PLUGINS; ++i) {
        if (!_G.used[i] || !_G.module_api[i]->after_update) {
            continue;
        }

        _G.module_api[i]->after_update(dt);
    }
}