//==============================================================================
// Includes
//==============================================================================

#include <stdlib.h>
#include <memory.h>

#include <corelib/buffer.inl>

#include <corelib/api_system.h>
#include <corelib/os.h>

#include <corelib/module.h>
#include <corelib/memory.h>
#include <corelib/config.h>

#include <corelib/hashlib.h>
#include <corelib/fs.h>
#include <corelib/cdb.h>
#include <cetech/kernel/kernel.h>

#include "corelib/log.h"

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
    struct module_functios modules[MAX_MODULES];
    char path[MAX_MODULES][MAX_PATH_LEN];
    char used[MAX_MODULES];
    uint64_t config;
    struct ct_alloc *allocator;
} _G = {};


//==============================================================================
// Private
//==============================================================================

static void add_module(const char *path,
                       struct module_functios *module) {

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
    const char *filename = ct_os_a0->path->filename(path);
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


static bool load_from_path(struct module_functios *module,
                           const char *path) {
    uint32_t name_len;
    const char *name = get_module_name(path, &name_len);

    char *buffer = NULL;

    get_module_fce_name(&buffer, name, name_len, "_load_module");

    void *obj = ct_os_a0->object->load(path);
    if (obj == NULL) {
        return false;
    }

    ct_load_module_t load_fce = (ct_load_module_t) ct_os_a0->object->load_function(
            obj,
            (buffer));
    if (load_fce == NULL) {
        return false;
    }

    get_module_fce_name(&buffer, name, name_len, "_unload_module");

    ct_unload_module_t unload_fce = (ct_unload_module_t) ct_os_a0->object->load_function(
            obj,
            buffer);
    if (unload_fce == NULL) {
        return false;
    }

    get_module_fce_name(&buffer, name, name_len, "_initapi_module");

    ct_initapi_module_t initapi_fce = (ct_initapi_module_t) ct_os_a0->object->load_function(
            obj, buffer);
    if (initapi_fce == NULL) {
        return false;
    }

    *module = (struct module_functios) {
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

    struct module_functios module = {
            .load=load,
            .unload=unload,
            .initapi=initapi,
            .handler=NULL
    };

    add_module("__STATIC__", &module);
    initapi(ct_api_a0);
    load(ct_api_a0, 0);
}


static void load(const char *path) {
    ct_log_a0->info(LOG_WHERE, "Loading module %s", path);

    struct module_functios module;

    if (!load_from_path(&module, path)) {
        return;
    }

    module.initapi(ct_api_a0);
    module.load(ct_api_a0, 0);

    add_module(path, &module);
}

static void reload(const char *path) {
    for (size_t i = 0; i < MAX_MODULES; ++i) {
        struct module_functios old_module = _G.modules[i];

        if ((old_module.handler == NULL) ||
            (strcmp(_G.path[i], path)) != 0) {
            continue;
        }

        struct module_functios new_module;
        if (!load_from_path(&new_module, path)) {
            continue;
        }

        new_module.initapi(ct_api_a0);
        new_module.load(ct_api_a0, 1);

        old_module.unload(ct_api_a0, 1);
        _G.modules[i] = new_module;

        ct_os_a0->object->unload(old_module.handler);

        break;
    }

    for (size_t i = 0; i < MAX_MODULES; ++i) {
        if (!_G.used[i]) {
            continue;
        }

        if ((strcmp(_G.path[i], path)) == 0) {
            continue;
        }

        struct module_functios module = _G.modules[i];
        module.initapi(ct_api_a0);
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


static void load_dirs(const char *path) {
    char key[64];
    size_t len = strlen("load_module.");
    strcpy(key, "load_module.");

    char *buffer = NULL;
    for (int i = 0; true; ++i) {
        ct_buffer_clear(buffer);

        sprintf(key + len, "%d", i);

        const uint64_t key_id = ct_hashlib_a0->id64(key);

        if (!ct_cdb_a0->prop_exist(_G.config, key_id)) {
            break;
        }


        const char *module_file = ct_cdb_a0->read_str(_G.config, key_id, "");
        ct_os_a0->path->join(&buffer,
                             _G.allocator,
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

        _G.modules[i].unload(ct_api_a0, 0);
    }
}

//static void check_modules() {
//    ct_alloc *alloc = ct_memory_a0->system;
//
//    static uint64_t root = CT_ID64_0("modules", 0x6fd8ce9161fffc7ULL);
//
//    auto *wd_it = ct_fs_a0->event_begin(root);
//    const auto *wd_end = ct_fs_a0->event_end(root);
//
//    while (wd_it != wd_end) {
//        if (wd_it->type == CT_WATCHDOG_EVENT_FILE_MODIFIED) {
//            ct_wd_ev_file_write_end *ev = (ct_wd_ev_file_write_end *) wd_it;
//
//            const char *ext = ct_os_path_a0->extension(ev->filename);
//
//            if ((NULL != ext) && (strcmp(ext, "so") == 0)) {
//
//
//                char *path = NULL;
//                ct_os_path_a0->join(&path, alloc, 2, ev->dir, ev->filename);
//
//                char full_path[4096];
//                ct_fs_a0->get_full_path(root, path, full_path,
//                                               CT_ARRAY_LEN(full_path));
//
//                int pat_size = strlen(full_path);
//                int ext_size = strlen(ext);
//                full_path[pat_size - ext_size - 1] = '\0';
//
//                ct_log_a0->info(LOG_WHERE,
//                               "Reload module from path \"%s\"",
//                               full_path);
//
//                reload(full_path);
//
//                ct_buffer_free(path, alloc);
//            }
//        }
//
//        wd_it = ct_fs_a0->event_next(wd_it);
//    }
//}

static struct ct_module_a0 module_api = {
        .reload = reload,
        .reload_all = reload_all,
        .add_static = add_static,
        .load = load,
        .unload_all = unload_all,
        .load_dirs = load_dirs,
};

struct ct_module_a0 *ct_module_a0 = &module_api;

static void _init(struct ct_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ct_memory_a0->system,
            .config = ct_config_a0->obj()
    };

    ct_api_a0 = api;
    api->register_api("ct_module_a0", &module_api);
    
}

CETECH_MODULE_DEF(
        module,
        {

        },
        {
            CT_UNUSED(reload);
            _init(api);

        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);
            ct_log_a0->debug(LOG_WHERE, "Shutdown");

            _G = (struct _G) {};
        }
)
