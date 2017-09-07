#include "celib/map.inl"

#include <cetech/kernel/api_system.h>
#include <cetech/kernel/config.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/module.h>
#include <cetech/kernel/yamlng.h>
#include <cetech/kernel/hashlib.h>
#include <cetech/kernel/log.h>
#include <cetech/kernel/vio.h>
#include <cetech/kernel/ydb.h>
#include <cetech/kernel/filesystem.h>
#include <celib/fpumath.h>
#include <cetech/kernel/watchdog.h>
#include <cetech/kernel/path.h>

#include "yaml/yaml.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_filesystem_a0);
CETECH_DECL_API(ct_yamlng_a0);
CETECH_DECL_API(ct_path_a0);

using namespace celib;

#define _G ydb_global
#define LOG_WHERE "ydb"

static struct _G {
    Map<ct_yamlng_document *> document_cache;
} _G;

void expire_document_in_cache(const char *path,
                              uint64_t path_key) {
    ct_yamlng_document* doc = map::get<ct_yamlng_document *>(_G.document_cache, path_key, NULL);

    if(!doc) {
        return;
    }

    map::remove(_G.document_cache, path_key);
    ct_yamlng_a0.destroy(doc);
}

ct_yamlng_document* load_to_cache(const char* path, uint64_t path_key) {
    static const uint64_t fs_root = ct_hash_a0.id64_from_str("source");

    ct_log_a0.debug(LOG_WHERE, "Load file %s to cache", path);

    struct ct_yamlng_document *doc;
    ct_vio *f = ct_filesystem_a0.open(fs_root, path, FS_OPEN_READ);

    if (!f) {
        ct_log_a0.error(LOG_WHERE, "Could not read file %s", path);
        return NULL;
    }

    doc = ct_yamlng_a0.from_vio(f, ct_memory_a0.main_allocator());
    ct_filesystem_a0.close(f);

    if (!doc) {
        ct_log_a0.error(LOG_WHERE, "Could not parse file %s", path);
        return NULL;
    }

    map::set(_G.document_cache, path_key, doc);

    return doc;
}

ct_yamlng_document *get(const char *path) {
    uint64_t path_key = ct_hash_a0.id64_from_str(path);

    struct ct_yamlng_document *doc;
    doc = map::get<ct_yamlng_document *>(_G.document_cache, path_key, NULL);

    if (!doc) {
        return load_to_cache(path, path_key);
    }

    return doc;
};

void free(const char *path) {
    CEL_UNUSED(path);
    //expire_document_in_cache(path, ct_hash_a0.id64_from_str(path));

    // TODO: ref counting
}

ct_yamlng_node get_first_node_recursive(const char *path,
                                        uint64_t *keys,
                                        uint32_t keys_count,
                                        uint32_t max_depth) {

    struct ct_yamlng_document *d = get(path);

    uint64_t result_key = ct_yamlng_a0.combine_key(keys, keys_count);

    ct_yamlng_node n = d->get(d->inst, result_key);
    if (n.idx) {
        return n;
    }

    // Find parent
    // A.B.C , parent, A.parent, A.B.parent
    const char *parent_file_str;
    uint64_t tmp_keys[keys_count];
    memcpy(tmp_keys, keys, sizeof(uint64_t) * keys_count);

    static const uint64_t PARENT_KEY = ct_yamlng_a0.calc_key("PARENT");
    uint64_t tmp_key;

    for (uint64_t i = 0; i < max_depth; ++i) {
        tmp_key = tmp_keys[i];

        tmp_keys[i] = PARENT_KEY;
        uint64_t k = ct_yamlng_a0.combine_key(tmp_keys, i + 1);

        parent_file_str = d->get_string(d->inst, k, "");

        if ('\0' != parent_file_str[0]) {
            return get_first_node_recursive(parent_file_str, keys + i,
                                            keys_count - i,
                                            max_depth);
        }

        tmp_keys[i] = tmp_key;
    }

    return {};
}


