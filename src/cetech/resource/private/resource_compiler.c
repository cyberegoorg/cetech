//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>


#include <celib/api_system.h>
#include <celib/memory.h>
#include <celib/task.h>
#include <celib/config.h>
#include <celib/os.h>
#include <celib/log.h>
#include <celib/hashlib.h>
#include <celib/module.h>
#include <celib/fs.h>
#include <celib/ydb.h>
#include <celib/array.inl>
#include <celib/buffer.inl>
#include <celib/cdb.h>
#include <celib/cdb.h>

#include <cetech/resource/resource.h>
#include <cetech/asset/sourcedb.h>
#include <cetech/kernel/kernel.h>
#include <cetech/resource/resource_compiler.h>
#include "cetech/resource/builddb.h"


//==============================================================================
// Defines
//==============================================================================

#define MAX_TYPES 128
#define _G ResourceCompilerGlobal

#define LOG_WHERE "resource_compiler"

//==============================================================================
// Globals
//==============================================================================

struct compile_task_data {
    char *source_filename;
    time_t mtime;
};

static struct _G {
    uint64_t config;
    struct ce_alloc *allocator;
} _G;




//==============================================================================
// Private
//==============================================================================

bool type_name_from_filename(const char *fullname,
                             struct ct_resource_id *resource_id,
                             char *short_name) {

    const char *resource_type = ce_os_a0->path->extension(fullname);

    if (!resource_type) {
        return false;
    }

    size_t size = strlen(fullname) - strlen(resource_type) - 1;

    char resource_name[128] = {};
    memcpy(resource_name, fullname, size);

    resource_id->name = ce_id_a0->id64(resource_name);
    resource_id->type = ce_id_a0->id64(resource_type);

    if (short_name) {
        memcpy(short_name, fullname, sizeof(char) * size);
        short_name[size] = '\0';
    }

    return true;
}


ct_resource_compilator_t _find_compilator(uint64_t type) {
    struct ct_resource_i0 *i = ct_resource_a0->get_interface(type);

    if (!i) {
        return NULL;
    }

    return i->compilator;
}

static void _compile_task(void *data) {
    struct compile_task_data *tdata = (struct compile_task_data *) data;

    if (ce_id_a0->id64(tdata->source_filename) ==
        ce_id_a0->id64("global.yml")) {
        return;
    }

    ce_log_a0->info("resource_compiler.task",
                    "Compile resource \"%s\"", tdata->source_filename);

    uint64_t obj = ce_ydb_a0->get_obj(tdata->source_filename);

    const uint64_t n = ce_cdb_a0->prop_count(obj);
    const uint64_t* asset_keys = ce_cdb_a0->prop_keys(obj);

    for (uint32_t i = 0; i < n; ++i) {
        uint64_t k = asset_keys[i];

        const char *resource_name;
        resource_name = ce_id_a0->str_from_id64(k);

        struct ct_resource_id rid;
        if (!type_name_from_filename(resource_name, &rid, NULL)) {
            ce_log_a0->error(LOG_WHERE,
                             "Invalid format for resource %s:%s ",
                             tdata->source_filename, resource_name);
            continue;
        }

        uint64_t asset_obj = ct_sourcedb_a0->get(rid);

        uint64_t output_obj = 0;
        ct_resource_compilator_t compilator = _find_compilator(rid.type);
        if (!compilator) {
            output_obj = asset_obj;
        } else {
            output_obj = compilator(asset_obj, rid,
                                    resource_name);
        }

        if (!output_obj) {
            ce_log_a0->error("resource_compiler.task",
                             "Resource \"%s\" compilation fail", resource_name);
        } else {
            ce_cdb_a0->set_type(output_obj, rid.type);

            char *output = NULL;
            ce_cdb_a0->dump(output_obj, &output, _G.allocator);
            ct_builddb_a0->put_resource(rid,
                                        output,
                                        ce_array_size(output));
            ce_buffer_free(output, _G.allocator);

            ct_builddb_a0->set_file_depend(tdata->source_filename,
                                           tdata->source_filename);

            ce_log_a0->info("resource_compiler.task",
                            "Resource \"%s\" compiled", resource_name);
        }
    }

    CE_FREE(_G.allocator, tdata->source_filename);
}


