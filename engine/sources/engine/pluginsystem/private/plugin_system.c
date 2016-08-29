//==============================================================================
// Includes
//==============================================================================

#include <stdlib.h>
#include <memory.h>
#include <celib/os/object.h>
#include "engine/machine/machine.h"
#include "celib/string/string.h"

#include "engine/pluginsystem/plugin_system.h"

//==============================================================================
// Defines
//==============================================================================

#define MAX_PATH_LEN 256
#define PLUGIN_PREFIX "plugin_"
#define LOG_WHERE "plugin_system"


//==============================================================================
// Globals
//==============================================================================

static struct {
    get_api_fce_t get_plugin_api[MAX_PLUGINS];
    struct plugin_api_v0 *plugin_api[MAX_PLUGINS];
    void *plugin_handler[MAX_PLUGINS];
    char used[MAX_PLUGINS];
    char path[MAX_PLUGINS][MAX_PATH_LEN];
} _G = {0};

//==============================================================================
// Private
//==============================================================================

void _callm_init(get_api_fce_t fce) {
    struct plugin_api_v0 *api = fce(PLUGIN_API_ID, 0);
    if (api) {
        api->init(plugin_get_engine_api);
    }
}

void _callm_shutdown(get_api_fce_t fce) {
    struct plugin_api_v0 *api = fce(PLUGIN_API_ID, 0);
    if (api) {
        api->shutdown();
    }
}

void _add(const char *path, get_api_fce_t fce, void *handler) {
    for (size_t i = 0; i < MAX_PLUGINS; ++i) {
        if (_G.used[i]) {
            continue;
        }

        str_set(_G.path[i], path);
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
    _callm_init(fce);
}

void plugin_load(const char *path) {
    void *obj = os_load_object(path);
    if (obj != NULL) {
        return;
    }

    void *fce = os_load_function(obj, "get_plugin_api");
    if (fce != NULL) {
        return;
    }

    _add(path, fce, obj);
    _callm_init(fce);
}

void plugin_reload(const char *path) {
    for (size_t i = 0; i < MAX_PLUGINS; ++i) {
        if ((_G.plugin_handler[i] == NULL) || (str_compare(_G.path[i], path)) != 0) {
            continue;
        }

        struct plugin_api_v0 *api = _G.plugin_api[i];
        void *data = api->reload_begin(plugin_get_engine_api);

        os_unload_object(_G.plugin_handler[i]);

        void *obj = os_load_object(path);
        if (obj != NULL) {
            return;
        }

        void *fce = os_load_function(obj, "get_plugin_api");
        if (fce != NULL) {
            return;
        }

        _G.plugin_api[i] = api = ((get_api_fce_t) fce)(PLUGIN_API_ID, 0);
        if (api != NULL) {
            api->reload_end(plugin_get_engine_api, data);
        }
    }
}

void *plugin_get_engine_api(int api, int version) {
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
//    ALLOCATOR_CREATE_SCOPED(tmp_alloc, mallocator);
//    ARRAY_T(char_p) files;
//    ARRAY_INIT(char_p, &files, tmp_alloc);
//
//    os_dir_list(path, 0, &files, tmp_alloc);
//
//    char buffer[_64B];
//    for (int k = 0; k < ARRAY_SIZE(&files); ++k) {
//        str_set(buffer, path);
//
//        if (str_startswith(ARRAY_AT(&files, k), PLUGIN_PREFIX)) {
//            strcpy(buffer + str_lenght(path), ARRAY_AT(&files, k));
//            plugin_load(buffer);
//        }
//    }
//
//    os_dir_list_free(&files, tmp_alloc);
//    ARRAY_DESTROY(char_p, &files);
}

void plugin_callm_update() {
    for (size_t i = 0; i < MAX_PLUGINS; ++i) {
        if (!_G.used[i] || !_G.plugin_api[i]->update) {
            continue;
        }

        _G.plugin_api[i]->update();

        break;
    }
}