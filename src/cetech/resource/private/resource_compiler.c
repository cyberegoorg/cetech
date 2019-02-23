//==============================================================================
// Includes
//==============================================================================

#include <celib/memory/allocator.h>
#include <celib/api.h>
#include <celib/memory/memory.h>
#include <celib/task.h>
#include <celib/config.h>

#include <celib/log.h>
#include <celib/id.h>
#include <celib/module.h>
#include <celib/fs.h>
#include <celib/ydb.h>
#include <celib/containers/array.h>
#include <celib/containers/hash.h>
#include <celib/containers/buffer.h>
#include <celib/cdb.h>
#include <celib/cdb.h>
#include <celib/containers/bagraph.h>

#include <cetech/resource/resource.h>
#include <cetech/kernel/kernel.h>
#include <cetech/resource/resource_compiler.h>
#include <stdlib.h>
#include <celib/os/path.h>
#include <celib/os/time.h>

#include "cetech/resource/resourcedb.h"


//==============================================================================
// Defines
//==============================================================================

#define _G ResourceCompilerGlobal

#define LOG_WHERE "resource_compiler"

//==============================================================================
// Globals
//==============================================================================

static struct _G {
    uint64_t config;
    ce_alloc_t0 *allocator;
} _G;


//==============================================================================
// Private
//==============================================================================

static ct_resource_compilator_t _find_compilator(uint64_t type) {
    ce_api_entry_t0 it = ce_api_a0->first(RESOURCE_I);
    while (it.api) {
        struct ct_resource_i0 *i = (it.api);
        if (i->cdb_type && (i->cdb_type() == type)) {
            return i->compilator;
        }
        it = ce_api_a0->next(it);
    }

    return NULL;
}

static uint64_t _uid_from_str(const char *str) {
    uint64_t v = strtoul(str, NULL, 0);
    return v;
}

static bool _is_ref(const char *str) {
    return (str[0] == '0') && (str[1] == 'x');
}

uint64_t compile_obj(ce_cdb_t0 db,
                     uint64_t input_obj,
                     uint64_t uid) {

    const ce_cdb_obj_o0 *input_r = ce_cdb_a0->read(ce_cdb_a0->db(), input_obj);

    const char *type_s = ce_cdb_a0->read_str(input_r, CDB_TYPE_PROP, NULL);

    uint64_t type = ce_id_a0->id64(type_s);

    uint64_t obj = 0;
    ce_cdb_a0->create_object_uid(db, uid, 0);
    obj = uid;

    uint64_t n = ce_cdb_a0->prop_count(input_r);
    const uint64_t *k = ce_cdb_a0->prop_keys(input_r);

    ce_cdb_obj_o0 *obj_w = ce_cdb_a0->write_begin(db, obj);

    for (int i = 0; i < n; ++i) {
        uint64_t p = k[i];

        if (CDB_UID_PROP == p) {
            continue;
        }

        if (CDB_TYPE_PROP == p) {
            continue;
        }

        enum ce_cdb_type_e0 t = ce_cdb_a0->prop_type(input_r, p);

        switch (t) {
            case CDB_TYPE_UINT64: {
                uint64_t v = ce_cdb_a0->read_uint64(input_r, p, 0);
                ce_cdb_a0->set_uint64(obj_w, p, v);
            }
                break;

            case CDB_TYPE_FLOAT: {
                float v = ce_cdb_a0->read_float(input_r, p, 0);
                ce_cdb_a0->set_float(obj_w, p, v);
            }
                break;

            case CDB_TYPE_BOOL: {
                bool v = ce_cdb_a0->read_bool(input_r, p, 0);
                ce_cdb_a0->set_bool(obj_w, p, v);
            }
                break;

            case CDB_TYPE_STR: {
                const char *s = ce_cdb_a0->read_str(input_r, p, "");

                if (_is_ref(s)) {
                    uint64_t ref_uid = _uid_from_str(s);
                    ce_cdb_a0->set_ref(obj_w, p, ref_uid);
                } else {
                    ce_cdb_a0->set_str(obj_w, p, s);
                }

            }
                break;

            case CDB_TYPE_SUBOBJECT: {
                uint64_t subobj = ce_cdb_a0->read_subobject(input_r, p, 0);
                const ce_cdb_obj_o0 *sr = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                          subobj);
                const char *suid_s = ce_cdb_a0->read_str(sr, CDB_UID_PROP,

                                                         NULL);
                uint64_t ref_uid = 0;
                if (suid_s) {
                    ref_uid = _uid_from_str(suid_s);
                }

                ce_cdb_a0->set_subobject(obj_w, p, ref_uid);
            }
                break;

            case CDB_TYPE_PTR:
                break;
            case CDB_TYPE_NONE:
                break;
            case CDB_TYPE_BLOB:
                break;
            default:
                break;
        }
    }

    ce_cdb_a0->write_commit(obj_w);

    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);
    uint64_t cdb_instance = ce_cdb_a0->read_ref(r, CDB_INSTANCE_PROP, 0);
    if (cdb_instance) {
        ce_cdb_a0->set_from(db, cdb_instance, obj);
    }

    ct_resource_compilator_t compilator = _find_compilator(type);
    if (compilator) {
        compilator(db, obj);
    }

    ce_cdb_a0->set_type(db, obj, type);

    return obj;
}

