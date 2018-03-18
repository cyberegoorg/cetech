#include "cetech/core/containers/map.inl"

#include <cetech/core/api/api_system.h>
#include <cetech/core/config/config.h>
#include <cetech/core/memory/memory.h>
#include <cetech/core/module/module.h>
#include <cetech/core/yaml/yng.h>
#include <cetech/core/hashlib/hashlib.h>
#include <cetech/core/log/log.h>
#include <cetech/core/os/vio.h>
#include <cetech/core/yaml/ydb.h>
#include <cetech/core/fs/fs.h>

#include <cetech/core/os/watchdog.h>
#include <cetech/core/os/path.h>
#include <cetech/core/math/fmath.h>
#include <cetech/core/containers/hash.h>
#include <cetech/core/containers/buffer.h>
#include <cetech/core/os/thread.h>

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_fs_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_path_a0);
CETECH_DECL_API(ct_thread_a0);

using namespace celib;

#define _G ydb_global
#define LOG_WHERE "ydb"

static struct _G {
    ct_hash_t document_cache_map;
    ct_yng_doc **document_cache;
    char **document_path;

    ct_spinlock cache_lock;
    ct_hash_t modified_files_set;
    ct_alloc *allocator;
} _G;

void expire_document_in_cache(const char *path,
                              uint64_t path_key) {
    uint32_t idx = ct_hash_lookup(&_G.document_cache_map, path_key, UINT32_MAX);
    if (UINT32_MAX == idx) {
        return;
    }

    ct_thread_a0.spin_lock(&_G.cache_lock);
    ct_yng_doc *doc = _G.document_cache[idx];
    ct_hash_remove(&_G.document_cache_map, path_key);
    ct_yng_a0.destroy(doc);
    ct_thread_a0.spin_unlock(&_G.cache_lock);
}

ct_yng_doc *load_to_cache(const char *path,
                          uint64_t path_key) {

    static const uint64_t fs_root = CT_ID64_0("source");

    ct_log_a0.debug(LOG_WHERE, "Load file %s to cache", path);

    struct ct_yng_doc *doc;
    ct_vio *f = ct_fs_a0.open(fs_root, path, FS_OPEN_READ);

    if (!f) {
        ct_log_a0.error(LOG_WHERE, "Could not read file %s", path);
        return NULL;
    }

    doc = ct_yng_a0.from_vio(f, ct_memory_a0.main_allocator());
    ct_fs_a0.close(f);

    if (!doc) {
        ct_log_a0.error(LOG_WHERE, "Could not parse file %s", path);
        return NULL;
    }

    ct_thread_a0.spin_lock(&_G.cache_lock);
    uint32_t idx = ct_hash_lookup(&_G.document_cache_map, path_key, UINT32_MAX);
    if (UINT32_MAX == idx) {
        ct_array_push(_G.document_cache, doc, _G.allocator);
        ct_array_push(_G.document_path, ct_memory_a0.str_dup(path,_G.allocator), _G.allocator);
        ct_hash_add(&_G.document_cache_map, path_key,
                    ct_array_size(_G.document_cache) - 1, _G.allocator);
    } else {
        _G.document_cache[idx] = doc;
    }
    ct_thread_a0.spin_unlock(&_G.cache_lock);

    return doc;
}

ct_yng_doc *get(const char *path) {
    ct_thread_a0.spin_lock(&_G.cache_lock);

    uint64_t path_key = CT_ID64_0(path);

    uint32_t idx = ct_hash_lookup(&_G.document_cache_map, path_key, UINT32_MAX);
    if (UINT32_MAX == idx) {
        ct_thread_a0.spin_unlock(&_G.cache_lock);
        return load_to_cache(path, path_key);
    }

    struct ct_yng_doc *doc;
    doc = _G.document_cache[idx];
    ct_thread_a0.spin_unlock(&_G.cache_lock);
    return doc;
};

void free(const char *path) {
    CT_UNUSED(path);
    //expire_document_in_cache(path, CT_ID64_0(path));

    // TODO: ref counting
}

