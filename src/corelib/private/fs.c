//==============================================================================
// Includes
//==============================================================================

#include <stdlib.h>

#include <corelib/api_system.h>
#include <corelib/os.h>
#include <corelib/log.h>
#include <corelib/memory.h>
#include <corelib/fs.h>
#include <corelib/module.h>
#include <corelib/hash.inl>
#include <corelib/buffer.inl>

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
    char *root_path;
    struct ct_watchdog *wd;
};

struct fs_root {
    struct fs_mount_point *mount_points;
};

static struct _G {
    struct ct_hash_t root_map;
    struct fs_root *roots;
    struct ct_alloc *allocator;
} _G;

//==============================================================================
// Interface
//==============================================================================

static uint32_t new_fs_root(uint64_t root) {
//    ct_alloc *a = ct_memory_a0->main_allocator();

    uint32_t new_idx = ct_array_size(_G.roots);

    ct_array_push(_G.roots, (struct fs_root) {0}, _G.allocator);

//    fs_root *root_inst = &_G.roots[new_idx];

    ct_hash_add(&_G.root_map, root, new_idx, _G.allocator);

    return new_idx;
}

static struct fs_root *get_fs_root(uint64_t root) {
    uint32_t idx = ct_hash_lookup(&_G.root_map, root, UINT32_MAX);

    if (idx == UINT32_MAX) {
        return NULL;
    }

    struct fs_root *fs_inst = &_G.roots[idx];
    return fs_inst;
}

static struct fs_root *get_or_crate_root(uint64_t root) {
    uint32_t idx = ct_hash_lookup(&_G.root_map, root, UINT32_MAX);

    if (idx == UINT32_MAX) {
        uint32_t root_idx = new_fs_root(root);
        return &_G.roots[root_idx];
    }

    struct fs_root *fs_inst = &_G.roots[idx];
    return fs_inst;
}


static uint32_t new_fs_mount(uint64_t root,
                             struct fs_mount_point mp) {
    struct fs_root *fs_inst = get_or_crate_root(root);

    uint32_t new_idx = ct_array_size(fs_inst->mount_points);
    ct_array_push(fs_inst->mount_points, mp, _G.allocator);
    return new_idx;
}


static void map_root_dir(uint64_t root,
                         const char *base_path,
                         bool watch) {


    struct fs_mount_point mp = {};

    if (watch) {
        struct ct_watchdog *wd = ct_os_a0->watchdog_a0->create(_G.allocator);
        wd->add_dir(wd->inst, base_path, true);

        mp.wd = wd;
    }

    mp.root_path = ct_memory_a0->str_dup(base_path, _G.allocator);
    new_fs_mount(root, mp);
}

static bool exist_dir(const char *full_path) {
    char path_buffer[4096];
    ct_os_a0->path_a0->dir(path_buffer, full_path);
    return ct_os_a0->path_a0->is_dir(path_buffer);
}

static bool exist(const char *full_path) {
    struct ct_vio *f = ct_os_a0->vio_a0->from_file(full_path, VIO_OPEN_READ);
    if (f != NULL) {
        f->close(f);
        return true;
    }

    return false;
}

static char *get_full_path(uint64_t root,
                           struct ct_alloc *allocator,
                           const char *filename,
                           bool test_dir) {

    struct fs_root *fs_inst = get_fs_root(root);
    const uint32_t mp_count = ct_array_size(fs_inst->mount_points);

    for (uint32_t i = 0; i < mp_count; ++i) {
        struct fs_mount_point *mp = &fs_inst->mount_points[i];

        char *fullpath = NULL;
        ct_os_a0->path_a0->join(&fullpath, allocator, 2, mp->root_path,
                                filename);

        if (((!test_dir) && exist(fullpath)) ||
            (test_dir && exist_dir(fullpath))) {
            return fullpath;
        }
    }

    return NULL;
}

static struct ct_vio *open(uint64_t root,
                           const char *path,
                           enum ct_fs_open_mode mode) {

    char *full_path = get_full_path(root, _G.allocator, path,
                                    mode == FS_OPEN_WRITE);

    struct ct_vio *file = ct_os_a0->vio_a0->from_file(full_path,
                                                      (enum ct_vio_open_mode) mode);

    if (!file) {
        ct_log_a0->error(LOG_WHERE, "Could not load file %s", full_path);
        ct_buffer_free(full_path, _G.allocator);
        return NULL;
    }

    ct_buffer_free(full_path, _G.allocator);
    return file;
}

static void close(struct ct_vio *file) {
    file->close(file);
}

static int create_directory(uint64_t root,
                            const char *path) {


    char *full_path = get_full_path(root, _G.allocator, path, true);

    int ret = ct_os_a0->path_a0->make_path(full_path);
    CT_FREE(_G.allocator, full_path);

    return ret;
}

static void listdir(uint64_t root,
                    const char *path,
                    const char *filter,
                    bool only_dir,
                    bool recursive,
                    char ***files,
                    uint32_t *count,
                    struct ct_alloc *allocator) {

    char **all_files = NULL;

    struct fs_root *fs_inst = get_fs_root(root);
    const uint32_t mp_count = ct_array_size(fs_inst->mount_points);

    for (uint32_t i = 0; i < mp_count; ++i) {
        struct fs_mount_point *mp = &fs_inst->mount_points[i];

        const char *mount_point_dir = mp->root_path;
        uint32_t mount_point_dir_len = strlen(mount_point_dir);
        char **_files;
        uint32_t _count;

        char *final_path = NULL;
        ct_os_a0->path_a0->join(&final_path, allocator, 2, mount_point_dir,
                                path);
        ct_os_a0->path_a0->list(final_path, filter, recursive, only_dir,
                                &_files,
                                &_count, allocator);

        for (uint32_t i = 0; i < _count; ++i) {
            ct_array_push(all_files,
                          strdup(_files[i] + mount_point_dir_len + 1),
                          _G.allocator);
        }

        ct_os_a0->path_a0->list_free(_files, _count, allocator);
        ct_buffer_free(final_path, allocator);
    }

    char **result_files = CT_ALLOC(allocator, char*,
                                   sizeof(char *) * ct_array_size(all_files));

    for (uint32_t i = 0; i < ct_array_size(all_files); ++i) {
        result_files[i] = all_files[i];
    }

    *files = result_files;
    *count = ct_array_size(all_files);


}

