//==============================================================================
// Includes
//==============================================================================

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
#include <cetech/asset_io/asset_io.h>
#include <celib/uuid64.h>
#include <celib/yaml_cdb.h>
#include <celib/os/path.h>
#include <celib/os/thread.h>
#include <celib/task.h>


#include "cetech/asset/asset.h"

//==============================================================================
// Gloals
//==============================================================================

#define _G AssetManagerGlobals
#define LOG_WHERE "asset"

//==============================================================================
// Gloals
//==============================================================================

CE_MODULE(ct_assetdb_a0);

struct _G {
    ce_hash_t type_map;

    ce_cdb_t0 db;

    ce_alloc_t0 *allocator;


    ///
    ce_spinlock_t0 uid_2_file_lock;
    ce_hash_t uid_2_file;

    ce_spinlock_t0 file_2_uid_lock;
    ce_hash_t file_2_uid;
} _G = {};

//==============================================================================
// Private
//==============================================================================


//==============================================================================
// Public interface
//==============================================================================

static void _asset_api_add(uint64_t name,
                           void *api) {
    if (name == CT_ASSET_I0) {
        ct_asset_i0 *ct_asset_i = api;
        CE_ASSERT(LOG_WHERE, ct_asset_i->name);

        ce_hash_add(&_G.type_map, ct_asset_i->cdb_type(), (uint64_t) api,
                    _G.allocator);
    }
}

static struct ct_asset_i0 *asset_get_interface(uint64_t type) {
    return (ct_asset_i0 *) ce_hash_lookup(&_G.type_map, type, 0);
}


