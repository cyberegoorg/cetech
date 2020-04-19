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
#include <celib/cdb_yaml.h>
#include <celib/containers/array.h>
#include <celib/containers/hash.h>
#include <celib/containers/buffer.h>
#include <celib/cdb.h>
#include <celib/containers/bagraph.h>

#include <cetech/asset/asset.h>
#include <cetech/kernel/kernel.h>
#include <cetech/asset_io/asset_io.h>
#include <celib/os/path.h>
#include <celib/os/time.h>
#include <celib/containers/bitset.h>
#include <celib/os/vio.h>
#include <stdlib.h>
#include <celib/os/thread.h>


//==============================================================================
// Defines
//==============================================================================

#define _G AssetCompilerGlobal

#define LOG_WHERE "asset_compiler"

//==============================================================================
// Globals
//==============================================================================

CE_MODULE(ct_assetdb_a0);

static struct _G {
    ce_alloc_t0 *allocator;

    ce_hash_t import_group_map;
    uint64_t *import_group_name;
    float *import_group_order;
    uint64_t *import_group_ordered;
} _G;


//==============================================================================
// Private
//==============================================================================

ct_asset_io_i0 *_find_asset_io(const char *extension) {
    ce_api_entry_t0 it = ce_api_a0->first(CT_ASSET_IO_I0);
    while (it.api) {
        ct_asset_io_i0 *i = (it.api);

        if (i->supported_extension(extension)) {
            return i;
        }

        it = ce_api_a0->next(it);
    }

    return NULL;
}

ct_asset_dcc_io_i0 *_find_dcc_asset_io(const char *extension,
                                       uint64_t group) {
    ce_api_entry_t0 it = ce_api_a0->first(CT_DCC_ASSET_IO_I0);
    while (it.api) {
        ct_asset_dcc_io_i0 *i = (it.api);

        if (group) {
            if (i->import_group != group) {
                it = ce_api_a0->next(it);
                continue;
            }
        }

        if (i->supported_extension(extension)) {
            return i;
        }

        it = ce_api_a0->next(it);
    }

    return NULL;
}


////
static const ce_cdb_prop_def_t0 dcc_asset_prop[] = {
        {.name = "filename", .type = CE_CDB_TYPE_STR},
        {.name = "file_mtime", .type = CE_CDB_TYPE_UINT64},
        {.name = "assets", .type = CE_CDB_TYPE_SET_SUBOBJECT},
};



//==============================================================================
// Interface
//==============================================================================


