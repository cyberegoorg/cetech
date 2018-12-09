//==============================================================================
// Includes
//==============================================================================
#include <inttypes.h>

#include <celib/array.inl>
#include <celib/hash.inl>
#include <celib/ebus.h>
#include <celib/api_system.h>
#include <celib/memory.h>
#include <celib/fs.h>
#include <celib/config.h>
#include <celib/os.h>
#include <celib/log.h>
#include <celib/module.h>
#include <celib/cdb.h>
#include <celib/buffer.inl>

#include <cetech/resource/package.h>
#include <cetech/kernel/kernel.h>
#include <cetech/resource/builddb.h>
#include <cetech/resource/resource_compiler.h>
#include <celib/hashlib.h>
#include <cetech/asset/sourcedb.h>

#include "../resource.h"

//==============================================================================
// Gloals
//==============================================================================

#define _G ResourceManagerGlobals
#define LOG_WHERE "resource"

//==============================================================================
// Gloals
//==============================================================================

struct _G {
    struct ce_hash_t type_map;

    struct ce_cdb_t db;
    uint64_t resource_db;

    uint64_t config;
    struct ce_alloc *allocator;
} _G = {};

//==============================================================================
// Private
//==============================================================================


//==============================================================================
// Public interface
//==============================================================================

static int type_name_string(char *str,
                            size_t max_len,
                            struct ct_resource_id resourceid) {
    return snprintf(str, max_len, "%"
            PRIx64
            "%"
            PRIx64, resourceid.type, resourceid.name);
}

static void _resource_api_add(uint64_t name,
                              void *api) {
    struct ct_resource_i0 *ct_resource_i = api;

    uint64_t type_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);

    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(_G.resource_db);
    ce_cdb_a0->set_subobject(w, ct_resource_i->cdb_type(), type_obj);
    ce_cdb_a0->write_commit(w);

    ce_hash_add(&_G.type_map, ct_resource_i->cdb_type(), (uint64_t) api,
                _G.allocator);
}

static void load(uint64_t type,
                 const uint64_t *names,
                 size_t count,
                 int force);

static uint64_t get_obj(struct ct_resource_id resource_id);

static void load_now(uint64_t type,
                     const uint64_t *names,
                     size_t count) {
    load(type, names, count, 0);
}

static int can_get(uint64_t type,
                   uint64_t name) {
    const ce_cdb_obj_o * reader = ce_cdb_a0->read(_G.resource_db);
    uint64_t type_obj = ce_cdb_a0->read_ref(reader, type, 0);

    return ce_cdb_a0->prop_exist(type_obj, name);
}

static int can_get_all(uint64_t type,
                       const uint64_t *names,
                       size_t count) {
    const ce_cdb_obj_o * reader = ce_cdb_a0->read(_G.resource_db);
    uint64_t type_obj = ce_cdb_a0->read_ref(reader, type, 0);

    for (size_t i = 0; i < count; ++i) {
        if (!ce_cdb_a0->prop_exist(type_obj, names[i])) {
            return 0;
        }
    }

    return 1;
}

static struct ct_resource_i0 *get_resource_interface(uint64_t type) {
    return (struct ct_resource_i0 *) ce_hash_lookup(&_G.type_map, type, 0);
}

//static void _load_obj(uint64_t from,
//                      uint64_t parent) {
//    if (from == parent) {
//        ce_log_a0->error(LOG_WHERE, "from == parent, fix it");
//        return;
//    }
//
//    const uint32_t prop_count = ce_cdb_a0->prop_count(from);
//    uint64_t keys[prop_count];
//    ce_cdb_a0->prop_keys(from, keys);
//
//    const char *prefab = ce_cdb_a0->read_str(from, PREFAB_NAME_PROP, NULL);
//
//    uint64_t prefab_res = 0;
//    if (prefab) {
//        struct ct_resource_id prefab_rid = {};
//        ct_resource_compiler_a0->type_name_from_filename(prefab, &prefab_rid, NULL);
//
//        prefab_res = get_obj(prefab_rid);
//
//        ce_cdb_a0->set_prefab(from, prefab_res);
//    }
//
//    for (int i = 0; i < prop_count; ++i) {
//        uint64_t key = keys[i];
//        enum ce_cdb_type type = ce_cdb_a0->prop_type(from, keys[i]);
//
//        if (type == CDB_TYPE_SUBOBJECT) {
//            uint64_t from_subobj;
//            from_subobj = ce_cdb_a0->read_ref(from, key, 0);
//
//            uint64_t parent_subobj = 0;
//
//            if (parent) {
//                parent_subobj = ce_cdb_a0->read_ref(parent, key, 0);
//            } else if (prefab_res) {
//                parent_subobj = ce_cdb_a0->read_ref(prefab_res, key, 0);
//            }
//
//            if (parent_subobj) {
//                ce_cdb_a0->set_prefab(from_subobj, parent_subobj);
//            }
//
//            _load_obj(from_subobj, parent_subobj);
//        }
//    }
//}


