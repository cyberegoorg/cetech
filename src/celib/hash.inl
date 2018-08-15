//
//                          **Hash table**
//
// # Description
//
// Hash table that map uint64_t **key** to uint64_t **value**.
//
//
// # Example
//
// ## Simple table char -> uint64_t
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
// ce_hash_t hash = {};
//
// ce_hash_add(&hash, 'h', 1, ce_memory_a0->system);
// ce_hash_add(&hash, 'e', 1, ce_memory_a0->system);
// ce_hash_add(&hash, 'l', 1, ce_memory_a0->system);
// ce_hash_add(&hash, 'l', 1, ce_memory_a0->system);
// ce_hash_add(&hash, 'o', 1, ce_memory_a0->system);
//
// printf("h in hash == %d", ce_hash_contain(&hash, 'h')); // h in hash == 1
// printf("H in hash == %d", ce_hash_contain(&hash, 'H')); // H in hash == 0
//
// printf("h -> %d", ct_hash_find(&hash, 'h', 0));  // h -> 1
// printf("H -> %d", ct_hash_find(&hash, 'H', 0));  // H -> 0
//
// ce_hash_free(&hash, ce_memory_a0->system);
// ~~~~~~~~~~~~~~~~~~~~~~~~~~
// # API

#ifndef CE_HASHH_H
#define CE_HASHH_H

#include <stdint.h>
#include <stdbool.h>

#include "array.inl"

#define EMPTY_SLOT UINT64_MAX

// Hash table struct
// ***************************************
// *         +---+---+---+---+---+
// *  Keys   | O | 1 | 2 |...| n |
// *         +---+---+---+---+---+
// *  Values | O | 1 | 2 |...| n |
// *         +---+---+---+---+---+
// ***************************************
//
// - *n* - bucket size
// - *keys* - keys [array](array.md.html)
// - *values* - values [array](array.md.html)
struct ce_hash_t {
    uint32_t n;
    uint64_t *keys;
    uint64_t *values;
};


// Clean hash table
static inline void ce_hash_clean(struct ce_hash_t *hash) {
    memset(hash->keys, 255, sizeof(uint64_t) * hash->n);
    hash->n = 0;

    ce_array_clean(hash->keys);
    ce_array_clean(hash->values);
}

// Free hash table
static inline void ce_hash_free(struct ce_hash_t *hash,
                                const struct ce_alloc *allocator) {
    ce_array_free(hash->keys, allocator);
    ce_array_free(hash->values, allocator);
    hash->n = 0;
}

static inline uint32_t ce_hash_find_slot(const struct ce_hash_t *hash,
                                         uint64_t k) {
    if(!k) {
        return 0;
    }

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

// Find *k* in hash table. If key does not exist return *default_value*
static inline uint64_t ce_hash_lookup(const struct ce_hash_t *hash,
                                      uint64_t k,
                                      uint64_t default_value) {
    if (!hash->n) {
        return default_value;
    }

    const uint32_t idx = ce_hash_find_slot(hash, k);
    return hash->keys[idx] == k ? hash->values[idx] : default_value;
}

// Is *k* hash table?
static inline bool ce_hash_contain(const struct ce_hash_t *hash,
                                   uint64_t k) {
    if (!hash->n) {
        return false;
    }

    const uint32_t idx = ce_hash_find_slot(hash, k);
    return hash->keys[idx] == k;
}

// Add *k* -> *value*
static inline void ce_hash_add(struct ce_hash_t *hash,
                               uint64_t k,
                               uint64_t value,
                               const struct ce_alloc *allocator) {
    if (!hash->n) {
        hash->n = 16;
        ce_array_set_capacity(hash->keys, hash->n, allocator);
        ce_array_set_capacity(hash->values, hash->n, allocator);
        memset(hash->keys, 255, sizeof(uint64_t) * hash->n);
    }

    uint32_t idx = 0;

    begin:
    idx = ce_hash_find_slot(hash, k);
    if ((hash->keys[idx] != EMPTY_SLOT) && (hash->keys[idx] != k)) {
        uint32_t new_size = hash->n * 2;

        struct ce_hash_t new_hash = {.n = new_size};

        ce_array_resize(new_hash.values, new_size, allocator);
        ce_array_resize(new_hash.keys, new_size, allocator);
        memset(new_hash.keys, 255, sizeof(uint64_t) * new_size);
        for (uint32_t i = 0; i < hash->n; ++i) {
            if (hash->keys[i] == EMPTY_SLOT) {
                continue;
            }

            ce_hash_add(&new_hash, hash->keys[i], hash->values[i], allocator);
        }

        ce_hash_free(hash, allocator);

        *hash = new_hash;
        goto begin;
    }

    hash->values[idx] = value;
    hash->keys[idx] = k;
}


static inline void ce_hash_remove(struct ce_hash_t *hash,
                                  uint64_t k) {
    if(hash->n == 0) {
        return;
    }

    const uint32_t idx = ce_hash_find_slot(hash, k);
    if (hash->keys[idx] == k) {
        hash->keys[idx] = EMPTY_SLOT;
        hash->values[idx] = 0;
        return;
    }
}

static inline void ce_hash_clone(const struct ce_hash_t *from,
                                 struct ce_hash_t *to,
                                 const struct ce_alloc *alloc) {
    struct ce_hash_t tmp_hash = {};
    tmp_hash.n = from->n;

    if (tmp_hash.n) {
        ce_array_resize(tmp_hash.values, tmp_hash.n, alloc);
        ce_array_resize(tmp_hash.keys, tmp_hash.n, alloc);

        memcpy(tmp_hash.keys, from->keys, sizeof(uint64_t) * tmp_hash.n);
        memcpy(tmp_hash.values, from->values, sizeof(uint64_t) * tmp_hash.n);
    }
    *to = tmp_hash;
}

#endif // CE_HASHH_H
