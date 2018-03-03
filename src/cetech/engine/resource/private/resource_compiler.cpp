
//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>

#include "include/SDL2/SDL.h"

#include <cetech/engine/application/application.h>
#include <cetech/core/api/api_system.h>
#include <cetech/core/memory/memory.h>
#include <cetech/core/task/task.h>
#include <cetech/core/config/config.h>
#include <cetech/core/os/path.h>
#include <cetech/core/log/log.h>
#include <cetech/core/os/vio.h>
#include <cetech/core/hashlib/hashlib.h>
#include <cetech/engine/resource/resource.h>
#include <cetech/core/module/module.h>
#include <cetech/core/os/watchdog.h>
#include <cetech/core/containers/map.inl>
#include <cetech/core/fs/fs.h>
#include <cetech/core/yaml/ydb.h>
#include <cetech/engine/kernel/kernel.h>
#include <cetech/core/containers/array.h>
#include <cetech/core/containers/buffer.h>


using namespace celib;

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_task_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_watchdog_a0);
CETECH_DECL_API(ct_fs_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_cdb_a0);

//==============================================================================
// Defines
//==============================================================================

#define MAX_TYPES 128
#define _G ResourceCompilerGlobal

#define CONFIG_KERNEL_PLATFORM CT_ID64_0(CONFIG_PLATFORM_ID)
#define CONFIG_SOURCE_DIR CT_ID64_0(CONFIG_SRC_ID)
#define CONFIG_CORE_DIR CT_ID64_0(CONFIG_CORE_ID)
#define CONFIG_EXTERNAL_DIR CT_ID64_0(CONFIG_EXTRENAL_ID)

//==============================================================================
// Globals
//==============================================================================

struct compilator {
    ct_resource_compilator_t compilator;
    bool yaml_based;
};

struct compile_task_data {
    char *source_filename;
    char *build_filename;
    ct_resource_id rid;
    time_t mtime;
    struct compilator compilator;
    atomic_int completed;
};

static struct _G {
    uint32_t count;
    uint32_t compilator_map_type[MAX_TYPES]; // TODO: MAP
    compilator compilator_map_compilator[MAX_TYPES]; // TODO: MAP

    ct_cdb_obj_t *config;
    ct_alloc *allocator;
} _G;


#include "builddb.h"
#include "resource.h"

//CE_STATIC_ASSERT(sizeof(struct compile_task_data) < 64);



//==============================================================================
// Private
//==============================================================================

void type_name_from_filename(const char *fullname,
                             struct ct_resource_id *resource_id,
                             char *short_name) {

    const char *resource_type = ct_path_a0.extension(fullname);

    size_t size = strlen(fullname) - strlen(resource_type) - 1;

    char resource_name[128] = {};
    memcpy(resource_name, fullname, size);

    resource_id->name = CT_ID32_0(resource_name);
    resource_id->type = CT_ID32_0(resource_type);

    if (short_name) {
        memcpy(short_name, fullname, sizeof(char) * size);
        short_name[size] = '\0';
    }
}

void _add_dependency(const char *who_filename,
                     const char *depend_on_filename) {
    builddb_set_file_depend(who_filename, depend_on_filename);
    builddb_set_file(depend_on_filename,
                     ct_fs_a0.file_mtime(
                             CT_ID64_0("source"),
                             depend_on_filename));
}

static ct_compilator_api _compilator_api = {
        .add_dependency = _add_dependency
};

