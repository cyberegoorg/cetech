//==============================================================================
// Includes
//==============================================================================

#include <stdio.h>

#include "include/SDL2/SDL.h"

#include <celib/os/fs.h>
#include <celib/string/string.h>
#include <celib/os/path.h>
#include <celib/stringid/types.h>
#include <celib/stringid/stringid.h>
#include <engine/task_manager/task_manager.h>
#include <engine/resource_compiler/resource_compiler.h>
#include <engine/application/application.h>
#include <engine/resource_manager/resource_manager.h>
#include "engine/memory_system/memory_system.h"

#include "celib/os/vio.h"
#include "engine/config_system/config_system.h"

#include "builddb.h"


//==============================================================================
// Defines
//==============================================================================

#define MAX_TYPES 128
#define _G ResourceCompilerGlobal


//==============================================================================
// Globals
//==============================================================================

struct G {
    stringid64_t compilator_map_type[MAX_TYPES]; // TODO: MAP
    resource_compilator_t compilator_map_compilator[MAX_TYPES]; // TODO: MAP
    config_var_t cv_source_dir;
    config_var_t cv_core_dir;
    config_var_t cv_build_dir; // TODO: MOVE TO RESOURCE MANAGER
} ResourceCompilerGlobal = {0};


struct compile_task_data {
    char *source_filename;
    stringid64_t type;
    stringid64_t name;
    struct vio *source;
    struct vio *build;
    time_t mtime;
    resource_compilator_t compilator;
};
CE_STATIC_ASSERT(sizeof(struct compile_task_data) < 64);


//==============================================================================
// Private
//==============================================================================

void _add_dependency(const char *who_filename, const char *depend_on_filename) {
    builddb_set_file_depend(who_filename, depend_on_filename);

    char path[1024] = {0};
    os_path_join(path, CE_ARRAY_LEN(path), resource_compiler_get_source_dir(), depend_on_filename);

    builddb_set_file(depend_on_filename, os_file_mtime(path));
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

    CE_DEALLOCATE(memsys_main_scratch_allocator(), tdata->source_filename);
    vio_close(tdata->source);
    vio_close(tdata->build);
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

void _compile_dir(task_t root_task, struct array_pchar *files, const char *source_dir, const char *build_dir_full) {
    os_dir_list(source_dir, 1, files, memsys_main_scratch_allocator());
    for (int i = 0; i < ARRAY_SIZE(files); ++i) {
        const char *source_filename_full = ARRAY_AT(files, i);
        const char *source_filename_short = ARRAY_AT(files, i) + str_lenght(source_dir) + 1;
        const char *resource_type = os_path_extension(source_filename_short);

        char resource_name[128] = {0};
        memory_copy(resource_name, source_filename_short,
                    str_lenght(source_filename_short) - 1 - str_lenght(resource_type));

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
        snprintf(build_name, CE_ARRAY_LEN(build_name), "%" SDL_PRIX64 "%" SDL_PRIX64, type_id.id, name_id.id);

        builddb_set_file_hash(source_filename_short, build_name);

        struct vio *source_vio = vio_from_file(source_filename_full, VIO_OPEN_READ, memsys_main_scratch_allocator());
        if (source_vio == NULL) {
            vio_close(source_vio);
            continue;
        }

        char build_path[1024] = {0};
        os_path_join(build_path, CE_ARRAY_LEN(build_path), build_dir_full, build_name);

        struct vio *build_vio = vio_from_file(build_path, VIO_OPEN_WRITE, memsys_main_scratch_allocator());
        if (build_vio == NULL) {
            vio_close(build_vio);
            continue;
        }

        struct compile_task_data data = {
                .name = name_id,
                .type = type_id,
                .build = build_vio,
                .source = source_vio,
                .compilator = compilator,
                .source_filename = str_duplicate(source_filename_short, memsys_main_scratch_allocator()),
                .mtime = os_file_mtime(source_filename_full)
        };

        task_t t = taskmanager_add_begin("compiler_task",
                                         _compile_task, &data, sizeof(data),
                                         task_null, root_task,
                                         TASK_PRIORITY_NORMAL, TASK_AFFINITY_NONE);

        taskmanager_add_end(&t, 1);
    }
    os_dir_list_free(files, memsys_main_scratch_allocator());
}


//==============================================================================
// Interface
//==============================================================================

int resource_compiler_init() {
    _G = (struct G) {0};

    _G.cv_source_dir = config_new_string("resource_compiler.source_dir", "Resource source dir", "data/src");
    _G.cv_core_dir = config_new_string("resource_compiler.core_dir", "Resource core source dir", "core");
    _G.cv_build_dir = config_new_string("resource_compiler.build_dir", "Resource build dir", "data/build");

    const char *build_dir = config_get_string(_G.cv_build_dir);
    char build_dir_full[1024] = {0};
    os_path_join(build_dir_full, CE_ARRAY_LEN(build_dir_full), build_dir, application_platform());

    os_dir_make_path(build_dir_full);
    builddb_init_db(build_dir_full);

    char tmp_dir_full[1024] = {0};
    os_path_join(tmp_dir_full, CE_ARRAY_LEN(tmp_dir_full), build_dir_full, "tmp");
    os_dir_make_path(tmp_dir_full);

    return 1;
}

void resource_compiler_shutdown() {
    _G = (struct G) {0};
}

void resource_compiler_register(stringid64_t type, resource_compilator_t compilator) {
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
    const char *core_dir = config_get_string(_G.cv_core_dir);
    const char *build_dir = config_get_string(_G.cv_build_dir);
    const char *source_dir = config_get_string(_G.cv_source_dir);
    const char *platform = application_platform();

    char build_dir_full[1024] = {0};
    os_path_join(build_dir_full, CE_ARRAY_LEN(build_dir_full), build_dir, platform);

    task_t root_task = taskmanager_add_null("compile", task_null, task_null, TASK_PRIORITY_HIGH, TASK_AFFINITY_NONE);

    struct array_pchar files;
    array_init_pchar(&files, memsys_main_scratch_allocator());


    const char *dirs[] = {source_dir, core_dir};
    for (int i = 0; i < CE_ARRAY_LEN(dirs); ++i) {
        array_resize_pchar(&files, 0);
        _compile_dir(root_task, &files, dirs[i], build_dir_full);
    }

    array_destroy_pchar(&files);

    taskmanager_add_end(&root_task, 1);
    taskmanager_wait(root_task);
}

int resource_compiler_get_filename(char *filename, size_t max_ken, stringid64_t type, stringid64_t name) {
    char build_name[33] = {0};
    resource_type_name_string(build_name, CE_ARRAY_LEN(build_name), type, name);
    return builddb_get_filename_by_hash(filename, max_ken, build_name);
}

const char *resource_compiler_get_source_dir() {
    return config_get_string(_G.cv_source_dir);
}

int resource_compiler_get_build_dir(char *build_dir, size_t max_len, const char *platform) {
    const char *build_dir_str = config_get_string(_G.cv_build_dir);
    return os_path_join(build_dir, max_len, build_dir_str, platform);
}

int resource_compiler_get_tmp_dir(char *tmp_dir, size_t max_len, const char *platform) {
    char build_dir[1024] = {0};
    resource_compiler_get_build_dir(build_dir, CE_ARRAY_LEN(build_dir), platform);

    return os_path_join(tmp_dir, max_len, build_dir, "tmp");
}