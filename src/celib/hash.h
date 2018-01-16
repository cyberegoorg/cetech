#ifndef CETECH_HASHH_H
#define CETECH_HASHH_H

#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <memory.h>

#include "allocator.h"
#include "array.h"

struct cel_hash_t {
    uint32_t n;
    uint64_t *keys;
    uint64_t *values;
};


static void cel_hash_free(struct cel_hash_t *hash,
                          struct cel_alloc *allocator) {
    cel_array_free(hash->keys, allocator);
    cel_array_free(hash->values, allocator);
    hash->n = 0;
}

#define EMPTY_SLOT UINT64_MAX

static uint32_t cel_hash_find_slot(const struct cel_hash_t *hash, uint64_t k) {
    const uint32_t idx_first = k % hash->n;
    uint32_t idx = idx_first;

    uint32_t i = 0;
    while((hash->keys[idx] != EMPTY_SLOT) && (hash->keys[idx] != k) && (i < hash->n)) {
        idx = (idx + 1) % hash->n;
        ++i;
    }

    return idx;
}

static uint64_t cel_hash_lookup(const struct cel_hash_t *hash,
                                uint64_t k,
                                uint64_t default_value) {
    if(!hash->n){
        return default_value;
    }

    const uint32_t idx = cel_hash_find_slot(hash, k);
    return hash->keys[idx] == k ? hash->values[idx] : default_value;
}

static bool cel_hash_contain(const struct cel_hash_t *hash,
                             uint64_t k) {
    if(!hash->n){
        return false;
    }

    const uint32_t idx = cel_hash_find_slot(hash, k);
    return hash->keys[idx] == k;
}

static void cel_hash_add(struct cel_hash_t *hash,
                         uint64_t k,
                         uint64_t value,
                         struct cel_alloc *allocator) {
    if(!hash->n) {
        hash->n = 16;
        cel_array_set_capacity(hash->keys, hash->n, allocator);
        cel_array_set_capacity(hash->values, hash->n, allocator);
        memset(hash->keys, 255, sizeof(uint64_t) * hash->n);
    }

    uint32_t idx = 0;

begin:
    idx = cel_hash_find_slot(hash, k);
    if((hash->keys[idx] != EMPTY_SLOT) && (hash->keys[idx] != k)) {
        uint32_t new_size = hash->n * 2;

        struct cel_hash_t new_hash = {.n = new_size};

        cel_array_resize(new_hash.values, new_size, allocator);
        cel_array_resize(new_hash.keys, new_size, allocator);
        memset(new_hash.keys, 255, sizeof(uint64_t) * new_size);
        for (int i = 0; i < hash->n; ++i) {
            if(hash->keys[i] == EMPTY_SLOT) {
                continue;
            }

            cel_hash_add(&new_hash, hash->keys[i], hash->values[i], allocator);
        }

        cel_hash_free(hash, allocator);

        *hash = new_hash;
        goto begin;
    }

    hash->values[idx] = value;
    hash->keys[idx] = k;
}


static void cel_hash_remove(struct cel_hash_t *hash,
                            uint64_t k) {
    uint32_t idx = k % hash->n;

    if (hash->keys[idx] == k) {
        hash->keys[idx] = EMPTY_SLOT;
        return;
    }
}

static void *_2 = (void *) &cel_hash_contain; // UNUSED
static void *_3 = (void *) &cel_hash_add; // UNUSED
static void *_4 = (void *) &cel_hash_free; // UNUSED
static void *_5 = (void *) &cel_hash_remove; // UNUSED

#endif //CETECH_HASHH_H