void _compile_files(struct ce_task_item **tasks,
                    char **files,
                    uint32_t files_count) {
    for (uint32_t i = 0; i < files_count; ++i) {
        if (!ct_builddb_a0->need_compile(files[i])) {
            continue;
        }

        struct compile_task_data *data = CE_ALLOC(_G.allocator,
                                                  struct compile_task_data,
                                                  sizeof(struct compile_task_data));

        *data = (struct compile_task_data) {
                .source_filename = ce_memory_a0->str_dup(files[i],
                                                         _G.allocator),
                .mtime = ce_fs_a0->file_mtime(SOURCE_ROOT, files[i]),
        };

        struct ce_task_item item = {
                .name = "compiler_task",
                .work = _compile_task,
                .data = data
        };

        ce_array_push(*tasks, item, _G.allocator);
    }
}

void _scan_files(char **files,
                 uint32_t files_count) {
    for (uint32_t i = 0; i < files_count; ++i) {
        const char *filename = files[i];
        if (ce_id_a0->id64(filename) == ce_id_a0->id64("global.yml")) {
            return;
        }


        uint64_t obj = ce_ydb_a0->get_obj(filename);

        const uint64_t n = ce_cdb_a0->prop_count(obj);
        const uint64_t* asset_keys = ce_cdb_a0->prop_keys(obj);

        for (uint32_t i = 0; i < n; ++i) {
            uint64_t k = asset_keys[i];

            const char *resource_name;
            resource_name = ce_id_a0->str_from_id64(k);

            struct ct_resource_id rid;
            if (!type_name_from_filename(resource_name, &rid, NULL)) {
                ce_log_a0->error(LOG_WHERE,
                                 "Invalid format for resource %s:%s ",
                                 filename, resource_name);
                continue;
            }


            int64_t mtime = ce_fs_a0->file_mtime(SOURCE_ROOT, filename);
            ct_builddb_a0->put_file(filename, mtime);
            ct_builddb_a0->put_resource_2(resource_name, rid, filename);
        }
    }
}

//==============================================================================
// Interface
//==============================================================================


char *resource_compiler_get_build_dir(struct ce_alloc *a,
                                      const char *platform) {
    const ce_cdb_obj_o * reader = ce_cdb_a0->read(_G.config);

    const char *build_dir_str = ce_cdb_a0->read_str(reader,
                                                    CONFIG_BUILD, "");

    char *buffer = NULL;
    ce_os_a0->path->join(&buffer, a, 2, build_dir_str, platform);

    return buffer;
}

void resource_compiler_create_build_dir(struct ce_config_a0 config) {
    CE_UNUSED(config);

    const ce_cdb_obj_o * reader = ce_cdb_a0->read(_G.config);

    char *build_dir_full = resource_compiler_get_build_dir(
            _G.allocator,
            ce_cdb_a0->read_str(reader, CONFIG_PLATFORM, ""));

    ce_os_a0->path->make_path(build_dir_full);

    CE_FREE(_G.allocator, build_dir_full);
}

//void resource_compiler_register(const char *type,
//                                ct_resource_compilator_t compilator,
//                                bool yaml_based) {
//    const uint32_t idx = _G.count++;
//
//    _G.compilator_map_type[idx] = CT_ID32_0(type);
//    _G.compilator_map_compilator[idx] = (struct compilator) {.compilator = compilator, .yaml_based = yaml_based};
//}

void _compile_all() {
    uint32_t start_ticks = ce_os_a0->time->ticks();

    struct ce_task_item *tasks = NULL;
    const char *glob_patern = "**.yml";
    char **files = NULL;
    uint32_t files_count = 0;

    ce_fs_a0->listdir(SOURCE_ROOT,
                      "", glob_patern, false, true, &files, &files_count,
                      _G.allocator);

    _scan_files(files, files_count);
    _compile_files(&tasks, files, files_count);

    ce_fs_a0->listdir_free(files, files_count,
                           _G.allocator);

    struct ce_task_counter_t *counter = NULL;
    ce_task_a0->add(tasks, ce_array_size(tasks), &counter);
    ce_task_a0->wait_for_counter(counter, 0);

    for (uint32_t i = 0; i < ce_array_size(tasks); ++i) {
        struct compile_task_data *data = (struct compile_task_data *) tasks[i].data;
        CE_FREE(_G.allocator, data);
    }
    ce_array_free(tasks, _G.allocator);

    uint32_t now_ticks = ce_os_a0->time->ticks();
    uint32_t dt = now_ticks - start_ticks;
    ce_log_a0->debug("resource_compiler", "compile time %f", dt * 0.001);
}


void resource_compiler_compile_all() {
//    Map<uint64_t> compieled(_G.allocator);

//    _compile_all(compieled);
    _compile_all();
}