static void _compile_task(void *data) {
    struct compile_task_data *tdata = (compile_task_data *) data;

    ct_log_a0.info("resource_compiler.task",
                   "Compile resource \"%s\" to \"" "%" SDL_PRIX64 "\"",
                   tdata->source_filename, tdata->rid.i64);

    char *output_blob = NULL;

    if (tdata->compilator.compilator) {
        if (tdata->compilator.yaml_based) {
            const char **files;
            uint32_t files_count;

            ct_ydb_a0.parent_files(tdata->source_filename, &files,
                                   &files_count);

            for (int i = 0; i < files_count; ++i) {
                _compilator_api.add_dependency(tdata->source_filename,
                                               files[i]);
            }
        }

        tdata->compilator.compilator(tdata->source_filename,
                                     &output_blob,
                                     &_compilator_api);

    }

    if (!ct_array_size(output_blob)) {
        ct_log_a0.error("resource_compiler.task",
                        "Resource \"%s\" compilation fail",
                        tdata->source_filename);
    } else {
        builddb_set_file(tdata->source_filename, tdata->mtime);
        builddb_set_file_depend(tdata->source_filename, tdata->source_filename);

        ct_vio *build_vio = ct_fs_a0.open(
                CT_ID64_0("build"),
                tdata->build_filename, FS_OPEN_WRITE);

        ct_buffer_free(tdata->build_filename, ct_memory_a0.main_allocator());

        if (build_vio == NULL) {
            goto end;
        }

        build_vio->write(build_vio, output_blob, sizeof(char),
                         ct_array_size(output_blob));

        ct_fs_a0.close(build_vio);

        ct_log_a0.info("resource_compiler.task",
                       "Resource \"%s\" compiled", tdata->source_filename);
    }

    end:
    ct_array_free(output_blob, _G.allocator);

    CT_FREE(ct_memory_a0.main_allocator(),
            tdata->source_filename);

    atomic_store_explicit(&tdata->completed, 1, memory_order_release);
}

compilator _find_compilator(uint32_t type) {
    for (int i = 0; i < MAX_TYPES; ++i) {
        if (_G.compilator_map_type[i] != type) {
            continue;
        }

        return _G.compilator_map_compilator[i];
    }

    return {.compilator = NULL};
}


void _compile_files(ct_task_item **tasks,
                    char **files,
                    uint32_t files_count,
                    celib::Map<uint64_t> &compiled) {
    for (uint32_t i = 0; i < files_count; ++i) {
        ct_resource_id rid;

        type_name_from_filename(files[i], &rid, NULL);

        compilator compilator = _find_compilator(rid.type);
        if (compilator.compilator == NULL) {
            continue;
        }

        if (!builddb_need_compile(files[i], &ct_fs_a0)) {
            continue;
        }

        char build_name[33] = {};
        snprintf(build_name,
                 CT_ARRAY_LEN(build_name), "%" SDL_PRIX64, rid.i64);

        builddb_set_file_hash(files[i], build_name);

        char *build_full = NULL;
        ct_path_a0.join(&build_full,
                        ct_memory_a0.main_allocator(), 2,
                        ct_cdb_a0.read_str(_G.config,
                                           CONFIG_KERNEL_PLATFORM, ""),
                        build_name);

        struct compile_task_data *data = CT_ALLOC(_G.allocator,
                                                  struct compile_task_data,
                                                  sizeof(struct compile_task_data));

        *data = (struct compile_task_data) {
                .rid = rid,
                .compilator = compilator,
                .build_filename = build_full,
                .source_filename = ct_memory_a0.str_dup(files[i],
                                                        ct_memory_a0.main_allocator()),
                .mtime = ct_fs_a0.file_mtime(
                        CT_ID64_0("source"),
                        files[i]),

                .completed = 0
        };

        ct_task_item item = {
                .name = "compiler_task",
                .work = _compile_task,
                .data = data
        };

        ct_array_push(*tasks, item, _G.allocator);
        multi_map::insert(compiled, (uint64_t)rid.type, (uint64_t)rid.name);
    }
}


//==============================================================================
// Interface
//==============================================================================


void resource_compiler_create_build_dir(struct ct_config_a0 config,
                                        struct ct_app_a0 app) {
    CT_UNUSED(config, app);

    char *build_dir_full = resource_compiler_get_build_dir(
            ct_memory_a0.main_allocator(),
            ct_cdb_a0.read_str(_G.config, CONFIG_KERNEL_PLATFORM, ""));

    ct_path_a0.make_path(build_dir_full);

    CT_FREE(ct_memory_a0.main_allocator(), build_dir_full);
}

