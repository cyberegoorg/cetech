//==============================================================================
// Includes
//==============================================================================

#include <cetech/core/containers/array.h>
#include <cetech/core/containers/hash.h>

#include <cetech/engine/application/application.h>
#include <cetech/core/api/api_system.h>
#include <cetech/core/memory/memory.h>
#include <cetech/engine/filesystem/filesystem.h>
#include <cetech/engine/config/config.h>
#include <cetech/core/os/path.h>
#include <cetech/core/os/vio.h>
#include <cetech/core/log/log.h>
#include <cetech/core/os/thread.h>
#include <cetech/core/os/errors.h>
#include <cetech/engine/resource/package.h>
#include <cetech/core/module/module.h>
#include <cetech/core/coredb/coredb.h>
#include <cetech/engine/kernel/kernel.h>
#include <cetech/core/containers/buffer.h>

#include "include/SDL2/SDL.h"

#include "resource.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_filesystem_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_cdb_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_thread_a0);


void resource_register_type(uint64_t type,
                            ct_resource_type_t callbacks);

//==============================================================================
// Gloals
//==============================================================================

#define _G ResourceManagerGlobals
#define LOG_WHERE "resource_manager"

//==============================================================================
// Gloals
//==============================================================================

#define MAX_TYPES 64

struct type_item_t {
    struct ct_cdb_obj_t *type_objects;
    struct ct_spinlock lock;
    uint64_t type;
};

struct _G {
    struct ct_hash_t type_map;
    struct type_item_t type_items[MAX_TYPES + 1];
    uint32_t type_items_count;

    ct_resource_type_t *resource_callbacks;

    bool autoload_enabled;

    struct ct_cdb_obj_t *config;
    struct ct_alloc *allocator;
} _G = {};

//==============================================================================
// Private
//==============================================================================

#define CONFIG_BUILD_DIR CT_ID64_0(CONFIG_BUILD_ID)
#define CONFIG_KERNEL_PLATFORM CT_ID64_0(CONFIG_PLATFORM_ID)

char *resource_compiler_get_build_dir(struct ct_alloc *a,
                                      const char *platform) {

    const char *build_dir_str = ct_cdb_a0.read_str(_G.config,
                                                         CONFIG_BUILD_DIR, "");

    char *buffer = NULL;
    ct_path_a0.join(&buffer, a, 2, build_dir_str, platform);

    return buffer;
}


static uint32_t _find_type(uint64_t type) {
    for (uint32_t i = 0; i < _G.type_items_count; ++i) {
        if (_G.type_items[i].type != type) {
            continue;
        }

        return i;
    }

    return 0;
}

//==============================================================================
// Public interface
//==============================================================================

static int type_name_string(char *str,
                            size_t max_len,
                            uint64_t type,
                            uint64_t name) {
    return snprintf(str, max_len, "%"
            SDL_PRIX64
            "%"
            SDL_PRIX64, type,
                    name);
}


static void set_autoload(bool enable) {
    _G.autoload_enabled = enable;
}

void resource_register_type(uint64_t type,
                            ct_resource_type_t callbacks) {

    const uint32_t idx = ct_array_size(_G.resource_callbacks);

    ct_array_push(_G.resource_callbacks, callbacks, _G.allocator);
    ct_hash_add(&_G.type_map, type, idx, _G.allocator);

    struct type_item_t *type_item = &_G.type_items[_G.type_items_count++];
    *type_item = (struct type_item_t) {
            .type = type,
            .type_objects = ct_cdb_a0.create_object()
    };
}

static void load(uint64_t type,
                 uint64_t *names,
                 size_t count,
                 int force);

static void load_now(uint64_t type,
                     uint64_t *names,
                     size_t count) {
    load(type, names, count, 0);
}

static int can_get(uint64_t type,
                   uint64_t name) {

    const uint32_t type_item_idx = _find_type(type);
    struct type_item_t *type_item = &_G.type_items[type_item_idx];

    return ct_cdb_a0.prop_exist(type_item->type_objects, name);
}

static int can_get_all(uint64_t type,
                       uint64_t *names,
                       size_t count) {
    for (size_t i = 0; i < count; ++i) {
        if (!can_get(type, names[i])) {
            return 0;
        }
    }

    return 1;
}