void _scan_obj(const char *filename,
               uint64_t key,
               uint64_t obj,
               struct ce_ba_graph_t *obj_graph,
               struct ce_hash_t *obj_hash) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    const char *cdb_instance = ce_cdb_a0->read_str(reader, CDB_INSTANCE_PROP,
                                                   NULL);
    const char *type = ce_cdb_a0->read_str(reader, CDB_TYPE_PROP, "");
    const char *name = ce_cdb_a0->read_str(reader, ASSET_NAME_PROP, "");
    const char *uid_s = ce_cdb_a0->read_str(reader, CDB_UID_PROP, NULL);

    const uint64_t n = ce_cdb_a0->prop_count(reader);
    const uint64_t *keys = ce_cdb_a0->prop_keys(reader);

    if (!uid_s) {
        ce_log_a0->warning(LOG_WHERE, "In %s exist element without cdb_uid",
                           filename);
        return;
    }

    uint64_t uid = strtoul(uid_s, NULL, 0);

    ct_resource_id_t0 rid = {.uid = uid};
    ct_resourcedb_a0->put_resource(rid, type, filename, name);

    ce_hash_add(obj_hash, uid, obj, _G.allocator);

    uint64_t *after = NULL;

    if (cdb_instance) {
        uint64_t cdb_instance_uid = strtoul(cdb_instance, NULL, 0);
        ce_array_push(after, cdb_instance_uid, _G.allocator);
    }

    for (uint32_t i = 0; i < n; ++i) {
        uint64_t k = keys[i];

        if (CDB_UID_PROP == k) {
            continue;
        }

        if (CDB_INSTANCE_PROP == k) {
            continue;
        }

        if (CDB_TYPE_PROP == k) {
            continue;
        }

        enum ce_cdb_type_e0 t = ce_cdb_a0->prop_type(reader, k);

        if (t == CDB_TYPE_SUBOBJECT) {
            uint64_t sub_obj = ce_cdb_a0->read_subobject(reader, k, 0);
            _scan_obj(filename, k, sub_obj, obj_graph, obj_hash);

            const ce_cdb_obj_o0 *subr = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                        sub_obj);

            const char *uid_s = ce_cdb_a0->read_str(subr, CDB_UID_PROP, NULL);

            if (!uid_s) {
                ce_log_a0->warning(LOG_WHERE,
                                   "In %s exist element without cdb_uid",
                                   filename);
                continue;
            }

            uint64_t ref_uid = 0;
            ref_uid = strtoul(uid_s, NULL, 0);
            ce_array_push(after, ref_uid, _G.allocator);

        } else if (t == CDB_TYPE_STR) {
            const char *str = ce_cdb_a0->read_str(reader, k, NULL);
            if (_is_ref(str)) {
                uint64_t ref_uid = _uid_from_str(str);
                ce_array_push(after, ref_uid, _G.allocator);
            }
        } else if (t == CDB_TYPE_REF) {
            uint64_t ref = ce_cdb_a0->read_ref(reader, k, 0);
            ce_array_push(after, ref, _G.allocator);
        }
    }

    ce_bag_add(obj_graph, uid,
               NULL, 0,
               after, ce_array_size(after),
               _G.allocator);
}

