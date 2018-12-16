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
#include "cetech/asset/sourcedb.h"
#include <cetech/kernel/kernel.h>
#include <cetech/editor/resource_browser.h>

#define LOG_WHERE "sourcedb"

#define _G sourcedb_globals

static struct _G {
    struct ce_hash_t cache_map;
    struct ce_spinlock cache_lock;

    uint64_t modified;

//    struct ce_hash_t objects;

    struct ce_alloc *allocator;
} _G;

//
//static void _revert_keys(uint64_t *keys,
//                         uint64_t n) {
//    uint64_t end = n - 1;
//    for (uint64_t c = 0; c < n / 2; c++) {
//        uint64_t t = keys[c];
//        keys[c] = keys[end];
//        keys[end] = t;
//        end--;
//    }
//}
//
//static void _collect_keys(struct ct_resource_id rid,
//                          uint64_t obj,
//                          uint64_t **keys,
//                          struct ce_alloc *alloc) {
//
//    do {
//        if (ce_cdb_a0->read_uint64(obj, RESOURCE_NAME, 0) == rid.name) {
//            break;
//        }
//
//        uint64_t k = ce_cdb_a0->key(obj);
//        ce_array_push(*keys, k, alloc);
//
//        obj = ce_cdb_a0->parent(obj);
//    } while (true);
//
//    _revert_keys(*keys, ce_array_size(*keys));
//}

uint64_t _find_root(uint64_t obj) {

    uint64_t ret = 0;

    while (true) {
        uint64_t parent = ce_cdb_a0->parent(obj);
        if (!parent) {
            break;
        }

        if (ce_cdb_a0->prop_exist(obj, RESOURCE_NAME)) {
            ret = obj;
        }

        obj = parent;
    }

    return ret;
}

//static void _put_modified(uint64_t asset_obj) {
//    ce_cdb_obj_o *w;
//    do {
//        w = ce_cdb_a0->write_begin(_G.modified);
//        ce_cdb_a0->set_uint64(w, asset_obj, asset_obj);
//    } while (!ce_cdb_a0->write_try_commit(w));
//}

//static void _on_obj_removed(uint64_t obj,
//                            const uint64_t *prop,
//                            uint32_t prop_count,
//                            void *data) {
//
//    uint64_t asset_obj = _find_root(obj);
//    uint64_t asset_type = ce_cdb_a0->type(asset_obj);
//    uint64_t asset_name = ce_cdb_a0->read_uint64(asset_obj, RESOURCE_NAME, 0);
//
//    _put_modified(asset_obj);
//
//    struct ct_resource_i0 *ri = ct_resource_a0->get_interface(asset_type);
//    if (ri && ri->get_interface) {
//        const struct ct_sourcedb_asset_i0 *sa;
//        sa = (struct ct_sourcedb_asset_i0 *) (ri->get_interface(
//                SOURCEDB_I));
//
//        if (sa) {
//            if (sa->removed) {
//                sa->removed(asset_obj, obj, prop, prop_count);
//            } else {
//                ct_resource_compiler_a0->compile_and_reload(asset_type,
//                                                            asset_name);
//            }
//        } else {
//            ct_resource_compiler_a0->compile_and_reload(asset_type, asset_name);
//        }
//    }
//
////    ce_cdb_obj_o *w;
////    uint64_t prop_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);
////    w = ce_cdb_a0->write_begin(prop_obj);
////    for (int i = 0; i < prop_count; ++i) {
////        ce_cdb_a0->set_uint64(w, prop[i], prop[i]);
////    }
////    ce_cdb_a0->write_commit(w);
////
////    uint64_t resource_chanegd;
////    resource_chanegd = ce_cdb_a0->create_object(ce_cdb_a0->db(),
////                                                SOURCEDB_CHANGED);
////    w = ce_cdb_a0->write_begin(resource_chanegd);
////    ce_cdb_a0->set_uint64(w, ASSET_OBJ, asset_obj);
////    ce_cdb_a0->set_uint64(w, ASSET_CHANGED_OBJ, obj);
////    ce_cdb_a0->set_subobject(w, ASSET_CHANGED_PROP, prop_obj);
////    ce_cdb_a0->write_commit(w);
////
////    ce_ebus_a0->send_obj(SOURCEDB_EBUS, asset_type, SOURCEDB_CHANGED,
////                         resource_chanegd);
//}

