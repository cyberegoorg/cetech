//==============================================================================
// Includes
//==============================================================================

#include <corelib/array.inl>
#include <corelib/hash.inl>

#include <corelib/ebus.h>
#include <corelib/api_system.h>
#include <corelib/memory.h>
#include <corelib/fs.h>
#include <corelib/config.h>
#include <corelib/os.h>
#include <corelib/log.h>
#include <cetech/resource/package.h>
#include <corelib/module.h>
#include <corelib/cdb.h>
#include <cetech/kernel/kernel.h>
#include <corelib/buffer.inl>

#include "include/SDL2/SDL.h"

#include "resource.h"

//==============================================================================
// Gloals
//==============================================================================

#define _G ResourceManagerGlobals
#define LOG_WHERE "resource_manager"

//==============================================================================
// Gloals
//==============================================================================

#define MAX_TYPES 64

struct _G {
    struct ct_spinlock lock;
    struct ct_hash_t resource_map;

    struct ct_hash_t type_map;
    ct_resource_type_t *resource_callbacks;

    bool autoload_enabled;

    struct ct_cdb_t db;

    uint64_t config;
    struct ct_alloc *allocator;
} _G = {};

//==============================================================================
// Private
//==============================================================================

#define CONFIG_BUILD_DIR CT_ID64_0(CONFIG_BUILD_ID)
#define CONFIG_KERNEL_PLATFORM CT_ID64_0(CONFIG_PLATFORM_ID)

char *resource_compiler_get_build_dir(struct ct_alloc *a,
                                      const char *platform) {

    const char *build_dir_str = ct_cdb_a0->read_str(_G.config,
                                                    CONFIG_BUILD_DIR, "");

    char *buffer = NULL;
    ct_os_a0->path_a0->join(&buffer, a, 2, build_dir_str, platform);

    return buffer;
}

//==============================================================================
// Public interface
//==============================================================================

static int type_name_string(char *str,
                            size_t max_len,
                            struct ct_resource_id resourceid) {
    return snprintf(str, max_len, "%"
            SDL_PRIX64, resourceid.i64);
}


static void set_autoload(bool enable) {
    _G.autoload_enabled = enable;
}

void resource_register_type(const char *type,
                            ct_resource_type_t callbacks) {

    const uint32_t idx = ct_array_size(_G.resource_callbacks);

    uint32_t typeid = CT_ID32_0(type);
    ct_array_push(_G.resource_callbacks, callbacks, _G.allocator);
    ct_hash_add(&_G.type_map, typeid, idx, _G.allocator);
}

static void load(uint32_t type,
                 uint32_t *names,
                 size_t count,
                 int force);

static void load_now(uint32_t type,
                     uint32_t *names,
                     size_t count) {
    load(type, names, count, 0);
}

static int can_get(uint32_t type,
                   uint32_t name) {

    struct ct_resource_id rid = {
            .name = name,
            .type = type,
    };

    return ct_hash_contain(&_G.resource_map, rid.i64);
}

static int can_get_all(uint32_t type,
                       uint32_t *names,
                       size_t count) {
    for (size_t i = 0; i < count; ++i) {
        if (!can_get(type, names[i])) {
            return 0;
        }
    }

    return 1;
}

static void load(uint32_t type,
                 uint32_t *names,
                 size_t count,
                 int force) {
    const uint32_t type_idx = ct_hash_lookup(&_G.type_map, type, UINT32_MAX);
    if (type_idx == UINT32_MAX) {
        return;
    }

    const uint64_t root_name = CT_ID64_0("build");

    for (uint32_t i = 0; i < count; ++i) {
        const uint32_t asset_name = names[i];

        if (!force && can_get(type, asset_name)) {
            continue;
        };

        uint64_t object = ct_cdb_a0->create_object(_G.db, type);

        char build_name[33] = {};

        struct ct_resource_id rid = (struct ct_resource_id) {
                .name = asset_name,
                .type = type,
        };

        type_name_string(build_name, CT_ARRAY_LEN(build_name), rid);

        char filename[1024] = {};
        resource_compiler_get_filename(filename, CT_ARRAY_LEN(filename), rid);

        ct_log_a0->debug("resource", "Loading resource %s from %s",
                         filename, build_name);

        char *build_full = NULL;
        ct_os_a0->path_a0->join(&build_full,
                                _G.allocator, 2,
                                ct_cdb_a0->read_str(_G.config,
                                                    CONFIG_KERNEL_PLATFORM, ""),
                                build_name);

        struct ct_vio *resource_file = ct_fs_a0->open(root_name,
                                                      build_full,
                                                      FS_OPEN_READ);

        ct_buffer_free(build_full, _G.allocator);

        if (resource_file != NULL) {
            _G.resource_callbacks[type_idx].online(names[i],
                                                   resource_file,
                                                   object);


            ct_os_a0->thread_a0->spin_lock(&_G.lock);
            ct_hash_add(&_G.resource_map, rid.i64,
                        (uint64_t) object, _G.allocator);
            ct_os_a0->thread_a0->spin_unlock(&_G.lock);
            ct_fs_a0->close(resource_file);
        }
    }
}

