//==============================================================================
// Includes
//==============================================================================
#include <inttypes.h>

#include <celib/array.inl>
#include <celib/hash.inl>

#include <celib/api_system.h>
#include <celib/memory.h>
#include <celib/fs.h>
#include <celib/config.h>
#include <celib/os.h>
#include <celib/log.h>
#include <celib/module.h>
#include <celib/cdb.h>
#include <celib/buffer.inl>
#include <celib/hashlib.h>

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

struct _G {
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

static struct ct_resource_i0 *get_resource_interface(uint64_t type) {
    return (struct ct_resource_i0 *) ce_hash_lookup(&_G.type_map, type, 0);
}


static void load(const uint64_t *names,
                 size_t count,
                 int force) {
    uint32_t start_ticks = ce_os_a0->time->ticks();


    for (uint32_t i = 0; i < count; ++i) {
        const uint64_t asset_name = names[i];

        struct ct_resource_id rid = {.uid = asset_name};

        if (!ct_resourcedb_a0->obj_exist(rid)) {
//            abort();
            ce_log_a0->error(LOG_WHERE,
                             "Obj 0x%llx does not exist in DB", rid.uid);
            continue;
        };

        uint64_t type = ct_resourcedb_a0->get_resource_type(
                (struct ct_resource_id) {.uid=asset_name});

        if (!ct_resourcedb_a0->load_cdb_file(rid, asset_name, type,
                                          _G.allocator)) {
            ce_log_a0->warning(LOG_WHERE,
                               "Could not load resource 0x%llx", rid.uid);
            continue;
        }

        struct ct_resource_i0 *resource_i = get_resource_interface(type);

        if (!resource_i) {
            continue;
        }

        if (resource_i->online) {
            resource_i->online(names[i], asset_name);
        }
    }

    uint32_t now_ticks = ce_os_a0->time->ticks();
    uint32_t dt = now_ticks - start_ticks;
    ce_log_a0->debug(LOG_WHERE,
                     "load time %f for %zu resource", dt * 0.001, count);
}

void unload(const uint64_t *names,
            size_t count) {

    for (uint32_t i = 0; i < count; ++i) {
        if (1) {// TODO: ref counting
            struct ct_resource_id rid = (struct ct_resource_id) {
                    .uid = names[i],
            };

            uint64_t type = ct_resourcedb_a0->get_resource_type(rid);

            struct ct_resource_i0 *resource_i = get_resource_interface(type);
            if (!resource_i) {
                continue;
            }

            ce_log_a0->debug(LOG_WHERE, "Unload resource 0x%llx", rid.uid);


            if (resource_i->offline) {
                resource_i->offline(names[i], rid.uid);
            }
        }
    }
}

static bool cdb_loader(uint64_t uid) {
    load(&uid, 1, 0);
    return true;
}

static bool dump_recursive(const char* filename, uint64_t obj) {
    const ce_cdb_obj_o *r = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    const uint64_t k_n = ce_cdb_a0->prop_count(r);
    const uint64_t *ks = ce_cdb_a0->prop_keys(r);

    char *output = NULL;
    ce_cdb_a0->dump(ce_cdb_a0->db(),
                    obj, &output, _G.allocator);

    struct ct_resource_id rid={.uid=obj};
    ct_resourcedb_a0->put_resource_blob(rid,
                                     output,
                                     ce_array_size(output));
    ce_buffer_free(output, _G.allocator);

    for (int i = 0; i < k_n; ++i) {
        uint64_t k = ks[i];
        if(ce_cdb_a0->prop_type(r, k) != CDB_TYPE_SUBOBJECT) {
            continue;
        }
        uint64_t subobj = ce_cdb_a0->read_subobject(r, k, 0);
        dump_recursive(filename, subobj);
    }

    return true;
}

static bool save_to_db(uint64_t uid) {
    uint64_t root = ce_cdb_a0->find_root(ce_cdb_a0->db(), uid);

    struct ct_resource_id r = {.uid=root};
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

    struct ct_resource_id r = {.uid=root};
    char filename[256] = {};
    bool exist = ct_resourcedb_a0->get_resource_filename(r,
                                                      filename,
                                                      CE_ARRAY_LEN(filename));

    if (exist) {
        char *buf = NULL;
        ce_cdb_a0->dump_str(ce_cdb_a0->db(), &buf, uid, 0);

        struct ce_vio *f = ce_fs_a0->open(SOURCE_ROOT, filename, FS_OPEN_WRITE);
        f->write(f, buf, ce_buffer_size(buf), 1);
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
    api->register_api(CT_RESOURCE_API, &resource_api);

}

static void _init_cvar(struct ce_config_a0 *config) {
    _G = (struct _G) {};

    ce_config_a0 = config;
    _G.config = ce_config_a0->obj();

    ce_cdb_obj_o *writer = ce_cdb_a0->write_begin(ce_cdb_a0->db(), _G.config);
    if (!ce_cdb_a0->prop_exist(writer, CONFIG_BUILD)) {
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


    const ce_cdb_obj_o *reader = ce_cdb_a0->read(ce_cdb_a0->db(), _G.config);

    ce_fs_a0->map_root_dir(BUILD_ROOT,
                           ce_cdb_a0->read_str(reader, CONFIG_BUILD, ""),
                           false);

    ce_api_a0->register_on_add(RESOURCE_I, _resource_api_add);
}

static void _shutdown() {
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