void _scan_files(char **files,
                 uint32_t files_count) {
    ce_ba_graph_t obj_graph = {};
    ce_hash_t obj_hash = {};

    ce_cdb_t0 db = ce_cdb_a0->create_db(1000000);

    for (uint32_t i = 0; i < files_count; ++i) {
        const char *filename = files[i];
        if (ce_id_a0->id64(filename) == ce_id_a0->id64("global.yml")) {
            continue;
        }

        int64_t mtime = ce_fs_a0->file_mtime(SOURCE_ROOT, filename);
        ct_resourcedb_a0->put_file(filename, mtime);

        uint64_t obj = ce_ydb_a0->get_obj(filename);
        _scan_obj(filename, 0, obj, &obj_graph, &obj_hash);
    }

    ce_bag_build(&obj_graph, _G.allocator);

    const uint64_t output_n = ce_array_size(obj_graph.output);
    for (int k = 0; k < output_n; ++k) {
        uint64_t obj = obj_graph.output[k];

        char filename[256] = {};
        ct_resourcedb_a0->get_resource_filename(
                (ct_resource_id_t0) {.uid=obj},
                filename, CE_ARRAY_LEN(filename));

        ce_log_a0->info(LOG_WHERE, "Compile 0x%llx from %s", obj, filename);

        uint64_t cobj = ce_hash_lookup(&obj_hash, obj, 0);
        compile_obj(db, cobj, obj);
    }

    for (int k = 0; k < output_n; ++k) {
        uint64_t obj = obj_graph.output[k];

        char *output = NULL;
        ce_cdb_a0->dump(db, obj, &output, _G.allocator);
        ct_resourcedb_a0->put_resource_blob((ct_resource_id_t0) {.uid=obj},
                                            output,
                                            ce_array_size(output));

        ce_buffer_free(output, _G.allocator);
    }

    ce_cdb_a0->destroy_db(db);
}

//==============================================================================
// Interface
//==============================================================================


char *resource_compiler_get_build_dir(ce_alloc_t0 *a,
                                      const char *platform) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), _G.config);

    const char *build_dir_str = ce_cdb_a0->read_str(reader,
                                                    CONFIG_BUILD, "");

    char *buffer = NULL;
    ce_os_path_a0->join(&buffer, a, 2, build_dir_str, platform);

    return buffer;
}

void resource_compiler_compile_all() {
    uint32_t start_ticks = ce_os_time_a0->ticks();

    const char *glob_patern = "**.yml";
    char **files = NULL;
    uint32_t files_count = 0;

    ce_fs_a0->listdir(SOURCE_ROOT,
                      "", glob_patern, false, true, &files, &files_count,
                      _G.allocator);

    _scan_files(files, files_count);

    ce_fs_a0->listdir_free(files, files_count,
                           _G.allocator);

    uint32_t now_ticks = ce_os_time_a0->ticks();
    uint32_t dt = now_ticks - start_ticks;
    ce_log_a0->debug("resource_compiler", "compile time %f", dt * 0.001);
}

char *resource_compiler_get_tmp_dir(ce_alloc_t0 *alocator,
                                    const char *platform) {

    char *build_dir = resource_compiler_get_build_dir(alocator, platform);

    char *buffer = NULL;
    ce_os_path_a0->join(&buffer, alocator, 2, build_dir, "tmp");
    return buffer;
}

