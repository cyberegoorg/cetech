//==============================================================================
// Includes
//==============================================================================

#include <stdlib.h>
#include <time.h>

#include <celib/memory/allocator.h>
#include <celib/api.h>

#include <celib/log.h>
#include <celib/memory/memory.h>
#include <celib/fs.h>
#include <celib/module.h>
#include <celib/containers/hash.h>
#include <celib/containers/buffer.h>
#include <celib/id.h>
#include <celib/os/path.h>
#include <celib/os/vio.h>

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

typedef struct fs_mount_point {
    char *root_path;
//    ce_watchdog_a0 *wd;
} fs_mount_point;

typedef struct fs_root {
    fs_mount_point *mount_points;
} fs_root;

static struct _G {
    ce_hash_t root_map;
    fs_root *roots;
    ce_alloc_t0 *allocator;
} _G;

//==============================================================================
// Interface
//==============================================================================

static uint32_t new_fs_root(uint64_t root) {
//    ce_alloc_t0 *a = ce_memory_a0->system;

    uint32_t new_idx = ce_array_size(_G.roots);

    ce_array_push(_G.roots, (fs_root) {}, _G.allocator);

//    fs_root *root_inst = &_G.roots[new_idx];

    ce_hash_add(&_G.root_map, root, new_idx, _G.allocator);

    return new_idx;
}

static struct fs_root *get_fs_root(uint64_t root) {
    uint32_t idx = ce_hash_lookup(&_G.root_map, root, UINT32_MAX);

    if (idx == UINT32_MAX) {
        return NULL;
    }

    fs_root *fs_inst = &_G.roots[idx];
    return fs_inst;
}

static struct fs_root *get_or_crate_root(uint64_t root) {
    uint32_t idx = ce_hash_lookup(&_G.root_map, root, UINT32_MAX);

    if (idx == UINT32_MAX) {
        uint32_t root_idx = new_fs_root(root);
        return &_G.roots[root_idx];
    }

    fs_root *fs_inst = &_G.roots[idx];
    return fs_inst;
}


static uint32_t new_fs_mount(uint64_t root,
                             struct fs_mount_point mp) {
    fs_root *fs_inst = get_or_crate_root(root);

    uint32_t new_idx = ce_array_size(fs_inst->mount_points);
    ce_array_push(fs_inst->mount_points, mp, _G.allocator);
    return new_idx;
}


static void map_root_dir(uint64_t root,
                         const char *base_path,
                         bool watch) {


    fs_mount_point mp = {};

    if (watch) {
//        struct ce_watchdog_a0 *wd = ce_os_a0->watchdog->create(_G.allocator);
//        wd->add_dir(wd->inst, base_path, true);

//        mp.wd = wd;
    }

    mp.root_path = ce_memory_a0->str_dup(base_path, _G.allocator);
    new_fs_mount(root, mp);
}

static bool exist_dir(const char *full_path) {
    char path_buffer[4096];
    ce_os_path_a0->dir(path_buffer, full_path);
    return ce_os_path_a0->is_dir(path_buffer);
}

static bool _exist(const char *full_path) {
    ce_vio_t0 *f = ce_os_vio_a0->from_file(full_path, VIO_OPEN_READ);
    if (f != NULL) {
        ce_os_vio_a0->close(f);
        return true;
    }

    return false;
}

static char *get_full_path(uint64_t root,
                           struct ce_alloc_t0 *allocator,
                           const char *filename,
                           bool test_dir) {

    fs_root *fs_inst = get_fs_root(root);
    const uint32_t mp_count = ce_array_size(fs_inst->mount_points);

    for (uint32_t i = 0; i < mp_count; ++i) {
        struct fs_mount_point *mp = &fs_inst->mount_points[i];

        char *fullpath = NULL;
        ce_os_path_a0->join(&fullpath, allocator, 2, mp->root_path,
                            filename);

        if (((!test_dir) && _exist(fullpath)) ||
            (test_dir && exist_dir(fullpath))) {
            return fullpath;
        }
    }

    return NULL;
}

static struct ce_vio_t0 *open(uint64_t root,
                              const char *path,
                              enum ce_fs_open_mode_e0 mode) {

    char *full_path = get_full_path(root, _G.allocator, path,
                                    mode == FS_OPEN_WRITE);

    ce_vio_t0 *file = ce_os_vio_a0->from_file(full_path,
                                              (enum ce_vio_open_mode) mode);

    if (!file) {
        ce_log_a0->error(LOG_WHERE, "Could not load file %s", full_path);
        ce_buffer_free(full_path, _G.allocator);
        return NULL;
    }

    ce_buffer_free(full_path, _G.allocator);
    return file;
}

static bool exist(uint64_t root,
                  const char *path) {
    char *full_path = get_full_path(root, _G.allocator, path, false);
    ce_vio_t0 *file = ce_os_vio_a0->from_file(full_path, VIO_OPEN_READ);

    if (file) {
        file->vt->close(file->inst);
        return true;
    }

    return false;
}

static void close(ce_vio_t0 *file) {
    ce_os_vio_a0->close(file);
}

