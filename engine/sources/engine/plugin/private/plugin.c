//==============================================================================
// Includes
//==============================================================================

#include <stdlib.h>
#include <memory.h>
#include <celib/os/object.h>
#include <celib/filesystem/filesystem.h>
#include <celib/filesystem/path.h>
#include <engine/memory/memsys.h>

#include "engine/plugin/plugin.h"
#include "engine/config/cvar.h"

//==============================================================================
// Defines
//==============================================================================

#define MAX_PLUGINS 256
#define MAX_PATH_LEN 256
#define PLUGIN_PREFIX "plugin_"
#define LOG_WHERE "plugin_system"

//==============================================================================
// Globals
//==============================================================================

#define _G PluginSystemGlobals

static struct G {
    get_api_fce_t get_plugin_api[MAX_PLUGINS];
    struct plugin_api_v0 *plugin_api[MAX_PLUGINS];
    void *plugin_handler[MAX_PLUGINS];
    char used[MAX_PLUGINS];
    char path[MAX_PLUGINS][MAX_PATH_LEN];
} PluginSystemGlobals = {0};

//==============================================================================
// Private
//==============================================================================

void _callm_init(get_api_fce_t fce) {
    struct plugin_api_v0 *api = fce(PLUGIN_API_ID, 0);


    if (api) {
        CEL_ASSERT("plugin", api->init != NULL);
        api->init(plugin_get_engine_api);
    }
}

void _callm_shutdown(get_api_fce_t fce) {
    struct plugin_api_v0 *api = fce(PLUGIN_API_ID, 0);

    if (api) {
        CEL_ASSERT("plugin", api->shutdown != NULL);
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

        cel_str_set(_G.path[i], path);
        _G.plugin_api[i] = fce(PLUGIN_API_ID, 0);
        _G.get_plugin_api[i] = fce;
        _G.plugin_handler[i] = handler;
        _G.used[i] = 1;

        break;
    }
}


//==============================================================================
// Interface
//==============================================================================

void plugin_add_static(get_api_fce_t fce) {
    _add("__STATIC__", fce, NULL);
//    _callm_init(fce);
}

void plugin_load(const char *path) {
    log_info(LOG_WHERE, "Loading plugin %s", path);

    void *obj = cel_load_object(path);
    if (obj == NULL) {
        return;
    }

    void *fce = cel_load_function(obj, "get_plugin_api");
    if (fce == NULL) {
        return;
    }

    _add(path, fce, obj);
//    _callm_init(fce);
}

void plugin_reload(const char *path) {
    for (size_t i = 0; i < MAX_PLUGINS; ++i) {
        if ((_G.plugin_handler[i] == NULL) || (cel_strcmp(_G.path[i], path)) != 0) {
            continue;
        }

        void *data = NULL;
        struct plugin_api_v0 *api = _G.plugin_api[i];
        if (api != NULL && api->reload_begin) {
            data = api->reload_begin(plugin_get_engine_api);
        }

        cel_unload_object(_G.plugin_handler[i]);

        void *obj = cel_load_object(path);
        if (obj == NULL) {
            return;
        }

        void *fce = cel_load_function(obj, "get_plugin_api");
        if (fce == NULL) {
            return;
        }

        _G.plugin_api[i] = api = ((get_api_fce_t) fce)(PLUGIN_API_ID, 0);
        if (api != NULL && api->reload_end) {
            api->reload_end(plugin_get_engine_api, data);
        }
    }
}

void plugin_reload_all() {
    for (size_t i = 0; i < MAX_PLUGINS; ++i) {
        if (_G.plugin_handler[i] == NULL) {
            continue;
        }

        void *data = NULL;
        struct plugin_api_v0 *api = _G.plugin_api[i];
        if (api != NULL && api->reload_begin) {
            data = api->reload_begin(plugin_get_engine_api);
        }

        cel_unload_object(_G.plugin_handler[i]);

        void *obj = cel_load_object(_G.path[i]);
        if (obj == NULL) {
            return;
        }

        void *fce = cel_load_function(obj, "get_plugin_api");
        if (fce == NULL) {
            return;
        }

        _G.plugin_api[i] = api = ((get_api_fce_t) fce)(PLUGIN_API_ID, 0);
        if (api != NULL && api->reload_end) {
            api->reload_end(plugin_get_engine_api, data);
        }
    }
}

void *plugin_get_engine_api(int api,
                            int version) {
    for (size_t i = 0; i < MAX_PLUGINS; ++i) {
        if (!_G.used[i]) {
            continue;
        }

        void *p_api = _G.get_plugin_api[i](api, version);
        if (p_api != NULL) {
            return p_api;
        }
    }

    return NULL;
}

void plugin_load_dirs(const char *path) {
    ARRAY_T(pchar) files;
    ARRAY_INIT(pchar, &files, _memsys_main_scratch_allocator());

    cel_dir_list(path, 1, &files, _memsys_main_scratch_allocator());

    for (int k = 0; k < ARRAY_SIZE(&files); ++k) {
        const char *filename = cel_path_filename(ARRAY_AT(&files, k));

        if (cel_str_startswith(filename, PLUGIN_PREFIX)) {
            plugin_load(ARRAY_AT(&files, k));
        }
    }

    cel_dir_list_free(&files, _memsys_main_scratch_allocator());
    ARRAY_DESTROY(pchar, &files);
}

void plugin_call_init() {
    for (size_t i = 0; i < MAX_PLUGINS; ++i) {
        if (!_G.used[i] || !_G.plugin_api[i]->init) {
            continue;
        }

        _G.plugin_api[i]->init(plugin_get_engine_api);
    }
}

void plugin_call_init_cvar() {
    for (size_t i = 0; i < MAX_PLUGINS; ++i) {
        if (!_G.used[i] || !_G.plugin_api[i]->init_cvar) {
            continue;
        }

        _G.plugin_api[i]->init_cvar(ConfigApiV1);
    }
}

void plugin_call_shutdown() {
    for (size_t i = 0; i < MAX_PLUGINS; ++i) {
        if (!_G.used[i] || !_G.plugin_api[i]->shutdown) {
            continue;
        }

        _G.plugin_api[i]->shutdown();
    }
}

void plugin_call_update() {
    for (size_t i = 0; i < MAX_PLUGINS; ++i) {
        if (!_G.used[i] || !_G.plugin_api[i]->update) {
            continue;
        }

        _G.plugin_api[i]->update();
    }
}

void plugin_call_after_update(float dt) {
    for (size_t i = 0; i < MAX_PLUGINS; ++i) {
        if (!_G.used[i] || !_G.plugin_api[i]->after_update) {
            continue;
        }

        _G.plugin_api[i]->after_update(dt);
    }
}