void get_map_keys(const char *path,
                            uint64_t *keys,
                            uint32_t keys_count,
                            uint64_t *map_keys,
                            uint32_t max_map_keys,
                            uint32_t *map_keys_count) {

    struct out_keys_s{
        uint64_t *keys;
        uint32_t max_keys;
        uint32_t* count;
    } out_keys = {
            .keys = map_keys,
            .max_keys = max_map_keys,
            .count = map_keys_count,
    };

    struct ct_yamlng_document *d = get(path);

    uint64_t result_key = ct_yamlng_a0.combine_key(keys, keys_count);

    ct_yamlng_node n = d->get(d->inst, result_key);
    if (n.idx) {
        d->foreach_dict_node(d->inst, n,
                             [](ct_yamlng_node k,
                                ct_yamlng_node v,
                                void *data) {
                                 CEL_UNUSED(v);

                                 auto *out = (out_keys_s*)data;

                                 const char* string = k.d->as_string(k.d->inst, k, NULL);
                                 if(!string) {
                                     return;
                                 }

                                 uint64_t str_key = ct_yamlng_a0.calc_key(string);

                                 const uint32_t s = *out->count;
                                 for (uint32_t i = 0; i < s; ++i) {
                                     if(out->keys[i] == str_key) {
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

    static const uint64_t PARENT_KEY = ct_yamlng_a0.calc_key("PARENT");
    uint64_t tmp_key;

    for (uint64_t i = 0; i < keys_count; ++i) {
        tmp_key = tmp_keys[i];

        tmp_keys[i] = PARENT_KEY;
        uint64_t k = ct_yamlng_a0.combine_key(tmp_keys, i + 1);

        parent_file_str = d->get_string(d->inst, k, "");

        if ('\0' != parent_file_str[0]) {
            get_map_keys(parent_file_str, keys + i, keys_count - i, map_keys, max_map_keys, map_keys_count);
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

    return n.d->as_string(n.d->inst, n, defaultt);
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

    return n.d->as_float(n.d->inst, n, defaultt);
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

    return n.d->as_bool(n.d->inst, n, defaultt);
}


void get_vec3(const char *path,
                 uint64_t *keys,
                 uint32_t keys_count,
                 float v[3],
                 float defaultt[3]) {
    ct_yamlng_node n = get_first_node_recursive(path, keys, keys_count,
                                                keys_count);

    if (0 == n.idx) {
        vec3_move(v, defaultt);
        return;
    }


    n.d->as_vec3(n.d->inst, n, v);
}

void get_vec4(const char *path,
                 uint64_t *keys,
                 uint32_t keys_count,
                 float v[4],
                 float defaultt[4]) {
    ct_yamlng_node n = get_first_node_recursive(path, keys, keys_count,
                                                keys_count);

    if (0 == n.idx) {
        vec4_move(v, defaultt);
        return;
    }


    n.d->as_vec4(n.d->inst, n, v);
}

void get_mat4(const char *path,
                 uint64_t *keys,
                 uint32_t keys_count,
                 float v[16],
                 float defaultt[16]) {
    ct_yamlng_node n = get_first_node_recursive(path, keys, keys_count,
                                                keys_count);

    if (0 == n.idx) {
        mat4_move(v, defaultt);
        return;
    }

    n.d->as_mat4(n.d->inst, n, v);
}


void set_float(const char *path,
               uint64_t* keys,
                  uint32_t keys_count,
                  float value) {
    ct_yamlng_node n = get_first_node_recursive(path, keys, keys_count,
                                                keys_count);
    if (!n.idx) {
        return;
    }

    n.d->set_float(n.d->inst, n, value);
}

void set_bool(const char *path,
              uint64_t* keys,
                 uint32_t keys_count,
                 bool value) {

    ct_yamlng_node n = get_first_node_recursive(path, keys, keys_count,
                                                keys_count);
    if (!n.idx) {
        return;
    }

    n.d->set_bool(n.d->inst, n, value);
}


void set_string(const char *path,
                uint64_t* keys,
                   uint32_t keys_count,
                   const char *value) {
    struct ct_yamlng_document *d = get(path);

    ct_yamlng_node n = get_first_node_recursive(path, keys, keys_count,
                                                keys_count);
    if (!n.idx) {
        return;
    }

    d->set_string(d->inst, n, value);
}

void set_vec3(const char *path,
              uint64_t* keys,
                 uint32_t keys_count,
                 float *value) {

    ct_yamlng_node n = get_first_node_recursive(path, keys, keys_count, keys_count);

    if (!n.idx) {
        return;
    }

    n.d->set_vec3(n.d->inst, n, value);
}

void set_vec4(const char *path,
              uint64_t* keys,
                 uint32_t keys_count,
                 float *value) {

    ct_yamlng_node n = get_first_node_recursive(path, keys, keys_count,
                                                keys_count);
    if (!n.idx) {
        return;
    }

    n.d->set_vec4(n.d->inst, n, value);
}

void set_mat4(const char *path,
              uint64_t* keys,
                 uint32_t keys_count,
                 float *value) {

    ct_yamlng_node n = get_first_node_recursive(path, keys, keys_count,
                                                keys_count);
    if (!n.idx) {
        return;
    }

    n.d->set_mat4(n.d->inst, n, value);
}

void parent_files(const char* path,
                  const char ***files,
                  uint32_t *count){

    struct ct_yamlng_document *d = get(path);

    if(!d) {
        *files = NULL;
        *count = 0;
        return;
    }

    d->parent_files(d->inst, files, count);
}

void check_fs() {
    cel_alloc *alloc = ct_memory_a0.main_allocator();

    static uint64_t root = ct_hash_a0.id64_from_str("source");

    auto *wd_it = ct_filesystem_a0.event_begin(root);
    const auto *wd_end = ct_filesystem_a0.event_end(root);

    while (wd_it != wd_end) {
        if (wd_it->type == CT_WATCHDOG_EVENT_FILE_MODIFIED) {
            ct_wd_ev_file_write_end *ev = (ct_wd_ev_file_write_end *)wd_it;

            char *path = ct_path_a0.join(alloc, 2, ev->dir, ev->filename);
            uint64_t path_key = ct_hash_a0.id64_from_str(path);

            if(map::has(_G.document_cache, path_key)) {
                ct_log_a0.debug(LOG_WHERE, "Reload cached file %s", path);

                expire_document_in_cache(path, path_key);
                load_to_cache(path, path_key);
            }

            CEL_FREE(alloc, path);
        }

        wd_it = ct_filesystem_a0.event_next(wd_it);
    }
}


void save(const char* path){
//    ct_vio *f = ct_filesystem_a0.open(ct_hash_a0.id64_from_str("source"), path, FS_OPEN_WRITE);
//
//    if (!f) {
//        ct_log_a0.error(LOG_WHERE, "Could not read file %s", path);
//        return;
//    }
//
//    ct_yamlng_document* d = get(path);
//    ct_yamlng_a0.save_to_vio(ct_memory_a0.main_allocator(), f, d);
//
//    ct_filesystem_a0.close(f);
}

static ct_ydb_a0 ydb_api = {
        .get = get,
        .free = free,

        .has_key = has_key,

        .get_string = get_string,
        .get_float = get_float,
        .get_bool = get_bool,
        .get_vec3 = get_vec3,
        .get_vec4 = get_vec4,
        .get_mat4 = get_mat4,

        .set_string = set_string,
        .set_float = set_float,
        .set_bool = set_bool,
        .set_vec3 = set_vec3,
        .set_vec4 = set_vec4,
        .set_mat4 = set_mat4,

        .get_map_keys = get_map_keys,
        .parent_files = parent_files,
        .save = save,
        .check_fs = check_fs
};

static void _init(ct_api_a0 *api) {
    _G = {};

    _G.document_cache.init(ct_memory_a0.main_allocator());
    api->register_api("ct_ydb_a0", &ydb_api);
}

static void _shutdown() {
    auto *it = map::begin(_G.document_cache);
    auto *it_end = map::end(_G.document_cache);

    while (it != it_end) {
        ct_yamlng_document *d = it->value;

        ct_yamlng_a0.destroy(d);
        ++it;
    }

    _G.document_cache.destroy();

    _G = {};
}

CETECH_MODULE_DEF(
        ydb,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hash_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_yamlng_a0);
            CETECH_GET_API(api, ct_filesystem_a0);
            CETECH_GET_API(api, ct_path_a0);
        },
        {
            CEL_UNUSED(reload);
            _init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);
            _shutdown();
        }
)