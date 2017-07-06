//==============================================================================
// Includes
//==============================================================================


#include <cetech/celib/container_types.inl>
#include <cetech/celib/array.inl>
#include <cetech/celib/map.inl>

#include <cetech/kernel/hash.h>
#include <cetech/modules/application.h>
#include <cetech/kernel/config.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/module.h>
#include <cetech/kernel/os.h>

#include <cetech/kernel/api_system.h>

#include <cetech/modules/resource.h>
#include <cetech/kernel/console_server.h>
#include <cetech/modules/filesystem.h>
#include <cetech/modules/package.h>

#include "include/SDL2/SDL.h"
#include "include/mpack/mpack.h"

#include "resource.h"
#include <cetech/kernel/log.h>
#include <cetech/kernel/errors.h>

CETECH_DECL_API(ct_memory_api_v0);
CETECH_DECL_API(ct_console_srv_api_v0);
CETECH_DECL_API(ct_filesystem_api_v0);
CETECH_DECL_API(ct_config_api_v0);
CETECH_DECL_API(ct_app_api_v0);
CETECH_DECL_API(ct_path_v0);
CETECH_DECL_API(ct_vio_api_v0);
CETECH_DECL_API(ct_log_api_v0);
CETECH_DECL_API(ct_hash_api_v0);
CETECH_DECL_API(ct_thread_api_v0);


void resource_register_type(uint64_t type,
                            ct_resource_callbacks_t callbacks);


using namespace cetech;


namespace resource {
    void reload_all();
}

//==============================================================================
// Gloals
//==============================================================================

#define LOG_WHERE "resource_manager"
#define is_item_null(item) (item.data == null_item.data)

//#define hash_combine(a, b) ((a * 11)^(b))

uint64_t hash_combine(uint64_t lhs,
                      uint64_t rhs) {
    lhs ^= rhs + 0x9e3779b9 + (lhs << 6) + (lhs >> 2);
    return lhs;
}

//==============================================================================
// Gloals
//==============================================================================



namespace {
    typedef struct {
        uint64_t type;
        uint64_t name;
        void *data;
        uint8_t ref_count;
    } resource_item_t;

    static const resource_item_t null_item = {0};
#define _G ResourceManagerGlobals
    struct ResourceManagerGlobals {
        Map<uint32_t> type_map;
        Array<ct_resource_callbacks_t> resource_callbacks;

        Map<uint32_t> resource_map;
        Array<resource_item_t> resource_data;

        int autoload_enabled;

        ct_spinlock_t add_lock;

        struct {
            ct_cvar_t build_dir;
        } config;

    } ResourceManagerGlobals = {0};


    int _cmd_reload_all(mpack_node_t args,
                        mpack_writer_t *writer) {
        resource::reload_all();
        return 0;
    }

//    Map<resource_item_t> *_get_resource_map(uint64_t type) {
//        const uint32_t idx = map::get(_G.type_map, type, UINT32_MAX);
//
//        if (idx == UINT32_MAX) {
//            return NULL;
//        }
//
//        return &_G.resource_data[idx];
//    }

}

//==============================================================================
// Private
//==============================================================================

char *resource_compiler_get_build_dir(ct_allocator *a,
                                      const char *platform) {

    const char *build_dir_str = ct_config_api_v0.get_string(_G.config.build_dir);
    return ct_path_v0.join(a, 2, build_dir_str, platform);
}

namespace package_resource {

    void *loader(struct ct_vio *input,
                 struct ct_allocator *allocator) {

        const int64_t size = input->size(input->inst);
        char *data = CETECH_ALLOCATE(allocator, char, size);
        input->read(input->inst, data, 1, size);

        return data;
    }

    void unloader(void *new_data,
                  struct ct_allocator *allocator) {
        CETECH_FREE(allocator, new_data);
    }

    void online(uint64_t name,
                void *data) {
    }

    void offline(uint64_t name,
                 void *data) {
    }

    void *reloader(uint64_t name,
                   void *old_data,
                   void *new_data,
                   struct ct_allocator *allocator) {
        CETECH_FREE(allocator, old_data);
        return new_data;
    }

    static const ct_resource_callbacks_t package_resource_callback = {
            .loader = loader,
            .unloader =unloader,
            .online =online,
            .offline =offline,
            .reloader = reloader
    };
};

//==============================================================================
// Public interface
//==============================================================================

namespace resource {

    int type_name_string(char *str,
                         size_t max_len,
                         uint64_t type,
                         uint64_t name) {
        return snprintf(str, max_len, "%" SDL_PRIX64 "%" SDL_PRIX64, type,
                        name);
    }


    void set_autoload(int enable) {
        _G.autoload_enabled = enable;
    }

