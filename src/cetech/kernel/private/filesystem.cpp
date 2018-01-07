//==============================================================================
// Includes
//==============================================================================

#include <cetech/kernel/api_system.h>
#include <cetech/kernel/path.h>
#include <cetech/kernel/log.h>
#include <cetech/kernel/vio.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/filesystem.h>
#include <cetech/kernel/module.h>
#include <cstdlib>
#include <cetech/kernel/watchdog.h>
#include <celib/eventstream.inl>
#include "celib/map.inl"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_vio_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_watchdog_a0);

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

struct fs_mount_point {
    char* root_path;
    ct_watchdog* wd;
};

struct fs_root {
    Array<fs_mount_point> mount_points;
    celib::EventStream event_stream;
};

static struct FilesystemGlobals {
    Map<uint32_t> root_map;
    Array<fs_root> roots;
} FilesystemGlobals;

//==============================================================================
// Interface
//==============================================================================

uint32_t new_fs_root(uint64_t root) {
    cel_alloc *a = ct_memory_a0.main_allocator();

    uint32_t new_idx = array::size(_G.roots);

    array::push_back(_G.roots, {});

    fs_root *root_inst = &_G.roots[new_idx];
    root_inst->event_stream.init(a);
    root_inst->mount_points.init(a);

    map::set(_G.root_map, root, new_idx);

    return new_idx;
}

fs_root* get_fs_root(uint64_t root) {
    uint32_t idx = map::get(_G.root_map, root, UINT32_MAX);

    if(idx == UINT32_MAX) {
        return NULL;
    }

    fs_root* fs_inst= &_G.roots[idx];
    return  fs_inst;
}

fs_root* get_or_crate_root(uint64_t root) {
    uint32_t idx = map::get(_G.root_map, root, UINT32_MAX);

    if(idx == UINT32_MAX) {
        uint32_t root_idx = new_fs_root(root);
        return &_G.roots[root_idx];
    }

    fs_root* fs_inst= &_G.roots[idx];
    return  fs_inst;
}


uint32_t new_fs_mount(uint64_t root, fs_mount_point mp) {
    fs_root* fs_inst= get_or_crate_root(root);

    auto& mount_points = fs_inst->mount_points;

    uint32_t new_idx = array::size(mount_points);
    array::push_back(mount_points, mp);
    return new_idx;
}


