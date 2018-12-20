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
    struct ce_hash_t res_map;
    struct ce_hash_t type_map;

    struct ce_cdb_t db;

    uint64_t config;
    struct ce_alloc *allocator;
} _G = {};

//==============================================================================
// Private
//==============================================================================


//==============================================================================
// Public interface
//==============================================================================

static void _resource_api_add(uint64_t name,
                              void *api) {
    struct ct_resource_i0 *ct_resource_i = api;
    ce_hash_add(&_G.type_map, ct_resource_i->cdb_type(), (uint64_t) api,
                _G.allocator);
}

static void load(const uint64_t *names,
                 size_t count,
                 int force);

static uint64_t get_obj(struct ct_resource_id resource_id);

static void load_now(const uint64_t *names,
                     size_t count) {
    load(names, count, 0);
}

static int can_get(uint64_t name) {
    return ce_hash_contain(&_G.res_map, name);
}

static int can_get_all(const uint64_t *names,
                       size_t count) {
    for (size_t i = 0; i < count; ++i) {
        if (!ce_hash_contain(&_G.res_map, names[i])) {
            return 0;
        }
    }

    return 1;
}

static struct ct_resource_i0 *get_resource_interface(uint64_t type) {
    return (struct ct_resource_i0 *) ce_hash_lookup(&_G.type_map, type, 0);
}


static void load(const uint64_t *names,
                 size_t count,
                 int force) {
    uint32_t start_ticks = ce_os_a0->time->ticks();

    uint64_t resource_objects[count];

    for (uint32_t i = 0; i < count; ++i) {
        resource_objects[i] = 0;

        const uint64_t asset_name = names[i];

        if (!force && can_get(asset_name)) {
            continue;
        };

        uint64_t type = ct_builddb_a0->get_resource_type(
                (struct ct_resource_id) {.uid=asset_name});

        struct ct_resource_i0 *resource_i = get_resource_interface(type);

        if (!resource_i) {
            return;
        }

        uint64_t object = ce_cdb_a0->create_object(_G.db,
                                                   resource_i->cdb_type());
        resource_objects[i] = object;

        struct ct_resource_id rid = {.uid = asset_name};

        ce_log_a0->debug(LOG_WHERE, "Loading resource 0x%llx", rid.uid);
        if (!ct_builddb_a0->load_cdb_file(rid, object, _G.allocator)) {
            ce_log_a0->error(LOG_WHERE,
                             "Could not load resource 0x%llx", rid.uid);
            ce_cdb_a0->destroy_object(object);
            continue;
        }

        if (resource_i->online) {
            resource_i->online(names[i], object);
        }
    }

    for (uint32_t i = 0; i < count; ++i) {
        if (!resource_objects[i]) continue;

        const uint64_t asset_name = names[i];

        ce_hash_add(&_G.res_map, asset_name, resource_objects[i], _G.allocator);
    }

    uint32_t now_ticks = ce_os_a0->time->ticks();
    uint32_t dt = now_ticks - start_ticks;
    ce_log_a0->debug(LOG_WHERE,
                     "load time %f for %zu resource", dt * 0.001, count);
}

static void unload(const uint64_t *names,
                   size_t count) {

    for (uint32_t i = 0; i < count; ++i) {
        if (1) {// TODO: ref counting
            struct ct_resource_id rid = (struct ct_resource_id) {
                    .uid = names[i],
            };

            uint64_t type = ct_builddb_a0->get_resource_type(rid);

            struct ct_resource_i0 *resource_i = get_resource_interface(type);
            if (!resource_i) {
                continue;
            }

            ce_log_a0->debug(LOG_WHERE, "Unload resource %llux ", rid.uid);

            uint64_t object = ce_hash_lookup(&_G.res_map, rid.uid, 0);
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
    uint64_t object = ce_hash_lookup(&_G.res_map, resource_id.uid, 0);
    if (!object) {
        ce_log_a0->warning(LOG_WHERE, "Loading resource 0x%llx",
                           resource_id.uid);
        load_now(&resource_id.uid, 1);

        object = ce_hash_lookup(&_G.res_map, resource_id.uid, 0);
    }

    return object;
}

static void reload(const uint64_t *names,
                   size_t count) {
}

static void reload_all() {
}

static void put(struct ct_resource_id resource_id,
                uint64_t obj) {

    uint64_t object = ce_hash_lookup(&_G.res_map, resource_id.uid, 0);

    if (!object) {
        return;
    }

    uint64_t type = ct_builddb_a0->get_resource_type(resource_id);

    struct ct_resource_i0 *resource_i = get_resource_interface(type);

    if (!resource_i) {
        return;
    }

    if (resource_i->online) {
        resource_i->online(resource_id.uid, obj);
    }

    if (resource_i->offline) {
        resource_i->offline(resource_id.uid, object);
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
//        .type_name_string = type_name_string,
};


struct ct_resource_a0 *ct_resource_a0 = &resource_api;

static void _init_api(struct ce_api_a0 *api) {
    api->register_api("ct_resource_a0", &resource_api);

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


    const ce_cdb_obj_o *reader = ce_cdb_a0->read(_G.config);

    ce_fs_a0->map_root_dir(BUILD_ROOT,
                           ce_cdb_a0->read_str(reader, CONFIG_BUILD, ""),
                           false);

    ce_api_a0->register_on_add(RESOURCE_I, _resource_api_add);
}

static void _shutdown() {
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
