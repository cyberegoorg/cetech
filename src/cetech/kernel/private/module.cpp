//==============================================================================
// Includes
//==============================================================================

#include <stdlib.h>
#include <memory.h>

#include <celib/buffer.inl>

#include <cetech/kernel/api_system.h>
#include <cetech/kernel/path.h>
#include <cetech/kernel/object.h>
#include <cetech/kernel/module.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/config.h>
#include <cetech/kernel/watchdog.h>
#include <cetech/kernel/hashlib.h>
#include <celib/map.inl>
#include <cetech/kernel/filesystem.h>

#include "cetech/kernel/log.h"

//==============================================================================
// Defines
//==============================================================================

#define MAX_MODULES 128
#define MAX_PATH_LEN 256

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

static struct ModuleSystemGlobals {
    module_functios modules[MAX_MODULES];
    char path[MAX_MODULES][MAX_PATH_LEN];
    char used[MAX_MODULES];
    ct_cvar module_dir;
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


using namespace celib;

//==============================================================================
// Private
//==============================================================================

void add_module(const char *path,
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

const char *get_module_name(const char *path,
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

void get_module_fce_name(Buffer &buffer,
                         const char *name,
                         uint32_t name_len,
                         const char *fce_name) {
    celib::array::clear(buffer);
    celib::buffer::push(buffer, name, name_len);
    buffer::printf(buffer, fce_name);
}


bool load_from_path(module_functios *module,
                    const char *path) {
    uint32_t name_len;
    const char *name = get_module_name(path, &name_len);
    celib::Buffer buffer(ct_memory_a0.main_allocator());

    get_module_fce_name(buffer, name, name_len, "_load_module");

    void *obj = ct_object_a0.load(path);
    if (obj == NULL) {
        return false;
    }

    auto load_fce = (ct_load_module_t) ct_object_a0.load_function(
            obj,
            celib::buffer::c_str(buffer));
    if (load_fce == NULL) {
        return false;
    }

    get_module_fce_name(buffer, name, name_len, "_unload_module");

    auto unload_fce = (ct_unload_module_t) ct_object_a0.load_function(
            obj,
            celib::buffer::c_str(buffer));
    if (unload_fce == NULL) {
        return false;
    }

    get_module_fce_name(buffer, name, name_len, "_initapi_module");

    ct_initapi_module_t initapi_fce = (ct_initapi_module_t) ct_object_a0.load_function(
            obj,
            celib::buffer::c_str(buffer));
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

namespace module {

    void add_static(ct_load_module_t load,
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


    void load(const char *path) {
        ct_log_a0.info(LOG_WHERE, "Loading module %s", path);

        module_functios module;

        if (!load_from_path(&module, path)) {
            return;
        }

        module.initapi(&ct_api_a0);
        module.load(&ct_api_a0, 0);

        add_module(path, &module);
    }

    void reload(const char *path) {
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

    void reload_all() {
        for (size_t i = 0; i < MAX_MODULES; ++i) {
            if (_G.modules[i].handler == NULL) {
                continue;
            }

            reload(_G.path[i]);
        }
    }


    void load_dirs() {
        char key[64];
        size_t len = strlen("load_module.");
        strcpy(key, "load_module.");
        const char *path = ct_config_a0.get_string(_G.module_dir);

        for (int i = 0; true; ++i) {
            sprintf(key + len, "%d", i);
            ct_cvar n = ct_config_a0.find(key);

            if (n.idx == 0) {
                break;
            }

            const char *module_file = ct_config_a0.get_string(n);
            char *module_path = ct_path_a0.join(ct_memory_a0.main_allocator(),
                                                2, path, module_file);
            load(module_path);

            CEL_FREE(ct_memory_a0.main_allocator(), module_path);
        }
    }

    void unload_all() {
        for (int i = MAX_MODULES - 1; i >= 0; --i) {
            if (!_G.used[i]) {
                continue;
            }

            _G.modules[i].unload(&ct_api_a0, 0);
        }
    }

    void check_modules() {
        cel_alloc *alloc = ct_memory_a0.main_allocator();

        static uint64_t root = CT_ID64_0("modules");

        auto *wd_it = ct_filesystem_a0.event_begin(root);
        const auto *wd_end = ct_filesystem_a0.event_end(root);

        while (wd_it != wd_end) {
            if (wd_it->type == CT_WATCHDOG_EVENT_FILE_MODIFIED) {
                ct_wd_ev_file_write_end *ev = (ct_wd_ev_file_write_end *) wd_it;

                const char *ext = ct_path_a0.extension(ev->filename);

                if ((NULL != ext) && (strcmp(ext, "so") == 0)) {


                    char *path = ct_path_a0.join(alloc, 2, ev->dir,
                                                 ev->filename);


                    char full_path[4096];
                    ct_filesystem_a0.get_full_path(root, path, full_path,
                                                   CETECH_ARRAY_LEN(full_path));

                    int pat_size = strlen(full_path);
                    int ext_size = strlen(ext);
                    full_path[pat_size - ext_size - 1] = '\0';

                    ct_log_a0.info(LOG_WHERE,
                                   "Reload module from path \"%s\"",
                                   full_path);

                    module::reload(full_path);


                    CEL_FREE(alloc, path);
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
};


static const char* _get_load_dir() {
#if defined(CETECH_LINUX)
    return "./bin/linux64/";
#elif defined(CETECH_DARWIN)
    return "./bin/darwin64/";
#endif
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
        },
        {
            CEL_UNUSED(reload);
            _G = {};

            ct_api_a0 = *api;

            api->register_api("ct_module_a0", &module::module_api);



            _G.module_dir = ct_config_a0.new_str(
                    "module_dir",
                    "Path where is modules",
                    _get_load_dir()
            );


            static uint64_t root = CT_ID64_0("modules");
            ct_filesystem_a0.map_root_dir(root, ct_config_a0.get_string(
                    _G.module_dir), true);

        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);
            ct_log_a0.debug(LOG_WHERE, "Shutdown");

            _G = {};
        }
)
