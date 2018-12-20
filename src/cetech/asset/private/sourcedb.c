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
#include <stdlib.h>

#define LOG_WHERE "sourcedb"

#define _G sourcedb_globals

static struct _G {
    struct ce_hash_t cache_map;
    struct ce_spinlock cache_lock;

    uint64_t modified;

    struct ce_alloc *allocator;
} _G;

static uint64_t get(struct ct_resource_id resource_id);

static void _expand(uint64_t to,
                    uint64_t from) {

    const ce_cdb_obj_o *to_reader = ce_cdb_a0->read(to);

    uint64_t prefab = ce_cdb_a0->read_ref(to_reader, PREFAB_NAME_PROP, 0);

    if (prefab) {
        uint64_t uid = prefab;
        struct ct_resource_id prefab_rid = {.uid=uid};
        from = get(prefab_rid);
    }

    const ce_cdb_obj_o *from_reader = ce_cdb_a0->read(from);

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
        const ce_cdb_obj_o *to_reader = ce_cdb_a0->read(to);
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
    uint64_t resource_key = resource_id.uid;

    ce_os_a0->thread->spin_lock(&_G.cache_lock);
    uint64_t resource_obj = ce_hash_lookup(&_G.cache_map, resource_key, 0);
    ce_os_a0->thread->spin_unlock(&_G.cache_lock);

    if (!resource_obj) {
        ce_log_a0->debug(LOG_WHERE, "Load resource 0x%llx to cache.",
                         resource_key);

        char filename[256] = {0};
        ct_builddb_a0->get_resource_filename(resource_id, CE_ARR_ARG(filename));
        uint64_t type = ct_builddb_a0->get_resource_type(resource_id);

        uint64_t obj = ce_ydb_a0->get_obj(filename);
        if (!obj) {
            return 0;
        }


        char tmp[128] = {};
        snprintf(tmp, sizeof(tmp), "0x%llx", resource_key);

        const ce_cdb_obj_o *reader = ce_cdb_a0->read(obj);
        resource_obj = ce_cdb_a0->read_subobject(reader,
                                                 ce_id_a0->id64(tmp), 0);
        if (!resource_obj) {
            return 0;
        }

        ce_cdb_a0->set_type(resource_obj, type);

//        char *buf = NULL;
//        ce_cdb_a0->dump_str(&buf, resource_obj, 0);
//        ce_log_a0->debug(LOG_WHERE, "%s", buf);

        _expand(resource_obj, 0);

        const struct ct_resource_i0 *ri;
        ri = ct_resource_a0->get_interface(type);
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


static struct ct_sourcedb_a0 source_db_api = {
        .get = get,
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