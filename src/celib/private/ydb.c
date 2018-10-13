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

struct ce_yng_doc *load_to_cache(const char *path,
                          uint64_t path_key) {

    const uint64_t fs_root = ce_id_a0->id64("source");

    ce_log_a0->debug(LOG_WHERE, "Load file %s to cache", path);

    struct ce_yng_doc *doc;
    struct ce_vio *f = ce_fs_a0->open(fs_root, path, FS_OPEN_READ);

    if (!f) {
        ce_log_a0->error(LOG_WHERE, "Could not read file %s", path);
        return NULL;
    }

    doc = ce_yng_a0->from_vio(f, _G.allocator);
    ce_fs_a0->close(f);

    if (!doc) {
        ce_log_a0->error(LOG_WHERE, "Could not parse file %s", path);
        return NULL;
    }

    ce_os_a0->thread->spin_lock(&_G.cache_lock);
    uint32_t idx = ce_hash_lookup(&_G.document_cache_map, path_key, UINT32_MAX);
    if (UINT32_MAX == idx) {
        ce_array_push(_G.document_cache, doc, _G.allocator);
        ce_array_push(_G.document_path,
                      ce_memory_a0->str_dup(path, _G.allocator), _G.allocator);
        ce_hash_add(&_G.document_cache_map, path_key,
                    ce_array_size(_G.document_cache) - 1, _G.allocator);
    } else {
        _G.document_cache[idx] = doc;
    }
    ce_os_a0->thread->spin_unlock(&_G.cache_lock);

    return doc;
}

struct ce_yng_doc *get(const char *path) {
    ce_os_a0->thread->spin_lock(&_G.cache_lock);

    uint64_t path_key = ce_id_a0->id64(path);

    uint32_t idx = ce_hash_lookup(&_G.document_cache_map, path_key, UINT32_MAX);
    if (UINT32_MAX == idx) {
        ce_os_a0->thread->spin_unlock(&_G.cache_lock);
        return load_to_cache(path, path_key);
    }

    struct ce_yng_doc *doc;
    doc = _G.document_cache[idx];
    ce_os_a0->thread->spin_unlock(&_G.cache_lock);
    return doc;
};

void free(const char *path) {
    CE_UNUSED(path);
    //expire_document_in_cache(path, CE_ID64_0(path));

    // TODO: ref counting
}

struct ce_yng_node get_first_node_recursive(const char *path,
                                     const uint64_t *keys,
                                     uint32_t keys_count,
                                     uint32_t max_depth) {

    struct ce_yng_doc *d = get(path);

    uint64_t result_key = ce_yng_a0->combine_key(keys, keys_count);

    struct ce_yng_node n = d->get(d, result_key);
    if (n.idx) {
        return n;
    }

    return (struct ce_yng_node){} ;
    // Find parent
    // A.B.C , parent, A.parent, A.B.parent
    const char *parent_file_str;
    uint64_t tmp_keys[keys_count];
    memcpy(tmp_keys, keys, sizeof(uint64_t) * keys_count);

    const uint64_t PREFAB_KEY = ce_yng_a0->key("PREFAB");
    uint64_t tmp_key;

    for (uint64_t i = 0; i < max_depth; ++i) {
        tmp_key = tmp_keys[i];

        tmp_keys[i] = PREFAB_KEY;
        uint64_t k = ce_yng_a0->combine_key(tmp_keys, i + 1);

        parent_file_str = d->get_str(d, k, "");

        if ('\0' != parent_file_str[0]) {
            return get_first_node_recursive(parent_file_str, keys + i,
                                            keys_count - i,
                                            max_depth);
        }

        tmp_keys[i] = tmp_key;
    }

    return (struct ce_yng_node){};
}


struct out_keys_s {
    uint64_t *keys;
    uint32_t max_keys;
    uint32_t *count;
};

static void _foreach_keys(struct ce_yng_node k,
                          struct ce_yng_node v,
                            void *data) {
    CE_UNUSED(v);

    struct out_keys_s *out = (struct out_keys_s *) data;

    const char *string = k.d->as_string(k.d, k, NULL);
    if (!string) {
        return;
    }

    uint64_t str_key = ce_id_a0->id64(string);

    const uint32_t s = *out->count;
    for (uint32_t i = 0; i < s; ++i) {
        if (out->keys[i] == str_key) {
            return;
        }
    }

    out->keys[(*out->count)++] = str_key;
}