static void load(uint64_t type,
                 const uint64_t *names,
                 size_t count,
                 int force) {
    uint32_t start_ticks = ce_os_a0->time->ticks();

    struct ct_resource_i0 *resource_i = get_resource_interface(type);

    if (!resource_i) {
        return;
    }

    uint64_t resource_objects[count];

    for (uint32_t i = 0; i < count; ++i) {
        resource_objects[i] = 0;

        const uint64_t asset_name = names[i];

        if (!force && can_get(type, asset_name)) {
            continue;
        };

        uint64_t object = ce_cdb_a0->create_object(_G.db,
                                                   resource_i->cdb_type());
        resource_objects[i] = object;

        struct ct_resource_id rid = (struct ct_resource_id) {
                .name = asset_name,
                .type = type,
        };

        char filename[1024] = {};
        ct_resource_compiler_a0->get_filename(filename, CE_ARRAY_LEN(filename),
                                              rid);

        ce_log_a0->debug(LOG_WHERE, "Loading resource %s", filename);
        if (!ct_builddb_a0->load_cdb_file(rid, object, _G.allocator)) {
            ce_log_a0->error(LOG_WHERE, "Could not load resource %s", filename);
            ce_cdb_a0->destroy_object(object);
            continue;
        }

        if (resource_i->online) {
            resource_i->online(names[i], object);
        }

        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(object);
        ce_cdb_a0->set_uint64(w, RESOURCE_NAME_PROP, names[i]);
        ce_cdb_a0->set_uint64(w, RESOURCE_TYPE_PROP, type);
        ce_cdb_a0->write_commit(w);
    }

    const ce_cdb_obj_o * reader = ce_cdb_a0->read(_G.resource_db);
    uint64_t type_obj = ce_cdb_a0->read_ref(reader, type, 0);
    ce_cdb_obj_o *w;
    do {
        w = ce_cdb_a0->write_begin(type_obj);
        for (uint32_t i = 0; i < count; ++i) {
            if (!resource_objects[i]) continue;

            const uint64_t asset_name = names[i];

            ce_cdb_a0->set_ref(w, asset_name, resource_objects[i]);
        }
    } while (!ce_cdb_a0->write_try_commit(w));

    uint32_t now_ticks = ce_os_a0->time->ticks();
    uint32_t dt = now_ticks - start_ticks;
    ce_log_a0->debug(LOG_WHERE, "load time %f for %zu resource", dt * 0.001,
                     count);
}

static void unload(uint64_t type,
                   const uint64_t *names,
                   size_t count) {

    const ce_cdb_obj_o * reader = ce_cdb_a0->read(_G.resource_db);
    uint64_t type_obj = ce_cdb_a0->read_ref(reader, type, 0);

    struct ct_resource_i0 *resource_i = get_resource_interface(type);

    if (!resource_i) {
        return;
    }

    for (uint32_t i = 0; i < count; ++i) {
        if (1) {// TODO: ref counting
            char build_name[128] = {};
            struct ct_resource_id rid = (struct ct_resource_id) {
                    .type = type,
                    .name = names[i],
            };

            type_name_string(build_name, CE_ARRAY_LEN(build_name), rid);

            char filename[1024] = {};
            ct_resource_compiler_a0->get_filename(filename,
                                                  CE_ARRAY_LEN(filename),
                                                  rid);

            ce_log_a0->debug(LOG_WHERE, "Unload resource %s ", filename);


            const ce_cdb_obj_o * reader = ce_cdb_a0->read(type_obj);
            uint64_t object;
            object = ce_cdb_a0->read_ref(reader, rid.name, 0);

            if (!object) {
                continue;
            }

            if (resource_i->offline) {
                resource_i->offline(names[i], object);
            }
        }
    }
}

