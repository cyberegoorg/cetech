//==============================================================================
// Includes
//==============================================================================

#include <cetech/core/os/path.h>

#include <cetech/core/memory/memory.h>
#include <cetech/modules/filesystem/filesystem.h>
#include <cetech/core/config.h>
#include <cetech/modules/resource/resource.h>
#include <cetech/core/module.h>
#include <cetech/core/api.h>
#include <cetech/core/os/vio.h>


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
        uint64_t id[MAX_ROOTS];
        char *path[MAX_ROOTS];
    } rootmap;
} FilesystemGlobals = {0};

IMPORT_API(memory_api_v0);
IMPORT_API(path_v0);
IMPORT_API(vio_api_v0);
IMPORT_API(log_api_v0);


//==============================================================================
// Interface
//==============================================================================

void filesystem_map_root_dir(uint64_t root,
                             const char *base_path) {
    for (int i = 0; i < MAX_ROOTS; ++i) {
        if (_G.rootmap.path[i] != 0) {
            continue;
        }

        _G.rootmap.id[i] = root;
        _G.rootmap.path[i] = memory_api_v0.str_dup(base_path,
                                                   memory_api_v0.main_allocator());
        break;
    }
}

const char *filesystem_get_root_dir(uint64_t root) {
    for (int i = 0; i < MAX_ROOTS; ++i) {
        if (_G.rootmap.id[i] != root) {
            continue;
        }

        return _G.rootmap.path[i];
    }

    return NULL;
}

int filesystem_get_fullpath(uint64_t root,
                            char *result,
                            uint64_t maxlen,
                            const char *filename) {
    const char *root_path = filesystem_get_root_dir(root);

    return path_v0.path_join(result, maxlen, root_path, filename) ==
           (strlen(root_path) + strlen(filename) + 1);
}

struct vio *filesystem_open(uint64_t root,
                            const char *path,
                            enum vio_open_mode mode) {
    char fullm_path[MAX_PATH_LEN] = {0};

    if (!filesystem_get_fullpath(root, fullm_path,
                                 sizeof(fullm_path) / sizeof(char), path)) {
        return NULL;
    }

    struct vio *file = vio_api_v0.from_file(fullm_path, mode,
                                            memory_api_v0.main_allocator());

    if (!file) {
        log_api_v0.log_error(LOG_WHERE, "Could not load file %s", fullm_path);
        return NULL;
    }

    return file;
}

void filesystem_close(struct vio *file) {
    vio_api_v0.close(file);
}

int filesystem_create_directory(uint64_t root,
                                const char *path) {
    char fullm_path[MAX_PATH_LEN] = {0};

    if (!filesystem_get_fullpath(root, fullm_path,
                                 sizeof(fullm_path) / sizeof(char), path)) {
        return 0;
    }

    return path_v0.dir_make_path(fullm_path);
}


void filesystem_listdir(uint64_t root,
                        const char *path,
                        const char *filter,
                        string_array_t *files,
                        struct allocator *allocator) {

    char fullm_path[MAX_PATH_LEN] = {0};
    if (!filesystem_get_fullpath(root, fullm_path,
                                 sizeof(fullm_path) / sizeof(char), path)) {
        return;
    }

    path_v0.dir_list(fullm_path, 1, files, allocator);
}

void filesystem_listdir_free(string_array_t *files,
                             struct allocator *allocator) {
    path_v0.dir_list_free(files, allocator);
}


time_t filesystem_get_file_mtime(uint64_t root,
                                 const char *path) {
    char fullm_path[MAX_PATH_LEN] = {0};
    if (!filesystem_get_fullpath(root, fullm_path,
                                 sizeof(fullm_path) / sizeof(char), path)) {
        return 0;
    }

    return path_v0.file_mtime(fullm_path);
}

static void _init_api(struct api_v0 *api) {
    static struct filesystem_api_v0 _api = {0};
    _api.filesystem_get_root_dir = filesystem_get_root_dir;
    _api.filesystem_open = filesystem_open;
    _api.filesystem_map_root_dir = filesystem_map_root_dir;
    _api.filesystem_close = filesystem_close;
    _api.filesystem_listdir = filesystem_listdir;
    _api.filesystem_listdir_free = filesystem_listdir_free;
    _api.filesystem_create_directory = filesystem_create_directory;
    _api.filesystem_get_file_mtime = filesystem_get_file_mtime;
    _api.filesystem_get_fullpath = filesystem_get_fullpath;

    api->register_api("filesystem_api_v0", &_api);
}


static void _init(struct api_v0 *api) {
    GET_API(api, memory_api_v0);
    GET_API(api, path_v0);
    GET_API(api, vio_api_v0);
    GET_API(api, log_api_v0);

    _G = (struct G) {0};

    log_api_v0.log_debug(LOG_WHERE, "Init");
}

static void _shutdown() {
    log_api_v0.log_debug(LOG_WHERE, "Shutdown");

    for (int i = 0; i < MAX_ROOTS; ++i) {
        if (_G.rootmap.path[i] == 0) {
            continue;
        }

        CETECH_DEALLOCATE(memory_api_v0.main_allocator(), _G.rootmap.path[i]);
    }

    _G = (struct G) {0};
}


void *filesystem_get_module_api(int api) {

    switch (api) {
        case PLUGIN_EXPORT_API_ID: {
            static struct module_api_v0 module = {0};

            module.init = _init;
            module.init_api = _init_api;
            module.shutdown = _shutdown;

            return &module;
        }

        default:
            return NULL;
    }

}
