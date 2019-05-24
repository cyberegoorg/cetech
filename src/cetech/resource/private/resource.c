//==============================================================================
// Includes
//==============================================================================
#include <inttypes.h>

#include <celib/macros.h>
#include <celib/memory/allocator.h>
#include <celib/containers/array.h>
#include <celib/containers/hash.h>
#include <celib/api.h>
#include <celib/memory/memory.h>
#include <celib/fs.h>
#include <celib/config.h>
#include <celib/log.h>
#include <celib/module.h>
#include <celib/cdb.h>
#include <celib/containers/buffer.h>
#include <celib/id.h>
#include <celib/os/time.h>
#include <celib/os/vio.h>

#include <cetech/kernel/kernel.h>
#include <cetech/resource/resourcedb.h>
#include <cetech/resource/resource_compiler.h>
#include <stdlib.h>


#include "../resource.h"

//==============================================================================
// Gloals
//==============================================================================

#define _G ResourceManagerGlobals
#define LOG_WHERE "resource"

//==============================================================================
// Gloals
//==============================================================================

CE_MODULE(ct_resourcedb_a0);

struct _G {
    ce_hash_t type_map;

    ce_cdb_t0 db;

    ce_alloc_t0 *allocator;
} _G = {};

//==============================================================================
// Private
//==============================================================================


//==============================================================================
// Public interface
//==============================================================================

static void _resource_api_add(uint64_t name,
                              void *api) {
    ct_resource_i0 *ct_resource_i = api;
    ce_hash_add(&_G.type_map, ct_resource_i->cdb_type(), (uint64_t) api,
                _G.allocator);
}

static struct ct_resource_i0 *get_resource_interface(uint64_t type) {
    return (ct_resource_i0 *) ce_hash_lookup(&_G.type_map, type, 0);
}


static bool load(ce_cdb_t0 db,
                 const uint64_t *names,
                 size_t count,
                 int force) {
    uint32_t start_ticks = ce_os_time_a0->ticks();


    for (uint32_t i = 0; i < count; ++i) {
        const uint64_t asset_name = names[i];

        ct_resource_id_t0 rid = {.uid = asset_name};

        if (!ct_resourcedb_a0->obj_exist(rid)) {
            ce_log_a0->error(LOG_WHERE, "Obj 0x%llx does not exist in DB", rid.uid);
            return false;
        };

        if (!ct_resourcedb_a0->load_cdb_file(db, rid, asset_name, _G.allocator)) {
            ce_log_a0->warning(LOG_WHERE, "Could not load resource 0x%llx", rid.uid);
            continue;
        }

        uint64_t type = ce_cdb_a0->obj_type(db, asset_name);

        struct ct_resource_i0 *resource_i = get_resource_interface(type);

        if (!resource_i) {
            continue;
        }

        if (resource_i->online) {
            resource_i->online(db, names[i], asset_name);
        }
    }

    uint32_t now_ticks = ce_os_time_a0->ticks();
    uint32_t dt = now_ticks - start_ticks;
    ce_log_a0->debug(LOG_WHERE, "load time %f for %zu resource", dt * 0.001, count);

    return true;
}

void unload(ce_cdb_t0 db,
            const uint64_t *names,
            size_t count) {

    for (uint32_t i = 0; i < count; ++i) {
        if (1) {// TODO: ref counting
            struct ct_resource_id_t0 rid = (ct_resource_id_t0) {
                    .uid = names[i],
            };

            uint64_t type = ct_resourcedb_a0->get_resource_type(rid);

            struct ct_resource_i0 *resource_i = get_resource_interface(type);
            if (!resource_i) {
                continue;
            }

            ce_log_a0->debug(LOG_WHERE, "Unload resource 0x%llx", rid.uid);


            if (resource_i->offline) {
                resource_i->offline(db, names[i], rid.uid);
            }
        }
    }
}

static bool cdb_loader(ce_cdb_t0 db,
                       uint64_t uid) {
    return load(db, &uid, 1, 0);
}

