//==============================================================================
// Includes
//==============================================================================

#include <stdlib.h>
#include <memory.h>

#include <cetech/core/containers/buffer.h>

#include <cetech/core/api/api_system.h>
#include <cetech/core/os/path.h>
#include <cetech/core/os/object.h>
#include <cetech/core/module/module.h>
#include <cetech/core/memory/memory.h>
#include <cetech/engine/config/config.h>
#include <cetech/core/os/watchdog.h>
#include <cetech/core/hashlib/hashlib.h>
#include <cetech/core/containers/map.inl>
#include <cetech/engine/filesystem/filesystem.h>
#include <cetech/engine/coredb/coredb.h>
#include <cetech/engine/kernel/kernel.h>

#include "cetech/core/log/log.h"

//==============================================================================
// Defines
//==============================================================================

#define MAX_MODULES 128
#define MAX_PATH_LEN 256

#define _G ModuleGlobals
#define MODULE_PREFIX "module_"
#define LOG_WHERE "module_system"

//==============================================================================
// Globals
//==============================================================================
struct module_functios {
    void *handler;
    ct_load_module_t load;
    ct_unload_module_t unload;
    ct_initapi_module_t initapi;
};

static struct _G {
    module_functios modules[MAX_MODULES];
    char path[MAX_MODULES][MAX_PATH_LEN];
    char used[MAX_MODULES];
    ct_coredb_object_t *config;
    ct_alloc *allocator;
} _G = {};

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_api_a0);
CETECH_DECL_API(ct_object_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_watchdog_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_filesystem_a0);
CETECH_DECL_API(ct_coredb_a0);

#define CONFIG_MODULE_DIR CT_ID64_0(CONFIG_MODULE_DIR_ID)

using namespace celib;

//==============================================================================
// Private
//==============================================================================

static void add_module(const char *path,
                       module_functios *module) {

    for (size_t i = 0; i < MAX_MODULES; ++i) {
        if (_G.used[i]) {
            continue;
        }

        memcpy(_G.path[i], path, strlen(path) + 1);

        _G.used[i] = 1;
        _G.modules[i] = *module;

        break;
    }
}

static const char *get_module_name(const char *path,
                                   uint32_t *len) {
    const char *filename = ct_path_a0.filename(path);
    const char *name = strchr(filename, '_');
    if (NULL == name) {
        return NULL;
    }

    ++name;
    *len = strlen(name);

    return name;
}

static void get_module_fce_name(char **buffer,
                                const char *name,
                                uint32_t name_len,
                                const char *fce_name) {
    ct_buffer_clear(*buffer);
    ct_buffer_printf(buffer, _G.allocator, "%s%s", name, fce_name);
}


static bool load_from_path(module_functios *module,
                           const char *path) {
    uint32_t name_len;
    const char *name = get_module_name(path, &name_len);

    char *buffer = NULL;

    get_module_fce_name(&buffer, name, name_len, "_load_module");

    void *obj = ct_object_a0.load(path);
    if (obj == NULL) {
        return false;
    }

    auto load_fce = (ct_load_module_t) ct_object_a0.load_function(obj,
                                                                  (buffer));
    if (load_fce == NULL) {
        return false;
    }

    get_module_fce_name(&buffer, name, name_len, "_unload_module");

    auto unload_fce = (ct_unload_module_t) ct_object_a0.load_function(obj,
                                                                      buffer);
    if (unload_fce == NULL) {
        return false;
    }

    get_module_fce_name(&buffer, name, name_len, "_initapi_module");

    ct_initapi_module_t initapi_fce = (ct_initapi_module_t) ct_object_a0.load_function(
            obj, buffer);
    if (initapi_fce == NULL) {
        return false;
    }

    *module = {
            .handler = obj,
            .load = load_fce,
            .unload = unload_fce,
            .initapi = initapi_fce
    };
    return true;
}

//==============================================================================
// Interface
//==============================================================================


static void add_static(ct_load_module_t load,
                       ct_unload_module_t unload,
                       ct_initapi_module_t initapi) {

    module_functios module = {
            .load=load,
            .unload=unload,
            .initapi=initapi,
            .handler=NULL
    };

    add_module("__STATIC__", &module);
    initapi(&ct_api_a0);
    load(&ct_api_a0, 0);
}


static void load(const char *path) {
    ct_log_a0.info(LOG_WHERE, "Loading module %s", path);

    module_functios module;

    if (!load_from_path(&module, path)) {
        return;
    }

    module.initapi(&ct_api_a0);
    module.load(&ct_api_a0, 0);

    add_module(path, &module);
}

