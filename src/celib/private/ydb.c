#include <celib/api_system.h>
#include <celib/config.h>
#include <celib/memory.h>
#include <celib/module.h>
#include <celib/yng.h>
#include <celib/hashlib.h>
#include <celib/log.h>
#include <celib/os.h>
#include <celib/ydb.h>
#include <celib/fs.h>


#include <celib/fmath.inl>
#include <celib/hash.inl>
#include <celib/buffer.inl>

#define _G ydb_global
#define LOG_WHERE "ydb"

static struct _G {
    struct ce_hash_t document_cache_map;
    struct ce_hash_t obj_cache_map;
    struct ce_yng_doc **document_cache;
    char **document_path;

    struct ce_spinlock cache_lock;
    struct ce_hash_t modified_files_set;
    struct ce_alloc *allocator;
} _G;

void expire_document_in_cache(const char *path,
                              uint64_t path_key) {
    uint32_t idx = ce_hash_lookup(&_G.document_cache_map, path_key, UINT32_MAX);
    if (UINT32_MAX == idx) {
        return;
    }

    ce_os_a0->thread->spin_lock(&_G.cache_lock);
    struct ce_yng_doc *doc = _G.document_cache[idx];
    ce_hash_remove(&_G.document_cache_map, path_key);
    ce_yng_a0->destroy(doc);
    ce_os_a0->thread->spin_unlock(&_G.cache_lock);
}

uint64_t load_obj_to_cache(const char *path,
                           uint64_t path_key) {

    const uint64_t fs_root = ce_id_a0->id64("source");

    ce_log_a0->debug(LOG_WHERE, "Load file %s to cache", path);

    struct ce_vio *f = ce_fs_a0->open(fs_root, path, FS_OPEN_READ);

    if (!f) {
        ce_log_a0->error(LOG_WHERE, "Could not read file %s", path);
        return 0;
    }

    uint64_t obj = ce_yng_a0->cdb_from_vio(f, _G.allocator);
    ce_fs_a0->close(f);

    if (!obj) {
        ce_log_a0->error(LOG_WHERE, "Could not load yaml to cdb parse file %s", path);
        return 0;
    }

    ce_os_a0->thread->spin_lock(&_G.cache_lock);
    ce_hash_add(&_G.obj_cache_map, path_key, obj, _G.allocator);
    ce_os_a0->thread->spin_unlock(&_G.cache_lock);

    return obj;
}

uint64_t get_obj(const char *path) {
    uint64_t path_key = ce_id_a0->id64(path);

    ce_os_a0->thread->spin_lock(&_G.cache_lock);
    uint64_t obj = ce_hash_lookup(&_G.obj_cache_map, path_key, 0);
    ce_os_a0->thread->spin_unlock(&_G.cache_lock);

    if (!obj) {
        obj = load_obj_to_cache(path, path_key);
    }

    return obj;
};

void free(const char *path) {
    CE_UNUSED(path);
    //expire_document_in_cache(path, CE_ID64_0(path));

    // TODO: ref counting
}


struct out_keys_s {
    uint64_t *keys;
    uint32_t max_keys;
    uint32_t *count;
};


void modified(const char *path) {
    uint64_t hash = ce_id_a0->id64(path);
    ce_hash_add(&_G.modified_files_set, hash, true, _G.allocator);
}

void unmodified(const char *path) {
    uint64_t hash = ce_id_a0->id64(path);
    ce_hash_remove(&_G.modified_files_set, hash);
}

void parent_files(const char *path,
                  const char ***files,
                  uint32_t *count) {
    *count = 0;

//    struct ce_yng_doc *d = get(path);
//
//    if (!d) {
//        *files = NULL;
//        *count = 0;
//        return;
//    }
//
//    d->parent_files(d, files, count);
}

//void check_fs() {
//    ce_alloc *alloc = ce_memory_a0->system;
//
//    static uint64_t root = CE_ID64_0("source", 0x921f1370045bad6eULL);
//
//    auto *wd_it = ce_fs_a0->event_begin(root);
//    const auto *wd_end = ce_fs_a0->event_end(root);
//
//    while (wd_it != wd_end) {
//        if (wd_it->type == CE_WATCHDOG_EVENT_FILE_MODIFIED) {
//            ce_wd_ev_file_write_end *ev = (ce_wd_ev_file_write_end *) wd_it;
//
//            char *path = NULL;
//            ce_os_path_a0->join(&path, alloc, 2, ev->dir, ev->filename);
//
//            uint64_t path_key = CE_ID64_0(path);
//
//            if (ce_hash_contain(&_G.document_cache_map, path_key)) {
//                ce_log_a0->debug(LOG_WHERE, "Reload cached file %s", path);
//
//                expire_document_in_cache(path, path_key);
//                load_to_cache(path, path_key);
//            }
//
//            ce_buffer_free(path, alloc);
//        }
//
//        wd_it = ce_fs_a0->event_next(wd_it);
//    }
//}


void save(const char *path) {
    struct ce_vio *f = ce_fs_a0->open(ce_id_a0->id64("source"), path,
                                      FS_OPEN_WRITE);

    if (!f) {
        ce_log_a0->error(LOG_WHERE, "Could not read file %s", path);
        return;
    }

//    struct ce_yng_doc *d = get(path);
//    ce_yng_a0->save_to_vio(_G.allocator, f, d);

    ce_fs_a0->close(f);
    unmodified(path);
}


void save_all_modified() {
    for (int i = 0; i < _G.document_cache_map.n; ++i) {
        if (!ce_hash_contain(&_G.modified_files_set,
                             _G.document_cache_map.keys[i])) {
            continue;
        }

        save(_G.document_path[_G.document_cache_map.values[i]]);
        return;
    }
}

static struct ce_ydb_a0 ydb_api = {
        .get_obj = get_obj,
        .free = free,
        .parent_files = parent_files,
        .save = save,
        .save_all_modified = save_all_modified,
};

struct ce_ydb_a0 *ce_ydb_a0 = &ydb_api;

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {.allocator = ce_memory_a0->system};

    api->register_api("ce_ydb_a0", &ydb_api);
}

static void _shutdown() {
    for (int i = 0; i < ce_array_size(_G.document_cache); ++i) {
        ce_yng_a0->destroy(_G.document_cache[i]);
    }

    ce_array_free(_G.document_path, _G.allocator);
    ce_array_free(_G.document_cache, _G.allocator);
    ce_hash_free(&_G.document_cache_map, _G.allocator);
    ce_hash_free(&_G.modified_files_set, _G.allocator);

    _G = (struct _G) {};
}

CE_MODULE_DEF(
        ydb,
        {

        },
        {
            CE_UNUSED(reload);
            _init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);
            _shutdown();
        }
)