    void resource_register_type(uint64_t type,
                                ct_resource_callbacks_t callbacks) {

        const uint32_t idx = array::size(_G.resource_callbacks);

        array::push_back(_G.resource_callbacks, callbacks);
        map::set(_G.type_map, type, idx);
    }

    void add_loaded(uint64_t type,
                    uint64_t *names,
                    void **resource_data,
                    size_t count) {
        ct_thread_api_v0.spin_lock(&_G.add_lock);


        const uint32_t type_idx = map::get(_G.type_map, type, UINT32_MAX);

        if (type_idx == UINT32_MAX) {
            ct_thread_api_v0.spin_unlock(&_G.add_lock);
            return;
        }


        for (size_t i = 0; i < count; i++) {

            if (resource_data[i] == 0) {
                continue;
            }

            resource_item_t item = {
                    .ref_count=1,
                    .name = names[i],
                    .type = type,
                    .data=resource_data[i]
            };

            uint64_t id = hash_combine(type, names[i]);

            if (!map::has(_G.resource_map, id)) {
                uint32_t idx = array::size(_G.resource_data);
                array::push_back(_G.resource_data, item);
                map::set(_G.resource_map, id, idx);
            } else {
                uint32_t idx = map::get(_G.resource_map, id, UINT32_MAX);
                _G.resource_data[idx] = item;
            }


            _G.resource_callbacks[type_idx].online(names[i], resource_data[i]);
        }

        ct_thread_api_v0.spin_unlock(&_G.add_lock);
    }

    void load(void **loaded_data,
              uint64_t type,
              uint64_t *names,
              size_t count,
              int force);

    void load_now(uint64_t type,
                  uint64_t *names,
                  size_t count) {
        void *loaded_data[count];

        load(loaded_data, type, names, count, 0);
        add_loaded(type, names, loaded_data, count);
    }

    int can_get(uint64_t type,
                uint64_t name) {

        if (!map::has(_G.type_map, type)) {
            return 1;
        }

        ct_thread_api_v0.spin_lock(&_G.add_lock);

        uint64_t id = hash_combine(type, name);
        int h = map::has(_G.resource_map, id);

        ct_thread_api_v0.spin_unlock(&_G.add_lock);

        return h;
    }

    int can_get_all(uint64_t type,
                    uint64_t *names,
                    size_t count) {

//        ct_thread_api_v0.spin_lock(&_G.add_lock);

        for (size_t i = 0; i < count; ++i) {
            if (!can_get(type, names[i])) {
                //ce_thread_api_v0.spin_unlock(&_G.add_lock);
                return 0;
            }
        }

//        ct_thread_api_v0.spin_unlock(&_G.add_lock);

        return 1;
    }

    void load(void **loaded_data,
              uint64_t type,
              uint64_t *names,
              size_t count,
              int force) {

        ct_thread_api_v0.spin_lock(&_G.add_lock);

        const uint32_t idx = map::get(_G.type_map, type, UINT32_MAX);

        if (idx == UINT32_MAX) {
            ct_log_api_v0.error(LOG_WHERE,
                             "Loader for resource is not is not registred");
            memset(loaded_data, sizeof(void *), count);
            ct_thread_api_v0.spin_unlock(&_G.add_lock);
            return;
        }

        const uint64_t root_name = ct_hash_api_v0.id64_from_str("build");
        ct_resource_callbacks_t type_clb = _G.resource_callbacks[idx];


        for (int i = 0; i < count; ++i) {
            uint64_t id = hash_combine(type, names[i]);
            uint32_t idx = map::get(_G.resource_map, id, UINT32_MAX);
            resource_item_t item = {0};
            if (idx != UINT32_MAX) {
                item = _G.resource_data[idx];
            }

            if (!force && (item.ref_count > 0)) {
                ++item.ref_count;
                _G.resource_data[idx] = item;
                loaded_data[i] = 0;
                continue;
            }

            char build_name[33] = {0};
            type_name_string(build_name, CETECH_ARRAY_LEN(build_name),
                             type,
                             names[i]);

#ifdef CETECH_CAN_COMPILE
            char filename[1024] = {0};
            resource_compiler_get_filename(filename, CETECH_ARRAY_LEN(filename),
                                           type,
                                           names[i]);
#else
            char *filename = build_name;
#endif
            ct_log_api_v0.debug("resource", "Loading resource %s from %s/%s",
                             filename,
                             ct_filesystem_api_v0.root_dir(
                                     root_name),
                             build_name);

            struct ct_vio *resource_file = ct_filesystem_api_v0.open(root_name,
                                                                  build_name,
                                                                  FS_OPEN_READ);

            if (resource_file != NULL) {
                loaded_data[i] = type_clb.loader(resource_file,
                                                 ct_memory_api_v0.main_allocator());
                ct_filesystem_api_v0.close(resource_file);
            } else {
                loaded_data[i] = 0;
            }
        }

        ct_thread_api_v0.spin_unlock(&_G.add_lock);
    }