static void load(uint64_t type,
                 uint64_t *names,
                 size_t count,
                 int force) {
    const uint32_t type_idx = ct_hash_lookup(&_G.type_map, type, UINT32_MAX);
    if (type_idx == UINT32_MAX) {
        return;
    }

    const uint32_t type_item_idx = _find_type(type);
    struct type_item_t *type_item = &_G.type_items[type_item_idx];

    const uint64_t root_name = CT_ID64_0("build");

    ct_thread_a0.spin_lock(&type_item->lock);
    struct ct_cdb_writer_t *type_writer = ct_cdb_a0.write_begin(
            type_item->type_objects);
    for (uint32_t i = 0; i < count; ++i) {
        const uint64_t asset_name = names[i];

        if (!force &&
            ct_cdb_a0.prop_exist(type_item->type_objects, asset_name)) {
            continue;
        };

        struct ct_cdb_obj_t *object = ct_cdb_a0.create_object();

        char build_name[33] = {};
        type_name_string(build_name, CETECH_ARRAY_LEN(build_name),
                         type,
                         asset_name);

        char filename[1024] = {};
        resource_compiler_get_filename(filename, CETECH_ARRAY_LEN(filename),
                                       type,
                                       names[i]);

        ct_log_a0.debug("resource", "Loading resource %s from %s",
                        filename, build_name);

        char *build_full = NULL;
        ct_path_a0.join(&build_full,
                        _G.allocator, 2,
                        ct_cdb_a0.read_str(_G.config,
                                                 CONFIG_KERNEL_PLATFORM, ""),
                        build_name);

        struct ct_vio *resource_file = ct_filesystem_a0.open(root_name,
                                                             build_full,
                                                             FS_OPEN_READ);

        ct_buffer_free(build_full, _G.allocator);

        if (resource_file != NULL) {
            _G.resource_callbacks[type_idx].online(names[i],
                                                   resource_file,
                                                   object);

            ct_cdb_a0.set_ref(type_writer, asset_name, object);
        }
    }
    ct_cdb_a0.write_commit(type_writer);
    ct_thread_a0.spin_unlock(&type_item->lock);
}

static void unload(uint64_t type,
                   uint64_t *names,
                   size_t count) {

    const uint32_t idx = ct_hash_lookup(&_G.type_map, type, UINT32_MAX);
    if (idx == UINT32_MAX) {
        return;
    }

    const uint32_t type_item_idx = _find_type(type);
    struct type_item_t *type_item = &_G.type_items[type_item_idx];

    ct_resource_type_t type_clb = _G.resource_callbacks[idx];

    for (uint32_t i = 0; i < count; ++i) {
        if (1) {// TODO: ref counting
            char build_name[33] = {};
            type_name_string(build_name,
                             CETECH_ARRAY_LEN(build_name),
                             type, names[i]);

            char filename[1024] = {};
            resource_compiler_get_filename(filename,
                                           CETECH_ARRAY_LEN(filename),
                                           type,
                                           names[i]);

            ct_log_a0.debug("resource", "Unload resource %s ", filename);

            struct ct_cdb_obj_t *object = ct_cdb_a0.read_ref(
                    type_item->type_objects, names[i], NULL);

            type_clb.offline(names[i], object);
        }
    }

}

static struct ct_cdb_obj_t *get_obj(uint64_t type,
                                       uint64_t name) {

    const uint32_t type_item_idx = _find_type(type);
    struct type_item_t *type_item = &_G.type_items[type_item_idx];
    struct ct_cdb_obj_t *type_object;
    type_object = type_item->type_objects;

    struct ct_cdb_obj_t *object = ct_cdb_a0.read_ref(type_object, name, NULL);

    if(object) {
        return object;
    }

    char build_name[33] = {};
    type_name_string(build_name, CETECH_ARRAY_LEN(build_name),
                     type,
                     name);

    if (_G.autoload_enabled) {
        char filename[1024] = {};
        resource_compiler_get_filename(filename,
                                       CETECH_ARRAY_LEN(filename),
                                       type,
                                       name);

        ct_log_a0.warning(LOG_WHERE, "Autoloading resource %s",
                          filename);
        load_now(type, &name, 1);
    } else {
        // TODO: fallback resource #205
        CETECH_ASSERT(LOG_WHERE, false);
    }

    return ct_cdb_a0.read_ref(type_object, name, NULL);
}

static void reload(uint64_t type,
                   uint64_t *names,
                   size_t count) {

//        const uint32_t idx = map::get<uint32_t>(_G.type_map, type, 0);
//        void* data = NULL;
//        ct_resource_type_t type_clb = _G.resource_callbacks[idx];
//
//        const uint32_t type_item_idx = _find_type(type);
//        type_item_t* type_item = &_G.type_items[type_item_idx];
//
//        load(type, names, count, 1);
//        for (uint32_t i = 0; i < count; ++i) {
//
//            char filename[1024] = {};
//            resource_compiler_get_filename(filename, CETECH_ARRAY_LEN(filename),
//                                           type,
//                                           names[i]);
//#else
//            char build_name[33] = {};
//            type_name_string(build_name, CETECH_ARRAY_LEN(build_name),
//                                       type, names[i]);
//
//            char *filename = build_name;
//#endif
//            ct_log_a0.debug("resource", "Reload resource %s ", filename);

//            void *old_data = get(type, names[i]);
//
//            void *new_data = type_clb.reloader(names[i], old_data,
//                                               loaded_data[i],
//                                               _G.allocator);
//
//            uint64_t id = hash_combine(type, names[i]);
//            uint32_t item_idx = map::get(_G.resource_map, id, UINT32_MAX);
//            if (item_idx == UINT32_MAX) {
//                continue;
//            }
//
//            resource_item_t item = _G.resource_data[item_idx];
//            item.data = new_data;
//            //--item.ref_count; // Load call increase item.ref_count, because is loaded
//            _G.resource_data[item_idx] = item;
//        }
}