static uint64_t get_obj(struct ct_resource_id resource_id) {
    const ce_cdb_obj_o * reader = ce_cdb_a0->read(_G.resource_db);
    uint64_t type_obj = ce_cdb_a0->read_ref(reader,
                                            resource_id.type, 0);

    if (!type_obj) {
        return 0;
    }


    const ce_cdb_obj_o * type_reader = ce_cdb_a0->read(type_obj);

    uint64_t object;
    object = ce_cdb_a0->read_ref(type_reader, resource_id.name, 0);

    if (!object) {
        char build_name[128] = {};
        type_name_string(build_name, CE_ARRAY_LEN(build_name), resource_id);

        char filename[1024] = {};
        ct_resource_compiler_a0->get_filename(filename,
                                              CE_ARRAY_LEN(filename),
                                              resource_id);

        ce_log_a0->warning(LOG_WHERE, "Autoloading resource %s", filename);
        load_now(resource_id.type, &resource_id.name, 1);

        const ce_cdb_obj_o * type_reader = ce_cdb_a0->read(type_obj);
        object = ce_cdb_a0->read_ref(type_reader, resource_id.name, 0);
    }

    return object;
}

static void reload(uint64_t type,
                   const uint64_t *names,
                   size_t count) {
}

static void reload_all() {
}

static void put(struct ct_resource_id resource_id,
                uint64_t obj) {
    const ce_cdb_obj_o * reader = ce_cdb_a0->read(_G.resource_db);
    uint64_t type_obj = ce_cdb_a0->read_ref(reader,
                                            resource_id.type, 0);

    if (!type_obj) {
        return;
    }

    const ce_cdb_obj_o * type_reader = ce_cdb_a0->read(type_obj);
    uint64_t object;
    object = ce_cdb_a0->read_ref(type_reader, resource_id.name, 0);

    if (!object) {
        return;
    }

    struct ct_resource_i0 *resource_i = get_resource_interface(
            resource_id.type);

    if (!resource_i) {
        return;
    }

    if (resource_i->online) {
        resource_i->online(resource_id.name, obj);
    }

    if (resource_i->offline) {
        resource_i->offline(resource_id.name, object);
    }

    ce_cdb_a0->move(obj, object);
}

static struct ct_resource_a0 resource_api = {
        .get_interface = get_resource_interface,
        .load = load,
        .load_now = load_now,
        .unload = unload,
        .reload = reload,
        .reload_all = reload_all,
        .can_get = can_get,
        .can_get_all = can_get_all,
        .get = get_obj,
        .reload_from_obj = put,
        .type_name_string = type_name_string,
};

int package_init(struct ce_api_a0 *api);

void package_shutdown();

struct ce_task_counter_t *package_load(uint64_t name);

void package_unload(uint64_t name);

int package_is_loaded(uint64_t name);

void package_flush(struct ce_task_counter_t *counter);

static struct ct_package_a0 package_api = {
        .load = package_load,
        .unload = package_unload,
        .is_loaded = package_is_loaded,
        .flush = package_flush,
};

struct ct_resource_a0 *ct_resource_a0 = &resource_api;
struct ct_package_a0 *ct_package_a0 = &package_api;

static void _init_api(struct ce_api_a0 *api) {
    api->register_api("ct_resource_a0", &resource_api);
    api->register_api("ct_package_a0", &package_api);

}


static void _init_cvar(struct ce_config_a0 *config) {
    _G = (struct _G) {};

    ce_config_a0 = config;
    _G.config = ce_config_a0->obj();

    ce_cdb_obj_o *writer = ce_cdb_a0->write_begin(_G.config);
    if (!ce_cdb_a0->prop_exist(_G.config, CONFIG_BUILD)) {
        ce_cdb_a0->set_str(writer, CONFIG_BUILD, "build");
    }
    ce_cdb_a0->write_commit(writer);
}


static void _init(struct ce_api_a0 *api) {
    _init_api(api);
    _init_cvar(ce_config_a0);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .config = ce_config_a0->obj(),
            .db = ce_cdb_a0->db()
    };

    _G.resource_db = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);

    const ce_cdb_obj_o * reader = ce_cdb_a0->read(_G.config);

    ce_fs_a0->map_root_dir(BUILD_ROOT,
                           ce_cdb_a0->read_str(reader, CONFIG_BUILD, ""),
                           false);

    ce_api_a0->register_on_add(RESOURCE_I, _resource_api_add);

    package_init(api);
}

static void _shutdown() {
    package_shutdown();

    ce_cdb_a0->destroy_db(_G.db);

    ce_hash_free(&_G.type_map, _G.allocator);
}


CE_MODULE_DEF(
        resourcesystem,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_fs_a0);
            CE_INIT_API(api, ce_config_a0);
            CE_INIT_API(api, ce_os_a0);
            CE_INIT_API(api, ce_log_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ce_cdb_a0);
        },
        {
            CE_UNUSED(reload);
            _init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);

            _shutdown();

        }
)
