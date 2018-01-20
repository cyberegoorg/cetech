//==============================================================================
// Includes
//==============================================================================

#include <cetech/core/array.h>
#include <cetech/core/hash.h>

#include <cetech/engine/application/application.h>
#include <cetech/core/api_system.h>
#include <cetech/core/memory.h>
#include <cetech/engine/filesystem/filesystem.h>
#include <cetech/engine/config/config.h>
#include <cetech/core/path.h>
#include <cetech/core/vio.h>
#include <cetech/core/log.h>
#include <cetech/core/thread.h>
#include <cetech/core/errors.h>
#include <cetech/engine/resource/package.h>
#include <cetech/core/module.h>
#include <cetech/engine/coredb/coredb.h>
#include <cetech/engine/kernel/kernel.h>

#include "include/SDL2/SDL.h"

#include "resource.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_filesystem_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_coredb_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_thread_a0);


void resource_register_type(uint64_t type,
                            ct_resource_callbacks_t callbacks);

//namespace resource {
//    void reload_all();
//}

//==============================================================================
// Gloals
//==============================================================================

#define LOG_WHERE "resource_manager"
#define is_item_null(item) ((item).data == null_item.data)


//static uint64_t hash_combine(uint64_t lhs,
//                      uint64_t rhs) {
//    if(lhs == 0) return rhs;
//    lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
//    return lhs;
//}

//==============================================================================
// Gloals
//==============================================================================

#define MAX_TYPES 64

#define PROP_RESOURECE_DATA "data"

struct type_item_t {
    struct ct_hash_t name_map;
    struct ct_coredb_object_t **resource_objects;
    uint64_t type;
    struct ct_spinlock lock;
};

#define _G ResourceManagerGlobals
struct _G {
    struct ct_hash_t type_map;
    ct_resource_callbacks_t *resource_callbacks;

    struct type_item_t type_items[MAX_TYPES + 1];
    uint32_t type_items_count;

    int autoload_enabled;

    struct ct_coredb_object_t *config;
    struct ct_alloc *allocator;
} _G = {};

//==============================================================================
// Private
//==============================================================================


#define CONFIG_BUILD_DIR CT_ID64_0(CONFIG_BUILD_ID)
#define CONFIG_KERNEL_PLATFORM CT_ID64_0(CONFIG_PLATFORM_ID)

char *resource_compiler_get_build_dir(struct ct_alloc *a,
                                      const char *platform) {

    const char *build_dir_str = ct_coredb_a0.read_string(_G.config,
                                                         CONFIG_BUILD_DIR, "");
    return ct_path_a0.join(a, 2, build_dir_str, platform);
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


static void set_autoload(int enable) {
    _G.autoload_enabled = enable;
}

void resource_register_type(uint64_t type,
                            ct_resource_callbacks_t callbacks) {

    const uint32_t idx = ct_array_size(_G.resource_callbacks);

    ct_array_push(_G.resource_callbacks, callbacks, _G.allocator);
    ct_hash_add(&_G.type_map, type, idx, _G.allocator);

    struct type_item_t *type_item = &_G.type_items[_G.type_items_count++];
    *type_item = (struct type_item_t) {.type = type};
}

static void *get(uint64_t type,
                 uint64_t name);

static void add_loaded(uint64_t type,
                       uint64_t *names,
                       size_t count) {

    const uint32_t type_idx = ct_hash_lookup(&_G.type_map, type, UINT32_MAX);

    if (type_idx == UINT32_MAX) {
        return;
    }

    for (size_t i = 0; i < count; i++) {
        _G.resource_callbacks[type_idx].online(names[i], get(type, names[i]));
    }
}

static void load(uint64_t type,
                 uint64_t *names,
                 size_t count,
                 int force);

static void load_now(uint64_t type,
                     uint64_t *names,
                     size_t count) {
    load(type, names, count, 0);
    add_loaded(type, names, count);
}

static int can_get(uint64_t type,
                   uint64_t name) {

    const uint32_t type_item_idx = _find_type(type);
    struct type_item_t *type_item = &_G.type_items[type_item_idx];

    return ct_hash_contain(&type_item->name_map, name);
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

    const uint32_t idx = ct_hash_lookup(&_G.type_map, type, UINT32_MAX);

    const uint32_t type_item_idx = _find_type(type);
    struct type_item_t *type_item = &_G.type_items[type_item_idx];

    const uint64_t root_name = CT_ID64_0("build");
    ct_resource_callbacks_t type_clb = _G.resource_callbacks[idx];

    ct_thread_a0.spin_lock(&type_item->lock);
    for (uint32_t i = 0; i < count; ++i) {
        uint64_t id = names[i];
        uint32_t res_idx = ct_hash_lookup(&type_item->name_map, id, UINT32_MAX);

        struct ct_coredb_object_t *object = NULL;

        if (res_idx != UINT32_MAX) {
            continue;
        };

        object = ct_coredb_a0.create_object();
        ct_hash_add(&type_item->name_map, id,
                     ct_array_size(type_item->resource_objects), _G.allocator);
        ct_array_push(type_item->resource_objects, object, _G.allocator);

        char build_name[33] = {};
        type_name_string(build_name, CETECH_ARRAY_LEN(build_name),
                         type,
                         names[i]);

        char filename[1024] = {};
        resource_compiler_get_filename(filename, CETECH_ARRAY_LEN(filename),
                                       type,
                                       names[i]);

        ct_log_a0.debug("resource", "Loading resource %s from %s",
                        filename,
                        build_name);

        char *build_full = ct_path_a0.join(
                ct_memory_a0.main_allocator(), 2,
                ct_coredb_a0.read_string(_G.config,
                                         CONFIG_KERNEL_PLATFORM, ""),
                build_name);

        struct ct_vio *resource_file = ct_filesystem_a0.open(root_name,
                                                             build_full,
                                                             FS_OPEN_READ);
        CT_FREE(ct_memory_a0.main_allocator(), build_full);

        if (resource_file != NULL) {
            void *data = type_clb.loader(resource_file,
                                         ct_memory_a0.main_allocator());

            struct ct_coredb_writer_t *writer = ct_coredb_a0.write_begin(
                    object);
            ct_coredb_a0.set_ptr(writer, CT_ID64_0(PROP_RESOURECE_DATA), data);
            ct_coredb_a0.write_commit(writer);
        }
    }
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

    ct_resource_callbacks_t type_clb = _G.resource_callbacks[idx];

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

            uint32_t idx = 0;
            idx = ct_hash_lookup(&type_item->name_map, names[i], UINT32_MAX);
            struct ct_coredb_object_t *object = type_item->resource_objects[idx];

            void *data = ct_coredb_a0.read_ptr(object,
                                               CT_ID64_0(PROP_RESOURECE_DATA),
                                               NULL);

            type_clb.offline(names[i], data);
            type_clb.unloader(data, ct_memory_a0.main_allocator());

            struct ct_coredb_writer_t *writer = ct_coredb_a0.write_begin(
                    object);
            ct_coredb_a0.set_ptr(writer, CT_ID64_0(PROP_RESOURECE_DATA), NULL);
            ct_coredb_a0.write_commit(writer);
        }
    }

}

