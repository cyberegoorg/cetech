#ifdef CETECH_CAN_COMPILE
//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>

#include "include/SDL2/SDL.h"

#include <cetech/celib/array.inl>
#include <cetech/core/path.h>
#include <cetech/core/hash.h>
#include <cetech/modules/task.h>
#include <cetech/core/application.h>
#include <cetech/core/config.h>
#include <cetech/modules/resource.h>
#include <cetech/core/memory.h>
#include <cetech/core/module.h>
#include <cetech/core/log.h>
#include <cetech/core/api.h>
#include <cetech/core/vio.h>


using namespace cetech;

//==============================================================================
// Defines
//==============================================================================

#define MAX_TYPES 128
#define _G ResourceCompilerGlobal


//==============================================================================
// Globals
//==============================================================================

struct compile_task_data {
    char *source_filename;
    uint64_t type;
    uint64_t name;
    struct vio *source;
    struct vio *build;
    time_t mtime;
    resource_compilator_t compilator;
    atomic_int completed;
};

struct G {
    uint64_t compilator_map_type[MAX_TYPES]; // TODO: MAP
    resource_compilator_t compilator_map_compilator[MAX_TYPES]; // TODO: MAP

    cvar_t cv_source_dir;
    cvar_t cv_core_dir;
    cvar_t cv_external_dir;
} ResourceCompilerGlobal = {0};

IMPORT_API(memory_api_v0);
IMPORT_API(resource_api_v0);
IMPORT_API(task_api_v0);
IMPORT_API(config_api_v0);
IMPORT_API(app_api_v0);
IMPORT_API(path_v0);
IMPORT_API(vio_api_v0);
IMPORT_API(log_api_v0);
IMPORT_API(hash_api_v0);

#include "builddb.h"
#include "resource.h"

//CE_STATIC_ASSERT(sizeof(struct compile_task_data) < 64);


//==============================================================================
// Private
//==============================================================================

void _add_dependency(const char *who_filename,
                     const char *depend_on_filename) {
    builddb_set_file_depend(who_filename, depend_on_filename);

    char path[1024] = {0};
    path_v0.join(path, CETECH_ARRAY_LEN(path),
                 resource_api_v0.compiler_get_source_dir(),
                 depend_on_filename);

    builddb_set_file(depend_on_filename, path_v0.file_mtime(path));
}

static struct compilator_api _compilator_api = {
        .add_dependency = _add_dependency
};


static void _compile_task(void *data) {
    struct compile_task_data *tdata = (compile_task_data *) data;

    log_api_v0.info("resource_compiler.task",
                    "Compile resource \"%s\" to \"" "%" SDL_PRIX64 "%" SDL_PRIX64 "\"",
                    tdata->source_filename, tdata->type, tdata->name);


    if (tdata->compilator(tdata->source_filename, tdata->source, tdata->build,
                          &_compilator_api)) {
        builddb_set_file(tdata->source_filename, tdata->mtime);
        builddb_set_file_depend(tdata->source_filename, tdata->source_filename);

        log_api_v0.info("resource_compiler.task",
                        "Resource \"%s\" compiled", tdata->source_filename);
    } else {
        log_api_v0.error("resource_compiler.task",
                         "Resource \"%s\" compilation fail",
                         tdata->source_filename);
    }

    CETECH_DEALLOCATE(memory_api_v0.main_scratch_allocator(),
                      tdata->source_filename);
    vio_api_v0.close(tdata->source);
    vio_api_v0.close(tdata->build);

    atomic_store_explicit(&tdata->completed, 1, memory_order_release);
}

resource_compilator_t _find_compilator(uint64_t type) {
    for (int i = 0; i < MAX_TYPES; ++i) {
        if (_G.compilator_map_type[i] != type) {
            continue;
        }

        return _G.compilator_map_compilator[i];
    }

    return NULL;
}