    void unload(uint64_t type,
                uint64_t *names,
                size_t count) {
        ct_thread_api_v0.spin_lock(&_G.add_lock);

        const uint32_t idx = map::get(_G.type_map, type, UINT32_MAX);

        if (idx == UINT32_MAX) {
            ct_thread_api_v0.spin_unlock(&_G.add_lock);
            return;
        }

        ct_resource_callbacks_t type_clb = _G.resource_callbacks[idx];

        for (int i = 0; i < count; ++i) {
            uint64_t id = hash_combine(type, names[i]);
            uint32_t idx = map::get(_G.resource_map, id, UINT32_MAX);

            if (idx == UINT32_MAX) {
                continue;
            }

            resource_item_t &item = _G.resource_data[idx];

            if (item.ref_count == 0) {
                continue;
            }

            if (--item.ref_count == 0) {
                char build_name[33] = {0};
                type_name_string(build_name,
                                 CETECH_ARRAY_LEN(build_name),
                                 type, names[i]);

#ifdef CETECH_CAN_COMPILE
                char filename[1024] = {0};
                resource_compiler_get_filename(filename,
                                               CETECH_ARRAY_LEN(filename),
                                               type,
                                               names[i]);
#else
                char *filename = build_name;
#endif

                ct_log_api_v0.debug("resource", "Unload resource %s ", filename);

                type_clb.offline(names[i], item.data);
                type_clb.unloader(item.data, ct_memory_api_v0.main_allocator());

                map::remove(_G.resource_map, hash_combine(type, names[i]));
            }

            //_G.resource_data[idx] = item;
        }
        ct_thread_api_v0.spin_unlock(&_G.add_lock);
    }

    void *get(uint64_t type,
              uint64_t name) {
        //ce_thread_api_v0.spin_lock(&_G.add_lock);

        uint64_t id = hash_combine(type, name);
        uint32_t idx = map::get(_G.resource_map, id, UINT32_MAX);
        resource_item_t item = {0};
        if (idx != UINT32_MAX) {
            item = _G.resource_data[idx];
        }

        if (is_item_null(item)) {
            char build_name[33] = {0};
            type_name_string(build_name, CETECH_ARRAY_LEN(build_name),
                             type,
                             name);

            if (_G.autoload_enabled) {
#ifdef CETECH_CAN_COMPILE
                char filename[1024] = {0};
                resource_compiler_get_filename(filename,
                                               CETECH_ARRAY_LEN(filename),
                                               type,
                                               name);
#else
                char *filename = build_name;
#endif
                ct_log_api_v0.warning(LOG_WHERE, "Autoloading resource %s",
                                   filename);
                load_now(type, &name, 1);

                uint64_t id = hash_combine(type, name);
                uint32_t idx = map::get(_G.resource_map, id, UINT32_MAX);
                item = {0};
                if (idx != UINT32_MAX) {
                    item = _G.resource_data[idx];
                }

            } else {
                // TODO: fallback resource #205
                CETECH_ASSERT(LOG_WHERE, false);
            }
        }

        //ce_thread_api_v0.spin_unlock(&_G.add_lock);

        return item.data;
    }

    void reload(uint64_t type,
                uint64_t *names,
                size_t count) {
//        reload_all();

        void *loaded_data[count];

        const uint32_t idx = map::get<uint32_t>(_G.type_map, type, 0);

        ct_resource_callbacks_t type_clb = _G.resource_callbacks[idx];

        load(loaded_data, type, names, count, 1);
        for (int i = 0; i < count; ++i) {
#ifdef CETECH_CAN_COMPILE
            char filename[1024] = {0};
            resource_compiler_get_filename(filename, CETECH_ARRAY_LEN(filename),
                                           type,
                                           names[i]);
#else
            char build_name[33] = {0};
            resource::type_name_string(build_name, CETECH_ARRAY_LEN(build_name),
                                       type, names[i]);

            char *filename = build_name;
#endif
            ct_log_api_v0.debug("resource", "Reload resource %s ", filename);

            void *old_data = get(type, names[i]);

            void *new_data = type_clb.reloader(names[i], old_data,
                                               loaded_data[i],
                                               ct_memory_api_v0.main_allocator());

            uint64_t id = hash_combine(type, names[i]);
            uint32_t item_idx = map::get(_G.resource_map, id, UINT32_MAX);
            if (item_idx == UINT32_MAX) {
                continue;
            }

            resource_item_t item = _G.resource_data[item_idx];
            item.data = new_data;
            //--item.ref_count; // Load call increase item.ref_count, because is loaded
            _G.resource_data[item_idx] = item;
        }
    }