ct_yamlng_node get_first_node_recursive(const char *path,
                                        const uint64_t *keys,
                                        uint32_t keys_count,
                                        uint32_t max_depth) {

    struct ct_yng_doc *d = get(path);

    uint64_t result_key = ct_yng_a0.combine_key(keys, keys_count);

    ct_yamlng_node n = d->get(d, result_key);
    if (n.idx) {
        return n;
    }

    // Find parent
    // A.B.C , parent, A.parent, A.B.parent
    const char *parent_file_str;
    uint64_t tmp_keys[keys_count];
    memcpy(tmp_keys, keys, sizeof(uint64_t) * keys_count);

    static const uint64_t PARENT_KEY = ct_yng_a0.key("PARENT");
    uint64_t tmp_key;

    for (uint64_t i = 0; i < max_depth; ++i) {
        tmp_key = tmp_keys[i];

        tmp_keys[i] = PARENT_KEY;
        uint64_t k = ct_yng_a0.combine_key(tmp_keys, i + 1);

        parent_file_str = d->get_str(d, k, "");

        if ('\0' != parent_file_str[0]) {
            return get_first_node_recursive(parent_file_str, keys + i,
                                            keys_count - i,
                                            max_depth);
        }

        tmp_keys[i] = tmp_key;
    }

    return {0};
}