void _compile_dir(Array<task_item> &tasks,
                  const char *source_dir,
                  const char *build_dir_full) {

    char **files = nullptr;
    uint32_t files_count = 0;

    path_v0.list(source_dir, 1, &files, &files_count,
                 memory_api_v0.main_scratch_allocator());

    for (int i = 0; i < files_count; ++i) {
        const char *source_filename_full = files[i];
        const char *source_filename_short = files[i] + strlen(source_dir) + 1;
        const char *resource_type = path_v0.extension(
                source_filename_short);

        char resource_name[128] = {0};
        memcpy(resource_name, source_filename_short,
               strlen(source_filename_short) - 1 -
               strlen(resource_type));

        uint64_t type_id = hash_api_v0.id64_from_str(resource_type);
        uint64_t name_id = hash_api_v0.id64_from_str(resource_name);

        resource_compilator_t compilator = _find_compilator(type_id);
        if (compilator == NULL) {
            //log_warning("resource_compilator", "Type \"%s\" does not register compilator", resource_type);
            continue;
        }

        if (!builddb_need_compile(source_dir, source_filename_short,
                                  &path_v0)) {
            continue;
        }

        char build_name[33] = {0};
        snprintf(build_name, CETECH_ARRAY_LEN(build_name),
                 "%" SDL_PRIX64 "%" SDL_PRIX64, type_id, name_id);

        builddb_set_file_hash(source_filename_short, build_name);

        struct vio *source_vio = vio_api_v0.from_file(source_filename_full,
                                                      VIO_OPEN_READ,
                                                      memory_api_v0.main_scratch_allocator());
        if (source_vio == NULL) {
            vio_api_v0.close(source_vio);
            continue;
        }

        char build_path[4096] = {0};
        path_v0.join(build_path, CETECH_ARRAY_LEN(build_path),
                     build_dir_full,
                     build_name);

        struct vio *build_vio = vio_api_v0.from_file(build_path, VIO_OPEN_WRITE,
                                                     memory_api_v0.main_scratch_allocator());
        if (build_vio == NULL) {
            vio_api_v0.close(build_vio);
            continue;
        }

        struct compile_task_data *data =
                CETECH_ALLOCATE(
                        memory_api_v0.main_allocator(),
                        struct compile_task_data, 1);

        *data = (struct compile_task_data) {
                .name = name_id,
                .type = type_id,
                .build = build_vio,
                .source = source_vio,
                .compilator = compilator,
                .source_filename = memory_api_v0.str_dup(source_filename_short,
                                                         memory_api_v0.main_scratch_allocator()),
                .mtime = path_v0.file_mtime(source_filename_full),
                .completed = 0
        };

        struct task_item item = {
                .name = "compiler_task",
                .work = _compile_task,
                .data = data,
                .affinity = TASK_AFFINITY_NONE
        };

        array::push_back(tasks, item);
    }

    path_v0.list_free(files, files_count,
                      memory_api_v0.main_scratch_allocator());
}


//==============================================================================
// Interface
//==============================================================================

static void _init_cvar(struct config_api_v0 config) {
    config_api_v0 = config;

    _G.cv_source_dir = config.new_str("src", "Resource source dir", "data/src");
    _G.cv_core_dir = config.new_str("core", "Resource application source dir",
                                    "core");
    _G.cv_external_dir = config.new_str("external", "External build dir",
                                        "externals/build");
}

static void _init(struct api_v0 *api) {
    GET_API(api, memory_api_v0);
    GET_API(api, resource_api_v0);
    GET_API(api, task_api_v0);
    GET_API(api, app_api_v0);
    GET_API(api, path_v0);
    GET_API(api, vio_api_v0);
    GET_API(api, log_api_v0);
    GET_API(api, hash_api_v0);

    char build_dir_full[1024] = {0};
    resource_api_v0.compiler_get_build_dir(build_dir_full,
                                           CETECH_ARRAY_LEN(build_dir_full),
                                           app_api_v0.platform());

    path_v0.make_path(build_dir_full);
    builddb_init_db(build_dir_full, &path_v0);

    char tmp_dir_full[1024] = {0};
    path_v0.join(tmp_dir_full, CETECH_ARRAY_LEN(tmp_dir_full),
                 build_dir_full,
                 "tmp");
    path_v0.make_path(tmp_dir_full);
}