static void unload(uint32_t type,
                   uint32_t *names,
                   size_t count) {

    const uint32_t idx = ct_hash_lookup(&_G.type_map, type, UINT32_MAX);
    if (idx == UINT32_MAX) {
        return;
    }
    ct_resource_type_t type_clb = _G.resource_callbacks[idx];

    for (uint32_t i = 0; i < count; ++i) {
        if (1) {// TODO: ref counting
            char build_name[33] = {};
            struct ct_resource_id rid = (struct ct_resource_id) {
                    .type = type,
                    .name = names[i],
            };

            type_name_string(build_name, CT_ARRAY_LEN(build_name), rid);

            char filename[1024] = {};
            resource_compiler_get_filename(filename,
                                           CT_ARRAY_LEN(filename),
                                           rid);

            ct_log_a0->debug("resource", "Unload resource %s ", filename);

            uint64_t object;
            object = (uint64_t) ct_hash_lookup(&_G.resource_map,
                                               rid.i64, 0);

            type_clb.offline(names[i], object);
        }
    }

}

static uint64_t get_obj(struct ct_resource_id resource_id) {
    uint64_t object;
    object = (uint64_t) ct_hash_lookup(&_G.resource_map,
                                       resource_id.i64, 0);

    if (!object) {
        char build_name[33] = {};
        type_name_string(build_name, CT_ARRAY_LEN(build_name), resource_id);

        if (_G.autoload_enabled) {
            char filename[1024] = {};
            resource_compiler_get_filename(filename,
                                           CT_ARRAY_LEN(filename),
                                           resource_id);

            ct_log_a0->warning(LOG_WHERE, "Autoloading resource %s", filename);
            load_now(resource_id.type, &resource_id.name, 1);
        } else {
            // TODO: fallback resource #205
            CETECH_ASSERT(LOG_WHERE, false);
        }

        object = (uint64_t) ct_hash_lookup(&_G.resource_map,
                                           resource_id.i64, 0);
    }

    return object;
}

static void reload(uint32_t type,
                   uint32_t *names,
                   size_t count) {

//        const uint32_t idx = map::get<uint32_t>(_G.type_map, ptype, 0);
//        void* data = NULL;
//        ct_resource_type_t type_clb = _G.resource_callbacks[idx];
//
//        const uint32_t type_item_idx = _find_type(ptype);
//        type_item_t* type_item = &_G.type_items[type_item_idx];
//
//        load(ptype, names, count, 1);
//        for (uint32_t i = 0; i < count; ++i) {
//
//            char filename[1024] = {};
//            resource_compiler_get_filename(filename, CETECH_ARRAY_LEN(filename),
//                                           ptype,
//                                           names[i]);
//#else
//            char build_name[33] = {};
//            type_name_string(build_name, CETECH_ARRAY_LEN(build_name),
//                                       ptype, names[i]);
//
//            char *filename = build_name;
//#endif
//            ct_log_a0->debug("resource", "Reload resource %s ", filename);

//            void *old_data = get(ptype, names[i]);
//
//            void *new_data = type_clb.reloader(names[i], old_data,
//                                               loaded_data[i],
//                                               _G.allocator);
//
//            uint64_t id = hash_combine(ptype, names[i]);
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
}

void resource_memory_reload(struct ct_resource_id resource_id,
                            char **blob) {
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
        .get = get_obj,
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

};

static struct ct_package_a0 package_api = {
        .load = package_load,
        .unload = package_unload,
        .is_loaded = package_is_loaded,
        .flush = package_flush,
};

struct ct_resource_a0 *ct_resource_a0 = &resource_api;
struct ct_package_a0 *ct_package_a0 = &package_api;

static void _init_api(struct ct_api_a0 *api) {
    api->register_api("ct_resource_a0", &resource_api);
    api->register_api("ct_package_a0", &package_api);
}


static void _init_cvar(struct ct_config_a0 *config) {
    _G = (struct _G) {};

    ct_config_a0 = config;
    _G.config = ct_config_a0->config_object();

    ct_cdb_obj_o *writer = ct_cdb_a0->write_begin(_G.config);
    if (!ct_cdb_a0->prop_exist(_G.config, CONFIG_BUILD_DIR)) {
        ct_cdb_a0->set_str(writer, CONFIG_BUILD_DIR, "build");
    }
    ct_cdb_a0->write_commit(writer);
}


static void _init(struct ct_api_a0 *api) {
    _init_api(api);
    _init_cvar(ct_config_a0);

    _G = (struct _G) {
            .allocator = ct_memory_a0->main_allocator(),
            .config = ct_config_a0->config_object(),
            .db = ct_cdb_a0->global_db()
    };

    ct_fs_a0->map_root_dir(CT_ID64_0("build"),
                           ct_cdb_a0->read_str(_G.config, CONFIG_BUILD_DIR, ""),
                           false);

}

static void _shutdown() {
    package_shutdown();

    ct_cdb_a0->destroy_db(_G.db);

    ct_hash_free(&_G.type_map, _G.allocator);
    ct_array_free(_G.resource_callbacks, _G.allocator);
}


CETECH_MODULE_DEF(
        resourcesystem,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_fs_a0);
            CETECH_GET_API(api, ct_config_a0);
            CETECH_GET_API(api, ct_os_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
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
