//==============================================================================
// Includes
//==============================================================================

#include <cetech/core/path.h>

#include <cetech/core/memory.h>
#include <cetech/modules/filesystem.h>
#include <cetech/core/config.h>
#include <cetech/modules/resource.h>
#include <cetech/core/module.h>
#include <cetech/core/api.h>
#include <cetech/celib/map2.inl>
#include <cetech/core/vio.h>

IMPORT_API(memory_api_v0);
IMPORT_API(path_v0);
IMPORT_API(vio_api_v0);
IMPORT_API(log_api_v0);

using namespace cetech;

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "filesystem"

#define MAX_PATH_LEN 128
#define MAX_ROOTS 32

//==============================================================================
// Global
//==============================================================================


static struct FilesystemGlobals {
    Map<char *> root_map;
} _G = {0};



//==============================================================================
// Interface
//==============================================================================

namespace filesystem {
    void map_root_dir(uint64_t root,
                      const char *base_path) {
        map::set(_G.root_map, root,
                 memory_api_v0.str_dup(base_path,
                                       memory_api_v0.main_allocator()));
    }

    const char *get_root_dir(uint64_t root) {
        return map::get<char *>(_G.root_map, root, nullptr);
    }

    int get_fullpath(uint64_t root,
                     char *result,
                     uint64_t maxlen,
                     const char *filename) {
        const char *root_path = get_root_dir(root);

        return path_v0.join(result, maxlen, root_path, filename) ==
               (strlen(root_path) + strlen(filename) + 1);
    }

    struct vio *open(uint64_t root,
                     const char *path,
                     fs_open_mode mode) {
        char fullm_path[MAX_PATH_LEN] = {0};

        if (!get_fullpath(root, fullm_path,
                          sizeof(fullm_path) / sizeof(char), path)) {
            return NULL;
        }

        struct vio *file = vio_api_v0.from_file(fullm_path,
                                                (vio_open_mode) mode,
                                                memory_api_v0.main_allocator());

        if (!file) {
            log_api_v0.error(LOG_WHERE, "Could not load file %s", fullm_path);
            return NULL;
        }

        return file;
    }

    void close(struct vio *file) {
        vio_api_v0.close(file);
    }

    int create_directory(uint64_t root,
                         const char *path) {
        char fullm_path[MAX_PATH_LEN] = {0};

        if (!get_fullpath(root, fullm_path,
                          sizeof(fullm_path) / sizeof(char), path)) {
            return 0;
        }

        return path_v0.make_path(fullm_path);
    }


    void listdir(uint64_t root,
                 const char *path,
                 const char *filter,
                 char ***files,
                 uint32_t *count,
                 struct allocator *allocator) {

        char fullm_path[MAX_PATH_LEN] = {0};
        if (!get_fullpath(root, fullm_path,
                          sizeof(fullm_path) / sizeof(char), path)) {
            return;
        }

        path_v0.list(fullm_path, 1, files, count, allocator);
    }

    void listdir_free(char **files,
                      uint32_t count,
                      struct allocator *allocator) {
        path_v0.list_free(files, count, allocator);
    }


    time_t get_file_mtime(uint64_t root,
                          const char *path) {
        char fullm_path[MAX_PATH_LEN] = {0};
        if (!get_fullpath(root, fullm_path,
                          sizeof(fullm_path) / sizeof(char), path)) {
            return 0;
        }

        return path_v0.file_mtime(fullm_path);
    }
}

namespace filesystem_module {
    static struct filesystem_api_v0 _api = {
            .root_dir = filesystem::get_root_dir,
            .open = filesystem::open,
            .map_root_dir = filesystem::map_root_dir,
            .close = filesystem::close,
            .listdir = filesystem::listdir,
            .listdir_free = filesystem::listdir_free,
            .create_directory = filesystem::create_directory,
            .file_mtime = filesystem::get_file_mtime,
            .fullpath = filesystem::get_fullpath
    };

    void _init_api(struct api_v0 *api) {
        api->register_api("filesystem_api_v0", &_api);
    }


    void _init(struct api_v0 *api) {
        GET_API(api, memory_api_v0);
        GET_API(api, path_v0);
        GET_API(api, vio_api_v0);
        GET_API(api, log_api_v0);

        _G = {0};

        _G.root_map.init(memory_api_v0.main_allocator());

        log_api_v0.debug(LOG_WHERE, "Init");
    }

    void _shutdown() {
        log_api_v0.debug(LOG_WHERE, "Shutdown");

        auto it = map::begin(_G.root_map);
        auto end_it = map::end(_G.root_map);

        while (it != end_it) {
            CETECH_DEALLOCATE(memory_api_v0.main_allocator(), it->value);
            ++it;
        }

        _G = {0};
    }

    extern "C" void *filesystem_get_module_api(int api) {

        switch (api) {
            case PLUGIN_EXPORT_API_ID: {
                static struct module_export_api_v0 module = {0};

                module.init = _init;
                module.init_api = _init_api;
                module.shutdown = _shutdown;

                return &module;
            }

            default:
                return NULL;
        }

    }

}