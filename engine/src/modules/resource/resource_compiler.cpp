#ifdef CETECH_CAN_COMPILE
//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>

#include "include/SDL2/SDL.h"

#include <cetech/celib/array.inl>
#include <cetech/kernel/os.h>
#include <cetech/kernel/hash.h>
#include <cetech/kernel/task.h>
#include <cetech/kernel/application.h>
#include <cetech/kernel/config.h>
#include <cetech/modules/resource.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/module.h>
#include <cetech/kernel/log.h>
#include <cetech/kernel/api_system.h>

#include <cetech/celib/string_stream.h>


using namespace cetech;
using namespace string_stream;

CETECH_DECL_API(memory_api_v0);
CETECH_DECL_API(resource_api_v0);
CETECH_DECL_API(task_api_v0);
CETECH_DECL_API(config_api_v0);
CETECH_DECL_API(app_api_v0);
CETECH_DECL_API(os_path_v0);
CETECH_DECL_API(os_vio_api_v0);
CETECH_DECL_API(log_api_v0);
CETECH_DECL_API(hash_api_v0);

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
    struct os_vio *source;
    struct os_vio *build;
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


#include "builddb.h"
#include "resource.h"

//CE_STATIC_ASSERT(sizeof(struct compile_task_data) < 64);


//==============================================================================
// Private
//==============================================================================

void _add_dependency(const char *who_filename,
                     const char *depend_on_filename) {
    auto a = memory_api_v0.main_allocator();

    builddb_set_file_depend(who_filename, depend_on_filename);

    char *path = os_path_v0.join(a, 2,
                                 resource_api_v0.compiler_get_source_dir(),
                                 depend_on_filename);

    builddb_set_file(depend_on_filename, os_path_v0.file_mtime(path));

    CETECH_FREE(a, path);
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

    CETECH_FREE(memory_api_v0.main_scratch_allocator(),
                      tdata->source_filename);
    os_vio_api_v0.close(tdata->source);
    os_vio_api_v0.close(tdata->build);

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

    auto a = memory_api_v0.main_allocator();

    char **files = nullptr;
    uint32_t files_count = 0;

    os_path_v0.list(source_dir, 1, &files, &files_count,
                    memory_api_v0.main_scratch_allocator());

    for (int i = 0; i < files_count; ++i) {
        const char *source_filename_full = files[i];
        const char *source_filename_short = files[i] + strlen(source_dir) + 1;
        const char *resource_type = os_path_v0.extension(
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
                                  &os_path_v0)) {
            continue;
        }

        char build_name[33] = {0};
        snprintf(build_name, CETECH_ARRAY_LEN(build_name),
                 "%" SDL_PRIX64 "%" SDL_PRIX64, type_id, name_id);

        builddb_set_file_hash(source_filename_short, build_name);

        struct os_vio *source_vio = os_vio_api_v0.from_file(
                source_filename_full,
                VIO_OPEN_READ);
        if (source_vio == NULL) {
            os_vio_api_v0.close(source_vio);
            continue;
        }

        char *build_path = os_path_v0.join(a, 2, build_dir_full, build_name);

        struct os_vio *build_vio = os_vio_api_v0.from_file(build_path,
                                                           VIO_OPEN_WRITE);

        CETECH_FREE(a, build_path);

        if (build_vio == NULL) {
            os_vio_api_v0.close(build_vio);
            continue;
        }

        struct compile_task_data *data =
                CETECH_ALLOCATE(
                        memory_api_v0.main_allocator(),
                        struct compile_task_data, sizeof(struct compile_task_data));

        *data = (struct compile_task_data) {
                .name = name_id,
                .type = type_id,
                .build = build_vio,
                .source = source_vio,
                .compilator = compilator,
                .source_filename = memory_api_v0.str_dup(source_filename_short,
                                                         memory_api_v0.main_scratch_allocator()),
                .mtime = os_path_v0.file_mtime(source_filename_full),
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

    os_path_v0.list_free(files, files_count,
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
    CETECH_GET_API(api, memory_api_v0);
    CETECH_GET_API(api, resource_api_v0);
    CETECH_GET_API(api, task_api_v0);
    CETECH_GET_API(api, app_api_v0);
    CETECH_GET_API(api, os_path_v0);
    CETECH_GET_API(api, os_vio_api_v0);
    CETECH_GET_API(api, log_api_v0);
    CETECH_GET_API(api, hash_api_v0);
    CETECH_GET_API(api, config_api_v0);

    _init_cvar(config_api_v0);

    char *build_dir_full = resource_api_v0.compiler_get_build_dir(
            memory_api_v0.main_allocator(), app_api_v0.platform());

    os_path_v0.make_path(build_dir_full);
    builddb_init_db(build_dir_full, &os_path_v0, &memory_api_v0);

    char *tmp_dir_full = os_path_v0.join(memory_api_v0.main_allocator(), 2,
                                         build_dir_full, "tmp");

    os_path_v0.make_path(tmp_dir_full);

    CETECH_FREE(memory_api_v0.main_allocator(), tmp_dir_full);
    CETECH_FREE(memory_api_v0.main_allocator(), build_dir_full);
}

static void _shutdown() {
    _G = (struct G) {0};
}


void resource_compiler_create_build_dir(struct config_api_v0 config,
                                        struct app_api_v0 app) {

    const char *platform = app_api_v0.platform();
    char *build_dir_full = resource_compiler_get_build_dir(
            memory_api_v0.main_allocator(), platform);

    os_path_v0.make_path(build_dir_full);

    CETECH_FREE(memory_api_v0.main_allocator(), build_dir_full);
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

    char *build_dir_full = resource_api_v0.compiler_get_build_dir(
            memory_api_v0.main_allocator(),
            app_api_v0.platform()
    );

    Array<task_item> tasks(memory_api_v0.main_allocator());

    const char *dirs[] = {source_dir, core_dir};
    for (int i = 0; i < CETECH_ARRAY_LEN(dirs); ++i) {
        _compile_dir(tasks, dirs[i], build_dir_full);
    }

    task_api_v0.add(tasks._data, tasks._size);

    for (int i = 0; i < array::size(tasks); ++i) {
        compile_task_data *data = (compile_task_data *) tasks[i].data;

        task_api_v0.wait_atomic(&data->completed, 0);
        CETECH_FREE(memory_api_v0.main_allocator(), data);
    }

    CETECH_FREE(memory_api_v0.main_allocator(), build_dir_full);
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

char *resource_compiler_get_tmp_dir(allocator *alocator,
                                    const char *platform) {

    char *build_dir = resource_compiler_get_build_dir(alocator, platform);

    return os_path_v0.join(alocator, 2, build_dir, "tmp");
}

char *resource_compiler_external_join(allocator *alocator,
                                      const char *name) {
    const char *external_dir_str = config_api_v0.get_string(_G.cv_external_dir);

    char *tmp_dir = os_path_v0.join(alocator, 2, external_dir_str,
                                    app_api_v0.native_platform());

    string_stream::Buffer buffer(alocator);
    string_stream::printf(buffer, "%s64", tmp_dir);
    CETECH_FREE(alocator, tmp_dir);

    string_stream::c_str(buffer);
    return os_path_v0.join(alocator, 4, string_stream::c_str(buffer), "release",
                           "bin", name);
}

extern "C" void resourcecompiler_load_module(struct api_v0 *api) {
    _init(api);
}

extern "C" void resourcecompiler_unload_module(struct api_v0 *api) {
    _shutdown();
}

#endif