static void reload(const char *path) {
    for (size_t i = 0; i < MAX_MODULES; ++i) {
        module_functios old_module = _G.modules[i];

        if ((old_module.handler == NULL) ||
            (strcmp(_G.path[i], path)) != 0) {
            continue;
        }

        module_functios new_module;
        if (!load_from_path(&new_module, path)) {
            continue;
        }

        new_module.initapi(&ct_api_a0);
        new_module.load(&ct_api_a0, 1);

        old_module.unload(&ct_api_a0, 1);
        _G.modules[i] = new_module;

        ct_object_a0.unload(old_module.handler);

        break;
    }

    for (size_t i = 0; i < MAX_MODULES; ++i) {
        if (!_G.used[i]) {
            continue;
        }

        if ((strcmp(_G.path[i], path)) == 0) {
            continue;
        }

        module_functios module = _G.modules[i];
        module.initapi(&ct_api_a0);
    }
}

static void reload_all() {
    for (size_t i = 0; i < MAX_MODULES; ++i) {
        if (_G.modules[i].handler == NULL) {
            continue;
        }

        reload(_G.path[i]);
    }
}


static void load_dirs() {
    char key[64];
    size_t len = strlen("load_module.");
    strcpy(key, "load_module.");
    const char *path = ct_coredb_a0.read_string(_G.config,
                                                CONFIG_MODULE_DIR,
                                                "bin/darwin64");
    char* buffer = NULL;
    for (int i = 0; true; ++i) {
        ct_buffer_clear(buffer);

        sprintf(key + len, "%d", i);

        const uint64_t key_id = CT_ID64_0(key);

        if (!ct_coredb_a0.prop_exist(_G.config, key_id)) {
            break;
        }


        const char *module_file = ct_coredb_a0.read_string(_G.config,
                                                           key_id, "");
        ct_path_a0.join(&buffer,
                                            ct_memory_a0.main_allocator(),
                                            2, path, module_file);
        load(buffer);

        ct_buffer_free(buffer, _G.allocator);
    }
}

static void unload_all() {
    for (int i = MAX_MODULES - 1; i >= 0; --i) {
        if (!_G.used[i]) {
            continue;
        }

        _G.modules[i].unload(&ct_api_a0, 0);
    }
}

static void check_modules() {
    ct_alloc *alloc = ct_memory_a0.main_allocator();

    static uint64_t root = CT_ID64_0("modules");

    auto *wd_it = ct_filesystem_a0.event_begin(root);
    const auto *wd_end = ct_filesystem_a0.event_end(root);

    while (wd_it != wd_end) {
        if (wd_it->type == CT_WATCHDOG_EVENT_FILE_MODIFIED) {
            ct_wd_ev_file_write_end *ev = (ct_wd_ev_file_write_end *) wd_it;

            const char *ext = ct_path_a0.extension(ev->filename);

            if ((NULL != ext) && (strcmp(ext, "so") == 0)) {


                char *path = NULL;
                ct_path_a0.join(&path, alloc, 2, ev->dir, ev->filename);

                char full_path[4096];
                ct_filesystem_a0.get_full_path(root, path, full_path,
                                               CETECH_ARRAY_LEN(full_path));

                int pat_size = strlen(full_path);
                int ext_size = strlen(ext);
                full_path[pat_size - ext_size - 1] = '\0';

                ct_log_a0.info(LOG_WHERE,
                               "Reload module from path \"%s\"",
                               full_path);

                reload(full_path);

                ct_buffer_free(path, alloc);
            }
        }

        wd_it = ct_filesystem_a0.event_next(wd_it);
    }
}

static ct_module_a0 module_api = {
        .reload = reload,
        .reload_all = reload_all,
        .add_static = add_static,
        .load = load,
        .unload_all = unload_all,
        .load_dirs = load_dirs,
        .check_modules = check_modules
};

//static const char* _get_load_dir() {
//#if defined(CETECH_LINUX)
//    return "./bin/linux64/";
//#elif defined(CETECH_DARWIN)
//    return "./bin/darwin64/";
//#endif
//}

static void _init(struct ct_api_a0* api){
    _G = (struct _G) {
            .allocator = ct_memory_a0.main_allocator(),
            .config = ct_config_a0.config_object()
    };

    ct_api_a0 = *api;
    api->register_api("ct_module_a0", &module_api);

    static uint64_t root = CT_ID64_0("modules");
    ct_filesystem_a0.map_root_dir(root,
                                  ct_coredb_a0.read_string(
                                          _G.config,
                                          CONFIG_MODULE_DIR,
                                          "bin/darwin64"), true);
}

CETECH_MODULE_DEF(
        module,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_object_a0);
            CETECH_GET_API(api, ct_config_a0);
            CETECH_GET_API(api, ct_watchdog_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_filesystem_a0);
            CETECH_GET_API(api, ct_coredb_a0);
        },
        {
            CT_UNUSED(reload);
            _init(api);

        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);
            ct_log_a0.debug(LOG_WHERE, "Shutdown");

            _G = (struct _G){};
        }
)