static void *get(uint64_t type,
                 uint64_t name) {
    //ce_thread_a0.spin_lock(&_G.add_lock);

    const uint32_t type_item_idx = _find_type(type);
    struct type_item_t *type_item = &_G.type_items[type_item_idx];
    uint32_t idx = 0;

    find:
    idx = ct_hash_lookup(&type_item->name_map, name, UINT32_MAX);
    if (idx == UINT32_MAX) {
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

            goto find;
        } else {
            // TODO: fallback resource #205
            CETECH_ASSERT(LOG_WHERE, false);
        }
    }

    struct ct_coredb_object_t *object = type_item->resource_objects[idx];
    void *data = ct_coredb_a0.read_ptr(object, CT_ID64_0(PROP_RESOURECE_DATA),
                                       NULL);

    return data;
}

static void reload(uint64_t type,
                   uint64_t *names,
                   size_t count) {

//        const uint32_t idx = map::get<uint32_t>(_G.type_map, type, 0);
//        void* data = NULL;
//        ct_resource_callbacks_t type_clb = _G.resource_callbacks[idx];
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
//                                               ct_memory_a0.main_allocator());
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
//        ct_resource_callbacks_t type_clb = _G.resource_callbacks[idx];
//
//        void *old_data = get(type, name);
//
//        void *new_data = type_clb.reloader(name, old_data,
//                                           blob,
//                                           ct_memory_a0.main_allocator());
//
//        resource_item_t item = _G.resource_data[item_idx];
//        item.data = new_data;
//        _G.resource_data[item_idx] = item;
}

static struct ct_resource_a0 resource_api = {
        .set_autoload = set_autoload,
        .register_type = resource_register_type,
        .load = load,
        .add_loaded = add_loaded,
        .load_now = load_now,
        .unload = unload,
        .reload = reload,
        .reload_all = reload_all,
        .can_get = can_get,
        .can_get_all = can_get_all,
        .get = get,
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

    struct ct_coredb_writer_t *writer = ct_coredb_a0.write_begin(_G.config);
    if (!ct_coredb_a0.prop_exist(_G.config, CONFIG_BUILD_DIR)) {
        ct_coredb_a0.set_string(writer, CONFIG_BUILD_DIR, "build");
    }
    ct_coredb_a0.write_commit(writer);
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
                                  ct_coredb_a0.read_string(_G.config,
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
            CETECH_GET_API(api, ct_coredb_a0);
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