static int create_directory(uint64_t root,
                            const char *path) {


    char *full_path = get_full_path(root, _G.allocator, path, true);

    int ret = ce_os_path_a0->make_path(full_path);
//    CE_FREE(_G.allocator, full_path);

    return ret;
}

static void listdir(uint64_t root,
                    const char *path,
                    const char *filter,
                    bool only_dir,
                    bool recursive,
                    char ***files,
                    uint32_t *count,
                    struct ce_alloc_t0 *allocator) {

    char **all_files = NULL;

    fs_root *fs_inst = get_fs_root(root);
    const uint32_t mp_count = ce_array_size(fs_inst->mount_points);

    for (uint32_t i = 0; i < mp_count; ++i) {
        struct fs_mount_point *mp = &fs_inst->mount_points[i];

        const char *mount_point_dir = mp->root_path;
        uint32_t mount_point_dir_len = strlen(mount_point_dir);
        char **_files;
        uint32_t _count;

        char *final_path = NULL;
        ce_os_path_a0->join(&final_path, allocator, 2, mount_point_dir,
                            path);
        ce_os_path_a0->list(final_path, (const char *[]) {filter}, 1,
                            recursive, only_dir, &_files, &_count, allocator);

        for (uint32_t i = 0; i < _count; ++i) {
            ce_array_push(all_files,
                          strdup(_files[i] + mount_point_dir_len + 1),
                          _G.allocator);
        }

        ce_os_path_a0->list_free(_files, _count, allocator);
        ce_buffer_free(final_path, allocator);
    }

    char **result_files = CE_ALLOC(allocator, char*,
                                   sizeof(char *) * ce_array_size(all_files));

    for (uint32_t i = 0; i < ce_array_size(all_files); ++i) {
        result_files[i] = all_files[i];
    }

    *files = result_files;
    *count = ce_array_size(all_files);


}

static void listdir_free(char **files,
                         uint32_t count,
                         struct ce_alloc_t0 *allocator) {
    for (uint32_t i = 0; i < count; ++i) {
        free(files[i]);
    }

    CE_FREE(allocator, files);
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

    time_t ret = ce_os_path_a0->file_mtime(full_path);

    ce_buffer_free(full_path, _G.allocator);

    return ret;
}


//static void check_wd() {
//    ce_alloc_t0 *alloc = ce_memory_a0->system;
//    const uint32_t root_count = ce_array_size(_G.roots);
//
//    for (uint32_t i = 0; i < root_count; ++i) {
//        fs_root *fs_inst = &_G.roots[i];
//        const uint32_t mp_count = ce_array_size(fs_inst->mount_points);
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
//                if (wd_it->type == CE_WATCHDOG_EVENT_FILE_MODIFIED) {
//                    ce_wd_ev_file_write_end *ev = (ce_wd_ev_file_write_end *) wd_it;
//
//                    ce_wd_ev_file_write_end new_ev = *ev;
//
//                    new_ev.dir = ce_memory_a0->str_dup(
//                            ev->dir + root_path_len + 1, alloc);
//                    new_ev.filename = ce_memory_a0->str_dup(ev->filename, alloc);
//
//                    celib::eventstream::push<ce_watchdog_ev_header>(
//                            fs_inst->event_stream,
//                            CE_WATCHDOG_EVENT_FILE_MODIFIED,
//                            new_ev);
//                }
//
//                wd_it = wd->event_next(wd, wd_it);
//            }
//        }
//    }
//}

//static ce_watchdog_ev_header *event_begin(uint64_t root) {
//    fs_root *fs_inst = get_fs_root(root);
//
//    return celib::eventstream::begin<ce_watchdog_ev_header>(
//            fs_inst->event_stream);
//}
//
//static ce_watchdog_ev_header *event_end(uint64_t root) {
//    fs_root *fs_inst = get_fs_root(root);
//
//    return celib::eventstream::end<ce_watchdog_ev_header>(
//            fs_inst->event_stream);
//}
//
//static ce_watchdog_ev_header *event_next(ce_watchdog_ev_header *header) {
//    return celib::eventstream::next<ce_watchdog_ev_header>(header);
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
    CE_FREE(_G.allocator, fp);
}

static struct ce_fs_a0 _api = {
        .open = open,
        .map_root_dir = map_root_dir,
        .close = close,
        .listdir = listdir,
        .listdir_free = listdir_free,
        .listdir_iter = listdir2,
        .create_directory = create_directory,
        .file_mtime = get_file_mtime,
        .get_full_path = _get_full_path,
        .exist = exist,
};


struct ce_fs_a0 *ce_fs_a0 = &_api;

void CE_MODULE_LOAD(filesystem)(struct ce_api_a0 *api,
                                int reload) {
    CE_UNUSED(reload);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    api->add_api(CE_FS_API, &_api, sizeof(_api));

    ce_log_a0->debug(LOG_WHERE, "Init");
}

void CE_MODULE_UNLOAD(filesystem)(struct ce_api_a0 *api,
                                  int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);

    ce_log_a0->debug(LOG_WHERE, "Shutdown");

    ce_array_free(_G.roots, _G.allocator);
    ce_hash_free(&_G.root_map, _G.allocator);
}
