//==============================================================================
// Includes
//==============================================================================

#include <cetech/api/api_system.h>
#include <cetech/os/path.h>
#include <cetech/log/log.h>
#include <cetech/os/vio.h>
#include <cetech/memory/memory.h>
#include <cetech/filesystem/filesystem.h>
#include <cetech/module/module.h>
#include <cstdlib>
#include "celib/map.inl"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_log_a0);

using namespace celib;

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
        multi_map::insert(_G.root_map, root,
                          ct_memory_a0.str_dup(base_path,
                                               ct_memory_a0.main_allocator()));
    }

    const char *get_root_dir(uint64_t root) {
        return map::get<char *>(_G.root_map, root, nullptr);
    }

    bool exist_dir(const char *full_path) {
        char path_buffer[4096];
        ct_path_a0.dir(path_buffer, full_path);
        return ct_path_a0.is_dir(path_buffer);
    }

    bool exist(const char *full_path) {
        ct_vio *f = ct_vio_a0.from_file(full_path, VIO_OPEN_READ);
        if (f != NULL) {
            f->close(f->inst);
            return true;
        }

        return false;
    }

    char *get_full_path(uint64_t root,
                        cel_alloc *allocator,
                        const char *filename,
                        bool test_dir) {

        auto it = multi_map::find_first(_G.root_map, root);
        while (it != nullptr) {
            char *fullpath = ct_path_a0.join(allocator, 2, it->value, filename);

            if (((!test_dir) && exist(fullpath)) ||
                (test_dir && exist_dir(fullpath))) {
                return fullpath;
            }

            it = multi_map::find_next(_G.root_map, it);
        }
        return NULL;
    }

    ct_vio *open(uint64_t root,
                 const char *path,
                 ct_fs_open_mode mode) {
        auto a = ct_memory_a0.main_allocator();

        char *full_path = get_full_path(root, a, path, mode == FS_OPEN_WRITE);

        ct_vio *file = ct_vio_a0.from_file(full_path,
                                           (ct_vio_open_mode) mode);

        if (!file) {
            ct_log_a0.error(LOG_WHERE, "Could not load file %s", full_path);
            CEL_FREE(a, full_path);
            return NULL;
        }

        CEL_FREE(a, full_path);
        return file;
    }

    void close(ct_vio *file) {
        file->close(file->inst);
    }

    int create_directory(uint64_t root,
                         const char *path) {
        auto a = ct_memory_a0.main_allocator();


        char *full_path = get_full_path(root, a, path, true);

        int ret = ct_path_a0.make_path(full_path);
        CEL_FREE(a, full_path);

        return ret;
    }

    void listdir(uint64_t root,
                 const char *path,
                 const char *filter,
                 bool only_dir,
                 bool recursive,
                 char ***files,
                 uint32_t *count,
                 cel_alloc *allocator) {

        auto a = ct_memory_a0.main_allocator();

        auto it = multi_map::find_first(_G.root_map, root);

        Array<char *> all_files(a);

        while (it != nullptr) {
            const char *mount_point_dir = it->value;
            uint32_t mount_point_dir_len = strlen(it->value);
            char **_files;
            uint32_t _count;

            char *final_path = ct_path_a0.join(a, 2, mount_point_dir, path);
            ct_path_a0.list(final_path, filter, recursive, only_dir, &_files,
                            &_count, allocator);

            for (uint32_t i = 0; i < _count; ++i) {
                array::push_back(all_files,
                                 strdup(_files[i] + mount_point_dir_len + 1));
            }

            ct_path_a0.list_free(_files, _count, allocator);
            CEL_FREE(a, final_path);
            it = multi_map::find_next(_G.root_map, it);
        }

        char **result_files = CEL_ALLOCATE(a, char*, sizeof(char *) *
                                                     array::size(all_files));

        for (uint32_t i = 0; i < array::size(all_files); ++i) {
            result_files[i] = all_files[i];
        }

        *files = result_files;
        *count = array::size(all_files);
    }

    void listdir_free(char **files,
                      uint32_t count,
                      cel_alloc *allocator) {
        for (uint32_t i = 0; i < count; ++i) {
            free(files[i]);
        }

        CEL_FREE(allocator, files);
    }

    void listdir2(uint64_t root,
                  const char *path,
                  const char *filter,
                  bool only_dir,
                  bool recursive,
                  void (*on_item)(const char *path)) {

        char **files;
        uint32_t count;
        auto a = ct_memory_a0.main_allocator();

        listdir(root, path, filter, only_dir, recursive, &files, &count, a);

        for (uint32_t i = 0; i < count; ++i) {
            on_item(files[i]);
        }

        listdir_free(files, count, a);
    }


    time_t get_file_mtime(uint64_t root,
                          const char *path) {
        auto a = ct_memory_a0.main_allocator();

        char *full_path = get_full_path(root, a, path, false);

        time_t ret = ct_path_a0.file_mtime(full_path);

        CEL_FREE(a, full_path);
        return ret;
    }
}

namespace filesystem_module {
    static ct_filesystem_a0 _api = {
            .root_dir = filesystem::get_root_dir,
            .open = filesystem::open,
            .map_root_dir = filesystem::map_root_dir,
            .close = filesystem::close,
            .listdir = filesystem::listdir,
            .listdir_iter = filesystem::listdir2,
            .listdir_free = filesystem::listdir_free,
            .create_directory = filesystem::create_directory,
            .file_mtime = filesystem::get_file_mtime,
//            .fullpath = filesystem::get_fullpath
    };

    void _init_api(ct_api_a0 *api) {
        api->register_api("ct_filesystem_a0", &_api);
    }


    void _init(ct_api_a0 *api) {
        _init_api(api);

        _G = {};

        _G.root_map.init(ct_memory_a0.main_allocator());

        ct_log_a0.debug(LOG_WHERE, "Init");
    }

    void _shutdown() {
        ct_log_a0.debug(LOG_WHERE, "Shutdown");

        auto it = map::begin(_G.root_map);
        auto end_it = map::end(_G.root_map);

        while (it != end_it) {
            CEL_FREE(ct_memory_a0.main_allocator(), it->value);
            ++it;
        }

        _G.root_map.destroy();
    }

}

CETECH_MODULE_DEF(
        filesystem,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_vio_a0);
            CETECH_GET_API(api, ct_log_a0);
        },
        {
            filesystem_module::_init(api);
        },
        {
            CEL_UNUSED(api);

            filesystem_module::_shutdown();
        }
)