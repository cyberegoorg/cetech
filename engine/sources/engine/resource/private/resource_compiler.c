//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>

#include "include/SDL2/SDL.h"

#include <celib/filesystem/filesystem.h>
#include <celib/string/string.h>
#include <celib/filesystem/path.h>
#include <celib/string/stringid.h>
#include <engine/task/task.h>
#include <engine/resource/resource.h>
#include <engine/application/application.h>
#include <engine/memory/memsys.h>
#include <engine/plugin/plugin_api.h>

#include "celib/filesystem/vio.h"
#include "engine/config/cvar.h"

#include "builddb.h"
#include "engine/memory/memsys.h"


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
    stringid64_t type;
    stringid64_t name;
    struct vio *source;
    struct vio *build;
    time_t mtime;
    resource_compilator_t compilator;
    atomic_int completed;
};

ARRAY_PROTOTYPE_N(struct task_item, task_item);

struct G {
    stringid64_t compilator_map_type[MAX_TYPES]; // TODO: MAP
    resource_compilator_t compilator_map_compilator[MAX_TYPES]; // TODO: MAP

    cvar_t cv_source_dir;
    cvar_t cv_core_dir;
    cvar_t cv_build_dir; // TODO: MOVE TO RESOURCE MANAGER
    cvar_t cv_external_dir;
} ResourceCompilerGlobal = {0};

static struct MemSysApiV1 MemSysApiV1;
static struct ResourceApiV1 ResourceApiV1;
static struct TaskApiV1 TaskApiV1;
static struct ConfigApiV1 ConfigApiV1;


//CE_STATIC_ASSERT(sizeof(struct compile_task_data) < 64);


//==============================================================================
// Private
//==============================================================================

void _add_dependency(const char *who_filename,
                     const char *depend_on_filename) {
    builddb_set_file_depend(who_filename, depend_on_filename);

    char path[1024] = {0};
    cel_path_join(path, CEL_ARRAY_LEN(path), ResourceApiV1.compiler_get_source_dir(), depend_on_filename);

    builddb_set_file(depend_on_filename, cel_file_mtime(path));
}

static struct compilator_api _compilator_api = {
        .add_dependency = _add_dependency
};


static void _compile_task(void *data) {
    struct compile_task_data *tdata = data;

    log_info("resource_compiler.task",
             "Compile resource \"%s\" to \"" "%" SDL_PRIX64 "%" SDL_PRIX64 "\"",
             tdata->source_filename, tdata->type.id, tdata->name.id);


    if (tdata->compilator(tdata->source_filename, tdata->source, tdata->build, &_compilator_api)) {
        builddb_set_file(tdata->source_filename, tdata->mtime);
        builddb_set_file_depend(tdata->source_filename, tdata->source_filename);

        log_info("resource_compiler.task",
                 "Resource \"%s\" compiled", tdata->source_filename);
    } else {
        log_error("resource_compiler.task",
                  "Resource \"%s\" compilation fail", tdata->source_filename);
    }

    CEL_DEALLOCATE(MemSysApiV1.main_scratch_allocator(), tdata->source_filename);
    cel_vio_close(tdata->source);
    cel_vio_close(tdata->build);

    atomic_store_explicit(&tdata->completed, 1, memory_order_release);
}

resource_compilator_t _find_compilator(stringid64_t type) {
    for (int i = 0; i < MAX_TYPES; ++i) {
        if (_G.compilator_map_type[i].id != type.id) {
            continue;
        }

        return _G.compilator_map_compilator[i];
    }

    return NULL;
}