//static void _on_obj_change(uint64_t obj,
//                           const uint64_t *prop,
//                           uint32_t prop_count,
//                           void *data) {
//    uint64_t resource_chanegd;
//    resource_chanegd = ce_cdb_a0->create_object(ce_cdb_a0->db(),
//                                                SOURCEDB_CHANGED);
//
//    uint64_t asset_obj = _find_root(obj);
//
//    if (!asset_obj) {
//        ce_log_a0->error(LOG_WHERE, "Could not find asset from obj");
//        return;
//    }
//
//    _put_modified(asset_obj);
//
//    uint64_t asset_type = ce_cdb_a0->type(asset_obj);
//    uint64_t asset_name = ce_cdb_a0->read_uint64(asset_obj, RESOURCE_NAME, 0);
//
//    struct ct_resource_i0 *ri = ct_resource_a0->get_interface(asset_type);
//    if (ri && ri->get_interface) {
//        const struct ct_sourcedb_asset_i0 *sa;
//        sa = (struct ct_sourcedb_asset_i0 *) (ri->get_interface(SOURCEDB_I));
//
//        if (sa) {
//            if (sa->changed) {
//                sa->changed(asset_obj, obj, prop, prop_count);
//            } else {
//                ct_resource_compiler_a0->compile_and_reload(asset_type,
//                                                            asset_name);
//            }
//        } else {
//            ct_resource_compiler_a0->compile_and_reload(asset_type, asset_name);
//        }
//    }
//
//    for (int i = 0; i < prop_count; ++i) {
//        uint64_t p = prop[i];
//        enum ce_cdb_type t = ce_cdb_a0->prop_type(obj, p);
//        if (CDB_TYPE_SUBOBJECT == t) {
//            uint64_t o = ce_cdb_a0->read_subobject(obj, p, 0);
//            ce_hash_add(&_G.objects, o, o, _G.allocator);
//        }
//    }
//
//    ce_cdb_obj_o *w;
//    uint64_t prop_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);
//    w = ce_cdb_a0->write_begin(prop_obj);
//    for (int i = 0; i < prop_count; ++i) {
//        ce_cdb_a0->set_uint64(w, prop[i], prop[i]);
//    }
//    ce_cdb_a0->write_commit(w);
//
//    w = ce_cdb_a0->write_begin(resource_chanegd);
//    ce_cdb_a0->set_uint64(w, ASSET_OBJ, asset_obj);
//    ce_cdb_a0->set_uint64(w, ASSET_CHANGED_OBJ, obj);
//    ce_cdb_a0->set_subobject(w, ASSET_CHANGED_PROP, prop_obj);
//    ce_cdb_a0->write_commit(w);
//
//    ce_ebus_a0->send_obj(SOURCEDB_EBUS, asset_type, SOURCEDB_CHANGED,
//                         resource_chanegd);
//}

static uint64_t get(struct ct_resource_id resource_id);