int resource_compiler_get_filename(char *filename,
                                   size_t max_ken,
                                   struct ct_resource_id resource_id) {
    char build_name[128] = {};
    ct_resource_a0->type_name_string(build_name, CE_ARRAY_LEN(build_name),
                                     resource_id);

    return ct_builddb_a0->get_fullname(filename, max_ken,
                                       resource_id.type,
                                       resource_id.name);
}


char *resource_compiler_get_tmp_dir(struct ce_alloc *alocator,
                                    const char *platform) {

    char *build_dir = resource_compiler_get_build_dir(alocator, platform);

    char *buffer = NULL;
    ce_os_a0->path->join(&buffer, alocator, 2, build_dir, "tmp");
    return buffer;
}

char *resource_compiler_external_join(struct ce_alloc *alocator,
                                      const char *name) {

    const ce_cdb_obj_o * reader = ce_cdb_a0->read(_G.config);

    const char *external_dir_str = ce_cdb_a0->read_str(reader,
                                                       CONFIG_EXTERNAL,
                                                       "");

    char *tmp_dir = NULL;
    ce_os_a0->path->join(&tmp_dir, alocator, 2, external_dir_str,
                         ce_cdb_a0->read_str(reader,
                                             CONFIG_PLATFORM,
                                             ""));

    char *buffer = NULL;
    ce_buffer_printf(&buffer, alocator, "%s64", tmp_dir);
    ce_buffer_free(tmp_dir, alocator);

    char *result = NULL;
    ce_os_a0->path->join(&result, alocator, 4, buffer, "release", "bin",
                         name);
    ce_buffer_free(buffer, alocator);

    return result;
}


void compile_and_reload(uint64_t type,
                        uint64_t name) {
    struct ct_resource_id rid = {.type=type, .name=name};

    uint64_t asset_obj = ct_sourcedb_a0->get(rid);
    uint64_t output_obj = 0;
    ct_resource_compilator_t compilator = _find_compilator(rid.type);
    if (!compilator) {
        output_obj = asset_obj;
    } else {
        output_obj = compilator(asset_obj, rid, NULL);
    }

    ct_resource_a0->reload_from_obj(rid, output_obj);
}


static void _init_cvar(struct ce_config_a0 *config) {
    ce_cdb_obj_o *writer = ce_cdb_a0->write_begin(_G.config);
    if (!ce_cdb_a0->prop_exist(_G.config, CONFIG_SRC)) {
        ce_cdb_a0->set_str(writer, CONFIG_SRC, "src");
    }

    if (!ce_cdb_a0->prop_exist(_G.config, CONFIG_CORE)) {
        ce_cdb_a0->set_str(writer, CONFIG_CORE, "core");
    }

    if (!ce_cdb_a0->prop_exist(_G.config, CONFIG_EXTERNAL)) {
        ce_cdb_a0->set_str(writer, CONFIG_EXTERNAL, "externals/build");
    }

    ce_cdb_a0->write_commit(writer);
}

static struct ct_resource_compiler_a0 resource_compiler_api = {
        .compile_and_reload = compile_and_reload,
        .compile_all = resource_compiler_compile_all,
        .get_filename = resource_compiler_get_filename,
        .get_tmp_dir = resource_compiler_get_tmp_dir,
        .external_join = resource_compiler_external_join,
        .type_name_from_filename = type_name_from_filename,
};


struct ct_resource_compiler_a0 *ct_resource_compiler_a0 = &resource_compiler_api;

static void _init(struct ce_api_a0 *api) {
    CE_UNUSED(api);
    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
            .config = ce_config_a0->obj(),
    };

    _init_cvar(ce_config_a0);
    api->register_api("ct_resource_compiler_a0", &resource_compiler_api);

    const ce_cdb_obj_o * reader = ce_cdb_a0->read(_G.config);

    const char *platform = ce_cdb_a0->read_str(reader,
                                               CONFIG_PLATFORM, "");

    char *build_dir_full = resource_compiler_get_build_dir(_G.allocator,
                                                           platform);

    ce_os_a0->path->make_path(build_dir_full);


    char *tmp_dir_full = NULL;
    ce_os_a0->path->join(&tmp_dir_full, _G.allocator, 2,
                         build_dir_full, "tmp");

    ce_os_a0->path->make_path(tmp_dir_full);

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


CE_MODULE_DEF(
        resourcecompiler,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ct_resource_a0);
            CE_INIT_API(api, ce_task_a0);
            CE_INIT_API(api, ce_os_a0);
            CE_INIT_API(api, ce_log_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ce_config_a0);
            CE_INIT_API(api, ce_fs_a0);
            CE_INIT_API(api, ce_ydb_a0);
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