void _save_to_cdb(ce_cdb_t0 db,
                  uint64_t obj,
                  const char *filename) {
    const char *type_str = ce_id_a0->str_from_id64(ce_cdb_a0->obj_type(db, obj));
    CE_ASSERT(LOG_WHERE, type_str);

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

void unload(ce_cdb_t0 db,
            const uint64_t *names,
            size_t count) {

    for (uint32_t i = 0; i < count; ++i) {
        if (1) {// TODO: ref counting
            struct ce_cdb_uuid_t0 rid = (ce_cdb_uuid_t0) {
                    .id = names[i],
            };

            uint64_t type = ct_asset_a0->get_asset_type(rid);

            struct ct_asset_i0 *asset_i = asset_get_interface(type);
            if (!asset_i) {
                continue;
            }

            char uuid_str[64] = {};
            ce_uuid64_to_string(uuid_str, CE_ARRAY_LEN(uuid_str), &(ce_uuid64_t0) {rid.id});

            ce_log_a0->debug(LOG_WHERE, "Unload asset %s", uuid_str);


            if (asset_i->offline) {
                asset_i->offline(db, rid.id);
            }
        }
    }
}

const char *cdb_filename(struct ce_cdb_uuid_t0 uid) {
    ce_os_thread_a0->spin_lock(&_G.uid_2_file_lock);
    uint64_t filename_hash = ce_hash_lookup(&_G.uid_2_file, uid.id, 0);
    ce_os_thread_a0->spin_unlock(&_G.uid_2_file_lock);
    return ce_id_a0->str_from_id64(filename_hash);
}

struct ce_cdb_uuid_t0 filename_cdb(const char *filename) {
    uint64_t filename_hash = ce_id_a0->id64(filename);

    ce_os_thread_a0->spin_lock(&_G.file_2_uid_lock);
    uint64_t uid = ce_hash_lookup(&_G.file_2_uid, filename_hash, 0);
    ce_os_thread_a0->spin_unlock(&_G.file_2_uid_lock);
    return (struct ce_cdb_uuid_t0) {.id=uid};
}

static uint64_t cdb_loader(ce_cdb_t0 db,
                           ce_cdb_uuid_t0 uuid) {
    uint32_t start_ticks = ce_os_time_a0->ticks();

    const char *filename = cdb_filename(uuid);
    ce_vio_t0 *input = ce_fs_a0->open(SOURCE_ROOT, filename, FS_OPEN_READ);

    if (!input) {
        ce_log_a0->error(LOG_WHERE, "Asset %s does not exist", filename);
        return 0;
    }

    cnode_t *cnodes = NULL;
    ce_yaml_cdb_a0->cnodes_from_vio(input, &cnodes, _G.allocator);
    uint64_t obj = ce_cdb_a0->load_from_cnodes(cnodes, ce_cdb_a0->db());
    ce_array_free(cnodes, _G.allocator);

    if (!obj) {
        char uuid_str[64] = {};
        ce_uuid64_to_string(uuid_str, CE_ARRAY_LEN(uuid_str), &(ce_uuid64_t0) {uuid.id});
        ce_log_a0->error(LOG_WHERE, "Could not load asset %s", uuid_str);
        return 0;
    }

    uint64_t type = ce_cdb_a0->obj_type(db, obj);

    struct ct_asset_i0 *asset_i = asset_get_interface(type);

    if (!asset_i) {
        return 0;
    }

    if (asset_i->online) {
        asset_i->online(db, obj);
    }

    uint32_t now_ticks = ce_os_time_a0->ticks();
    uint32_t dt = now_ticks - start_ticks;
    ce_log_a0->debug(LOG_WHERE,
                     "load time %f for asset %s",
                     dt * 0.001,
                     filename);

    return obj;
}

static bool save(uint64_t uuid) {
    uint64_t root = ce_cdb_a0->find_root(ce_cdb_a0->db(), uuid);

    ce_cdb_uuid_t0 r = {.id=root};
    const char *filename = cdb_filename(r);

    if (filename) {
        char *buf = NULL;
        ce_yaml_cdb_a0->dump_str(ce_cdb_a0->db(), &buf, uuid, 0);

        struct ce_vio_t0 *f = ce_fs_a0->open(SOURCE_ROOT, filename, FS_OPEN_WRITE);
        f->vt->write(f->inst, buf, ce_buffer_size(buf), 1);
        ce_fs_a0->close(f);
        ce_buffer_free(buf, _G.allocator);

        return true;
    }

    return false;
}

static int list_assets_from_dirs(const char *dir,
                                 char ***filename,
                                 struct ce_alloc_t0 *alloc) {
    char **files = NULL;
    uint32_t files_count = 0;

    ce_fs_a0->listdir(SOURCE_ROOT,
                      dir, "*", false, true, &files, &files_count,
                      alloc);
    for (int i = 0; i < files_count; ++i) {
        const char *fn = files[i];
        const char* type = ce_os_path_a0->extension(fn);

        ct_asset_i0 *asset_i = ct_asset_a0->get_interface(ce_id_a0->id64(type));
        if (asset_i) {
            char* file = ce_memory_a0->str_dup(files[i], alloc);
            ce_array_push(*filename, file, alloc);
        }
    }
    ce_fs_a0->listdir_free(files, files_count, alloc);

    return 1;
}


int list_assets_by_type(const char *name,
                        const char *type,
                        char ***filename,
                        struct ce_alloc_t0 *alloc) {

    char **files = NULL;
    uint32_t files_count = 0;

    char filter[128] = {};

    snprintf(filter, CE_ARRAY_LEN(filter), "*%s*.%s", name, type);

    ce_fs_a0->listdir(SOURCE_ROOT,
                      "", filter, false, true, &files, &files_count,
                      alloc);

    for (int i = 0; i < files_count; ++i) {
        ct_asset_i0 *asset_i = ct_asset_a0->get_interface(ce_id_a0->id64(type));
        if (asset_i) {
            char* file = ce_memory_a0->str_dup(files[i], alloc);
            ce_array_push(*filename, file, alloc);
        }
    }

    ce_fs_a0->listdir_free(files, files_count, alloc);


    return 1;
}

void clean_assets_list(char **filename,
                       struct ce_alloc_t0 *alloc) {
    const uint32_t n = ce_array_size(filename);
    for (int i = 0; i < n; ++i) {
        CE_FREE(alloc, filename[i]);
    }
}

uint64_t get_asset_type(ce_cdb_uuid_t0 asset) {
    uint64_t obj = ce_cdb_a0->obj_from_uid(ce_cdb_a0->db(), asset);
    return ce_cdb_a0->obj_type(ce_cdb_a0->db(), obj);
}


void asset_get_tmp_dir(char *output,
                       uint64_t max_size,
                       const char *platform,
                       const char *filename,
                       const char *ext) {

    const char *build_dir = ce_config_a0->read_str(CONFIG_SRC, "");

    char *buffer = NULL;
    ce_os_path_a0->join(&buffer, _G.allocator, 3, build_dir, "tmp", platform);

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

char *asset_external_join(ce_alloc_t0 *alocator,
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



typedef struct import_asset_data_t {
    ce_cdb_t0 db;
    const char *filename;
    ct_asset_io_i0 *asseet_io;
} import_asset_data_t;

static void _import_asset_task(void *data) {
    import_asset_data_t *i_data = data;

//    int64_t sourcefile_mtime = ce_fs_a0->file_mtime(SOURCE_ROOT, i_data->filename);
//    ct_assetdb_a0->put_file(i_data->filename, sourcefile_mtime);

    ce_vio_t0 *vio = ce_fs_a0->open(SOURCE_ROOT, i_data->filename, FS_OPEN_READ);
    cnode_t *cnodes = NULL;
    char *outputs = NULL;
    ce_yaml_cdb_a0->cnodes_from_vio(vio, &cnodes, _G.allocator);
    ce_fs_a0->close(vio);

    uint64_t filename_hash = ce_id_a0->id64(i_data->filename);
    uint64_t n = ce_array_size(cnodes);
    for (int i = 0; i < n; ++i) {
        cnode_t node = cnodes[i];

        switch (node.type) {
            default:
            case CNODE_INVALID:
                break;

            case CNODE_OBJ_BEGIN: {
                ce_os_thread_a0->spin_lock(&_G.uid_2_file_lock);
                ce_hash_add(&_G.uid_2_file, node.uuid.id, filename_hash, _G.allocator);
                ce_os_thread_a0->spin_unlock(&_G.uid_2_file_lock);

                if (i == 0) {
                    ce_os_thread_a0->spin_lock(&_G.file_2_uid_lock);
                    ce_hash_add(&_G.file_2_uid, filename_hash, node.uuid.id, _G.allocator);
                    ce_os_thread_a0->spin_unlock(&_G.file_2_uid_lock);
                }

            }
                break;
        }
    }


    ce_cdb_a0->dump_cnodes(cnodes, &outputs);

    ce_array_free(outputs, _G.allocator);
//
//    if (i_data->asseet_io && i_data->asseet_io->import) {
//        uint64_t cdata_obj = ce_cdb_a0->obj_from_uid(i_data->db, cnodes[0].obj.uuid);
//        i_data->asseet_io->import(i_data->db, cdata_obj);
//    }
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

        ct_asset_io_i0 *asset_io = 0;
        if (0 != strcmp(extenison, "dcc_asset")) {
            asset_io = ct_asset_io_a0->find_asset_io(extenison);
            if (!asset_io) {
                continue;
            }
        }

        int64_t filemtime = ce_fs_a0->file_mtime(SOURCE_ROOT, filename);
        int64_t db_mtime = 0;

        if (db_mtime && (filemtime <= db_mtime)) {
            continue;
        }

        uint32_t idx = ce_array_size(import_data);
        ce_array_push(import_data, ((import_asset_data_t) {
                .db = db,
                .filename = filename,
                .asseet_io = asset_io,
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
        ct_asset_dcc_io_i0 *rio = ct_asset_io_a0->find_asset_dcc_io(extenison, 0);

        if (!rio) {
            continue;
        }

        char dcc_asset_path[512];
        snprintf(dcc_asset_path, CE_ARRAY_LEN(dcc_asset_path), "%s.dcc_asset", filename);

        if (!ce_fs_a0->exist(SOURCE_ROOT, dcc_asset_path)) {

            uint64_t dcc_asset_obj = ce_cdb_a0->create_object(db, CT_DCC_ASSET);
            ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(db, dcc_asset_obj);
            ce_cdb_a0->set_str(w, CT_DCC_FILENAME_PROP, filename);
            ce_cdb_a0->write_commit(w);

            _save_to_cdb(db, dcc_asset_obj, dcc_asset_path);
            _save_to_file(db, dcc_asset_obj, dcc_asset_path);
        }
    }

    ce_fs_a0->listdir_free(files, files_count, _G.allocator);
}

void asset_compiler_import_all() {
    uint32_t start_ticks = ce_os_time_a0->ticks();

    ce_cdb_t0 compile_db = ce_cdb_a0->create_db(1000000);

    _generate_dcc_files(compile_db);

    const char *glob_patern = "**.*";
    char **files = NULL;
    uint32_t files_count = 0;

    ce_fs_a0->listdir(SOURCE_ROOT,
                      "", glob_patern, false, true, &files, &files_count,
                      _G.allocator);

    _import_assets(ce_cdb_a0->db(), files, files_count);
//    _import_dcc_asset(compile_db, files, files_count);

    ce_fs_a0->listdir_free(files, files_count, _G.allocator);
    ce_cdb_a0->destroy_db(compile_db);

    uint32_t now_ticks = ce_os_time_a0->ticks();
    uint32_t dt = now_ticks - start_ticks;
    ce_log_a0->debug("asset_compiler", "compile time %f", dt * 0.001);
}

static struct ct_asset_a0 asset_api = {
        .get_interface = asset_get_interface,
        .cdb_loader = cdb_loader,
        .save = save,
        .list_assets_from_dirs = list_assets_from_dirs,
        .list_assets_by_type = list_assets_by_type,
        .clean_assets_list = clean_assets_list,
        .get_asset_type = get_asset_type,
        .gen_tmp_file = asset_get_tmp_dir,
        .external_join = asset_external_join,
        .compile_all = asset_compiler_import_all,
        .asset_filename = cdb_filename,
        .filename_asset = filename_cdb,
        .save_to_cdb = _save_to_cdb,
        .save_to_file = _save_to_file,
};

struct ct_asset_a0 *ct_asset_a0 = &asset_api;

static void _init_api(struct ce_api_a0 *api) {
    api->add_api(CT_ASSET_A0_STR, &asset_api, sizeof(asset_api));

}

static void _init_cvar(struct ce_config_a0 *config) {
    _G = (struct _G) {};

    ce_config_a0 = config;

    if (!ce_config_a0->exist(CONFIG_SRC)) {
        ce_config_a0->set_str(CONFIG_SRC, "src");
    }

    if (!ce_config_a0->exist(CONFIG_CORE)) {
        ce_config_a0->set_str(CONFIG_CORE, "core");
    }

    if (!ce_config_a0->exist(CONFIG_EXTERNAL)) {
        ce_config_a0->set_str(CONFIG_EXTERNAL, "externals/build");
    }

//    ce_cdb_obj_o0 *writer = ce_cdb_a0->write_begin(ce_cdb_a0->db(), _G.config);
//    if (!ce_cdb_a0->prop_exist(writer, CONFIG_BUILD)) {
//        ce_cdb_a0->set_str(writer, CONFIG_BUILD, "build");
//    }
//    ce_cdb_a0->write_commit(writer);
}

char *asset_get_dir(ce_alloc_t0 *a,
                    const char *platform) {
    const char *build_dir_str = ce_config_a0->read_str(CONFIG_SRC, "");

    char *buffer = NULL;
    ce_os_path_a0->join(&buffer, a, 2, build_dir_str, platform);

    return buffer;
}


void CE_MODULE_LOAD(asset)(struct ce_api_a0 *api,
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

    ce_api_a0->register_on_add(_asset_api_add);


    const char *platform = ce_config_a0->read_str(CONFIG_PLATFORM, "");

    char *build_dir_full = asset_get_dir(_G.allocator, platform);

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

void CE_MODULE_UNLOAD(asset)(struct ce_api_a0 *api,
                                   int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    ce_hash_free(&_G.type_map, _G.allocator);
}