namespace filesystem {
    void map_root_dir(uint64_t root,
                      const char *base_path,
                      bool watch) {
        cel_alloc *a = ct_memory_a0.main_allocator();

        fs_mount_point mp = {};

        if (watch) {
            ct_watchdog *wd = ct_watchdog_a0.create(a);
            wd->add_dir(wd->inst, base_path, true);

            mp.wd = wd;
        }

        mp.root_path = ct_memory_a0.str_dup(base_path, a);
        new_fs_mount(root, mp);
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

        fs_root* fs_inst= get_fs_root(root);
        const uint32_t mp_count = array::size(fs_inst->mount_points);

        for (uint32_t i = 0; i < mp_count; ++i) {
            fs_mount_point *mp = &fs_inst->mount_points[i];

            char *fullpath = ct_path_a0.join(allocator, 2, mp->root_path, filename);

            if (((!test_dir) && exist(fullpath)) ||
                (test_dir && exist_dir(fullpath))) {
                return fullpath;
            }

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

        Array<char *> all_files(a);

        fs_root* fs_inst= get_fs_root(root);
        const uint32_t mp_count = array::size(fs_inst->mount_points);

        for (uint32_t i = 0; i < mp_count; ++i) {
            fs_mount_point* mp = &fs_inst->mount_points[i];

            const char *mount_point_dir = mp->root_path;
            uint32_t mount_point_dir_len = strlen(mount_point_dir);
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


    int64_t get_file_mtime(uint64_t root,
                          const char *path) {
        auto a = ct_memory_a0.main_allocator();

        char *full_path = get_full_path(root, a, path, false);

        time_t ret = ct_path_a0.file_mtime(full_path);

        CEL_FREE(a, full_path);
        return ret;
    }

    void clean_events(fs_root *fs_inst) {
        auto *wd_it = celib::eventstream::begin<ct_watchdog_ev_header>(
                fs_inst->event_stream);
        const auto *wd_end = celib::eventstream::end<ct_watchdog_ev_header>(
                fs_inst->event_stream);

        while (wd_it != wd_end) {
            if (wd_it->type == CT_WATCHDOG_EVENT_FILE_MODIFIED) {
                ct_wd_ev_file_write_end *ev = (ct_wd_ev_file_write_end *)wd_it;
                CEL_FREE(ct_memory_a0.main_allocator(), ev->filename);
//              CEL_FREE(ct_memory_a0.main_allocator(), ev->dir);
            }

            wd_it = celib::eventstream::next<ct_watchdog_ev_header>(wd_it);
        }

        celib::eventstream::clear(fs_inst->event_stream);
    }

    void check_wd() {
        cel_alloc *alloc = ct_memory_a0.main_allocator();
        const uint32_t root_count = array::size(_G.roots);

        for (uint32_t i = 0; i < root_count; ++i) {
            fs_root *fs_inst = &_G.roots[i];
            const uint32_t mp_count = array::size(fs_inst->mount_points);

            clean_events(fs_inst);

            for (uint32_t j = 0; j < mp_count; ++j) {
                fs_mount_point* mp = &fs_inst->mount_points[j];
                const uint32_t root_path_len = strlen(mp->root_path);

                auto *wd = mp->wd;
                if(!wd) {
                    continue;
                }

                wd->fetch_events(wd->inst);

                auto *wd_it = wd->event_begin(wd->inst);
                const auto *wd_end = wd->event_end(wd->inst);

                while (wd_it != wd_end) {
                    if (wd_it->type == CT_WATCHDOG_EVENT_FILE_MODIFIED) {
                        ct_wd_ev_file_write_end *ev = (ct_wd_ev_file_write_end *)wd_it;

                        ct_wd_ev_file_write_end new_ev = *ev;

                        new_ev.dir = ct_memory_a0.str_dup(ev->dir + root_path_len + 1, alloc);
                        new_ev.filename = ct_memory_a0.str_dup(ev->filename, alloc);

                        celib::eventstream::push<ct_watchdog_ev_header>(
                                fs_inst->event_stream,
                                CT_WATCHDOG_EVENT_FILE_MODIFIED,
                                new_ev);
                    }

                    wd_it = wd->event_next(wd->inst, wd_it);
                  }
            }
        }
    }

    ct_watchdog_ev_header *event_begin(uint64_t root) {
        fs_root* fs_inst= get_fs_root(root);

        return celib::eventstream::begin<ct_watchdog_ev_header>(
                fs_inst->event_stream);
    }

    ct_watchdog_ev_header *event_end(uint64_t root) {
        fs_root* fs_inst = get_fs_root(root);

        return celib::eventstream::end<ct_watchdog_ev_header>(fs_inst->event_stream);
    }

    ct_watchdog_ev_header *event_next(ct_watchdog_ev_header *header) {
        return celib::eventstream::next<ct_watchdog_ev_header>(header);
    }

void _get_full_path(uint64_t root, const char* path, char* fullpath, uint32_t max_len) {
    cel_alloc *a = ct_memory_a0.main_allocator();

    char* fp = get_full_path(root, a, path, false);

    if(strlen(fp) > max_len) {
        goto end;
    }

    strcpy(fullpath, fp);

    end:
    CEL_FREE(a, fp);
}

}
namespace filesystem_module {
    static ct_filesystem_a0 _api = {
            .open = filesystem::open,
            .map_root_dir = filesystem::map_root_dir,
            .close = filesystem::close,
            .listdir = filesystem::listdir,
            .listdir_free = filesystem::listdir_free,
            .listdir_iter = filesystem::listdir2,
            .create_directory = filesystem::create_directory,
            .file_mtime = filesystem::get_file_mtime,
            .check_wd = filesystem::check_wd,
            .event_begin = filesystem::event_begin,
            .event_end = filesystem::event_end,
            .event_next = filesystem::event_next,
            .get_full_path = filesystem::_get_full_path,
//            .fullpath = filesystem::get_fullpath
    };

    void _init_api(ct_api_a0 *api) {
        api->register_api("ct_filesystem_a0", &_api);
    }



    void _init(ct_api_a0 *api) {
        _init_api(api);

        _G = {};

        _G.root_map.init(ct_memory_a0.main_allocator());
        _G.roots.init(ct_memory_a0.main_allocator());

        ct_log_a0.debug(LOG_WHERE, "Init");
    }

    void _shutdown() {
        ct_log_a0.debug(LOG_WHERE, "Shutdown");

        _G.root_map.destroy();
        _G.roots.destroy();
    }

}

CETECH_MODULE_DEF(
        filesystem,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_path_a0);
            CETECH_GET_API(api, ct_vio_a0);
            CETECH_GET_API(api, ct_watchdog_a0);
            CETECH_GET_API(api, ct_log_a0);
        },
        {
            CEL_UNUSED(reload);
            filesystem_module::_init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);

            filesystem_module::_shutdown();
        }
)