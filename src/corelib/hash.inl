#ifndef CETECH_HASHH_H
#define CETECH_HASHH_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <memory.h>

#include "allocator.h"
#include "array.inl"

struct ct_hash_t {
    uint32_t n;
    uint64_t *keys;
    uint64_t *values;
};


static inline void ct_hash_clean(struct ct_hash_t *hash) {
    memset(hash->keys, 255, sizeof(uint64_t) * hash->n);
    hash->n = 0;

    ct_array_clean(hash->keys);
    ct_array_clean(hash->values);
}

static inline void ct_hash_free(struct ct_hash_t *hash,
                                const struct ct_alloc *allocator) {
    ct_array_free(hash->keys, allocator);
    ct_array_free(hash->values, allocator);
    hash->n = 0;
}

#define EMPTY_SLOT UINT64_MAX

static inline uint32_t ct_hash_find_slot(const struct ct_hash_t *hash,
                                         uint64_t k) {
    const uint32_t idx_first = k % hash->n;
    uint32_t idx = idx_first;

    uint32_t i = 0;
    while ((hash->keys[idx] != EMPTY_SLOT) && (hash->keys[idx] != k) &&
           (i < hash->n)) {
        idx = (idx + 1) % hash->n;
        ++i;
    }

    return idx;
}

static inline uint64_t ct_hash_lookup(const struct ct_hash_t *hash,
                                      uint64_t k,
                                      uint64_t default_value) {
    if (!hash->n) {
        return default_value;
    }

    const uint32_t idx = ct_hash_find_slot(hash, k);
    return hash->keys[idx] == k ? hash->values[idx] : default_value;
}

static inline bool ct_hash_contain(const struct ct_hash_t *hash,
                                   uint64_t k) {
    if (!hash->n) {
        return false;
    }

    const uint32_t idx = ct_hash_find_slot(hash, k);
    return hash->keys[idx] == k;
}

static inline void ct_hash_add(struct ct_hash_t *hash,
                               uint64_t k,
                               uint64_t value,
                               const struct ct_alloc *allocator) {
    if (!hash->n) {
        hash->n = 16;
        ct_array_set_capacity(hash->keys, hash->n, allocator);
        ct_array_set_capacity(hash->values, hash->n, allocator);
        memset(hash->keys, 255, sizeof(uint64_t) * hash->n);
    }

    uint32_t idx = 0;

    begin:
    idx = ct_hash_find_slot(hash, k);
    if ((hash->keys[idx] != EMPTY_SLOT) && (hash->keys[idx] != k)) {
        uint32_t new_size = hash->n * 2;

        struct ct_hash_t new_hash = {.n = new_size};

        ct_array_resize(new_hash.values, new_size, allocator);
        ct_array_resize(new_hash.keys, new_size, allocator);
        memset(new_hash.keys, 255, sizeof(uint64_t) * new_size);
        for (uint32_t i = 0; i < hash->n; ++i) {
            if (hash->keys[i] == EMPTY_SLOT) {
                continue;
            }

            ct_hash_add(&new_hash, hash->keys[i], hash->values[i], allocator);
        }

        ct_hash_free(hash, allocator);

        *hash = new_hash;
        goto begin;
    }

    hash->values[idx] = value;
    hash->keys[idx] = k;
}


static inline void ct_hash_remove(struct ct_hash_t *hash,
                                  uint64_t k) {
    const uint32_t idx = ct_hash_find_slot(hash, k);
    if (hash->keys[idx] == k) {
        hash->keys[idx] = EMPTY_SLOT;
        hash->values[idx] = 0;
        return;
    }
}

static inline void ct_hash_clone(const struct ct_hash_t *from,
                                 struct ct_hash_t *to,
                                 const struct ct_alloc *alloc) {
    struct ct_hash_t tmp_hash = {0};
    tmp_hash.n = from->n;

    if (tmp_hash.n) {
        ct_array_resize(tmp_hash.values, tmp_hash.n, alloc);
        ct_array_resize(tmp_hash.keys, tmp_hash.n, alloc);

        memcpy(tmp_hash.keys, from->keys, sizeof(uint64_t) * tmp_hash.n);
        memcpy(tmp_hash.values, from->values, sizeof(uint64_t) * tmp_hash.n);
    }
    *to = tmp_hash;
}

#endif //CETECH_HASHH_H
