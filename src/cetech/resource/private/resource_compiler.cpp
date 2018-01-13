
//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>

#include "include/SDL2/SDL.h"

#include <cetech/application/application.h>
#include <cetech/api/api_system.h>
#include <cetech/os/memory.h>
#include <cetech/task/task.h>
#include <cetech/config/config.h>
#include <cetech/os/path.h>
#include <cetech/log/log.h>
#include <cetech/os/vio.h>
#include <cetech/hashlib/hashlib.h>
#include <cetech/resource/resource.h>
#include <cetech/module/module.h>
#include <cetech/os/watchdog.h>
#include <celib/map.inl>
#include <cetech/filesystem/filesystem.h>
#include <cetech/yaml/ydb.h>
#include <cetech/coredb/coredb.h>
#include <cetech/kernel/kernel.h>
#include <celib/array.h>

#include "celib/buffer.inl"


using namespace celib;
using namespace buffer;

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_task_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_watchdog_a0);
CETECH_DECL_API(ct_filesystem_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_coredb_a0);

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
    uint64_t type;
    uint64_t name;
    time_t mtime;
    struct compilator compilator;
    atomic_int completed;
};

static struct _G {
    uint32_t count;
    uint64_t compilator_map_type[MAX_TYPES]; // TODO: MAP
    compilator compilator_map_compilator[MAX_TYPES]; // TODO: MAP

    ct_coredb_object_t* config;
    cel_alloc * allocator;
} _G ;


#include "builddb.h"
#include "resource.h"

//CE_STATIC_ASSERT(sizeof(struct compile_task_data) < 64);



//==============================================================================
// Private
//==============================================================================

void type_name_from_filename(const char *fullname,
                             uint64_t *type,
                             uint64_t *name,
                             char *short_name) {

    const char *resource_type = ct_path_a0.extension(fullname);

    size_t size = strlen(fullname) - strlen(resource_type) - 1;

    char resource_name[128] = {};
    memcpy(resource_name, fullname, size);

    *type = CT_ID64_0(resource_type);
    *name = CT_ID64_0(resource_name);

    if (short_name) {
        memcpy(short_name, fullname, sizeof(char) * size);
        short_name[size] = '\0';
    }
}

void _add_dependency(const char *who_filename,
                     const char *depend_on_filename) {
    builddb_set_file_depend(who_filename, depend_on_filename);
    builddb_set_file(depend_on_filename,
                     ct_filesystem_a0.file_mtime(
                             CT_ID64_0("source"),
                             depend_on_filename));
}

static ct_compilator_api _compilator_api = {
        .add_dependency = _add_dependency
};

