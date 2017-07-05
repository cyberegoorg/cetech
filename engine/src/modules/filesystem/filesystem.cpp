//==============================================================================
// Includes
//==============================================================================

#include <cetech/kernel/os.h>

#include <cetech/kernel/memory.h>
#include <cetech/modules/filesystem.h>
#include <cetech/kernel/config.h>
#include <cetech/kernel/log.h>
#include <cetech/modules/resource.h>
#include <cetech/modules/resource.h>

#include <cetech/kernel/api_system.h>
#include <cetech/celib/map.inl>

CETECH_DECL_API(memory_api_v0);
CETECH_DECL_API(os_path_v0);
CETECH_DECL_API(os_vio_api_v0);
CETECH_DECL_API(log_api_v0);

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


#define _G FilesystemGlobals
static struct FilesystemGlobals {
    Map<char *> root_map;
} FilesystemGlobals;



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

    char *get_fullpath(uint64_t root,
                       struct allocator *allocator,
                       const char *filename) {

        const char *root_path = get_root_dir(root);
        return os_path_v0.join(allocator, 2, root_path, filename);
    }

    struct os_vio *open(uint64_t root,
                        const char *path,
                        fs_open_mode mode) {
        auto a = memory_api_v0.main_allocator();

        char *full_path = get_fullpath(root, a, path);

        struct os_vio *file = os_vio_api_v0.from_file(full_path,
                                                      (vio_open_mode) mode,
                                                      a);

        if (!file) {
            log_api_v0.error(LOG_WHERE, "Could not load file %s", full_path);
            CETECH_FREE(a, full_path);
            return NULL;
        }


        CETECH_FREE(a, full_path);
        return file;
    }

    void close(struct os_vio *file) {
        os_vio_api_v0.close(file);
    }

    int create_directory(uint64_t root,
                         const char *path) {
        auto a = memory_api_v0.main_allocator();

        char *full_path = get_fullpath(root, a, path);

        int ret = os_path_v0.make_path(full_path);
        CETECH_FREE(a, full_path);

        return ret;
    }


    void listdir(uint64_t root,
                 const char *path,
                 const char *filter,
                 char ***files,
                 uint32_t *count,
                 struct allocator *allocator) {

        auto a = memory_api_v0.main_allocator();

        char *full_path = get_fullpath(root, a, path);

        os_path_v0.list(full_path, 1, files, count, allocator);

        CETECH_FREE(a, full_path);
    }

    void listdir_free(char **files,
                      uint32_t count,
                      struct allocator *allocator) {
        os_path_v0.list_free(files, count, allocator);
    }


    time_t get_file_mtime(uint64_t root,
                          const char *path) {
        auto a = memory_api_v0.main_allocator();

        char *full_path = get_fullpath(root, a, path);

        time_t ret = os_path_v0.file_mtime(full_path);

        CETECH_FREE(a, full_path);
        return ret;
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
        _init_api(api);

        CETECH_GET_API(api, memory_api_v0);
        CETECH_GET_API(api, os_path_v0);
        CETECH_GET_API(api, os_vio_api_v0);
        CETECH_GET_API(api, log_api_v0);

        _G = {0};

        _G.root_map.init(memory_api_v0.main_allocator());

        log_api_v0.debug(LOG_WHERE, "Init");
    }

    void _shutdown() {
        log_api_v0.debug(LOG_WHERE, "Shutdown");

        auto it = map::begin(_G.root_map);
        auto end_it = map::end(_G.root_map);

        while (it != end_it) {
            CETECH_FREE(memory_api_v0.main_allocator(), it->value);
            ++it;
        }

        _G.root_map.destroy();
    }

    extern "C" void filesystem_load_module(struct api_v0 *api) {
        _init(api);
    }

    extern "C" void filesystem_unload_module(struct api_v0 *api) {
        _shutdown();
    }

}