//==============================================================================
// Includes
//==============================================================================

#include <stdlib.h>
#include <memory.h>

#include <celib/buffer.inl>
#include <celib/api_system.h>
#include <celib/os.h>
#include <celib/module.h>
#include <celib/memory.h>
#include <celib/config.h>
#include <celib/hashlib.h>
#include <celib/fs.h>
#include <celib/cdb.h>

#include "celib/log.h"

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
    ce_load_module_t load;
    ce_unload_module_t unload;
    ce_initapi_module_t initapi;
};

static struct _G {
    struct module_functios modules[MAX_MODULES];
    char path[MAX_MODULES][MAX_PATH_LEN];
    char used[MAX_MODULES];
    uint64_t config;
    struct ce_alloc *allocator;
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
    const char *filename = ce_os_a0->path->filename(path);
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
    ce_buffer_clear(*buffer);
    ce_buffer_printf(buffer, _G.allocator, "%s%s", name, fce_name);
}


static bool load_from_path(struct module_functios *module,
                           const char *path) {
    uint32_t name_len;
    const char *name = get_module_name(path, &name_len);

    char *buffer = NULL;

    get_module_fce_name(&buffer, name, name_len, "_load_module");

    void *obj = ce_os_a0->object->load(path);
    if (obj == NULL) {
        return false;
    }

    ce_load_module_t load_fce = (ce_load_module_t) ce_os_a0->object->load_function(
            obj,
            (buffer));
    if (load_fce == NULL) {
        return false;
    }

    get_module_fce_name(&buffer, name, name_len, "_unload_module");

    ce_unload_module_t unload_fce = (ce_unload_module_t) ce_os_a0->object->load_function(
            obj,
            buffer);
    if (unload_fce == NULL) {
        return false;
    }

    get_module_fce_name(&buffer, name, name_len, "_initapi_module");

    ce_initapi_module_t initapi_fce = (ce_initapi_module_t) ce_os_a0->object->load_function(
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


static void add_static(ce_load_module_t load,
                       ce_unload_module_t unload,
                       ce_initapi_module_t initapi) {

    struct module_functios module = {
            .load=load,
            .unload=unload,
            .initapi=initapi,
            .handler=NULL
    };

    add_module("__STATIC__", &module);
//    initapi(ce_api_a0);
    load(ce_api_a0, 0);
}


static void load(const char *path) {
    ce_log_a0->info(LOG_WHERE, "Loading module %s", path);

    struct module_functios module;

    if (!load_from_path(&module, path)) {
        return;
    }

    module.initapi(ce_api_a0);
    module.load(ce_api_a0, 0);

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

        new_module.initapi(ce_api_a0);
        new_module.load(ce_api_a0, 1);

        old_module.unload(ce_api_a0, 1);
        _G.modules[i] = new_module;

        ce_os_a0->object->unload(old_module.handler);

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
        module.initapi(ce_api_a0);
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
        ce_buffer_clear(buffer);

        sprintf(key + len, "%d", i);

        const uint64_t key_id = ce_id_a0->id64(key);

        const ce_cdb_obj_o * reader = ce_cdb_a0->read(ce_cdb_a0->db(), _G.config);

        if (!ce_cdb_a0->prop_exist(reader, key_id)) {
            break;
        }


        const char *module_file = ce_cdb_a0->read_str(reader, key_id, "");
        ce_os_a0->path->join(&buffer,
                             _G.allocator,
                             2, path, module_file);
        load(buffer);

        ce_buffer_free(buffer, _G.allocator);
    }
}

static void unload_all() {
    for (int i = MAX_MODULES - 1; i >= 0; --i) {
        if (!_G.used[i]) {
            continue;
        }

        _G.modules[i].unload(ce_api_a0, 0);
    }
}

//static void check_modules() {
//    ce_alloc *alloc = ce_memory_a0->system;
//
//    static uint64_t root = CE_ID64_0("modules", 0x6fd8ce9161fffc7ULL);
//
//    auto *wd_it = ce_fs_a0->event_begin(root);
//    const auto *wd_end = ce_fs_a0->event_end(root);
//
//    while (wd_it != wd_end) {
//        if (wd_it->type == CE_WATCHDOG_EVENT_FILE_MODIFIED) {
//            ce_wd_ev_file_write_end *ev = (ce_wd_ev_file_write_end *) wd_it;
//
//            const char *ext = ce_os_path_a0->extension(ev->filename);
//
//            if ((NULL != ext) && (strcmp(ext, "so") == 0)) {
//
//
//                char *path = NULL;
//                ce_os_path_a0->join(&path, alloc, 2, ev->dir, ev->filename);
//
//                char full_path[4096];
//                ce_fs_a0->get_full_path(root, path, full_path,
//                                               CE_ARRAY_LEN(full_path));
//
//                int pat_size = strlen(full_path);
//                int ext_size = strlen(ext);
//                full_path[pat_size - ext_size - 1] = '\0';
//
//                ce_log_a0->info(LOG_WHERE,
//                               "Reload module from path \"%s\"",
//                               full_path);
//
//                reload(full_path);
//
//                ce_buffer_free(path, alloc);
//            }
//        }
//
//        wd_it = ce_fs_a0->event_next(wd_it);
//    }
//}

static struct ce_module_a0 module_api = {
        .reload = reload,
        .reload_all = reload_all,
        .add_static = add_static,
        .load = load,
        .unload_all = unload_all,
        .load_dirs = load_dirs,
};

struct ce_module_a0 *ce_module_a0 = &module_api;

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .config = ce_config_a0->obj()
    };

    ce_api_a0 = api;
    api->register_api("ce_module_a0", &module_api);
    
}

CE_MODULE_DEF(
        module,
        {

        },
        {
            CE_UNUSED(reload);
            _init(api);

        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);
            ce_log_a0->debug(LOG_WHERE, "Shutdown");

            _G = (struct _G) {};
        }
)