void _compile_dir(ARRAY_T(task_item) *tasks,
                  struct array_pchar *files,
                  const char *source_dir,
                  const char *build_dir_full) {

    cel_dir_list(source_dir, 1, files, MemSysApiV1.main_scratch_allocator());

    for (int i = 0; i < ARRAY_SIZE(files); ++i) {
        const char *source_filename_full = ARRAY_AT(files, i);
        const char *source_filename_short = ARRAY_AT(files, i) + cel_strlen(source_dir) + 1;
        const char *resource_type = cel_path_extension(source_filename_short);

        char resource_name[128] = {0};
        memory_copy(resource_name, source_filename_short,
                    cel_strlen(source_filename_short) - 1 - cel_strlen(resource_type));

        stringid64_t type_id = stringid64_from_string(resource_type);
        stringid64_t name_id = stringid64_from_string(resource_name);

        resource_compilator_t compilator = _find_compilator(type_id);
        if (compilator == NULL) {
            //log_warning("resource_compilator", "Type \"%s\" does not register compilator", resource_type);
            continue;
        }

        if (!builddb_need_compile(source_dir, source_filename_short)) {
            continue;
        }

        char build_name[33] = {0};
        snprintf(build_name, CEL_ARRAY_LEN(build_name), "%" SDL_PRIX64 "%" SDL_PRIX64, type_id.id, name_id.id);

        builddb_set_file_hash(source_filename_short, build_name);

        struct vio *source_vio = cel_vio_from_file(source_filename_full, VIO_OPEN_READ,
                                                   MemSysApiV1.main_scratch_allocator());
        if (source_vio == NULL) {
            cel_vio_close(source_vio);
            continue;
        }

        char build_path[4096] = {0};
        cel_path_join(build_path, CEL_ARRAY_LEN(build_path), build_dir_full, build_name);

        struct vio *build_vio = cel_vio_from_file(build_path, VIO_OPEN_WRITE, MemSysApiV1.main_scratch_allocator());
        if (build_vio == NULL) {
            cel_vio_close(build_vio);
            continue;
        }

        struct compile_task_data *data = CEL_ALLOCATE(MemSysApiV1.main_allocator(), struct compile_task_data, 1);

        *data = (struct compile_task_data) {
                .name = name_id,
                .type = type_id,
                .build = build_vio,
                .source = source_vio,
                .compilator = compilator,
                .source_filename = cel_strdup(source_filename_short, MemSysApiV1.main_scratch_allocator()),
                .mtime = cel_file_mtime(source_filename_full),
                .completed = 0
        };

        struct task_item item = {
                .name = "compiler_task",
                .work = _compile_task,
                .data = data,
                .affinity = TASK_AFFINITY_NONE
        };

        ARRAY_PUSH_BACK(task_item, tasks, item);
    }
    cel_dir_list_free(files, MemSysApiV1.main_scratch_allocator());
}


//==============================================================================
// Interface
//==============================================================================

static void _init_cvar(struct ConfigApiV1 config) {
    ConfigApiV1 = config;

    _G.cv_source_dir = config.new_str("src", "Resource source dir", "data/src");
    _G.cv_core_dir = config.new_str("core", "Resource application source dir", "core");
    _G.cv_external_dir = config.new_str("external", "External build dir", "externals/build");

    _G.cv_build_dir = config.new_str("build", "Resource build dir", "data/build");
}

static void _init(get_api_fce_t get_engine_api) {
    MemSysApiV1 = *((struct MemSysApiV1 *) get_engine_api(MEMORY_API_ID, 0));
    ResourceApiV1 = *((struct ResourceApiV1 *) get_engine_api(RESOURCE_API_ID, 0));
    TaskApiV1 = *((struct TaskApiV1 *) get_engine_api(TASK_API_ID, 0));

    const char *build_dir = ConfigApiV1.get_string(_G.cv_build_dir);
    char build_dir_full[1024] = {0};
    cel_path_join(build_dir_full, CEL_ARRAY_LEN(build_dir_full), build_dir, application_platform());

    cel_dir_make_path(build_dir_full);
    builddb_init_db(build_dir_full);

    char tmp_dir_full[1024] = {0};
    cel_path_join(tmp_dir_full, CEL_ARRAY_LEN(tmp_dir_full), build_dir_full, "tmp");
    cel_dir_make_path(tmp_dir_full);
}

static void _shutdown() {
    _G = (struct G) {0};
}

void *resourcecompiler_get_plugin_api(int api,
                                      int version) {

    if (api == PLUGIN_EXPORT_API_ID && version == 0) {
        static struct plugin_api_v0 plugin = {0};

        plugin.init = _init;
        plugin.init_cvar = _init_cvar;
        plugin.shutdown = _shutdown;

        return &plugin;

    }

    return 0;
}