static void _expand(uint64_t to,
                    uint64_t from) {

    const ce_cdb_obj_o * to_reader = ce_cdb_a0->read(to);

    const char *prefab = ce_cdb_a0->read_str(to_reader, PREFAB_NAME_PROP, NULL);

    if (prefab) {
        struct ct_resource_id prefab_rid = {};
        ct_resource_compiler_a0->type_name_from_filename(prefab,
                                                         &prefab_rid,
                                                         NULL);
        from = get(prefab_rid);
    }

    const ce_cdb_obj_o * from_reader = ce_cdb_a0->read(from);

    const uint32_t prop_count = ce_cdb_a0->prop_count(from);
    const uint64_t *keys = ce_cdb_a0->prop_keys(from);

    for (int i = 0; i < prop_count; ++i) {
        uint64_t key = keys[i];
        enum ce_cdb_type type = ce_cdb_a0->prop_type(from, key);


        if (type == CDB_TYPE_SUBOBJECT) {
            if (ce_cdb_a0->prop_exist(to, key)) {
                uint64_t v = ce_cdb_a0->read_subobject(to_reader, key, 0);
                uint64_t f = ce_cdb_a0->read_subobject(from_reader, key, 0);
                _expand(v, f);
            } else {
                uint64_t v = ce_cdb_a0->read_subobject(from_reader, key, 0);
                uint64_t new_v = ce_cdb_a0->create_from(ce_cdb_a0->db(), v);
                ce_cdb_obj_o *w = ce_cdb_a0->write_begin(to);
                ce_cdb_a0->set_subobject(w, key, new_v);
                ce_cdb_a0->write_commit(w);
            }
        }


        if (ce_cdb_a0->prop_exist(to, key)) {
            continue;
        }

        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(to);

        switch (type) {
            case CDB_TYPE_UINT64: {
                uint64_t v = ce_cdb_a0->read_uint64(from_reader, key, 0);
                ce_cdb_a0->set_uint64(w, key, v);
            }
                break;
            case CDB_TYPE_PTR: {
                void *v = ce_cdb_a0->read_ptr(from_reader, key, 0);
                ce_cdb_a0->set_ptr(w, key, v);
            }
                break;
            case CDB_TYPE_REF: {
                uint64_t v = ce_cdb_a0->read_uint64(from_reader, key, 0);
                ce_cdb_a0->set_ref(w, key, v);
            }
                break;
            case CDB_TYPE_FLOAT: {
                float v = ce_cdb_a0->read_float(from_reader, key, 0);
                ce_cdb_a0->set_float(w, key, v);
            }
                break;
            case CDB_TYPE_BOOL: {
                bool v = ce_cdb_a0->read_bool(from_reader, key, 0);
                ce_cdb_a0->set_bool(w, key, v);
            }
                break;
            case CDB_TYPE_STR: {
                const char *v = ce_cdb_a0->read_str(from_reader, key, NULL);
                ce_cdb_a0->set_str(w, key, v);
            }
                break;
            case CDB_TYPE_BLOB: {
                uint64_t size = 0;
                void *v = ce_cdb_a0->read_blob(from_reader, key, &size,
                                               NULL);
                ce_cdb_a0->set_blob(w, key, v, size);
            }
                break;
            default:
                break;
        }

        ce_cdb_a0->write_commit(w);
    }


    {
        const ce_cdb_obj_o * to_reader = ce_cdb_a0->read(to);
        const uint32_t prop_count = ce_cdb_a0->prop_count(to);
        const uint64_t *keys = ce_cdb_a0->prop_keys(to);

        for (int i = 0; i < prop_count; ++i) {
            uint64_t key = keys[i];
            enum ce_cdb_type type = ce_cdb_a0->prop_type(to, key);

            if (type == CDB_TYPE_SUBOBJECT) {
                uint64_t v = ce_cdb_a0->read_subobject(to_reader, key, 0);
                _expand(v, 0);
            }
        }
    }

//    ce_hash_add(&_G.objects, to, to, _G.allocator);
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
        ct_builddb_a0->get_filename_from_type_name(CE_ARR_ARG(filename),
                                                   resource_id.type,
                                                   resource_id.name);

        uint64_t obj = ce_ydb_a0->get_obj(filename);
        if (!obj) {
            return 0;
        }

        const ce_cdb_obj_o * reader = ce_cdb_a0->read(obj);
        resource_obj = ce_cdb_a0->read_subobject(reader, resource_key, 0);
        if (!resource_obj) {
            return 0;
        }

        ce_cdb_a0->set_type(resource_obj, resource_id.type);

        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(resource_obj);
        ce_cdb_a0->set_str(w, RESOURCE_NAME,
                           ce_id_a0->str_from_id64(resource_id.name));
        ce_cdb_a0->write_commit(w);

        _expand(resource_obj, 0);

        const struct ct_resource_i0 *ri;
        ri = ct_resource_a0->get_interface(resource_id.type);
        if (ri && ri->get_interface) {
            const struct ct_sourcedb_asset_i0 *sa;
            sa = ri->get_interface(SOURCEDB_I);

            if (sa && sa->load) {
                uint64_t new_obj = sa->load(resource_obj);
                resource_obj = new_obj;
            }
        }

        ce_os_a0->thread->spin_lock(&_G.cache_lock);
        ce_hash_add(&_G.cache_map, resource_key, resource_obj,
                    _G.allocator);
        ce_os_a0->thread->spin_unlock(&_G.cache_lock);
    }

    return resource_obj;
}


static bool save(struct ct_resource_id resource_id) {
    char filename[256] = {};
    ct_builddb_a0->get_filename_from_type_name(CE_ARR_ARG(filename),
                                               resource_id.type,
                                               resource_id.name);

    ce_ydb_a0->save(filename);

    uint64_t obj = get(resource_id);

    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(_G.modified);
    ce_cdb_a0->remove_property(w, obj);
    ce_cdb_a0->write_commit(w);

    return true;
}

static bool save_all() {
    uint64_t n = ce_cdb_a0->prop_count(_G.modified);
    const uint64_t *k = ce_cdb_a0->prop_keys(_G.modified);

    for (int i = 0; i < n; ++i) {
        uint64_t p = k[i];

        const ce_cdb_obj_o * reader = ce_cdb_a0->read(p);

        uint64_t asset_name = ce_cdb_a0->read_uint64(reader, RESOURCE_NAME, 0);

        struct ct_resource_id rid = {
                .name = asset_name,
                .type = ce_cdb_a0->obj_type(p),
        };

        save(rid);
    }

    return true;
}

#define _SET_PROP \
    CE_ID64_0("set_prop", 0x8c6d1f1479cbdf46ULL)

#define _ADD_SUBOBJ \
    CE_ID64_0("add_subobj", 0xc7cc6c9b6a1049d6ULL)

#define _REMOVE_PROP \
    CE_ID64_0("remove_prop", 0x81c94db83d243c60ULL)


#define _NEW_VALUE \
    CE_ID64_0("new_value", 0x1d77a29c912111fULL)

#define _OLD_VALUE \
    CE_ID64_0("old_value", 0x8115d649f2a9636aULL)

#define _KEYS \
    CE_ID64_0("keys", 0xa62f9297dc969e85ULL)

#define _PROP \
    CE_ID64_0("property", 0xcbd168fb77919b23ULL)



static struct ct_sourcedb_a0 source_db_api = {
        .get = get,
        .save = save,
        .save_all = save_all,

};

struct ct_sourcedb_a0 *ct_sourcedb_a0 = &source_db_api;

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .modified = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0),
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