void get_map_keys(const char *path,
                  uint64_t *keys,
                  uint32_t keys_count,
                  uint64_t *map_keys,
                  uint32_t max_map_keys,
                  uint32_t *map_keys_count) {

    struct out_keys_s {
        uint64_t *keys;
        uint32_t max_keys;
        uint32_t *count;
    } out_keys = {
            .keys = map_keys,
            .max_keys = max_map_keys,
            .count = map_keys_count,
    };

    struct ct_yng_doc *d = get(path);

    uint64_t result_key = ct_yng_a0.combine_key(keys, keys_count);

    ct_yamlng_node n = d->get(d, result_key);
    if (n.idx) {
        d->foreach_dict_node(
                d, n,
                [](ct_yamlng_node k,
                   ct_yamlng_node v,
                   void *data) {
                    CT_UNUSED(v);

                    auto *out = (out_keys_s *) data;

                    const char *string = k.d->as_string(k.d, k, NULL);
                    if (!string) {
                        return;
                    }

                    uint64_t str_key = ct_yng_a0.key(string);

                    const uint32_t s = *out->count;
                    for (uint32_t i = 0; i < s; ++i) {
                        if (out->keys[i] == str_key) {
                            return;
                        }
                    }

                    out->keys[(*out->count)++] = str_key;
                }, &out_keys);
    }

    // Find parent
    // A.B.C , parent, A.parent, A.B.parent
    const char *parent_file_str;
    uint64_t tmp_keys[keys_count];
    memcpy(tmp_keys, keys, sizeof(uint64_t) * keys_count);

    static const uint64_t PARENT_KEY = ct_yng_a0.key("PARENT");
    uint64_t tmp_key;

    for (uint64_t i = 0; i < keys_count; ++i) {
        tmp_key = tmp_keys[i];

        tmp_keys[i] = PARENT_KEY;
        uint64_t k = ct_yng_a0.combine_key(tmp_keys, i + 1);

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
    ct_yamlng_node n = get_first_node_recursive(path, keys, keys_count,
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
    ct_yamlng_node n = get_first_node_recursive(path, keys, keys_count,
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
    ct_yamlng_node n = get_first_node_recursive(path, keys, keys_count,
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
    ct_yamlng_node n = get_first_node_recursive(path, keys, keys_count,
                                                keys_count);

    if (0 == n.idx) {
        ct_vec3_move(v, defaultt);
        return;
    }


    n.d->as_vec3(n.d, n, v);
}

void get_vec4(const char *path,
              uint64_t *keys,
              uint32_t keys_count,
              float v[4],
              float defaultt[4]) {
    ct_yamlng_node n = get_first_node_recursive(path, keys, keys_count,
                                                keys_count);

    if (0 == n.idx) {
        ct_vec4_move(v, defaultt);
        return;
    }


    n.d->as_vec4(n.d, n, v);
}

void get_mat4(const char *path,
              uint64_t *keys,
              uint32_t keys_count,
              float v[16],
              float defaultt[16]) {
    ct_yamlng_node n = get_first_node_recursive(path, keys, keys_count,
                                                keys_count);

    if (0 == n.idx) {
        ct_mat4_move(v, defaultt);
        return;
    }

    n.d->as_mat4(n.d, n, v);
}


void modified(const char *path) {
    uint64_t hash = CT_ID64_0(path);
    ct_hash_add(&_G.modified_files_set, hash, true, _G.allocator);
}

void unmodified(const char *path) {
    uint64_t hash = CT_ID64_0(path);
    ct_hash_remove(&_G.modified_files_set, hash);
}

void set_float(const char *path,
               uint64_t *keys,
               uint32_t keys_count,
               float value) {
    struct ct_yng_doc *d = get(path);
    uint64_t key = ct_yng_a0.combine_key(keys, keys_count);

    ct_yamlng_node n = d->get(d, key);

    if (!n.idx) {
        const char *str_keys[keys_count];
        for (int i = 0; i < keys_count; ++i) {
            str_keys[i] = ct_yng_a0.get_key(keys[i]);
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

    struct ct_yng_doc *d = get(path);
    uint64_t key = ct_yng_a0.combine_key(keys, keys_count);

    ct_yamlng_node n = d->get(d, key);

    if (!n.idx) {
        const char *str_keys[keys_count];
        for (int i = 0; i < keys_count; ++i) {
            str_keys[i] = ct_yng_a0.get_key(keys[i]);
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
    struct ct_yng_doc *d = get(path);
    uint64_t key = ct_yng_a0.combine_key(keys, keys_count);

    ct_yamlng_node n = d->get(d, key);

    if (!n.idx) {
        const char *str_keys[keys_count];
        for (int i = 0; i < keys_count; ++i) {
            str_keys[i] = ct_yng_a0.get_key(keys[i]);
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

    struct ct_yng_doc *d = get(path);
    uint64_t key = ct_yng_a0.combine_key(keys, keys_count);

    ct_yamlng_node n = d->get(d, key);

    if (!n.idx) {
        const char *str_keys[keys_count];
        for (int i = 0; i < keys_count; ++i) {
            str_keys[i] = ct_yng_a0.get_key(keys[i]);
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

    ct_yamlng_node n = get_first_node_recursive(path, keys, keys_count,
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

    ct_yamlng_node n = get_first_node_recursive(path, keys, keys_count,
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

    struct ct_yng_doc *d = get(path);

    if (!d) {
        *files = NULL;
        *count = 0;
        return;
    }

    d->parent_files(d, files, count);
}

void check_fs() {
    ct_alloc *alloc = ct_memory_a0.main_allocator();

    static uint64_t root = CT_ID64_0("source");

    auto *wd_it = ct_fs_a0.event_begin(root);
    const auto *wd_end = ct_fs_a0.event_end(root);

    while (wd_it != wd_end) {
        if (wd_it->type == CT_WATCHDOG_EVENT_FILE_MODIFIED) {
            ct_wd_ev_file_write_end *ev = (ct_wd_ev_file_write_end *) wd_it;

            char *path = NULL;
            ct_path_a0.join(&path, alloc, 2, ev->dir, ev->filename);

            uint64_t path_key = CT_ID64_0(path);

            if (ct_hash_contain(&_G.document_cache_map, path_key)) {
                ct_log_a0.debug(LOG_WHERE, "Reload cached file %s", path);

                expire_document_in_cache(path, path_key);
                load_to_cache(path, path_key);
            }

            ct_buffer_free(path, alloc);
        }

        wd_it = ct_fs_a0.event_next(wd_it);
    }
}


void save(const char *path) {
    ct_vio *f = ct_fs_a0.open(CT_ID64_0("source"), path, FS_OPEN_WRITE);

    if (!f) {
        ct_log_a0.error(LOG_WHERE, "Could not read file %s", path);
        return;
    }

    ct_yng_doc *d = get(path);
    ct_yng_a0.save_to_vio(ct_memory_a0.main_allocator(), f, d);

    ct_fs_a0.close(f);
    unmodified(path);
}


void save_all_modified() {
    for (int i = 0; i < _G.document_cache_map.n; ++i) {
        if(!ct_hash_contain(&_G.modified_files_set, _G.document_cache_map.keys[i])){
            continue;
        }

        save(_G.document_path[_G.document_cache_map.values[i]]);
        return;
    }
}

static ct_ydb_a0 ydb_api = {
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
        .check_fs = check_fs
};

static void _init(ct_api_a0 *api) {
    _G = {.allocator = ct_memory_a0.main_allocator()};

    api->register_api("ct_ydb_a0", &ydb_api);
}

static void _shutdown() {
    for (int i = 0; i < ct_array_size(_G.document_cache); ++i) {
        ct_yng_a0.destroy(_G.document_cache[i]);
    }

    ct_array_free(_G.document_path,_G.allocator);
    ct_array_free(_G.document_cache,_G.allocator);
    ct_hash_free(&_G.document_cache_map, _G.allocator);
    ct_hash_free(&_G.modified_files_set, _G.allocator);

    _G = {};
}

CETECH_MODULE_DEF(
        ydb,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_fs_a0);
            CETECH_GET_API(api, ct_thread_a0);
            CETECH_GET_API(api, ct_path_a0);
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