void resource_compiler_create_build_dir(struct ConfigApiV1 config) {
    const char *build_dir = config.get_string(_G.cv_build_dir);
    char build_dir_full[1024] = {0};
    cel_path_join(build_dir_full, CEL_ARRAY_LEN(build_dir_full), build_dir, application_platform());

    cel_dir_make_path(build_dir_full);
}

void resource_compiler_register(stringid64_t type,
                                resource_compilator_t compilator) {
    for (int i = 0; i < MAX_TYPES; ++i) {
        if (_G.compilator_map_type[i].id != 0) {
            continue;
        }

        _G.compilator_map_type[i].id = type.id;
        _G.compilator_map_compilator[i] = compilator;
        return;
    }
}


void resource_compiler_compile_all() {
    const char *core_dir = ConfigApiV1.get_string(_G.cv_core_dir);
    const char *build_dir = ConfigApiV1.get_string(_G.cv_build_dir);
    const char *source_dir = ConfigApiV1.get_string(_G.cv_source_dir);
    const char *platform = application_platform();

    char build_dir_full[1024] = {0};
    cel_path_join(build_dir_full, CEL_ARRAY_LEN(build_dir_full), build_dir, platform);

    struct array_pchar files;
    array_init_pchar(&files, MemSysApiV1.main_scratch_allocator());

    ARRAY_T(task_item) tasks;
    ARRAY_INIT(task_item, &tasks, MemSysApiV1.main_allocator());

    const char *dirs[] = {source_dir, core_dir};
    for (int i = 0; i < CEL_ARRAY_LEN(dirs); ++i) {
        array_resize_pchar(&files, 0);
        _compile_dir(&tasks, &files, dirs[i], build_dir_full);
    }

    array_destroy_pchar(&files);

    TaskApiV1.add(tasks.data, tasks.size);

    for (int i = 0; i < ARRAY_SIZE(&tasks); ++i) {
        struct compile_task_data *data = ARRAY_AT(&tasks, i).data;

        TaskApiV1.wait_atomic(&data->completed, 0);
        CEL_DEALLOCATE(MemSysApiV1.main_allocator(), data);
    }

    ARRAY_DESTROY(task_item, &tasks);
}

int resource_compiler_get_filename(char *filename,
                                   size_t max_ken,
                                   stringid64_t type,
                                   stringid64_t name) {
    char build_name[33] = {0};
    ResourceApiV1.type_name_string(build_name, CEL_ARRAY_LEN(build_name), type, name);
    return builddb_get_filename_by_hash(filename, max_ken, build_name);
}

const char *resource_compiler_get_source_dir() {
    return ConfigApiV1.get_string(_G.cv_source_dir);
}

const char *resource_compiler_get_core_dir() {
    return ConfigApiV1.get_string(_G.cv_core_dir);
}


int resource_compiler_get_build_dir(char *build_dir,
                                    size_t max_len,
                                    const char *platform) {
    const char *build_dir_str = ConfigApiV1.get_string(_G.cv_build_dir);
    return cel_path_join(build_dir, max_len, build_dir_str, platform);
}

int resource_compiler_get_tmp_dir(char *tmp_dir,
                                  size_t max_len,
                                  const char *platform) {
    char build_dir[1024] = {0};
    resource_compiler_get_build_dir(build_dir, CEL_ARRAY_LEN(build_dir), platform);

    return cel_path_join(tmp_dir, max_len, build_dir, "tmp");
}

int resource_compiler_external_join(char *output,
                                    u32 max_len,
                                    const char *name) {
    char tmp_dir[1024] = {0};
    char tmp_dir2[1024] = {0};

    const char *external_dir_str = ConfigApiV1.get_string(_G.cv_external_dir);
    cel_path_join(tmp_dir, CEL_ARRAY_LEN(tmp_dir), external_dir_str, application_native_platform());
    strncat(tmp_dir, "64", CEL_ARRAY_LEN(tmp_dir));
    cel_path_join(tmp_dir2, CEL_ARRAY_LEN(tmp_dir2), tmp_dir, "release/bin");

    return cel_path_join(output, max_len, tmp_dir2, name);
}