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
#include <celib/containers/bitset.h>

#include "cetech/resource/resourcedb.h"


//==============================================================================
// Defines
//==============================================================================

#define _G ResourceCompilerGlobal

#define LOG_WHERE "resource_compiler"

//==============================================================================
// Globals
//==============================================================================

CE_MODULE(ct_resourcedb_a0);

static struct _G {
    ce_alloc_t0 *allocator;
} _G;


//==============================================================================
// Private
//==============================================================================

static ct_resource_compilator_t _find_compilator(uint64_t type) {
    ce_api_entry_t0 it = ce_api_a0->first(CT_RESOURCE_I);
    while (it.api) {
        struct ct_resource_i0 *i = (it.api);
        if (i->cdb_type && (i->cdb_type() == type)) {
            return i->compilator;
        }
        it = ce_api_a0->next(it);
    }

    return NULL;
}

void _fill_obj_root_hash(const char *filename,
                         cnode_t *cnodes,
                         ce_hash_t *obj_root_hash,
                         uint64_t **all_obj) {
    uint64_t root_uid = cnodes[0].obj.uid;
    uint32_t cnodes_n = ce_array_size(cnodes);
    for (int j = 0; j < cnodes_n; ++j) {
        cnode_t node = cnodes[j];
        if (node.type == CNODE_OBJ_BEGIN) {
            ce_hash_add(obj_root_hash, node.obj.uid, root_uid, _G.allocator);

            ct_resource_id_t0 rid = {.uid = node.obj.uid};
            ct_resourcedb_a0->put_resource(rid, ce_id_a0->str_from_id64(node.obj.type),
                                           filename,
                                           j == 0);

            ce_array_push(*all_obj, rid.uid, _G.allocator);
        }
    }
}

void _fill_deps(cnode_t *cnodes,
                struct ce_hash_t *obj_root_hash,
                struct ce_ba_graph_t *obj_graph) {
    uint64_t *tmp_after = NULL;

    ce_bitset_t0 after_set = {};
    ce_bitset_init(&after_set, 1024, _G.allocator);

    uint32_t cnodes_n = ce_array_size(cnodes);
    for (int j = 0; j < cnodes_n; ++j) {
        cnode_t node = cnodes[j];

        if (node.type == CNODE_OBJ_BEGIN) {
            if (node.obj.instance_of) {
                uint64_t root_uid = ce_hash_lookup(obj_root_hash, node.obj.instance_of, 0);

                if (!ce_bitset_is_set(&after_set, root_uid)) {
                    ce_bitset_add(&after_set, root_uid);
                    ce_array_push(tmp_after, root_uid, _G.allocator);
                }
            }
        } else if (node.type == CNODE_REF) {
            uint64_t root_uid = ce_hash_lookup(obj_root_hash, node.value.ref, 0);

            if (!ce_bitset_is_set(&after_set, root_uid)) {
                ce_bitset_add(&after_set, root_uid);
                ce_array_push(tmp_after, root_uid, _G.allocator);
            }
        }
    }

    ce_bitset_free(&after_set, _G.allocator);

    ce_bag_add(obj_graph, cnodes[0].obj.uid,
               NULL, 0,
               tmp_after, ce_array_size(tmp_after),
               _G.allocator);
}

void _save(const char *filename,
           ce_cdb_t0 db,
           uint64_t obj,
           bool file_resource) {

    char *output = NULL;
    ce_cdb_a0->dump(db, obj, &output, _G.allocator);
    ct_resourcedb_a0->put_resource_blob((ct_resource_id_t0) {.uid=obj},
                                        output,
                                        ce_array_size(output));
    ce_buffer_free(output, _G.allocator);

    uint64_t type = ce_cdb_a0->obj_type(db, obj);
    const char *type_str = ce_id_a0->str_from_id64(type);

    ct_resourcedb_a0->put_resource((ct_resource_id_t0) {.uid=obj},
                                   type_str,
                                   filename,
                                   file_resource);

}

void _compile_files(char **files,
                    uint32_t files_count) {
    ce_ba_graph_t obj_graph = {};
    ce_hash_t obj_root_hash = {};
    ce_hash_t root_cnodes_hash = {};
    ce_hash_t obj_files = {};
    uint64_t *root_objs = NULL;
    uint64_t *all_objs = NULL;

    ce_cdb_t0 db = ce_cdb_a0->create_db(1000000);

    for (uint32_t i = 0; i < files_count; ++i) {
        const char *filename = files[i];
        int64_t mtime = ce_fs_a0->file_mtime(SOURCE_ROOT, filename);
        ct_resourcedb_a0->put_file(filename, mtime);

        cnode_t *cnodes = NULL;
        ce_ydb_a0->read_cnodes(filename, &cnodes);

        _fill_obj_root_hash(filename, cnodes, &obj_root_hash, &all_objs);

        ce_array_push(root_objs, cnodes[0].obj.uid, _G.allocator);
        ce_hash_add(&root_cnodes_hash, cnodes[0].obj.uid, (uint64_t) cnodes, _G.allocator);
        ce_hash_add(&obj_files, cnodes[0].obj.uid, (uint64_t) filename, _G.allocator);
    }

    uint32_t root_objs_n = ce_array_size(root_objs);
    for (int j = 0; j < root_objs_n; ++j) {
        cnode_t *cnodes = (cnode_t *) ce_hash_lookup(&root_cnodes_hash, root_objs[j], 0);
        _fill_deps(cnodes, &obj_root_hash, &obj_graph);
    }

    ce_bag_build(&obj_graph, _G.allocator);

    const uint64_t output_n = ce_array_size(obj_graph.output);
    for (int k = 0; k < output_n; ++k) {
        uint64_t obj = obj_graph.output[k];

        cnode_t *cnodes = (cnode_t *) ce_hash_lookup(&root_cnodes_hash, obj, 0);

        ce_log_a0->debug(LOG_WHERE, "COMPILE 0x%llx", cnodes[0].obj.uid);

        char *outputs = NULL;
        ce_ydb_a0->dump_cnodes(db, cnodes, &outputs);
        ct_resourcedb_a0->put_resource_blob((ct_resource_id_t0) {.uid=obj},
                                            outputs, ce_array_size(outputs));
        ce_array_free(outputs, _G.allocator);
    }

    for (int k = 0; k < output_n; ++k) {
        uint64_t obj = obj_graph.output[k];
        cnode_t *cnodes = (cnode_t *) ce_hash_lookup(&root_cnodes_hash, obj, 0);
        ct_resource_compilator_t compilator = _find_compilator(cnodes[0].obj.type);
        if (compilator) {
            compilator(db, obj);
        }

        const char *filename = (const char *) ce_hash_lookup(&obj_files, obj, 0);
        _save(filename, db, obj, true);
    }

    ce_cdb_a0->destroy_db(db);
}