//void _import_dcc_asset(ce_cdb_t0 compile_db,
//                       char **files,
//                       uint32_t files_count) {
//    uint32_t group_count = ce_array_size(_G.import_group_ordered);
//
//    typedef struct dcc_group_t {
//        const char **files;
//        uint64_t group;
//    } dcc_group_t;
//
//    dcc_group_t *groups = NULL;
//    for (int g = 0; g < group_count; ++g) {
//        if (_G.import_group_ordered[g] == CT_CORE_IMPORT_GROUP) {
//            continue;
//        }
//
//        ce_array_push(groups, (dcc_group_t) {.group=_G.import_group_ordered[g]}, _G.allocator);
//
//        dcc_group_t *group = &ce_array_back(groups);
//
//        for (int j = 0; j < files_count; ++j) {
//            const char *filename = files[j];
//            const char *extenison = ce_os_path_a0->extension(filename);
//
//            if (strcmp(extenison, "dcc_asset") != 0) {
//                continue;
//            }
//
////            int64_t sourcefile_mtime = ce_fs_a0->file_mtime(SOURCE_ROOT, filename);
////            ct_assetdb_a0->put_file(filename, sourcefile_mtime);
//
//            ce_vio_t0 *vio = ce_fs_a0->open(SOURCE_ROOT, filename, FS_OPEN_READ);
//            ct_cdb_node_t *cnodes = NULL;
//            char *outputs = NULL;
//            ce_cdb_yaml_a0->load_to_nodes(vio, &cnodes, _G.allocator);
//            ce_fs_a0->close(vio);
//            ce_cdb_a0->dump_cnodes(cnodes, &outputs);
//
//            ce_cdb_uuid_t0 dcc_obj_uid = cnodes[0].obj.uuid;
//
//            ce_array_free(outputs, _G.allocator);
//            ce_array_free(cnodes, _G.allocator);
//
//            uint64_t dcc_obj = ce_cdb_a0->obj_from_uid(compile_db, dcc_obj_uid);
//
//            const ce_cdb_obj_o0 *r = ce_cdb_a0->read(compile_db, dcc_obj);
//            const char *dcc_file = ce_cdb_a0->read_str(r, CT_DCC_FILENAME_PROP, "");
//            int64_t dcc_file_mtime = ce_cdb_a0->read_uint64(r, CT_DCC_FILE_MTIME_PROP, 0);
//            const char *dcc_ext = ce_os_path_a0->extension(dcc_file);
//
//            ct_asset_dcc_io_i0 *rio = _find_dcc_asset_io(dcc_ext, _G.import_group_ordered[g]);
//            if (!rio || !rio->import_dcc) {
//                continue;
//            }
//
//            int64_t filemtime = ce_fs_a0->file_mtime(SOURCE_ROOT, dcc_file);
//
//            if (dcc_file_mtime && (filemtime <= dcc_file_mtime)) {
//                continue;
//            }
//
//            ce_array_push(group->files, filename, _G.allocator);
//        }
//    }
//
//    uint32_t group_n = ce_array_size(groups);
//    for (int i = 0; i < group_n; ++i) {
//        dcc_group_t *group = &groups[i];
//
//        uint32_t importfiles_count = ce_array_size(group->files);
//        for (int j = 0; j < importfiles_count; ++j) {
//            ce_cdb_uuid_t0 dcc_obj_uid = ct_asset_io_a0->filename_asset(group->files[j]);
//
//            uint64_t dcc_obj = ce_cdb_a0->obj_from_uid(compile_db, dcc_obj_uid);
//
//            if (!dcc_obj) {
//                continue;
//            }
//
//            const ce_cdb_obj_o0 *r = ce_cdb_a0->read(compile_db, dcc_obj);
//            const char *dcc_file = ce_cdb_a0->read_str(r, CT_DCC_FILENAME_PROP, "");
//            const char *dcc_ext = ce_os_path_a0->extension(dcc_file);
//
//            ct_asset_dcc_io_i0 *rio = _find_dcc_asset_io(dcc_ext, group->group);
//
//            if (!rio->import_dcc(compile_db, dcc_obj)) {
//                continue;
//            }
//
//            int64_t dcc_file_mtime = ce_fs_a0->file_mtime(SOURCE_ROOT, dcc_file);
//            ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(compile_db, dcc_obj);
//            ce_cdb_a0->set_uint64(w, CT_DCC_FILE_MTIME_PROP, dcc_file_mtime);
//            ce_cdb_a0->write_commit(w);
//
//            _save_to_file(compile_db, dcc_obj, group->files[j]);
//            _save_to_cdb(compile_db, dcc_obj, group->files[j]);
//        }
//        ce_array_free(group->files, _G.allocator);
//    }
//
//    ce_array_free(groups, _G.allocator);
//}




static int cmp_order(const void *p1,
                     const void *p2) {
    const uint64_t *name1 = p1;
    const uint64_t *name2 = p2;

    const uint64_t idx1 = ce_hash_lookup(&_G.import_group_map, *name1, 0);
    const uint64_t idx2 = ce_hash_lookup(&_G.import_group_map, *name2, 0);

    return _G.import_group_order[idx1] - _G.import_group_order[idx2];
}

void create_import_group(uint64_t name,
                         float order) {
    uint32_t n = ce_array_size(_G.import_group_name);
    ce_array_push(_G.import_group_name, name, _G.allocator);
    ce_array_push(_G.import_group_order, order, _G.allocator);
    ce_array_push(_G.import_group_ordered, name, _G.allocator);
    ce_hash_add(&_G.import_group_map, name, n, _G.allocator);
    qsort(_G.import_group_ordered, n + 1, sizeof(uint64_t), cmp_order);
}


static struct ct_asset_io_a0 asset_compiler_api = {
        .find_asset_io = _find_asset_io,
        .find_asset_dcc_io = _find_dcc_asset_io,

        .create_import_group = create_import_group,
};


struct ct_asset_io_a0 *ct_asset_io_a0 = &asset_compiler_api;


void CE_MODULE_LOAD(asset_io)(struct ce_api_a0 *api,
                              int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ct_asset_a0);
    CE_INIT_API(api, ce_task_a0);
    CE_INIT_API(api, ce_log_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_config_a0);
    CE_INIT_API(api, ce_fs_a0);
    CE_INIT_API(api, ce_cdb_yaml_a0);
    CE_INIT_API(api, ce_cdb_a0);

    CE_UNUSED(api);


    ce_id_a0->id64("ct_dcc_asset");

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    ce_cdb_a0->reg_obj_type(CT_DCC_ASSET, dcc_asset_prop, CE_ARRAY_LEN(dcc_asset_prop));


    api->add_api(CT_ASSET_IO_A0_STR,
                 &asset_compiler_api,
                 sizeof(asset_compiler_api));


    create_import_group(CT_CORE_IMPORT_GROUP, 1.0);
    create_import_group(CT_DCC_IMPORT_GROUP, 2.0);
}

void CE_MODULE_UNLOAD(asset_io)(struct ce_api_a0 *api,
                                int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

}