static void listdir_free(char **files,
                         uint32_t count,
                         struct ct_alloc *allocator) {
    for (uint32_t i = 0; i < count; ++i) {
        free(files[i]);
    }

    CT_FREE(allocator, files);
}

static void listdir2(uint64_t root,
                     const char *path,
                     const char *filter,
                     bool only_dir,
                     bool recursive,
                     void (*on_item)(const char *path)) {

    char **files;
    uint32_t count;


    listdir(root, path, filter, only_dir, recursive, &files, &count,
            _G.allocator);

    for (uint32_t i = 0; i < count; ++i) {
        on_item(files[i]);
    }

    listdir_free(files, count, _G.allocator);
}


static int64_t get_file_mtime(uint64_t root,
                              const char *path) {


    char *full_path = get_full_path(root, _G.allocator, path, false);

    time_t ret = ct_os_a0->path_a0->file_mtime(full_path);

    ct_buffer_free(full_path, _G.allocator);

    return ret;
}


//static void check_wd() {
//    ct_alloc *alloc = ct_memory_a0->main_allocator();
//    const uint32_t root_count = ct_array_size(_G.roots);
//
//    for (uint32_t i = 0; i < root_count; ++i) {
//        fs_root *fs_inst = &_G.roots[i];
//        const uint32_t mp_count = ct_array_size(fs_inst->mount_points);
//
//        clean_events(fs_inst);
//
//        for (uint32_t j = 0; j < mp_count; ++j) {
//            fs_mount_point *mp = &fs_inst->mount_points[j];
//            const uint32_t root_path_len = strlen(mp->root_path);
//
//            auto *wd = mp->wd;
//            if (!wd) {
//                continue;
//            }
//
//            wd->fetch_events(wd->inst);
//
//            auto *wd_it = wd->event_begin(wd->inst);
//            const auto *wd_end = wd->event_end(wd->inst);
//
//            while (wd_it != wd_end) {
//                if (wd_it->type == CT_WATCHDOG_EVENT_FILE_MODIFIED) {
//                    ct_wd_ev_file_write_end *ev = (ct_wd_ev_file_write_end *) wd_it;
//
//                    ct_wd_ev_file_write_end new_ev = *ev;
//
//                    new_ev.dir = ct_memory_a0->str_dup(
//                            ev->dir + root_path_len + 1, alloc);
//                    new_ev.filename = ct_memory_a0->str_dup(ev->filename, alloc);
//
//                    celib::eventstream::push<ct_watchdog_ev_header>(
//                            fs_inst->event_stream,
//                            CT_WATCHDOG_EVENT_FILE_MODIFIED,
//                            new_ev);
//                }
//
//                wd_it = wd->event_next(wd, wd_it);
//            }
//        }
//    }
//}

//static ct_watchdog_ev_header *event_begin(uint64_t root) {
//    fs_root *fs_inst = get_fs_root(root);
//
//    return celib::eventstream::begin<ct_watchdog_ev_header>(
//            fs_inst->event_stream);
//}
//
//static ct_watchdog_ev_header *event_end(uint64_t root) {
//    fs_root *fs_inst = get_fs_root(root);
//
//    return celib::eventstream::end<ct_watchdog_ev_header>(
//            fs_inst->event_stream);
//}
//
//static ct_watchdog_ev_header *event_next(ct_watchdog_ev_header *header) {
//    return celib::eventstream::next<ct_watchdog_ev_header>(header);
//}

static void _get_full_path(uint64_t root,
                           const char *path,
                           char *fullpath,
                           uint32_t max_len) {


    char *fp = get_full_path(root, _G.allocator, path, false);

    if (strlen(fp) > max_len) {
        goto end;
    }

    strcpy(fullpath, fp);

    end:
    CT_FREE(_G.allocator, fp);
}

static struct ct_fs_a0 _api = {
        .open = open,
        .map_root_dir = map_root_dir,
        .close = close,
        .listdir = listdir,
        .listdir_free = listdir_free,
        .listdir_iter = listdir2,
        .create_directory = create_directory,
        .file_mtime = get_file_mtime,
        .get_full_path = _get_full_path,
};


struct ct_fs_a0 *ct_fs_a0 = &_api;

static void _init_api(struct ct_api_a0 *api) {
    api->register_api("ct_fs_a0", &_api);
}


static void _init(struct ct_api_a0 *api) {
    _init_api(api);

    _G = (struct _G) {
            .allocator = ct_memory_a0->main_allocator(),
    };

    ct_log_a0->debug(LOG_WHERE, "Init");
}

static void _shutdown() {
    ct_log_a0->debug(LOG_WHERE, "Shutdown");

    ct_array_free(_G.roots, _G.allocator);
    ct_hash_free(&_G.root_map, _G.allocator);
}

CETECH_MODULE_DEF(
        filesystem,
        {

        },
        {
            CT_UNUSED(reload);
            _init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);

            _shutdown();
        }
)