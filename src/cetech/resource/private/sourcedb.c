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
#include <cetech/command_system/command_system.h>
#include <cetech/asset_editor/asset_browser.h>

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
//        if (ce_cdb_a0->read_uint64(obj, ASSET_NAME, 0) == rid.name) {
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

        if (ce_cdb_a0->prop_exist(obj, ASSET_NAME)) {
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
//    uint64_t asset_name = ce_cdb_a0->read_uint64(asset_obj, ASSET_NAME, 0);
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
//    uint64_t asset_name = ce_cdb_a0->read_uint64(asset_obj, ASSET_NAME, 0);
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
    const char *prefab = ce_cdb_a0->read_str(to, PREFAB_NAME_PROP, NULL);

    if (prefab) {
        struct ct_resource_id prefab_rid = {};
        ct_resource_compiler_a0->type_name_from_filename(prefab,
                                                         &prefab_rid,
                                                         NULL);
        from = get(prefab_rid);
        ce_cdb_a0->set_prefab(to, from);
    }

    const uint32_t prop_count = ce_cdb_a0->prop_count(from);
    const uint64_t *keys = ce_cdb_a0->prop_keys(from);

    for (int i = 0; i < prop_count; ++i) {
        uint64_t key = keys[i];
        enum ce_cdb_type type = ce_cdb_a0->prop_type(from, key);


        if (type == CDB_TYPE_SUBOBJECT) {
            if (ce_cdb_a0->prop_exist(to, key)) {
                uint64_t v = ce_cdb_a0->read_subobject(to, key, 0);
                uint64_t f = ce_cdb_a0->read_subobject(from, key, 0);
                _expand(v, f);
            } else {
                uint64_t v = ce_cdb_a0->read_subobject(from, key, 0);
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
                uint64_t v = ce_cdb_a0->read_uint64(from, key, 0);
                ce_cdb_a0->set_uint64(w, key, v);
            }
                break;
            case CDB_TYPE_PTR: {
                void *v = ce_cdb_a0->read_ptr(from, key, 0);
                ce_cdb_a0->set_ptr(w, key, v);
            }
                break;
            case CDB_TYPE_REF: {
                uint64_t v = ce_cdb_a0->read_uint64(from, key, 0);
                ce_cdb_a0->set_ref(w, key, v);
            }
                break;
            case CDB_TYPE_FLOAT: {
                float v = ce_cdb_a0->read_float(from, key, 0);
                ce_cdb_a0->set_float(w, key, v);
            }
                break;
            case CDB_TYPE_BOOL: {
                bool v = ce_cdb_a0->read_bool(from, key, 0);
                ce_cdb_a0->set_bool(w, key, v);
            }
                break;
            case CDB_TYPE_STR: {
                const char *v = ce_cdb_a0->read_str(from, key, NULL);
                ce_cdb_a0->set_str(w, key, v);
            }
                break;
            case CDB_TYPE_BLOB: {
                uint64_t size = 0;
                void *v = ce_cdb_a0->read_blob(from, key, &size,
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
        const uint32_t prop_count = ce_cdb_a0->prop_count(to);
        const uint64_t *keys = ce_cdb_a0->prop_keys(to);

        for (int i = 0; i < prop_count; ++i) {
            uint64_t key = keys[i];
            enum ce_cdb_type type = ce_cdb_a0->prop_type(to, key);

            if (type == CDB_TYPE_SUBOBJECT) {
                uint64_t v = ce_cdb_a0->read_subobject(to, key, 0);
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

        resource_obj = ce_cdb_a0->read_subobject(obj, resource_key, 0);
        if (!resource_obj) {
            return 0;
        }

        ce_cdb_a0->set_type(resource_obj, resource_id.type);

        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(resource_obj);
        ce_cdb_a0->set_str(w, ASSET_NAME,
                           ce_id_a0->str_from_id64(resource_id.name));
        ce_cdb_a0->write_commit(w);


        _expand(resource_obj, 0);

        const struct ct_resource_i0 *ri;
        ri = ct_resource_a0->get_interface(resource_id.type);
        if (ri && ri->get_interface) {
            const struct ct_sourcedb_asset_i0 *sa;
            sa = ri->get_interface(SOURCEDB_I);

            if (sa && sa->anotate) {
                sa->anotate(resource_obj);
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

        uint64_t asset_name = ce_cdb_a0->read_uint64(p, ASSET_NAME, 0);

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

void set_str(struct ct_resource_id rid,
             uint64_t prop,
             uint64_t *keys,
             uint64_t keys_n,
             const char *value,
             const char *new_value) {

    uint64_t cmd_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);
    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(cmd_obj);
    ce_cdb_a0->set_uint64(w, ASSET_NAME, rid.name);
    ce_cdb_a0->set_uint64(w, ASSET_TYPE, rid.type);
    ce_cdb_a0->set_uint64(w, _PROP, prop);
    ce_cdb_a0->set_blob(w, _KEYS, keys, sizeof(uint64_t) * keys_n);
    ce_cdb_a0->set_str(w, _NEW_VALUE, new_value);

    if (value) {
        ce_cdb_a0->set_str(w, _OLD_VALUE, value);
    }

    ce_cdb_a0->write_commit(w);

    struct ct_cdb_cmd_s cmd = {
            .header = {
                    .size = sizeof(struct ct_cdb_cmd_s),
                    .type = _SET_PROP,
            },
            .cmd= cmd_obj,
    };

    ct_cmd_system_a0->execute(&cmd.header);
}

void set_float(struct ct_resource_id rid,
               uint64_t prop,
               uint64_t *keys,
               uint64_t keys_n,
               float value,
               float new_value) {
    uint64_t cmd_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);
    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(cmd_obj);
    ce_cdb_a0->set_uint64(w, ASSET_NAME, rid.name);
    ce_cdb_a0->set_uint64(w, ASSET_TYPE, rid.type);
    ce_cdb_a0->set_uint64(w, _PROP, prop);
    ce_cdb_a0->set_blob(w, _KEYS, keys, sizeof(uint64_t) * keys_n);
    ce_cdb_a0->set_float(w, _NEW_VALUE, new_value);
    ce_cdb_a0->set_float(w, _OLD_VALUE, value);

    ce_cdb_a0->write_commit(w);

    struct ct_cdb_cmd_s cmd = {
            .header = {
                    .size = sizeof(struct ct_cdb_cmd_s),
                    .type = _SET_PROP,
            },
            .cmd= cmd_obj,
    };

    ct_cmd_system_a0->execute(&cmd.header);
}

void add_subobj(struct ct_resource_id rid,
                uint64_t prop,
                uint64_t *keys,
                uint64_t keys_n,
                uint64_t value,
                uint64_t new_value) {

    uint64_t cmd_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);
    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(cmd_obj);
    ce_cdb_a0->set_uint64(w, ASSET_NAME, rid.name);
    ce_cdb_a0->set_uint64(w, ASSET_TYPE, rid.type);
    ce_cdb_a0->set_uint64(w, _PROP, prop);
    ce_cdb_a0->set_blob(w, _KEYS, keys, sizeof(uint64_t) * keys_n);
    ce_cdb_a0->set_ref(w, _NEW_VALUE, new_value);
    ce_cdb_a0->set_ref(w, _OLD_VALUE, value);

    ce_cdb_a0->write_commit(w);

    struct ct_cdb_cmd_s cmd = {
            .header = {
                    .size = sizeof(struct ct_cdb_cmd_s),
                    .type = _ADD_SUBOBJ,
            },
            .cmd= cmd_obj,
    };

    ct_cmd_system_a0->execute(&cmd.header);
}

void remove_prop(struct ct_resource_id rid,
                 uint64_t *keys,
                 uint64_t keys_n,
                 uint64_t prop) {

    uint64_t obj = get(rid);
    obj = ce_cdb_a0->read_subobject_deep(obj, keys, keys_n, 0);

    enum ce_cdb_type t = ce_cdb_a0->prop_type(obj, prop);

    uint64_t cmd_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);
    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(cmd_obj);
    ce_cdb_a0->set_uint64(w, ASSET_NAME, rid.name);
    ce_cdb_a0->set_uint64(w, ASSET_TYPE, rid.type);
    ce_cdb_a0->set_uint64(w, _PROP, prop);
    ce_cdb_a0->set_blob(w, _KEYS, keys, sizeof(uint64_t) * keys_n);

    switch (t) {
        case CDB_TYPE_SUBOBJECT:
        case CDB_TYPE_REF: {
            uint64_t value = ce_cdb_a0->read_ref(obj, prop, 0);
            ce_cdb_a0->set_ref(w, _OLD_VALUE, value);
        }
            break;

        default:
            break;

    }

    ce_cdb_a0->write_commit(w);

    struct ct_cdb_cmd_s cmd = {
            .header = {
                    .size = sizeof(struct ct_cdb_cmd_s),
                    .type = _REMOVE_PROP,
            },
            .cmd= cmd_obj,
    };

    ct_cmd_system_a0->execute(&cmd.header);
}


void set_bool(struct ct_resource_id rid,
              uint64_t prop,
              uint64_t *keys,
              uint64_t keys_n,
              bool value,
              bool new_value) {

    uint64_t cmd_obj = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0);
    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(cmd_obj);
    ce_cdb_a0->set_uint64(w, ASSET_NAME, rid.name);
    ce_cdb_a0->set_uint64(w, ASSET_TYPE, rid.type);
    ce_cdb_a0->set_uint64(w, _PROP, prop);
    ce_cdb_a0->set_blob(w, _KEYS, keys, sizeof(uint64_t) * keys_n);
    ce_cdb_a0->set_bool(w, _NEW_VALUE, new_value);
    ce_cdb_a0->set_bool(w, _OLD_VALUE, value);

    ce_cdb_a0->write_commit(w);

    struct ct_cdb_cmd_s cmd = {
            .header = {
                    .size = sizeof(struct ct_cdb_cmd_s),
                    .type = _SET_PROP,
            },
            .cmd= cmd_obj,
    };

    ct_cmd_system_a0->execute(&cmd.header);
}

static struct ct_sourcedb_a0 source_db_api = {
        .get = get,
        .save = save,
        .save_all = save_all,

//        .set_str = set_str,
//        .set_bool = set_bool,
//        .set_float = set_float,
//        .add_subobj = add_subobj,
//        .remove_prop = remove_prop,
};

struct ct_sourcedb_a0 *ct_sourcedb_a0 = &source_db_api;

//static uint64_t _create_recursive(uint64_t obj,
//                                  uint64_t *keys,
//                                  uint64_t keys_n,
//                                  uint64_t kidx) {
//    uint64_t k = keys[kidx];
//    if (!k) {
//        return obj;
//    }
//
//    uint64_t root = ce_cdb_a0->read_subobject(obj, k, 0);
//
//    uint64_t new_obj = ce_cdb_a0->create_from(ce_cdb_a0->db(), root);
//    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(obj);
//    ce_cdb_a0->set_subobject(w, k, new_obj);
//    ce_cdb_a0->write_commit(w);
//
//    return _create_recursive(new_obj, keys, keys_n, kidx + 1);
//}

static uint64_t _find_recursive_create(uint64_t obj,
                                       uint64_t *keys,
                                       uint64_t keys_n) {

    uint64_t it_obj = obj;
    for (int i = 0; i < keys_n; ++i) {
        uint64_t k = keys[i];

//        if (!ce_cdb_a0->prop_exist_norecursive(it_obj, k)) {
//            it_obj = _create_recursive(it_obj, keys, keys_n, i + 1);
//            break;
//        } else {
        it_obj = ce_cdb_a0->read_subobject(it_obj, k, 0);
//        }
    }

    return it_obj;
}

static void set_cmd(const struct ct_cmd *_cmd,
                    bool inverse) {
    const struct ct_cdb_cmd_s *pos_cmd = (const struct ct_cdb_cmd_s *) _cmd;

    struct ct_resource_id rid = {
            .name=ce_cdb_a0->read_uint64(pos_cmd->cmd, ASSET_NAME, 0),
            .type=ce_cdb_a0->read_uint64(pos_cmd->cmd, ASSET_TYPE, 0),
    };

    uint64_t asset_obj = ct_sourcedb_a0->get(rid);

    uint64_t keys_n = 0;
    uint64_t *keys = ce_cdb_a0->read_blob(pos_cmd->cmd, _KEYS, &keys_n, NULL);
    keys_n = keys_n / sizeof(uint64_t);

    uint64_t prop = ce_cdb_a0->read_uint64(pos_cmd->cmd, _PROP, 0);

    uint64_t obj = _find_recursive_create(asset_obj, keys, keys_n);

    enum ce_cdb_type t = ce_cdb_a0->prop_type(pos_cmd->cmd, inverse ? _OLD_VALUE
                                                                    : _NEW_VALUE);

    switch (t) {
        case CDB_TYPE_STR: {
            const char *f = ce_cdb_a0->read_str(pos_cmd->cmd,
                                                inverse ? _OLD_VALUE
                                                        : _NEW_VALUE, 0);

            ce_cdb_obj_o *w = ce_cdb_a0->write_begin(obj);
            ce_cdb_a0->set_str(w, prop, f);
            ce_cdb_a0->write_commit(w);
        }
            break;

        case CDB_TYPE_BOOL: {
            bool f = ce_cdb_a0->read_bool(pos_cmd->cmd,
                                          inverse ? _OLD_VALUE : _NEW_VALUE, 0);

            ce_cdb_obj_o *w = ce_cdb_a0->write_begin(obj);
            ce_cdb_a0->set_bool(w, prop, f);
            ce_cdb_a0->write_commit(w);
        }
            break;


        case CDB_TYPE_UINT64: {
            uint64_t f = ce_cdb_a0->read_uint64(pos_cmd->cmd,
                                                inverse ? _OLD_VALUE
                                                        : _NEW_VALUE, 0);

            ce_cdb_obj_o *w = ce_cdb_a0->write_begin(obj);
            ce_cdb_a0->set_uint64(w, prop, f);
            ce_cdb_a0->write_commit(w);
        }
            break;

        case CDB_TYPE_FLOAT: {
            float f = ce_cdb_a0->read_float(pos_cmd->cmd,
                                            inverse ? _OLD_VALUE : _NEW_VALUE,
                                            0);

            ce_cdb_obj_o *w = ce_cdb_a0->write_begin(obj);
            ce_cdb_a0->set_float(w, prop, f);
            ce_cdb_a0->write_commit(w);
        }
            break;

        case CDB_TYPE_NONE:
            break;
        case CDB_TYPE_PTR:
            break;
        case CDB_TYPE_REF:
            break;
        case CDB_TYPE_SUBOBJECT:
            break;
        case CDB_TYPE_BLOB:
            break;
    }

}

static void add_suobj_cmd(const struct ct_cmd *_cmd,
                          bool inverse) {
    const struct ct_cdb_cmd_s *pos_cmd = (const struct ct_cdb_cmd_s *) _cmd;

    struct ct_resource_id rid = {
            .name=ce_cdb_a0->read_uint64(pos_cmd->cmd, ASSET_NAME, 0),
            .type=ce_cdb_a0->read_uint64(pos_cmd->cmd, ASSET_TYPE, 0),
    };

    uint64_t asset_obj = ct_sourcedb_a0->get(rid);

    uint64_t keys_n = 0;
    uint64_t *keys = ce_cdb_a0->read_blob(pos_cmd->cmd, _KEYS, &keys_n, NULL);
    keys_n = keys_n / sizeof(uint64_t);

    uint64_t prop = ce_cdb_a0->read_uint64(pos_cmd->cmd, _PROP, 0);

    uint64_t obj = _find_recursive_create(asset_obj, keys, keys_n);

    uint64_t subobj = ce_cdb_a0->read_ref(pos_cmd->cmd,
                                          inverse ? _OLD_VALUE : _NEW_VALUE, 0);

    if (subobj) {
        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(obj);
        ce_cdb_a0->set_subobject(w, prop, subobj);
        ce_cdb_a0->write_commit(w);
    } else {
        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(obj);
        ce_cdb_a0->remove_property(w, prop);
        ce_cdb_a0->write_commit(w);
    }
}

static void remove_prop_cmd(const struct ct_cmd *_cmd,
                            bool inverse) {
    const struct ct_cdb_cmd_s *pos_cmd = (const struct ct_cdb_cmd_s *) _cmd;

    struct ct_resource_id rid = {
            .name=ce_cdb_a0->read_uint64(pos_cmd->cmd, ASSET_NAME, 0),
            .type=ce_cdb_a0->read_uint64(pos_cmd->cmd, ASSET_TYPE, 0),
    };

    uint64_t asset_obj = ct_sourcedb_a0->get(rid);

    uint64_t keys_n = 0;
    uint64_t *keys = ce_cdb_a0->read_blob(pos_cmd->cmd, _KEYS, &keys_n, NULL);
    keys_n = keys_n / sizeof(uint64_t);

    uint64_t prop = ce_cdb_a0->read_uint64(pos_cmd->cmd, _PROP, 0);

    uint64_t obj = _find_recursive_create(asset_obj, keys, keys_n);

    if (!inverse) {
        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(obj);
        ce_cdb_a0->remove_property(w, prop);
        ce_cdb_a0->write_commit(w);
    } else {
        enum ce_cdb_type t = ce_cdb_a0->prop_type(pos_cmd->cmd, _OLD_VALUE);

        switch (t) {
            case CDB_TYPE_REF: {
                uint64_t subobj = ce_cdb_a0->read_ref(pos_cmd->cmd, _OLD_VALUE,
                                                      0);
                ce_cdb_obj_o *w = ce_cdb_a0->write_begin(obj);
                ce_cdb_a0->set_subobject(w, prop, subobj);
                ce_cdb_a0->write_commit(w);
            }
                break;

            default:
                break;

        }
    }
}

static void cmd_description(char *buffer,
                            uint32_t buffer_size,
                            const struct ct_cmd *cmd,
                            bool inverse) {
//    const struct ct_cdb_cmd_s *pos_cmd = (const struct ct_cdb_cmd_s *) cmd;

    switch (cmd->type) {
//        case _SET_VEC3:
////            snprintf(buffer, buffer_size,
////                     "Set vec3 [%f, %f, %f]",
////                     pos_cmd->vec3.new_value[0],
////                     pos_cmd->vec3.new_value[1],
////                     pos_cmd->vec3.new_value[2]);
//            break;
//
//        case _SET_FLOAT:
////            snprintf(buffer, buffer_size,
////                     "Set float %f",
////                     pos_cmd->f.new_value);
//            break;
//
//        case _SET_STR:
////            snprintf(buffer, buffer_size,
////                     "Set str %s",
////                     pos_cmd->str.new_value);
//            break;

        default:
            snprintf(buffer, buffer_size, "(no description)");
            break;
    }
}


static void on_post_update(uint64_t type,
                           void *event) {

//    struct ebus_event_header *it = ce_ebus_a0->events(CDB_EBUS);
//    struct ebus_event_header *end_it = ce_ebus_a0->events_end(CDB_EBUS);
//
//    while (it != end_it) {
//        if (it->type == CDB_PROP_CHANGED_EVENT) {
//            struct ce_cdb_prop_ev0 *ev = CE_EBUS_BODY(it);
//            if (ce_hash_contain(&_G.objects, ev->obj)) {
//                _on_obj_change(ev->obj, ev->prop, ev->prop_count, NULL);
//            }
//        } else if (it->type == CDB_PROP_REMOVED_EVENT) {
//            struct ce_cdb_prop_ev0 *ev = CE_EBUS_BODY(it);
//            if (ce_hash_contain(&_G.objects, ev->obj)) {
//                _on_obj_removed(ev->obj, ev->prop, ev->prop_count, NULL);
//            }
//        }
//
//        it = CE_EBUS_NEXT(it);
//    }
}

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .modified = ce_cdb_a0->create_object(ce_cdb_a0->db(), 0),
    };

    ce_ebus_a0->create_ebus(SOURCEDB_EBUS);
    api->register_api("ct_sourcedb_a0", ct_sourcedb_a0);

    ce_ebus_a0->connect(KERNEL_EBUS, KERNEL_POST_UPDATE_EVENT,
                        on_post_update, 0);

    ct_cmd_system_a0->register_cmd_execute(_SET_PROP,
                                           (struct ct_cmd_fce) {
                                                   .execute = set_cmd,
                                                   .description = cmd_description});

    ct_cmd_system_a0->register_cmd_execute(_ADD_SUBOBJ,
                                           (struct ct_cmd_fce) {
                                                   .execute = add_suobj_cmd,
                                                   .description = cmd_description});

    ct_cmd_system_a0->register_cmd_execute(_REMOVE_PROP,
                                           (struct ct_cmd_fce) {
                                                   .execute = remove_prop_cmd,
                                                   .description = cmd_description});
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