static void _shutdown() {
    _G = (struct G) {0};
}


void resource_compiler_create_build_dir(struct config_api_v0 config,
                                        struct app_api_v0 app) {
    char build_dir_full[1024] = {0};
    const char *platform = app_api_v0.platform();
    resource_compiler_get_build_dir(build_dir_full,
                                    CETECH_ARRAY_LEN(build_dir_full), platform);

    path_v0.make_path(build_dir_full);
}

void resource_compiler_register(uint64_t type,
                                resource_compilator_t compilator) {
    for (int i = 0; i < MAX_TYPES; ++i) {
        if (_G.compilator_map_type[i] != 0) {
            continue;
        }

        _G.compilator_map_type[i] = type;
        _G.compilator_map_compilator[i] = compilator;
        return;
    }
}


void resource_compiler_compile_all() {
    const char *core_dir = config_api_v0.get_string(_G.cv_core_dir);
    const char *source_dir = config_api_v0.get_string(_G.cv_source_dir);
    const char *platform = app_api_v0.platform();

    char build_dir_full[1024] = {0};
    resource_api_v0.compiler_get_build_dir(build_dir_full,
                                           CETECH_ARRAY_LEN(build_dir_full),
                                           app_api_v0.platform());

    Array<task_item> tasks(memory_api_v0.main_allocator());

    const char *dirs[] = {source_dir, core_dir};
    for (int i = 0; i < CETECH_ARRAY_LEN(dirs); ++i) {
        _compile_dir(tasks, dirs[i], build_dir_full);
    }

    task_api_v0.add(tasks._data, tasks._size);

    for (int i = 0; i < array::size(tasks); ++i) {
        compile_task_data *data = (compile_task_data *) tasks[i].data;

        task_api_v0.wait_atomic(&data->completed, 0);
        CETECH_DEALLOCATE(memory_api_v0.main_allocator(), data);
    }
}

int resource_compiler_get_filename(char *filename,
                                   size_t max_ken,
                                   uint64_t type,
                                   uint64_t name) {
    char build_name[33] = {0};
    resource_api_v0.type_name_string(build_name, CETECH_ARRAY_LEN(build_name),
                                     type,
                                     name);
    return builddb_get_filename_by_hash(filename, max_ken, build_name);
}

const char *resource_compiler_get_source_dir() {
    return config_api_v0.get_string(_G.cv_source_dir);
}

const char *resource_compiler_get_core_dir() {
    return config_api_v0.get_string(_G.cv_core_dir);
}

int resource_compiler_get_tmp_dir(char *tmp_dir,
                                  size_t max_len,
                                  const char *platform) {
    char build_dir[1024] = {0};
    resource_compiler_get_build_dir(build_dir, CETECH_ARRAY_LEN(build_dir),
                                    platform);

    return path_v0.join(tmp_dir, max_len, build_dir, "tmp");
}

int resource_compiler_external_join(char *output,
                                    uint32_t max_len,
                                    const char *name) {
    char tmp_dir[1024] = {0};
    char tmp_dir2[1024] = {0};

    const char *external_dir_str = config_api_v0.get_string(_G.cv_external_dir);
    path_v0.join(tmp_dir, CETECH_ARRAY_LEN(tmp_dir), external_dir_str,
                 app_api_v0.native_platform());
    strncat(tmp_dir, "64", CETECH_ARRAY_LEN(tmp_dir));
    path_v0.join(tmp_dir2, CETECH_ARRAY_LEN(tmp_dir2), tmp_dir,
                 "release/bin");

    return path_v0.join(output, max_len, tmp_dir2, name);
}

extern "C" {
void *resourcecompiler_get_module_api(int api) {

    if (api == PLUGIN_EXPORT_API_ID) {
        static struct module_export_api_v0 module = {0};

        module.init = _init;
        module.init_cvar = _init_cvar;
        module.shutdown = _shutdown;

        return &module;

    }

    return 0;
}
}

#endif