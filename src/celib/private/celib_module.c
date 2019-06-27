//==============================================================================
// Includes
//==============================================================================

#include <stdlib.h>
#include <memory.h>

#include <celib/memory/allocator.h>
#include <celib/macros.h>
#include <celib/containers/buffer.h>
#include <celib/api.h>

#include <celib/module.h>
#include <celib/memory/memory.h>
#include <celib/config.h>
#include <celib/id.h>
#include <celib/fs.h>
#include <celib/cdb.h>
#include <celib/os/path.h>
#include <celib/os/object.h>
#include <celib/os/vio.h>

#include "celib/log.h"

//==============================================================================
// Defines
//==============================================================================

#define MAX_MODULES 128
#define MAX_PATH_LEN 256
#define MAX_MODULE_NAME 128

#define _G ModuleGlobals
#define LOG_WHERE "module"

//==============================================================================
// Globals
//==============================================================================
typedef struct module_t {
    char name[MAX_MODULE_NAME];
    void *handler;
    uint64_t mtime;
    ce_load_module_t0 *load;
    ce_unload_module_t0 *unload;
} module_t;

static struct _G {
    module_t modules[MAX_MODULES];
    char path[MAX_MODULES][MAX_PATH_LEN];
    char used[MAX_MODULES];
    ce_alloc_t0 *allocator;
    bool reload_all_rq;
} _G = {};


//==============================================================================
// Private
//==============================================================================

static void _add_module(const char *path,
                        struct module_t *module) {

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

static void _get_module_name(const char *path,
                             char *module_name,
                             uint32_t *len) {
    const char *filename = ce_os_path_a0->filename(path);

    char tmp[128];
    ce_os_path_a0->basename(filename, tmp);

    const char *name = strchr(tmp, '_');
    if (NULL == name) {
        *len = 0;
    }

    ++name;
    sprintf(module_name, "%s", name);
    *len = strlen(name);
}

static void _get_module_fce_name(char **buffer,
                                 const char *name,
                                 uint32_t name_len,
                                 const char *fce_name) {
    ce_buffer_clear(*buffer);
    ce_buffer_printf(buffer, _G.allocator, "%s%s", name, fce_name);
}


static bool _load_from_path(module_t *module,
                            const char *path) {
    uint32_t name_len;
    char name[128];
    _get_module_name(path, name, &name_len);

    char *buffer = NULL;

    _get_module_fce_name(&buffer, name, name_len, "_load_module");

    void *obj = ce_os_object_a0->load(path);
    if (obj == NULL) {
        return false;
    }

    ce_load_module_t0 *load_fce = ce_os_object_a0->load_function(obj, (buffer));
    if (load_fce == NULL) {
        return false;
    }

    _get_module_fce_name(&buffer, name, name_len, "_unload_module");

    ce_unload_module_t0 *unload_fce = ce_os_object_a0->load_function(obj, buffer);
    if (unload_fce == NULL) {
        return false;
    }

    char full_path[128] = {};
    snprintf(full_path, CE_ARRAY_LEN(full_path), "%s.dylib", path);
    uint64_t mtime = ce_os_path_a0->file_mtime(full_path);

    *module = (module_t) {
            .handler = obj,
            .load = load_fce,
            .unload = unload_fce,
            .mtime = mtime,
    };

    strncpy(module->name, name, MAX_MODULE_NAME);

    return true;
}

//==============================================================================
// Interface
//==============================================================================


static void add_static(const char *name,
                       ce_load_module_t0 load,
                       ce_unload_module_t0 unload) {
    module_t module = {
            .load = load,
            .unload = unload,
            .handler = NULL
    };

    strncpy(module.name, name, MAX_MODULE_NAME);

    ce_log_a0->info(LOG_WHERE, "Load module \"%s\"", name);

    _add_module("__STATIC__", &module);
    load(ce_api_a0, 0);
}


static void load(const char *path) {
    ce_log_a0->info(LOG_WHERE, "Loading module from %s", path);

    module_t module;

    if (!_load_from_path(&module, path)) {
        return;
    }

    module.load(ce_api_a0, 0);

    _add_module(path, &module);
}

module_t *_get_module(const char *path) {
    for (size_t i = 0; i < MAX_MODULES; ++i) {
        module_t *module = &_G.modules[i];

        if (strcmp(_G.path[i], path) != 0) {
            continue;
        }

        return module;
    }

    return NULL;
}

static void reload(const char *path) {
    module_t *old_module = _get_module(path);

    if (!old_module) {
        return;
    }

    char full_path[128] = {};
    snprintf(full_path, CE_ARRAY_LEN(full_path), "%s.dylib", path);
    uint64_t mtime = ce_os_path_a0->file_mtime(full_path);

    if (mtime == old_module->mtime) {
        return;
    }


    char new_path[128];
    snprintf(new_path, CE_ARRAY_LEN(new_path), "%s.dylib", path);
    ce_vio_t0 *in = ce_os_vio_a0->from_file(new_path, VIO_OPEN_READ);
    uint64_t size = in->vt->size(in->inst);
    void *meme = CE_ALLOC(ce_memory_a0->system, char, size);
    in->vt->read(in->inst, meme, 1, size);
    ce_os_vio_a0->close(in);

    static int cnt = 0;
    snprintf(new_path, CE_ARRAY_LEN(new_path), "%s.%d.dylib", path, cnt);
    ce_vio_t0 *out = ce_os_vio_a0->from_file(new_path, VIO_OPEN_WRITE);
    out->vt->write(out->inst, meme, 1, size);
    ce_os_vio_a0->close(out);
    CE_FREE(ce_memory_a0->system, meme);

    snprintf(new_path, CE_ARRAY_LEN(new_path), "%s.%d", path, cnt);
    cnt++;

    struct module_t new_module;
    if (!_load_from_path(&new_module, new_path)) {
        return;
    }

    new_module.load(ce_api_a0, 1);

    old_module->unload(ce_api_a0, 1);
    ce_os_object_a0->unload(old_module->handler);

    *old_module = new_module;
}

static void reload_all() {
    _G.reload_all_rq = true;
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

        if (!ce_config_a0->exist(key_id)) {
            break;
        }


        const char *module_file = ce_config_a0->read_str(key_id, "");
        ce_os_path_a0->join(&buffer,
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

        ce_log_a0->info(LOG_WHERE, "Unload module \"%s\"", _G.modules[i].name);

        _G.modules[i].unload(ce_api_a0, 0);
    }
}

void do_reload() {
    if (_G.reload_all_rq) {
        for (size_t i = 0; i < MAX_MODULES; ++i) {
            if (_G.modules[i].handler == NULL) {
                continue;
            }

            reload(_G.path[i]);
        }

        _G.reload_all_rq = false;
    }
}

static struct ce_module_a0 module_api = {
        .reload = reload,
        .reload_all = reload_all,
        .add_static = add_static,
        .load = load,
        .unload_all = unload_all,
        .load_dirs = load_dirs,
        .do_reload = do_reload,
};

struct ce_module_a0 *ce_module_a0 = &module_api;

void CE_MODULE_LOAD(module)(struct ce_api_a0 *api,
                            int reload) {
    CE_UNUSED(reload);
    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    ce_api_a0 = api;
    api->add_api(CE_MODULE0_API, &module_api, sizeof(module_api));


}

void CE_MODULE_UNLOAD(module)(struct ce_api_a0 *api,
                              int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
    ce_log_a0->debug(LOG_WHERE, "Shutdown");

    _G = (struct _G) {};
}
