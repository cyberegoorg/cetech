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
#include <celib/yaml_cdb.h>
#include <celib/containers/array.h>
#include <celib/containers/hash.h>
#include <celib/containers/buffer.h>
#include <celib/cdb.h>
#include <celib/containers/bagraph.h>

#include <cetech/resource/resource.h>
#include <cetech/kernel/kernel.h>
#include <cetech/asset_io/asset_io.h>
#include <celib/os/path.h>
#include <celib/os/time.h>
#include <celib/containers/bitset.h>
#include <celib/os/vio.h>
#include <stdlib.h>

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

    ce_hash_t import_group_map;
    uint64_t *import_group_name;
    float *import_group_order;
    uint64_t *import_group_ordered;
} _G;


//==============================================================================
// Private
//==============================================================================

ct_asset_io_i0 *_find_asset_io(const char *extension) {
    ce_api_entry_t0 it = ce_api_a0->first(CT_ASSET_IO_I);
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
    ce_api_entry_t0 it = ce_api_a0->first(CT_DCC_ASSET_IO_I);
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

void _save_to_cdb(ce_cdb_t0 db,
                  uint64_t obj,
                  const char *filename) {
    ce_cdb_uuid_t0 uuid = ce_cdb_a0->obj_uid(db, obj);

    int64_t sourcefile_mtime = ce_fs_a0->file_mtime(SOURCE_ROOT, filename);
    ct_resourcedb_a0->put_file(filename, sourcefile_mtime);

    ct_resourcedb_a0->put_obj(db, obj, _G.allocator);

    const char *type_str = ce_id_a0->str_from_id64(ce_cdb_a0->obj_type(db, obj));
    CE_ASSERT(LOG_WHERE, type_str);

    ct_resourcedb_a0->put_resource(uuid, type_str, filename);
}

static bool _save_to_file(ce_cdb_t0 db,
                          uint64_t obj,
                          const char *filename) {
    ce_vio_t0 *file = ce_fs_a0->open(SOURCE_ROOT, filename, FS_OPEN_WRITE);

    if (!file) {
        return false;
    }

    char *blob = NULL;
    ce_yaml_cdb_a0->dump_str(db, &blob, obj, 0);
    file->vt->write(file->inst, blob, ce_array_size(blob), 1);
    ce_fs_a0->close(file);
    ce_array_free(blob, _G.allocator);

    return true;
}

////
static const ce_cdb_prop_def_t0 dcc_resource_prop[] = {
        {.name = "filename", .type = CE_CDB_TYPE_STR},
        {.name = "file_mtime", .type = CE_CDB_TYPE_UINT64},
        {.name = "assets", .type = CE_CDB_TYPE_SET_SUBOBJECT},
};

typedef struct import_asset_data_t {
    ce_cdb_t0 db;
    const char *filename;
    ct_asset_io_i0 *asseet_io;
} import_asset_data_t;

static void _import_asset_task(void *data) {
    import_asset_data_t *i_data = data;

    int64_t sourcefile_mtime = ce_fs_a0->file_mtime(SOURCE_ROOT, i_data->filename);
    ct_resourcedb_a0->put_file(i_data->filename, sourcefile_mtime);

    ce_vio_t0* vio = ce_fs_a0->open(SOURCE_ROOT, i_data->filename, FS_OPEN_READ);
    cnode_t *cnodes = NULL;
    char *outputs = NULL;
    ce_yaml_cdb_a0->cnodes_from_vio(vio, &cnodes, _G.allocator);
    ce_cdb_a0->dump_cnodes(cnodes, &outputs);

    ct_resourcedb_a0->put_resource_blob(cnodes[0].obj.uuid,
                                        outputs, ce_array_size(outputs));
    ce_array_free(outputs, _G.allocator);

    const char *type_str = ce_id_a0->str_from_id64(cnodes[0].obj.type);
    ct_resourcedb_a0->put_resource(cnodes[0].obj.uuid,
                                   type_str,
                                   i_data->filename);

    if (i_data->asseet_io->import) {
        uint64_t cdata_obj = ce_cdb_a0->obj_from_uid(i_data->db, cnodes[0].obj.uuid);
        i_data->asseet_io->import(i_data->db, cdata_obj);
        ct_resourcedb_a0->put_obj(i_data->db, cdata_obj, _G.allocator);
    }

}

void _import_assets(ce_cdb_t0 db,
                    char **files,
                    uint32_t files_count) {

    import_asset_data_t *import_data = NULL;
    ce_array_set_capacity(import_data, files_count, _G.allocator);

    ce_task_item_t0 *tasks = NULL;

    for (uint32_t i = 0; i < files_count; ++i) {
        const char *filename = files[i];
        const char *extenison = ce_os_path_a0->extension(filename);

        ct_asset_io_i0 *resource_io = _find_asset_io(extenison);
        if (!resource_io) {
            continue;
        }

        int64_t filemtime = ce_fs_a0->file_mtime(SOURCE_ROOT, filename);
        int64_t db_mtime = ct_resourcedb_a0->get_file_mtime(filename);

        if (db_mtime && (filemtime <= db_mtime)) {
            continue;
        }

        uint32_t idx = ce_array_size(import_data);
        ce_array_push(import_data, ((import_asset_data_t) {
                .db = db,
                .filename = filename,
                .asseet_io = resource_io
        }), _G.allocator);

        ce_array_push(tasks, ((ce_task_item_t0) {
                .data = &import_data[idx],
                .name = "import",
                .work = _import_asset_task,
        }), _G.allocator);
    }

    ce_task_counter_t0 *import_task = NULL;
    ce_task_a0->add(tasks, ce_array_size(tasks), &import_task);
    ce_task_a0->wait_for_counter(import_task, 0);
    ce_array_free(tasks, _G.allocator);
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

void _generate_dcc_files(ce_cdb_t0 db) {
    const char *glob_patern = "**.*";
    char **files = NULL;
    uint32_t files_count = 0;

    ce_fs_a0->listdir(SOURCE_ROOT,
                      "", glob_patern, false, true, &files, &files_count,
                      _G.allocator);

    for (int i = 0; i < files_count; ++i) {
        const char *filename = files[i];
        const char *extenison = ce_os_path_a0->extension(filename);
        ct_asset_dcc_io_i0 *rio = _find_dcc_asset_io(extenison, 0);

        if (!rio) {
            continue;
        }

        char dcc_resource_path[512];
        snprintf(dcc_resource_path, CE_ARRAY_LEN(dcc_resource_path), "%s.dcc_asset", filename);

        if (!ce_fs_a0->exist(SOURCE_ROOT, dcc_resource_path)) {

            uint64_t dcc_resource_obj = ce_cdb_a0->create_object(db, CT_DCC_RESOURCE);
            ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(db, dcc_resource_obj);
            ce_cdb_a0->set_str(w, CT_DCC_FILENAME_PROP, filename);
            ce_cdb_a0->write_commit(w);

            _save_to_cdb(db, dcc_resource_obj, dcc_resource_path);
            _save_to_file(db, dcc_resource_obj, dcc_resource_path);
        }
    }

    ce_fs_a0->listdir_free(files, files_count, _G.allocator);
}

void _import_dcc_asset(ce_cdb_t0 compile_db,
                       char **files,
                       uint32_t files_count) {
    uint32_t group_count = ce_array_size(_G.import_group_ordered);

    typedef struct dcc_group_t {
        const char **files;
        uint64_t group;
    } dcc_group_t;

    dcc_group_t *groups = NULL;
    for (int g = 0; g < group_count; ++g) {
        if (_G.import_group_ordered[g] == CT_CORE_IMPORT_GROUP) {
            continue;
        }

        ce_array_push(groups, (dcc_group_t) {.group=_G.import_group_ordered[g]}, _G.allocator);

        dcc_group_t *group = &ce_array_back(groups);

        for (int j = 0; j < files_count; ++j) {
            const char *filename = files[j];
            const char *extenison = ce_os_path_a0->extension(filename);

            if (strcmp(extenison, "dcc_asset")) {
                continue;
            }

            int64_t sourcefile_mtime = ce_fs_a0->file_mtime(SOURCE_ROOT, filename);

            ct_resourcedb_a0->put_file(filename, sourcefile_mtime);


            ce_vio_t0* vio = ce_fs_a0->open(SOURCE_ROOT, filename, FS_OPEN_READ);
            cnode_t *cnodes = NULL;
            char *outputs = NULL;
            ce_yaml_cdb_a0->cnodes_from_vio(vio, &cnodes, _G.allocator);
            ce_cdb_a0->dump_cnodes(cnodes, &outputs);

            ce_cdb_uuid_t0 dcc_obj_uid = cnodes[0].obj.uuid;
            ct_resourcedb_a0->put_resource_blob(dcc_obj_uid, outputs, ce_array_size(outputs));

            ct_resourcedb_a0->put_resource(dcc_obj_uid,
                                           "ct_dcc_resource", filename);

            ce_array_free(outputs, _G.allocator);
            ce_array_free(cnodes, _G.allocator);

            uint64_t dcc_obj = ce_cdb_a0->obj_from_uid(compile_db, dcc_obj_uid);

            const ce_cdb_obj_o0 *r = ce_cdb_a0->read(compile_db, dcc_obj);
            const char *dcc_file = ce_cdb_a0->read_str(r, CT_DCC_FILENAME_PROP, "");
            int64_t dcc_file_mtime = ce_cdb_a0->read_uint64(r, CT_DCC_FILE_MTIME_PROP, 0);
            const char *dcc_ext = ce_os_path_a0->extension(dcc_file);

            ct_asset_dcc_io_i0 *rio = _find_dcc_asset_io(dcc_ext, _G.import_group_ordered[g]);
            if (!rio || !rio->import_dcc) {
                continue;
            }

            int64_t filemtime = ce_fs_a0->file_mtime(SOURCE_ROOT, dcc_file);

            if (dcc_file_mtime && (filemtime <= dcc_file_mtime)) {
                continue;
            }

            ce_array_push(group->files, filename, _G.allocator);
        }
    }

    uint32_t group_n = ce_array_size(groups);
    for (int i = 0; i < group_n; ++i) {
        dcc_group_t *group = &groups[i];

        uint32_t importfiles_count = ce_array_size(group->files);
        for (int j = 0; j < importfiles_count; ++j) {
            ce_cdb_uuid_t0 dcc_obj_uid = ct_resourcedb_a0->get_file_resource(group->files[j]);

            uint64_t dcc_obj = ce_cdb_a0->obj_from_uid(compile_db, dcc_obj_uid);

            const ce_cdb_obj_o0 *r = ce_cdb_a0->read(compile_db, dcc_obj);
            const char *dcc_file = ce_cdb_a0->read_str(r, CT_DCC_FILENAME_PROP, "");
            const char *dcc_ext = ce_os_path_a0->extension(dcc_file);

            ct_asset_dcc_io_i0 *rio = _find_dcc_asset_io(dcc_ext, group->group);

            if(!rio->import_dcc(compile_db, dcc_obj)){
                continue;
            }

            int64_t dcc_file_mtime = ce_fs_a0->file_mtime(SOURCE_ROOT, dcc_file);
            ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(compile_db, dcc_obj);
            ce_cdb_a0->set_uint64(w, CT_DCC_FILE_MTIME_PROP, dcc_file_mtime);
            ce_cdb_a0->write_commit(w);

            _save_to_file(compile_db, dcc_obj,  group->files[j]);
            _save_to_cdb(compile_db, dcc_obj, group->files[j]);
        }
        ce_array_free(group->files, _G.allocator);
    }

    ce_array_free(groups, _G.allocator);
}

void resource_compiler_import_all() {
    uint32_t start_ticks = ce_os_time_a0->ticks();

    ce_cdb_t0 compile_db = ce_cdb_a0->create_db(1000000);

    _generate_dcc_files(compile_db);

    const char *glob_patern = "**.*";
    char **files = NULL;
    uint32_t files_count = 0;

    ce_fs_a0->listdir(SOURCE_ROOT,
                      "", glob_patern, false, true, &files, &files_count,
                      _G.allocator);

    _import_assets(compile_db, files, files_count);
    _import_dcc_asset(compile_db, files, files_count);

    ce_fs_a0->listdir_free(files, files_count, _G.allocator);
    ce_cdb_a0->destroy_db(compile_db);

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

static struct ct_asset_io_a0 resource_compiler_api = {
        .compile_all = resource_compiler_import_all,
        .gen_tmp_file = resource_compiler_get_tmp_dir,
        .external_join = resource_compiler_external_join,
        .save_to_cdb = _save_to_cdb,
        .save_to_file = _save_to_file,
        .create_import_group = create_import_group,
};


struct ct_asset_io_a0 *ct_asset_io_a0 = &resource_compiler_api;


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
    CE_INIT_API(api, ce_yaml_cdb_a0);
    CE_INIT_API(api, ce_cdb_a0);

    CE_UNUSED(api);


    ce_id_a0->id64("ct_dcc_resource");

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    ce_cdb_a0->reg_obj_type(CT_DCC_RESOURCE, dcc_resource_prop, CE_ARRAY_LEN(dcc_resource_prop));

    _init_cvar(ce_config_a0);
    api->add_api(CT_ASSET_IO_API,
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

    create_import_group(CT_CORE_IMPORT_GROUP, 1.0);
    create_import_group(CT_DCC_IMPORT_GROUP, 2.0);
}

void CE_MODULE_UNLOAD(resourcecompiler)(struct ce_api_a0 *api,
                                        int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

}
