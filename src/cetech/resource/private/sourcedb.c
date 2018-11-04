#include <time.h>
#include <stdio.h>
#include <inttypes.h>
#include <stdbool.h>

#include <celib/log.h>
#include <celib/os.h>
#include <celib/memory.h>
#include <celib/fs.h>
#include <celib/hashlib.h>
#include <celib/module.h>
#include <celib/api_system.h>
#include <celib/cdb.h>
#include <celib/ydb.h>
#include <celib/config.h>
#include <celib/buffer.inl>
#include <celib/task.h>
#include <celib/hash.inl>
#include <celib/ebus.h>

#include <cetech/resource/resource.h>
#include <cetech/resource/resource_compiler.h>
#include "cetech/resource/builddb.h"
#include "cetech/resource/sourcedb.h"
#include <cetech/kernel/kernel.h>

#define LOG_WHERE "sourcedb"

#define _G sourcedb_globals

static struct _G {
    struct ce_hash_t cache_map;
    struct ce_spinlock cache_lock;

    struct ce_alloc *allocator;
} _G;


uint64_t _find_root(uint64_t obj) {
    if (ce_cdb_a0->prop_exist(obj, ASSET_NAME)) {
        return obj;
    }

    uint64_t parent = ce_cdb_a0->parent(obj);
    if (!parent) {
        return 0;
    }

    return _find_root(parent);
}

static void _on_obj_change(uint64_t obj,
                           const uint64_t *prop,
                           uint32_t prop_count,
                           void *data) {
    uint64_t resource_chanegd;
    resource_chanegd = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                                SOURCEDB_CHANGED);

    uint64_t asset_obj = _find_root(obj);
    uint64_t asset_type = ce_cdb_a0->type(asset_obj);
    uint64_t asset_name = ce_cdb_a0->read_uint64(asset_obj, ASSET_NAME, 0);

    struct ct_resource_i0 *ri = ct_resource_a0->get_interface(asset_type);
    if (ri && ri->get_interface) {
        const struct ct_sourcedb_asset_i0 *sa;
        sa = (struct ct_sourcedb_asset_i0 *) (ri->get_interface(
                SOURCEDB_I));

        if (sa) {
            if (sa->changed) {
                sa->changed(asset_obj, obj, prop, prop_count);
            } else {
                ct_resource_compiler_a0->compile_and_reload(asset_type,
                                                            asset_name);
            }
        } else {
            ct_resource_compiler_a0->compile_and_reload(asset_type, asset_name);
        }
    }

    ce_cdb_obj_o *w;
    uint64_t prop_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);
    w = ce_cdb_a0->write_begin(prop_obj);
    for (int i = 0; i < prop_count; ++i) {
        ce_cdb_a0->set_uint64(w, prop[i], prop[i]);
    }
    ce_cdb_a0->write_commit(w);

    w = ce_cdb_a0->write_begin(resource_chanegd);
    ce_cdb_a0->set_uint64(w, ASSET_OBJ, asset_obj);
    ce_cdb_a0->set_uint64(w, ASSET_CHANGED_OBJ, obj);
    ce_cdb_a0->set_subobject(w, ASSET_CHANGED_PROP, prop_obj);
    ce_cdb_a0->write_commit(w);

    ce_ebus_a0->send_obj(SOURCEDB_EBUS, asset_type, SOURCEDB_CHANGED,
                         resource_chanegd);
}

static uint64_t get(struct ct_resource_id resource_id);

static void _load(uint64_t from,
                  uint64_t parent) {

    const uint32_t prop_count = ce_cdb_a0->prop_count(from);
    uint64_t keys[prop_count];
    ce_cdb_a0->prop_keys(from, keys);

    uint64_t prefab_res = 0;
    const char *prefab = ce_cdb_a0->read_str(from, PREFAB_NAME_PROP, NULL);
    if (prefab) {
        struct ct_resource_id prefab_rid = {};
        ct_resource_compiler_a0->type_name_from_filename(prefab,
                                                         &prefab_rid,
                                                         NULL);
        prefab_res = get(prefab_rid);

        CE_ASSERT(LOG_WHERE, prefab_res != 0);
    }

    for (int i = 0; i < prop_count; ++i) {
        uint64_t key = keys[i];
        enum ce_cdb_type type = ce_cdb_a0->prop_type(from, keys[i]);

        if (type == CDB_TYPE_SUBOBJECT) {
            uint64_t from_subobj;
            from_subobj = ce_cdb_a0->read_subobject(from, key, 0);

            uint64_t parent_subobj = 0;

            CE_ASSERT(LOG_WHERE, !prefab || !parent);

            if (prefab_res) {
                parent_subobj = ce_cdb_a0->read_subobject(prefab_res, key, 0);
            } else if (parent) {
                parent_subobj = ce_cdb_a0->read_subobject(parent, key, 0);
            }

            _load(from_subobj, parent_subobj);

            if(parent_subobj) {
                ce_cdb_a0->set_prefab(from_subobj, parent_subobj);
            }
        }
    }

    if(prefab_res) {
        ce_cdb_a0->set_prefab(from, prefab_res);
    }

    ce_cdb_a0->register_notify(from, _on_obj_change, NULL);
}

static uint64_t get(struct ct_resource_id resource_id) {
    char fullname[256] = {0};
    ct_builddb_a0->get_fullname(CE_ARR_ARG(fullname),
                                resource_id.type,
                                resource_id.name);

    uint64_t resource_key = ce_id_a0->id64(fullname);

    ce_os_a0->thread->spin_lock(&_G.cache_lock);
    uint64_t resource_obj = ce_hash_lookup(&_G.cache_map, resource_key, 0);
    ce_os_a0->thread->spin_unlock(&_G.cache_lock);

    if (!resource_obj) {
        ce_log_a0->debug(LOG_WHERE, "Load resource %s to cache.", fullname);

        char filename[256] = {0};
        ct_builddb_a0->get_filename_type_name(CE_ARR_ARG(filename),
                                              resource_id.type,
                                              resource_id.name);

        uint64_t obj = ce_ydb_a0->get_obj(filename);
        if(!obj) {
            return 0;
        }

        resource_obj = ce_cdb_a0->read_subobject(obj, resource_key, 0);
        if(!resource_obj) {
            return 0;
        }

        ce_cdb_a0->set_type(resource_obj, resource_id.type);

        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(resource_obj);
        ce_cdb_a0->set_uint64(w, ASSET_NAME, resource_id.name);
        ce_cdb_a0->write_commit(w);

        const struct ct_resource_i0 *ri;
        ri = ct_resource_a0->get_interface(resource_id.type);

        if (ri && ri->get_interface) {
            const struct ct_sourcedb_asset_i0 *sa;
            sa = ri->get_interface(SOURCEDB_I);

            if (sa && sa->anotate) {
                sa->anotate(resource_obj);
            }
        }

        _load(resource_obj, 0);

        ce_os_a0->thread->spin_lock(&_G.cache_lock);
        ce_hash_add(&_G.cache_map, resource_key, resource_obj, _G.allocator);
        ce_os_a0->thread->spin_unlock(&_G.cache_lock);
    }

    return resource_obj;
}

static struct ct_sourcedb_a0 source_db_api = {
        .get = get,
};

struct ct_sourcedb_a0 *ct_sourcedb_a0 = &source_db_api;

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ce_memory_a0->system
    };

    ce_ebus_a0->create_ebus(SOURCEDB_EBUS);
    api->register_api("ct_sourcedb_a0", ct_sourcedb_a0);
}

static void _shutdown() {

    _G = (struct _G) {};
}

CE_MODULE_DEF(
        sourcedb,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);

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