void resource_compiler_register(const char* type,
                                ct_resource_compilator_t compilator,
                                bool yaml_based) {
    const uint32_t idx = _G.count++;

    _G.compilator_map_type[idx] = CT_ID32_0(type);
    _G.compilator_map_compilator[idx] = {.compilator = compilator, .yaml_based = yaml_based};
}

void _compile_all(celib::Map<uint64_t> &compiled) {
    ct_task_item *tasks = NULL;
    const char *glob_patern = "**.*";
    char **files = nullptr;
    uint32_t files_count = 0;

    ct_fs_a0.listdir(CT_ID64_0("source"),
                     "", glob_patern, false, true, &files, &files_count,
                     ct_memory_a0.main_allocator());

    _compile_files(&tasks, files, files_count, compiled);

    ct_fs_a0.listdir_free(files, files_count,
                          ct_memory_a0.main_allocator());

    struct ct_task_counter_t *counter = NULL;
    ct_task_a0.add(tasks, ct_array_size(tasks), &counter);
    ct_task_a0.wait_for_counter(counter, 0);

    for (uint32_t i = 0; i < ct_array_size(tasks); ++i) {
        compile_task_data *data = (compile_task_data *) tasks[i].data;
        CT_FREE(ct_memory_a0.main_allocator(), data);
    }
    ct_array_free(tasks, _G.allocator);
}


void resource_compiler_compile_all() {
    Map<uint64_t> compieled(ct_memory_a0.main_allocator());
    _compile_all(compieled);

}

int resource_compiler_get_filename(char *filename,
                                   size_t max_ken,
                                   ct_resource_id resource_id) {
    char build_name[33] = {};
    ct_resource_a0.type_name_string(build_name, CT_ARRAY_LEN(build_name),
                                    resource_id);

    return builddb_get_filename_by_hash(filename, max_ken, build_name);
}

const char *resource_compiler_get_source_dir() {
    return ct_cdb_a0.read_str(_G.config, CONFIG_SOURCE_DIR, "");
}

const char *resource_compiler_get_core_dir() {
    return ct_cdb_a0.read_str(_G.config, CONFIG_CORE_DIR, "");;
}

char *resource_compiler_get_tmp_dir(ct_alloc *alocator,
                                    const char *platform) {

    char *build_dir = resource_compiler_get_build_dir(alocator, platform);

    char *buffer = NULL;
    ct_path_a0.join(&buffer, alocator, 2, build_dir, "tmp");
    return buffer;
}

char *resource_compiler_external_join(ct_alloc *alocator,
                                      const char *name) {
    const char *external_dir_str = ct_cdb_a0.read_str(_G.config,
                                                      CONFIG_EXTERNAL_DIR,
                                                      "");

    char *tmp_dir = NULL;
    ct_path_a0.join(&tmp_dir, alocator, 2, external_dir_str,
                    ct_cdb_a0.read_str(_G.config, CONFIG_KERNEL_PLATFORM, ""));

    char *buffer = NULL;
    ct_buffer_printf(&buffer, alocator, "%s64", tmp_dir);
    ct_buffer_free(tmp_dir, alocator);

    char *result = NULL;
    ct_path_a0.join(&result, alocator, 4, buffer, "release", "bin", name);
    ct_buffer_free(buffer, alocator);

    return result;
}


extern "C" void resource_memory_reload(ct_resource_id rid, char **blob);

void compile_and_reload(const char *filename) {
    char *output_blob = NULL;

    struct ct_resource_id rid;

    type_name_from_filename(filename, &rid, NULL);

    compilator compilator = _find_compilator(rid.type);
    if (compilator.compilator == NULL) {
        goto error;
    }

    compilator.compilator(filename, &output_blob, &_compilator_api);

    resource_memory_reload(rid, &output_blob);
    ct_array_free(output_blob, _G.allocator);
    return;

    error:
    ct_array_free(output_blob, _G.allocator);
}