void get_map_keys(const char *path,
                  uint64_t *keys,
                  uint32_t keys_count,
                  uint64_t *map_keys,
                  uint32_t max_map_keys,
                  uint32_t *map_keys_count) {

    struct out_keys_s out_keys = {
            .keys = map_keys,
            .max_keys = max_map_keys,
            .count = map_keys_count,
    };

    struct ce_yng_doc *d = get(path);

    uint64_t result_key = ce_yng_a0->combine_key(keys, keys_count);

    struct ce_yng_node n = d->get(d, result_key);
    if (n.idx) {
        d->foreach_dict_node(d, n, _foreach_keys, &out_keys);
    }
    return;
    // Find parent
    // A.B.C , parent, A.parent, A.B.parent
    const char *parent_file_str;
    uint64_t tmp_keys[keys_count];
    memcpy(tmp_keys, keys, sizeof(uint64_t) * keys_count);

    const uint64_t PREFAB_KEY = ce_yng_a0->key("PREFAB");
    uint64_t tmp_key;

    for (uint64_t i = 0; i < keys_count; ++i) {
        tmp_key = tmp_keys[i];

        tmp_keys[i] = PREFAB_KEY;
        uint64_t k = ce_yng_a0->combine_key(tmp_keys, i + 1);

        parent_file_str = d->get_str(d, k, "");

        if ('\0' != parent_file_str[0]) {
            get_map_keys(parent_file_str, keys + i, keys_count - i, map_keys,
                         max_map_keys, map_keys_count);
        }

        tmp_keys[i] = tmp_key;
    }
}


bool has_key(const char *path,
             uint64_t *keys,
             uint32_t keys_count) {
    return get_first_node_recursive(path, keys, keys_count, keys_count).idx > 0;
}

const char *get_string(const char *path,
                       uint64_t *keys,
                       uint32_t keys_count,
                       const char *defaultt) {
    struct ce_yng_node n = get_first_node_recursive(path, keys, keys_count,
                                             keys_count);

    if (0 == n.idx) {
        return defaultt;
    }

    return n.d->as_string(n.d, n, defaultt);
}

float get_float(const char *path,
                uint64_t *keys,
                uint32_t keys_count,
                float defaultt) {
    struct ce_yng_node n = get_first_node_recursive(path, keys, keys_count,
                                             keys_count);

    if (0 == n.idx) {
        return defaultt;
    }

    return n.d->as_float(n.d, n, defaultt);
}

bool get_bool(const char *path,
              uint64_t *keys,
              uint32_t keys_count,
              bool defaultt) {
    struct ce_yng_node n = get_first_node_recursive(path, keys, keys_count,
                                             keys_count);

    if (0 == n.idx) {
        return defaultt;
    }

    return n.d->as_bool(n.d, n, defaultt);
}


void get_vec3(const char *path,
              uint64_t *keys,
              uint32_t keys_count,
              float v[3],
              float defaultt[3]) {
    struct ce_yng_node n = get_first_node_recursive(path, keys, keys_count,
                                             keys_count);

    if (0 == n.idx) {
        ce_vec3_move(v, defaultt);
        return;
    }


    n.d->as_vec3(n.d, n, v);
}

void get_vec4(const char *path,
              uint64_t *keys,
              uint32_t keys_count,
              float v[4],
              float defaultt[4]) {
    struct ce_yng_node n = get_first_node_recursive(path, keys, keys_count,
                                             keys_count);

    if (0 == n.idx) {
        ce_vec4_move(v, defaultt);
        return;
    }


    n.d->as_vec4(n.d, n, v);
}

void get_mat4(const char *path,
              uint64_t *keys,
              uint32_t keys_count,
              float v[16],
              float defaultt[16]) {
    struct ce_yng_node n = get_first_node_recursive(path, keys, keys_count,
                                             keys_count);

    if (0 == n.idx) {
        ce_mat4_move(v, defaultt);
        return;
    }

    n.d->as_mat4(n.d, n, v);
}


void modified(const char *path) {
    uint64_t hash = ce_id_a0->id64(path);
    ce_hash_add(&_G.modified_files_set, hash, true, _G.allocator);
}

void unmodified(const char *path) {
    uint64_t hash = ce_id_a0->id64(path);
    ce_hash_remove(&_G.modified_files_set, hash);
}