    void reload_all() {
        const Map<uint32_t>::Entry *type_it = map::begin(_G.type_map);
        const Map<uint32_t>::Entry *type_end = map::end(_G.type_map);

        Array<uint64_t> name_array(ct_memory_api_v0.main_allocator());

        while (type_it != type_end) {
            uint64_t type_id = type_it->key;

            array::resize(name_array, 0);

            for (int i = 0; i < array::size(_G.resource_data); ++i) {
                resource_item_t item = _G.resource_data[i];

                if (item.type == type_id) {
                    array::push_back(name_array, item.name);
                }
            }

            reload(type_id, &name_array[0], array::size(name_array));

            ++type_it;
        }
    }
}

namespace resource_module {
    static struct ct_resource_api_v0 resource_api = {
            .set_autoload = resource::set_autoload,
            .register_type = resource::resource_register_type,
            .load = resource::load,
            .add_loaded = resource::add_loaded,
            .load_now = resource::load_now,
            .unload = resource::unload,
            .reload = resource::reload,
            .reload_all = resource::reload_all,
            .can_get = resource::can_get,
            .can_get_all = resource::can_get_all,
            .get = resource::get,
            .type_name_string = resource::type_name_string,

            .compiler_get_build_dir = ::resource_compiler_get_build_dir,

#ifdef CETECH_CAN_COMPILE
            .compiler_get_core_dir = resource_compiler_get_core_dir,
            .compiler_register = resource_compiler_register,
            .compiler_compile_all = resource_compiler_compile_all,
            .compiler_get_filename = resource_compiler_get_filename,
            .compiler_get_tmp_dir = resource_compiler_get_tmp_dir,
            .compiler_external_join = resource_compiler_external_join,
            .compiler_create_build_dir = resource_compiler_create_build_dir,
            .compiler_get_source_dir = resource_compiler_get_source_dir,
#endif

    };

    static struct ct_package_api_v0 package_api = {
            .load = package_load,
            .unload = package_unload,
            .is_loaded = package_is_loaded,
            .flush = package_flush,
    };


    void _init_api(struct ct_api_v0 *api) {
        api->register_api("ct_resource_api_v0", &resource_api);
        api->register_api("ct_package_api_v0", &package_api);
    }


    void _init_cvar(struct ct_config_api_v0 config) {
        _G = {0};

        ct_config_api_v0 = config;

        _G.config.build_dir = config.new_str("build", "Resource build dir",
                                             "data/build");
    }


    void _init(struct ct_api_v0 *api) {
        _init_api(api);

        CETECH_GET_API(api, ct_console_srv_api_v0);
        CETECH_GET_API(api, ct_memory_api_v0);
        CETECH_GET_API(api, ct_filesystem_api_v0);
        CETECH_GET_API(api, ct_config_api_v0);
        CETECH_GET_API(api, ct_app_api_v0);
        CETECH_GET_API(api, ct_path_v0);
        CETECH_GET_API(api, ct_vio_api_v0);
        CETECH_GET_API(api, ct_log_api_v0);
        CETECH_GET_API(api, ct_hash_api_v0);
        CETECH_GET_API(api, ct_thread_api_v0);

        _init_cvar(ct_config_api_v0);

        _G.type_map.init(ct_memory_api_v0.main_allocator());
        _G.resource_data.init(ct_memory_api_v0.main_allocator());
        _G.resource_callbacks.init(ct_memory_api_v0.main_allocator());
        _G.resource_map.init(ct_memory_api_v0.main_allocator());

        char *build_dir_full = ct_path_v0.join(
                ct_memory_api_v0.main_allocator(), 2,
                ct_config_api_v0.get_string(_G.config.build_dir),
                ct_app_api_v0.platform());

        ct_filesystem_api_v0.map_root_dir(
                ct_hash_api_v0.id64_from_str("build"),
                build_dir_full);

        resource::resource_register_type(ct_hash_api_v0.id64_from_str("package"),
                                         package_resource::package_resource_callback);

        ct_console_srv_api_v0.register_command("resource.reload_all",
                                           _cmd_reload_all);

        package_init(api);

    }

    void _shutdown() {
        package_shutdown();

        _G.type_map.destroy();
        _G.resource_data.destroy();
        _G.resource_callbacks.destroy();
        _G.resource_map.destroy();
    }


    extern "C" void resourcesystem_load_module(struct ct_api_v0 *api) {
        _init(api);
    }

    extern "C" void resourcesystem_unload_module(struct ct_api_v0 *api) {
        _shutdown();
    }
}