static void _compile_task(void *data) {
    struct compile_task_data *tdata = (compile_task_data *) data;

    ct_log_a0.info("resource_compiler.task",
                   "Compile resource \"%s\" to \"" "%" SDL_PRIX64 "%" SDL_PRIX64 "\"",
                   tdata->source_filename, tdata->type, tdata->name);

    char* output_blob = NULL;

    if (tdata->compilator.compilator) {
        if(tdata->compilator.yaml_based) {
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

    if (!cel_array_size(output_blob)) {
        ct_log_a0.error("resource_compiler.task",
                        "Resource \"%s\" compilation fail",
                        tdata->source_filename);
    } else {
        builddb_set_file(tdata->source_filename, tdata->mtime);
        builddb_set_file_depend(tdata->source_filename, tdata->source_filename);

        ct_vio *build_vio = ct_filesystem_a0.open(
                CT_ID64_0("build"),
                tdata->build_filename, FS_OPEN_WRITE);

        CEL_FREE(ct_memory_a0.main_allocator(), tdata->build_filename);

        if (build_vio == NULL) {
            goto end;
        }

        build_vio->write(build_vio, output_blob, sizeof(char), cel_array_size(output_blob));

        ct_filesystem_a0.close(build_vio);

        ct_log_a0.info("resource_compiler.task",
                       "Resource \"%s\" compiled", tdata->source_filename);
    }

end:
    cel_array_free(output_blob, _G.allocator);

    CEL_FREE(ct_memory_a0.main_allocator(),
             tdata->source_filename);

    atomic_store_explicit(&tdata->completed, 1, memory_order_release);
}

compilator _find_compilator(uint64_t type) {
    for (int i = 0; i < MAX_TYPES; ++i) {
        if (_G.compilator_map_type[i] != type) {
            continue;
        }

        return _G.compilator_map_compilator[i];
    }

    return {.compilator = NULL};
}



void _compile_files(ct_task_item** tasks,
                    char **files,
                    uint32_t files_count,
                    celib::Map<uint64_t> &compiled) {
    for (uint32_t i = 0; i < files_count; ++i) {
        uint64_t type_id;
        uint64_t name_id;

        type_name_from_filename(files[i], &type_id, &name_id,
                                NULL);

        compilator compilator = _find_compilator(type_id);
        if (compilator.compilator == NULL) {
            continue;
        }

        if (!builddb_need_compile(files[i], &ct_filesystem_a0)) {
            continue;
        }

        char build_name[33] = {};
        snprintf(build_name, CETECH_ARRAY_LEN(build_name),
                 "%" SDL_PRIX64 "%" SDL_PRIX64, type_id, name_id);

        builddb_set_file_hash(files[i], build_name);

        char *build_full = ct_path_a0.join(
                ct_memory_a0.main_allocator(), 2,
                ct_coredb_a0.read_string(_G.config, CONFIG_KERNEL_PLATFORM, ""),
                build_name);

        struct compile_task_data *data =
                CEL_ALLOCATE(
                        ct_memory_a0.main_allocator(),
                        struct compile_task_data,
                        sizeof(struct compile_task_data));

        *data = (struct compile_task_data) {
                .name = name_id,
                .type = type_id,
                .compilator = compilator,
                .build_filename = build_full,
                .source_filename = ct_memory_a0.str_dup(files[i],
                                                        ct_memory_a0.main_allocator()),
                .mtime = ct_filesystem_a0.file_mtime(
                        CT_ID64_0("source"),
                        files[i]),

                .completed = 0
        };

        ct_task_item item = {
                .name = "compiler_task",
                .work = _compile_task,
                .data = data,
                .affinity = TASK_AFFINITY_NONE
        };

        cel_array_push(*tasks, item, _G.allocator);
        multi_map::insert(compiled, type_id, name_id);
    }
}


//==============================================================================
// Interface
//==============================================================================


void resource_compiler_create_build_dir(struct ct_config_a0 config,
                                        struct ct_app_a0 app) {
    CEL_UNUSED(config, app);

    char *build_dir_full = resource_compiler_get_build_dir(
            ct_memory_a0.main_allocator(), ct_coredb_a0.read_string(_G.config, CONFIG_KERNEL_PLATFORM, ""));

    ct_path_a0.make_path(build_dir_full);

    CEL_FREE(ct_memory_a0.main_allocator(), build_dir_full);
}

void resource_compiler_register(uint64_t type,
                                ct_resource_compilator_t compilator, bool yaml_based) {
    const uint32_t idx = _G.count++;

    _G.compilator_map_type[idx] = type;
    _G.compilator_map_compilator[idx] = {.compilator = compilator, .yaml_based = yaml_based};
}

void _compile_all(celib::Map<uint64_t> &compiled) {
    ct_task_item* tasks = NULL;
    const char *glob_patern = "**.*";
    char **files = nullptr;
    uint32_t files_count = 0;

    ct_filesystem_a0.listdir(CT_ID64_0("source"),
                             "", glob_patern, false, true, &files, &files_count,
                             ct_memory_a0.main_allocator());

    _compile_files(&tasks, files, files_count, compiled);

    ct_filesystem_a0.listdir_free(files, files_count,
                                  ct_memory_a0.main_allocator());

    ct_task_a0.add(tasks, cel_array_size(tasks));

    for (uint32_t i = 0; i < cel_array_size(tasks); ++i) {
        compile_task_data *data = (compile_task_data *) tasks[i].data;

        ct_task_a0.wait_atomic(&data->completed, 0);
        CEL_FREE(ct_memory_a0.main_allocator(), data);
    }

    cel_array_free(tasks, _G.allocator);
}


void resource_compiler_compile_all() {
    Map<uint64_t> compieled(ct_memory_a0.main_allocator());
    _compile_all(compieled);

}

int resource_compiler_get_filename(char *filename,
                                   size_t max_ken,
                                   uint64_t type,
                                   uint64_t name) {
    char build_name[33] = {};
    ct_resource_a0.type_name_string(build_name, CETECH_ARRAY_LEN(build_name),
                                    type,
                                    name);
    return builddb_get_filename_by_hash(filename, max_ken, build_name);
}

const char *resource_compiler_get_source_dir() {
    return ct_coredb_a0.read_string(_G.config, CONFIG_SOURCE_DIR, "");
}

const char *resource_compiler_get_core_dir() {
    return ct_coredb_a0.read_string(_G.config, CONFIG_CORE_DIR, "");;
}

char *resource_compiler_get_tmp_dir(cel_alloc *alocator,
                                    const char *platform) {

    char *build_dir = resource_compiler_get_build_dir(alocator, platform);

    return ct_path_a0.join(alocator, 2, build_dir, "tmp");
}

char *resource_compiler_external_join(cel_alloc *alocator,
                                      const char *name) {
    const char *external_dir_str = ct_coredb_a0.read_string(_G.config, CONFIG_EXTERNAL_DIR, "");

    char *tmp_dir = ct_path_a0.join(alocator, 2, external_dir_str,
                                    ct_coredb_a0.read_string(_G.config, CONFIG_KERNEL_PLATFORM, ""));

    celib::Buffer buffer(alocator);
    buffer::printf(buffer, "%s64", tmp_dir);
    CEL_FREE(alocator, tmp_dir);

    buffer::c_str(buffer);
    return ct_path_a0.join(alocator, 4,
                           buffer::c_str(buffer), "release", "bin", name);
}


void resource_memory_reload(uint64_t type, uint64_t name, char** blob);

void compile_and_reload(const char* filename) {
    char** output_blob = NULL;

    uint64_t type_id;
    uint64_t name_id;

    type_name_from_filename(filename, &type_id, &name_id, NULL);

    compilator compilator = _find_compilator(type_id);
    if (compilator.compilator == NULL) {
        goto error;
    }

    compilator.compilator(filename, output_blob, &_compilator_api);

    resource_memory_reload(type_id, name_id, output_blob);
    cel_array_free(output_blob, _G.allocator);
    return;

    error:
    cel_array_free(output_blob, _G.allocator);
}

void resource_compiler_check_fs() {
    static uint64_t root = CT_ID64_0("source");

    auto *wd_it = ct_filesystem_a0.event_begin(root);
    const auto *wd_end = ct_filesystem_a0.event_end(root);
    int need_compile = 0;

    while (wd_it != wd_end) {
        if (wd_it->type == CT_WATCHDOG_EVENT_FILE_MODIFIED) {
            need_compile = 1;
            break;
        }

        wd_it = ct_filesystem_a0.event_next(wd_it);
    }

    if (need_compile) {
        cel_alloc *alloc = ct_memory_a0.main_allocator();
        celib::Map<uint64_t> type_name(alloc);

        _compile_all(type_name);

        auto *type_it = map::begin(type_name);
        auto *type_end = map::end(type_name);

        uint64_t* name_array = NULL;

        while (type_it != type_end) {
            uint64_t type_id = type_it->key;

            cel_array_clean(name_array);

            auto it = multi_map::find_first(type_name, type_id);
            while (it != nullptr) {
                cel_array_push(name_array, it->value, _G.allocator);

                it = multi_map::find_next(type_name, it);
            }

            ct_resource_a0.reload(type_id, &name_array[0],cel_array_size(name_array));

            ++type_it;
        }

        cel_array_free(name_array, _G.allocator);
    }
}


static void _init_cvar(struct ct_config_a0 config) {
    ct_coredb_writer_t* writer = ct_coredb_a0.write_begin(_G.config);
    if(!ct_coredb_a0.prop_exist(_G.config, CONFIG_SOURCE_DIR)) {
        ct_coredb_a0.set_string(writer, CONFIG_SOURCE_DIR, "src");
    }

    if(!ct_coredb_a0.prop_exist(_G.config, CONFIG_CORE_DIR)) {
        ct_coredb_a0.set_string(writer, CONFIG_CORE_DIR, "core");
    }

    if(!ct_coredb_a0.prop_exist(_G.config, CONFIG_EXTERNAL_DIR)) {
        ct_coredb_a0.set_string(writer, CONFIG_EXTERNAL_DIR, "externals/build");
    }

    ct_coredb_a0.write_commit(writer);
}


static void _init(ct_api_a0 *api) {
    CEL_UNUSED(api);
    _G = {
            .allocator = ct_memory_a0.main_allocator(),
            .config = ct_config_a0.config_object(),
    };

    package_init(api);

    _init_cvar(ct_config_a0);
//    ct_app_a0.register_on_update(_update);

    auto platform = ct_coredb_a0.read_string(_G.config, CONFIG_KERNEL_PLATFORM, "");

    char *build_dir_full = ct_resource_a0.compiler_get_build_dir(
            ct_memory_a0.main_allocator(), platform);

    ct_path_a0.make_path(build_dir_full);
    builddb_init_db(build_dir_full, &ct_path_a0, &ct_memory_a0);

    char *tmp_dir_full = ct_path_a0.join(ct_memory_a0.main_allocator(), 2,
                                         build_dir_full, "tmp");

    ct_path_a0.make_path(tmp_dir_full);

    CEL_FREE(ct_memory_a0.main_allocator(), tmp_dir_full);
    CEL_FREE(ct_memory_a0.main_allocator(), build_dir_full);

    const char *core_dir = ct_coredb_a0.read_string(_G.config, CONFIG_CORE_DIR, "");
    const char *source_dir = ct_coredb_a0.read_string(_G.config, CONFIG_SOURCE_DIR, "");

    ct_filesystem_a0.map_root_dir(
            CT_ID64_0("source"),
            core_dir,
            true
    );

    ct_filesystem_a0.map_root_dir(
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
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_config_a0);
            CETECH_GET_API(api, ct_watchdog_a0);
            CETECH_GET_API(api, ct_filesystem_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_coredb_a0);
        },
        {
            CEL_UNUSED(reload);

            _init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);

            _shutdown();

        }
)