static void reload_all() {
//        uint64_t* name_array = NULL;

    for (int j = 0; j < _G.type_items_count; ++j) {
//            type_item_t* type_item = &_G.type_items[j];

//            reload(type_item->type, &name_array[0], ct_array_size(name_array));
    }
//
//        const Map<uint32_t>::Entry *type_it = map::begin(_G.type_map);
//        const Map<uint32_t>::Entry *type_end = map::end(_G.type_map);
//
//        while (type_it != type_end) {
//            uint64_t type_id = type_it->key;
//
//            ct_array_clean(name_array);
//
//            for (uint32_t i = 0; i < ct_array_size(_G.resource_data); ++i) {
//                resource_item_t item = _G.resource_data[i];
//
//                if (item.type == type_id) {
//                    ct_array_push(name_array, item.name, _G.allocator);
//                }
//            }
//
//            reload(type_id, &name_array[0], ct_array_size(name_array));
//
//            ++type_it;
//        }
}

void resource_memory_reload(uint64_t type,
                            uint64_t name,
                            char **blob) {
//        const uint32_t idx = map::get<uint32_t>(_G.type_map, type, UINT32_MAX);
//
//        const uint64_t id = hash_combine(type, name);
//        const uint32_t item_idx = map::get(_G.resource_map, id, UINT32_MAX);
//        if (item_idx == UINT32_MAX) {
//            return;
//        }
//
//        ct_resource_type_t type_clb = _G.resource_callbacks[idx];
//
//        void *old_data = get(type, name);
//
//        void *new_data = type_clb.reloader(name, old_data,
//                                           blob,
//                                           _G.allocator);
//
//        resource_item_t item = _G.resource_data[item_idx];
//        item.data = new_data;
//        _G.resource_data[item_idx] = item;
}

static struct ct_resource_a0 resource_api = {
        .set_autoload = set_autoload,
        .register_type = resource_register_type,
        .load = load,
        .load_now = load_now,
        .unload = unload,
        .reload = reload,
        .reload_all = reload_all,
        .can_get = can_get,
        .can_get_all = can_get_all,
//        .get = get,
        .get_obj = get_obj,
        .type_name_string = type_name_string,

        .compiler_get_build_dir = resource_compiler_get_build_dir,

        .compile_and_reload = compile_and_reload,
        .compiler_get_core_dir = resource_compiler_get_core_dir,
        .compiler_register = resource_compiler_register,
        .compiler_compile_all = resource_compiler_compile_all,
        .compiler_get_filename = resource_compiler_get_filename,
        .compiler_get_tmp_dir = resource_compiler_get_tmp_dir,
        .compiler_external_join = resource_compiler_external_join,
        .compiler_create_build_dir = resource_compiler_create_build_dir,
        .compiler_get_source_dir = resource_compiler_get_source_dir,
        .type_name_from_filename = type_name_from_filename,
        .compiler_check_fs = resource_compiler_check_fs,

};

static struct ct_package_a0 package_api = {
        .load = package_load,
        .unload = package_unload,
        .is_loaded = package_is_loaded,
        .flush = package_flush,
};


static void _init_api(struct ct_api_a0 *api) {
    api->register_api("ct_resource_a0", &resource_api);
    api->register_api("ct_package_a0", &package_api);
}


static void _init_cvar(struct ct_config_a0 config) {
    _G = (struct _G) {};

    ct_config_a0 = config;
    _G.config = ct_config_a0.config_object();

    struct ct_cdb_writer_t *writer = ct_cdb_a0.write_begin(_G.config);
    if (!ct_cdb_a0.prop_exist(_G.config, CONFIG_BUILD_DIR)) {
        ct_cdb_a0.set_string(writer, CONFIG_BUILD_DIR, "build");
    }
    ct_cdb_a0.write_commit(writer);
}


static void _init(struct ct_api_a0 *api) {
    _init_api(api);
    _init_cvar(ct_config_a0);

    _G = (struct _G) {
            .allocator = ct_memory_a0.main_allocator(),
            .config = ct_config_a0.config_object(),
            .type_items_count = 1,
    };

    ct_filesystem_a0.map_root_dir(CT_ID64_0("build"),
                                  ct_cdb_a0.read_str(_G.config,
                                                           CONFIG_BUILD_DIR,
                                                           ""), false);

}

static void _shutdown() {
    package_shutdown();

    ct_hash_free(&_G.type_map, _G.allocator);
    ct_array_free(_G.resource_callbacks, _G.allocator);
}


CETECH_MODULE_DEF(
        resourcesystem,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_filesystem_a0);
            CETECH_GET_API(api, ct_config_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_vio_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_thread_a0);
            CETECH_GET_API(api, ct_cdb_a0);
        },
        {
            CT_UNUSED(reload);
            _init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);

            _shutdown();

        }
)
