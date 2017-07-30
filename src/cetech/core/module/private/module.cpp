//==============================================================================
// Includes
//==============================================================================

#include <stdlib.h>
#include <memory.h>
#include <cetech/core/module/module.h>
#include <cetech/core/api/api_system.h>
#include <cetech/core/memory/memory.h>
#include <cetech/core/os/path.h>
#include <cetech/core/os/object.h>
#include <celib/buffer.inl>


#include "cetech/core/log/log.h"

//==============================================================================
// Defines
//==============================================================================

#define MAX_MODULES 256
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
} _G = {};

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_api_a0);
CETECH_DECL_API(ct_object_a0);

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

        memcpy(_G.path[i], path, strlen(path));

        _G.used[i] = 1;
        _G.modules[i] = *module;

        break;
    }
}

const char *get_module_name(const char *path,
                            uint32_t *len) {
    const char *filename = ct_path_a0.filename(path);
    char *name = strchr(filename, '_');
    if (NULL == name) {
        return NULL;
    }

    ++name;
    char *dot = strchr(filename, '.');
    if (NULL == dot) {
        return NULL;
    }

    *len = static_cast<uint32_t>(dot - name);

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


bool get_module_functions_from_path(module_functios *module,
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

        get_module_functions_from_path(&module, path);

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
            get_module_functions_from_path(&new_module, path);

            new_module.initapi(&ct_api_a0);
            new_module.load(&ct_api_a0, 1);

            old_module.unload(&ct_api_a0, 1);
            _G.modules[i] = new_module;

            ct_object_a0.unload(old_module.handler);

            break;
        }

        for (size_t i = 0; i < MAX_MODULES; ++i) {
            if(!_G.used[i]) {
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


    void load_dirs(const char *path) {
        char **files = nullptr;
        uint32_t files_count = 0;

        ct_path_a0.list(path, 1, &files, &files_count,
                        ct_memory_a0.main_allocator());

        for (uint32_t k = 0; k < files_count; ++k) {
            const char *filename = ct_path_a0.filename(files[k]);

            if (!strncmp(filename, MODULE_PREFIX, strlen(MODULE_PREFIX))) {
                load(files[k]);
            }
        }

        ct_path_a0.list_free(files, files_count,
                             ct_memory_a0.main_allocator());
    }

    void unload_all() {
        for (int i = MAX_MODULES - 1; i >= 0; --i) {
            if (!_G.used[i]) {
                continue;
            }

            _G.modules[i].unload(&ct_api_a0, 0);
        }
    }


    static ct_module_a0 module_api = {
            .reload = reload,
            .reload_all = reload_all,
            .add_static = add_static,
            .load = load,
            .unload_all = unload_all,
            .load_dirs = load_dirs
    };
};

CETECH_MODULE_DEF(
        module,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_object_a0);
        },
        {

            ct_api_a0 = *api;

            api->register_api("ct_module_a0", &module::module_api);

            _G = {};
        },
        {
            CEL_UNUSED(api);
            ct_log_a0.debug(LOG_WHERE, "Shutdown");
            _G = {};
        }
)
