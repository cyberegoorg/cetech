//==============================================================================
// Includes
//==============================================================================

#include <cetech/path.h>
#include "cetech/fs.h"
#include <cetech/module.h>

#include "cetech/vio.h"

#include <cetech/memory.h>
#include <cetech/filesystem.h>


//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "filesystem"

#define MAX_PATH_LEN 128
#define MAX_ROOTS 32



//==============================================================================
// Global
//==============================================================================

#define _G FilesystemGlobals


static struct G {
    struct {
        stringid64_t id[MAX_ROOTS];
        char *path[MAX_ROOTS];
    } rootmap;
} FilesystemGlobals = {0};

IMPORT_API(memory_api_v0);

static void _init(get_api_fce_t get_engine_api) {
    INIT_API(get_engine_api, memory_api_v0, MEMORY_API_ID);

    _G = (struct G) {0};

    log_debug(LOG_WHERE, "Init");
}

static void _shutdown() {
    log_debug(LOG_WHERE, "Shutdown");

    for (int i = 0; i < MAX_ROOTS; ++i) {
        if (_G.rootmap.path[i] == 0) {
            continue;
        }

        CETECH_DEALLOCATE(memory_api_v0.main_allocator(), _G.rootmap.path[i]);
    }

    _G = (struct G) {0};
}


//==============================================================================
// Interface
//==============================================================================

void filesystem_map_root_dir(stringid64_t root,
                             const char *base_path) {
    for (int i = 0; i < MAX_ROOTS; ++i) {
        if (_G.rootmap.path[i] != 0) {
            continue;
        }

        _G.rootmap.id[i] = root;
        _G.rootmap.path[i] = str_dup(base_path,
                                        memory_api_v0.main_allocator());
        break;
    }
}

const char *filesystem_get_root_dir(stringid64_t root) {
    for (int i = 0; i < MAX_ROOTS; ++i) {
        if (_G.rootmap.id[i].id != root.id) {
            continue;
        }

        return _G.rootmap.path[i];
    }

    return NULL;
}

int filesystem_get_fullpath(stringid64_t root,
                            char *result,
                            uint64_t maxlen,
                            const char *filename) {
    const char *root_path = filesystem_get_root_dir(root);

    return path_join(result, maxlen, root_path, filename) ==
           (strlen(root_path) + strlen(filename) + 1);
}

struct vio *filesystem_open(stringid64_t root,
                            const char *path,
                            enum vio_open_mode mode) {
    char fullm_path[MAX_PATH_LEN] = {0};

    if (!filesystem_get_fullpath(root, fullm_path,
                                 sizeof(fullm_path) / sizeof(char), path)) {
        return NULL;
    }

    struct vio *file = vio_from_file(fullm_path, mode,
                                         memory_api_v0.main_allocator());

    if (!file) {
        log_error(LOG_WHERE, "Could not load file %s", fullm_path);
        return NULL;
    }

    return file;
}

void filesystem_close(struct vio *file) {
    vio_close(file);
}

int filesystem_create_directory(stringid64_t root,
                                const char *path) {
    char fullm_path[MAX_PATH_LEN] = {0};

    if (!filesystem_get_fullpath(root, fullm_path,
                                 sizeof(fullm_path) / sizeof(char), path)) {
        return 0;
    }

    return dir_make_path(fullm_path);
}


void filesystem_listdir(stringid64_t root,
                        const char *path,
                        const char *filter,
                        string_array_t *files,
                        struct allocator *allocator) {

    char fullm_path[MAX_PATH_LEN] = {0};
    if (!filesystem_get_fullpath(root, fullm_path,
                                 sizeof(fullm_path) / sizeof(char), path)) {
        return;
    }

    dir_list(fullm_path, 1, files, allocator);
}

void filesystem_listdir_free(string_array_t *files,
                             struct allocator *allocator) {
    dir_list_free(files, allocator);
}


time_t filesystem_get_file_mtime(stringid64_t root,
                                 const char *path) {
    char fullm_path[MAX_PATH_LEN] = {0};
    if (!filesystem_get_fullpath(root, fullm_path,
                                 sizeof(fullm_path) / sizeof(char), path)) {
        return 0;
    }

    return file_mtime(fullm_path);
}


void *filesystem_get_module_api(int api) {

    switch (api) {
        case PLUGIN_EXPORT_API_ID:
                {
                    static struct module_api_v0 module = {0};

                    module.init = _init;
                    module.shutdown = _shutdown;

                    return &module;
                }

        case FILESYSTEM_API_ID:
                {
                    static struct filesystem_api_v0 api = {0};

                    api.filesystem_get_root_dir = filesystem_get_root_dir;
                    api.filesystem_open = filesystem_open;
                    api.filesystem_map_root_dir = filesystem_map_root_dir;
                    api.filesystem_close = filesystem_close;
                    api.filesystem_listdir = filesystem_listdir;
                    api.filesystem_listdir_free = filesystem_listdir_free;
                    api.filesystem_create_directory = filesystem_create_directory;
                    api.filesystem_get_file_mtime = filesystem_get_file_mtime;
                    api.filesystem_get_fullpath = filesystem_get_fullpath;

                    return &api;
                }

        default:
            return NULL;
    }

}