void set_float(const char *path,
               uint64_t *keys,
               uint32_t keys_count,
               float value) {
    struct ce_yng_doc *d = get(path);
    uint64_t key = ce_yng_a0->combine_key(keys, keys_count);

    struct ce_yng_node n = d->get(d, key);

    if (!n.idx) {
        const char *str_keys[keys_count];
        for (int i = 0; i < keys_count; ++i) {
            str_keys[i] = ce_yng_a0->get_key(keys[i]);
        }

        n.d->create_tree_float(n.d, str_keys, keys_count, value);
    } else {
        n.d->set_float(n.d, n, value);

    }

    modified(path);
}

void set_bool(const char *path,
              const uint64_t *keys,
              uint32_t keys_count,
              bool value) {

    struct ce_yng_doc *d = get(path);
    uint64_t key = ce_yng_a0->combine_key(keys, keys_count);

    struct ce_yng_node n = d->get(d, key);

    if (!n.idx) {
        const char *str_keys[keys_count];
        for (int i = 0; i < keys_count; ++i) {
            str_keys[i] = ce_yng_a0->get_key(keys[i]);
        }

        n.d->create_tree_bool(n.d, str_keys, keys_count, value);

    } else {
        n.d->set_bool(n.d, n, value);
    }

    modified(path);
}


void set_string(const char *path,
                const uint64_t *keys,
                uint32_t keys_count,
                const char *value) {
    struct ce_yng_doc *d = get(path);
    uint64_t key = ce_yng_a0->combine_key(keys, keys_count);

    struct ce_yng_node n = d->get(d, key);

    if (!n.idx) {
        const char *str_keys[keys_count];
        for (int i = 0; i < keys_count; ++i) {
            str_keys[i] = ce_yng_a0->get_key(keys[i]);
        }

        n.d->create_tree_string(n.d, str_keys, keys_count, value);
    } else {
        n.d->set_str(n.d, n, value);
    }

    modified(path);
}

void set_vec3(const char *path,
              const uint64_t *keys,
              uint32_t keys_count,
              float *value) {

    struct ce_yng_doc *d = get(path);
    uint64_t key = ce_yng_a0->combine_key(keys, keys_count);

    struct ce_yng_node n = d->get(d, key);

    if (!n.idx) {
        const char *str_keys[keys_count];
        for (int i = 0; i < keys_count; ++i) {
            str_keys[i] = ce_yng_a0->get_key(keys[i]);
        }

        n.d->create_tree_vec3(n.d, str_keys, keys_count, value);
    } else {
        n.d->set_vec3(n.d, n, value);
    }

    modified(path);
}

void set_vec4(const char *path,
              const uint64_t *keys,
              uint32_t keys_count,
              float *value) {

    struct ce_yng_node n = get_first_node_recursive(path, keys, keys_count,
                                             keys_count);
    if (!n.idx) {
        return;
    }

    n.d->set_vec4(n.d, n, value);
    modified(path);
}

void set_mat4(const char *path,
              const uint64_t *keys,
              uint32_t keys_count,
              float *value) {

    struct ce_yng_node n = get_first_node_recursive(path, keys, keys_count,
                                             keys_count);
    if (!n.idx) {
        return;
    }

    n.d->set_mat4(n.d, n, value);
    modified(path);
}

void parent_files(const char *path,
                  const char ***files,
                  uint32_t *count) {

    struct ce_yng_doc *d = get(path);

    if (!d) {
        *files = NULL;
        *count = 0;
        return;
    }

    d->parent_files(d, files, count);
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

    struct ce_yng_doc *d = get(path);
    ce_yng_a0->save_to_vio(_G.allocator, f, d);

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
        .get = get,
        .free = free,

        .has_key = has_key,

        .get_str = get_string,
        .get_float = get_float,
        .get_bool = get_bool,
        .get_vec3 = get_vec3,
        .get_vec4 = get_vec4,
        .get_mat4 = get_mat4,

        .set_str = set_string,
        .set_float = set_float,
        .set_bool = set_bool,
        .set_vec3 = set_vec3,
        .set_vec4 = set_vec4,
        .set_mat4 = set_mat4,

        .get_map_keys = get_map_keys,
        .parent_files = parent_files,
        .save = save,
        .save_all_modified = save_all_modified,
};

struct ce_ydb_a0 *ce_ydb_a0 = &ydb_api;

static void _init(struct ce_api_a0 *api) {
    _G = (struct _G){.allocator = ce_memory_a0->system};

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

    _G = (struct _G){};
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