void resource_compiler_check_fs() {
    static uint64_t root = CT_ID64_0("source");

    auto *wd_it = ct_fs_a0.event_begin(root);
    const auto *wd_end = ct_fs_a0.event_end(root);
    int need_compile = 0;

    while (wd_it != wd_end) {
        if (wd_it->type == CT_WATCHDOG_EVENT_FILE_MODIFIED) {
            need_compile = 1;
            break;
        }

        wd_it = ct_fs_a0.event_next(wd_it);
    }

    if (need_compile) {
        ct_alloc *alloc = ct_memory_a0.main_allocator();
        celib::Map<uint64_t> type_name(alloc);

        _compile_all(type_name);

        auto *type_it = map::begin(type_name);
        auto *type_end = map::end(type_name);

        uint64_t *name_array = NULL;

        while (type_it != type_end) {
            uint64_t type_id = type_it->key;

            ct_array_clean(name_array);

            auto it = multi_map::find_first(type_name, type_id);
            while (it != nullptr) {
                ct_array_push(name_array, it->value, _G.allocator);

                it = multi_map::find_next(type_name, it);
            }

            ct_resource_a0.reload(static_cast<uint32_t>(type_id),
                                  reinterpret_cast<uint32_t *>(&name_array[0]),
                                  ct_array_size(name_array));

            ++type_it;
        }

        ct_array_free(name_array, _G.allocator);
    }
}


static void _init_cvar(struct ct_config_a0 config) {
    ct_cdb_obj_t *writer = ct_cdb_a0.write_begin(_G.config);
    if (!ct_cdb_a0.prop_exist(_G.config, CONFIG_SOURCE_DIR)) {
        ct_cdb_a0.set_string(writer, CONFIG_SOURCE_DIR, "src");
    }

    if (!ct_cdb_a0.prop_exist(_G.config, CONFIG_CORE_DIR)) {
        ct_cdb_a0.set_string(writer, CONFIG_CORE_DIR, "core");
    }

    if (!ct_cdb_a0.prop_exist(_G.config, CONFIG_EXTERNAL_DIR)) {
        ct_cdb_a0.set_string(writer, CONFIG_EXTERNAL_DIR, "externals/build");
    }

    ct_cdb_a0.write_commit(writer);
}


static void _init(ct_api_a0 *api) {
    CT_UNUSED(api);
    _G = {
            .allocator = ct_memory_a0.main_allocator(),
            .config = ct_config_a0.config_object(),
    };

    package_init(api);

    _init_cvar(ct_config_a0);
//    ct_app_a0.register_on_update(_update);

    auto platform = ct_cdb_a0.read_str(_G.config, CONFIG_KERNEL_PLATFORM,
                                       "");

    char *build_dir_full = ct_resource_a0.compiler_get_build_dir(
            ct_memory_a0.main_allocator(), platform);

    ct_path_a0.make_path(build_dir_full);
    builddb_init_db(build_dir_full, &ct_path_a0, &ct_memory_a0);

    char *tmp_dir_full = NULL;
    ct_path_a0.join(&tmp_dir_full, ct_memory_a0.main_allocator(), 2,
                    build_dir_full, "tmp");

    ct_path_a0.make_path(tmp_dir_full);

    ct_buffer_free(tmp_dir_full, ct_memory_a0.main_allocator());
    ct_buffer_free(build_dir_full, ct_memory_a0.main_allocator());

    const char *core_dir = ct_cdb_a0.read_str(_G.config, CONFIG_CORE_DIR,
                                              "");
    const char *source_dir = ct_cdb_a0.read_str(_G.config,
                                                CONFIG_SOURCE_DIR, "");

    ct_fs_a0.map_root_dir(
            CT_ID64_0("source"),
            core_dir,
            true
    );

    ct_fs_a0.map_root_dir(
            CT_ID64_0("source"),
            source_dir,
            true
    );

}

static void _shutdown() {
    _G = {};
}


CETECH_MODULE_DEF(
        resourcecompiler,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_task_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_vio_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_config_a0);
            CETECH_GET_API(api, ct_watchdog_a0);
            CETECH_GET_API(api, ct_fs_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_cdb_a0);
        },
        {
            CT_UNUSED(reload);

            _init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);

            _shutdown();

        }
)
