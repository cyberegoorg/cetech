#ifndef CETECH_HASHH_H
#define CETECH_HASHH_H

#include <stdint.h>
#include <string.h>
#include <memory.h>

#include "allocator.h"
#include "array.h"

struct cel_hash_t {
    uint32_t n;
    uint64_t* keys;
    uint64_t* values;
};

static uint32_t cel_hash_lookup_idx(const struct cel_hash_t *hash, uint64_t k){
    // TODO : FAST !!!
    const uint32_t size = hash->n;
    for (uint32_t i = 0; i < size; ++i) {
        if(hash->keys[i] != k) {
            continue;
        }

        return i;
    }

    return UINT32_MAX;
}

static uint64_t cel_hash_lookup(const struct cel_hash_t *hash, uint64_t k, uint64_t default_value){
    const uint32_t idx = cel_hash_lookup_idx(hash, k);
    return idx != UINT32_MAX ? hash->values[idx] : default_value;
}

static bool cel_hash_contain(const struct cel_hash_t *hash, uint64_t k){
    const uint32_t idx = cel_hash_lookup_idx(hash, k);
    return idx != UINT32_MAX;
}

static void cel_hash_add(struct cel_hash_t *hash, uint64_t k, uint64_t value, struct cel_alloc *allocator){
    uint32_t idx = cel_hash_lookup_idx(hash, k);

    if(idx != UINT32_MAX){
        hash->values[idx] = value;
        return;
    }

    cel_array_push(hash->keys, k, allocator);
    cel_array_push(hash->values, value, allocator);
    hash->n++;
}

static void cel_hash_free(struct cel_hash_t *hash, struct cel_alloc *allocator){
    cel_array_free(hash->keys,allocator);
    cel_array_free(hash->values, allocator);
    hash->n = 0;
}

static void cel_hash_remove(struct cel_hash_t *hash, uint64_t k){
    uint32_t idx = cel_hash_lookup_idx(hash, k);

    if(idx == UINT32_MAX){
        return;
    }

    memmove(hash->keys+idx, hash->keys+idx+1, sizeof(uint64_t) * (hash->n - idx - 1));
    memmove(hash->values+idx, hash->values+idx+1, sizeof(uint64_t) * (hash->n - idx -1));

    hash->n--;
}

static void* _2 = (void*)&cel_hash_contain; // UNUSED
static void* _3 = (void*)&cel_hash_add; // UNUSED
static void* _4 = (void*)&cel_hash_free; // UNUSED
static void* _5 = (void*)&cel_hash_remove; // UNUSED

#endif //CETECH_HASHH_H