static bool dump_recursive(const char *filename,
                           uint64_t obj) {
    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    const uint64_t k_n = ce_cdb_a0->prop_count(r);
    const uint64_t *ks = ce_cdb_a0->prop_keys(r);

    char *output = NULL;
    ce_cdb_a0->dump(ce_cdb_a0->db(),
                    obj, &output, _G.allocator);

    ct_resource_id_t0 rid = {.uid=obj};
    ct_resourcedb_a0->put_resource_blob(rid,
                                        output,
                                        ce_array_size(output));
    ce_buffer_free(output, _G.allocator);

    for (int i = 0; i < k_n; ++i) {
        uint64_t k = ks[i];
        if (ce_cdb_a0->prop_type(r, k) != CE_CDB_TYPE_SUBOBJECT) {
            continue;
        }
        uint64_t subobj = ce_cdb_a0->read_subobject(r, k, 0);
        dump_recursive(filename, subobj);
    }

    return true;
}

static bool save_to_db(uint64_t uid) {
    uint64_t root = ce_cdb_a0->find_root(ce_cdb_a0->db(), uid);

    ct_resource_id_t0 r = {.uid=root};
    char filename[256] = {};
    bool exist = ct_resourcedb_a0->get_resource_filename(r,
                                                         filename,
                                                         CE_ARRAY_LEN(filename));

    if (exist) {
        dump_recursive(filename, root);
        return true;
    }

    return false;
}

static bool save(uint64_t uid) {
    uint64_t root = ce_cdb_a0->find_root(ce_cdb_a0->db(), uid);

    ct_resource_id_t0 r = {.uid=root};
    char filename[256] = {};
    bool exist = ct_resourcedb_a0->get_resource_filename(r,
                                                         filename,
                                                         CE_ARRAY_LEN(filename));

    if (exist) {
        char *buf = NULL;
        ce_cdb_a0->dump_str(ce_cdb_a0->db(), &buf, uid, 0);

        struct ce_vio_t0 *f = ce_fs_a0->open(SOURCE_ROOT, filename, FS_OPEN_WRITE);
        f->vt->write(f->inst, buf, ce_buffer_size(buf), 1);
        ce_fs_a0->close(f);
        ce_buffer_free(buf, _G.allocator);

        dump_recursive(filename, root);

        return true;
    }

    return false;
}

static struct ct_resource_a0 resource_api = {
        .get_interface = get_resource_interface,
        .cdb_loader = cdb_loader,
        .save = save,
        .save_to_db = save_to_db,
};


struct ct_resource_a0 *ct_resource_a0 = &resource_api;

static void _init_api(struct ce_api_a0 *api) {
    api->register_api(CT_RESOURCE_API, &resource_api, sizeof(resource_api));

}

static void _init_cvar(struct ce_config_a0 *config) {
    _G = (struct _G) {};

    ce_config_a0 = config;

//    ce_cdb_obj_o0 *writer = ce_cdb_a0->write_begin(ce_cdb_a0->db(), _G.config);
//    if (!ce_cdb_a0->prop_exist(writer, CONFIG_BUILD)) {
//        ce_cdb_a0->set_str(writer, CONFIG_BUILD, "build");
//    }
//    ce_cdb_a0->write_commit(writer);
}

void CE_MODULE_LOAD(resourcesystem)(struct ce_api_a0 *api,
                                    int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_config_a0);
    CE_INIT_API(api, ce_log_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_cdb_a0);

    _init_api(api);
    _init_cvar(ce_config_a0);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .db = ce_cdb_a0->db()
    };

    ce_fs_a0->map_root_dir(BUILD_ROOT,
                           ce_config_a0->read_str(CONFIG_BUILD, ""),
                           false);

    ce_api_a0->register_on_add(CT_RESOURCE_I, _resource_api_add);
}

void CE_MODULE_UNLOAD(resourcesystem)(struct ce_api_a0 *api,
                                      int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    ce_hash_free(&_G.type_map, _G.allocator);
}