//==============================================================================
// Interface
//==============================================================================


char *resource_compiler_get_build_dir(ce_alloc_t0 *a,
                                      const char *platform) {
    const char *build_dir_str = ce_config_a0->read_str(CONFIG_BUILD, "");

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

    _compile_files(files, files_count);

    ce_fs_a0->listdir_free(files, files_count,
                           _G.allocator);

    uint32_t now_ticks = ce_os_time_a0->ticks();
    uint32_t dt = now_ticks - start_ticks;
    ce_log_a0->debug("resource_compiler", "compile time %f", dt * 0.001);
}

void resource_compiler_get_tmp_dir(char *output,
                                   uint64_t max_size,
                                   const char *platform,
                                   const char *filename,
                                   const char *ext) {

    char *build_dir = resource_compiler_get_build_dir(_G.allocator, platform);

    char *buffer = NULL;
    ce_os_path_a0->join(&buffer, _G.allocator, 2, build_dir, "tmp");

    char dir[1024] = {};
    ce_os_path_a0->dir(dir, filename);

    char *tmp_dirname = NULL;
    ce_os_path_a0->join(&tmp_dirname, _G.allocator, 2, buffer, dir);
    ce_os_path_a0->make_path(tmp_dirname);

    if (!ext) {
        snprintf(output, max_size, "%s/%s", tmp_dirname,
                 ce_os_path_a0->filename(filename));
    } else {
        snprintf(output, max_size, "%s/%s.%s", tmp_dirname,
                 ce_os_path_a0->filename(filename), ext);
    }

    ce_buffer_free(tmp_dirname, _G.allocator);
    ce_buffer_free(buffer, _G.allocator);
}

char *resource_compiler_external_join(ce_alloc_t0 *alocator,
                                      const char *name) {
    const char *external_dir_str = ce_config_a0->read_str(CONFIG_EXTERNAL, "externals/build");

    char *tmp_dir = NULL;
    ce_os_path_a0->join(&tmp_dir, alocator, 2, external_dir_str,
                        ce_config_a0->read_str(CONFIG_PLATFORM, ""));

    char *buffer = NULL;
    ce_buffer_printf(&buffer, alocator, "%s64", tmp_dir);
    ce_buffer_free(tmp_dir, alocator);

    char *result = NULL;
    ce_os_path_a0->join(&result, alocator, 4, buffer, "release", "bin", name);
    ce_buffer_free(buffer, alocator);

    return result;
}

static void _init_cvar(struct ce_config_a0 *config) {
    if (!ce_config_a0->exist(CONFIG_SRC)) {
        ce_config_a0->set_str(CONFIG_SRC, "src");
    }

    if (!ce_config_a0->exist(CONFIG_CORE)) {
        ce_config_a0->set_str(CONFIG_CORE, "core");
    }

    if (!ce_config_a0->exist(CONFIG_EXTERNAL)) {
        ce_config_a0->set_str(CONFIG_EXTERNAL, "externals/build");
    }
}

static struct ct_resource_compiler_a0 resource_compiler_api = {
        .compile_all = resource_compiler_compile_all,
        .gen_tmp_file = resource_compiler_get_tmp_dir,
        .external_join = resource_compiler_external_join,
};


struct ct_resource_compiler_a0 *ct_resource_compiler_a0 = &resource_compiler_api;


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

    CE_UNUSED(api);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    _init_cvar(ce_config_a0);
    api->register_api(CT_RESOURCE_COMPILER_API,
                      &resource_compiler_api,
                      sizeof(resource_compiler_api));

    const char *platform = ce_config_a0->read_str(CONFIG_PLATFORM, "");

    char *build_dir_full = resource_compiler_get_build_dir(_G.allocator, platform);

    ce_os_path_a0->make_path(build_dir_full);


    char *tmp_dir_full = NULL;
    ce_os_path_a0->join(&tmp_dir_full, _G.allocator, 2,
                        build_dir_full, "tmp");

    ce_os_path_a0->make_path(tmp_dir_full);

    ce_buffer_free(tmp_dir_full, _G.allocator);
    ce_buffer_free(build_dir_full, _G.allocator);

    const char *core_dir = ce_config_a0->read_str(CONFIG_CORE, "");
    const char *source_dir = ce_config_a0->read_str(CONFIG_SRC, "");

    ce_fs_a0->map_root_dir(SOURCE_ROOT, core_dir, true);
    ce_fs_a0->map_root_dir(SOURCE_ROOT, source_dir, true);
}

void CE_MODULE_UNLOAD(resourcecompiler)(struct ce_api_a0 *api,
                                        int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

}