char *resource_compiler_external_join(ce_alloc_t0 *alocator,
                                      const char *name) {

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), _G.config);

    const char *external_dir_str = ce_cdb_a0->read_str(reader,
                                                       CONFIG_EXTERNAL,
                                                       "");

    char *tmp_dir = NULL;
    ce_os_path_a0->join(&tmp_dir, alocator, 2, external_dir_str,
                        ce_cdb_a0->read_str(reader,
                                            CONFIG_PLATFORM,
                                            ""));

    char *buffer = NULL;
    ce_buffer_printf(&buffer, alocator, "%s64", tmp_dir);
    ce_buffer_free(tmp_dir, alocator);

    char *result = NULL;
    ce_os_path_a0->join(&result, alocator, 4, buffer, "release", "bin",
                        name);
    ce_buffer_free(buffer, alocator);

    return result;
}

static void _init_cvar(struct ce_config_a0 *config) {
    ce_cdb_obj_o0 *writer = ce_cdb_a0->write_begin(ce_cdb_a0->db(), _G.config);
    if (!ce_cdb_a0->prop_exist(writer, CONFIG_SRC)) {
        ce_cdb_a0->set_str(writer, CONFIG_SRC, "src");
    }

    if (!ce_cdb_a0->prop_exist(writer, CONFIG_CORE)) {
        ce_cdb_a0->set_str(writer, CONFIG_CORE, "core");
    }

    if (!ce_cdb_a0->prop_exist(writer, CONFIG_EXTERNAL)) {
        ce_cdb_a0->set_str(writer, CONFIG_EXTERNAL, "externals/build");
    }

    ce_cdb_a0->write_commit(writer);
}

static struct ct_resource_compiler_a0 resource_compiler_api = {
        .compile_all = resource_compiler_compile_all,
        .get_tmp_dir = resource_compiler_get_tmp_dir,
        .external_join = resource_compiler_external_join,
};


struct ct_resource_compiler_a0 *ct_resource_compiler_a0 = &resource_compiler_api;

static void _init(struct ce_api_a0 *api) {
    CE_UNUSED(api);
    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .config = ce_config_a0->obj(),
    };

    _init_cvar(ce_config_a0);
    api->register_api(CT_RESOURCE_COMPILER_API,
                      &resource_compiler_api,
                      sizeof(resource_compiler_api));

    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), _G.config);

    const char *platform = ce_cdb_a0->read_str(reader,
                                               CONFIG_PLATFORM, "");

    char *build_dir_full = resource_compiler_get_build_dir(_G.allocator,
                                                           platform);

    ce_os_path_a0->make_path(build_dir_full);


    char *tmp_dir_full = NULL;
    ce_os_path_a0->join(&tmp_dir_full, _G.allocator, 2,
                        build_dir_full, "tmp");

    ce_os_path_a0->make_path(tmp_dir_full);

    ce_buffer_free(tmp_dir_full, _G.allocator);
    ce_buffer_free(build_dir_full, _G.allocator);

    const char *core_dir = ce_cdb_a0->read_str(reader, CONFIG_CORE, "");
    const char *source_dir = ce_cdb_a0->read_str(reader, CONFIG_SRC, "");

    ce_fs_a0->map_root_dir(SOURCE_ROOT, core_dir, true);
    ce_fs_a0->map_root_dir(SOURCE_ROOT, source_dir, true);

}

static void _shutdown() {
    _G = (struct _G) {};
}


void CE_MODULE_LOAD(resourcecompiler)(struct ce_api_a0 *api,
                                      int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_resource_a0);
    CE_INIT_API(api, ce_task_a0);
    CE_INIT_API(api, ce_log_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_config_a0);
    CE_INIT_API(api, ce_fs_a0);
    CE_INIT_API(api, ce_ydb_a0);
    CE_INIT_API(api, ce_cdb_a0);
    _init(api);
}

void CE_MODULE_UNLOAD(resourcecompiler)(struct ce_api_a0 *api,
                                        int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    _shutdown();

}
