#include <stdint.h>
#include <memory.h>
#include <cetech/kernel/hashlib/hashlib.h>
#include <cetech/kernel/module/module.h>
#include <cetech/kernel/api/api_system.h>
#include <cetech/kernel/containers/hash.h>
#include <cetech/kernel/memory/memory.h>
#include <cetech/kernel/memory/private/allocator_core_private.h>

#define _G hashlib_global

CETECH_DECL_API(ct_memory_a0);

struct _G {
    char *str_id64;
    struct ct_hash_t id64_to_str;

    char *str_id32;
    struct ct_hash_t id32_to_str;

    struct ct_alloc *allocator;
} _G;

uint64_t hash_murmur2_64(const void *key,
                         uint64_t len,
                         uint64_t seed) {
    if (!key) {
        return 0;
    }


    static const uint64_t m = 0xc6a4a7935bd1e995ULL;
    static const uint32_t r = 47;

    uint64_t h = seed ^(len * m);

    const uint64_t *data = (const uint64_t *) key;
    const uint64_t *end = data + (len / 8);

    while (data != end) {

#ifdef CETECH_BIG_ENDIAN
        uint64_t k = *data++;
        char* p = (char*)&k;
        char c;
        c = p[0]; p[0] = p[7]; p[7] = c;
        c = p[1]; p[1] = p[6]; p[6] = c;
        c = p[2]; p[2] = p[5]; p[5] = c;
        c = p[3]; p[3] = p[4]; p[4] = c;
#else
        uint64_t k = *data++;
#endif

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;
    }

    const unsigned char *data2 = (const unsigned char *) data;

    switch (len & 7) {
        case 7:
            h ^= ((uint64_t) data2[6]) << 48;
        case 6:
            h ^= ((uint64_t) data2[5]) << 40;
        case 5:
            h ^= ((uint64_t) data2[4]) << 32;
        case 4:
            h ^= ((uint64_t) data2[3]) << 24;
        case 3:
            h ^= ((uint64_t) data2[2]) << 16;
        case 2:
            h ^= ((uint64_t) data2[1]) << 8;
        case 1:
            h ^= ((uint64_t) data2[0]);
            h *= m;
    };

    h ^= h >> r;
    h *= m;
    h ^= h >> r;

    return h;
}

uint32_t hash_murmur2_32(const void *key,
                         uint32_t len,
                         uint32_t seed) {
    const unsigned int m = 0x5bd1e995;
    const int r = 24;

    unsigned int h = seed ^len;

    const unsigned char *data = (const unsigned char *) key;

    while (len >= 4) {
        unsigned int k = *(unsigned int *) data;

        k *= m;
        k ^= k >> r;
        k *= m;

        h *= m;
        h ^= k;

        data += 4;
        len -= 4;
    }

    switch (len) {
        case 3:
            h ^= data[2] << 16;
        case 2:
            h ^= data[1] << 8;
        case 1:
            h ^= data[0];
            h *= m;
    };

    h ^= h >> 13;
    h *= m;
    h ^= h >> 15;

    return h;
}


#define STRINGID64_SEED 22
#define STRINGID32_SEED 22

//==============================================================================
// Interface
//==============================================================================

uint64_t stringid64_from_string(const char *str) {
    if (!str) {
        return 0;
    }

    if ('\0' == str[0]) {
        return 0;
    }

    struct ct_alloc *alloc = coreallocator_get();

    const uint32_t str_len = strlen(str);

    const uint64_t hash = hash_murmur2_64(str, str_len, STRINGID64_SEED);
    if (!ct_hash_contain(&_G.id64_to_str, hash)) {
        const uint32_t idx = ct_array_size(_G.str_id64);
        ct_array_push_n(_G.str_id64, str, str_len + 1, alloc);

        ct_hash_add(&_G.id64_to_str, hash, idx, alloc);
    }

    return hash;
}

uint32_t stringid32_from_string(const char *str) {
    if (!str) {
        return 0;
    }

    struct ct_alloc *alloc = coreallocator_get();

    const uint32_t str_len = strlen(str);

    const uint32_t  hash = hash_murmur2_32(str, str_len, STRINGID32_SEED);
    if (!ct_hash_contain(&_G.id32_to_str, hash)) {
        const uint32_t idx = ct_array_size(_G.str_id32);
        ct_array_push_n(_G.str_id32, str, str_len + 1, alloc);

        ct_hash_add(&_G.id32_to_str, hash, idx, alloc);
    }

    return hash;
}

const char* str_from_id64(uint64_t key){
    uint32_t idx = ct_hash_lookup(&_G.id64_to_str, key, UINT32_MAX);

    if(UINT32_MAX != idx) {
        return &_G.str_id64[idx];
    }

    return NULL;
}

const char* str_from_id32(uint32_t key){
    uint32_t idx = ct_hash_lookup(&_G.id32_to_str, key, UINT32_MAX);

    if(UINT32_MAX != idx) {
        return &_G.str_id32[idx];
    }

    return NULL;
}

static struct ct_hashlib_a0 hash_api = {
        .hash_murmur2_64 = hash_murmur2_64,
        .hash_murmur2_32 = hash_murmur2_32,

        .id64_from_str = stringid64_from_string,
        .id32_from_str = stringid32_from_string,

        .str_from_id32 = str_from_id32,
        .str_from_id64 = str_from_id64,
};

void CETECH_MODULE_INITAPI(hashlib)(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_memory_a0);
}

void CETECH_MODULE_LOAD (hashlib)(struct ct_api_a0 *api,
                                  int reload) {
    CT_UNUSED(reload);

    api->register_api("ct_hashlib_a0", &hash_api);

    CETECH_GET_API(api, ct_memory_a0);

    _G = (struct _G) {
            .allocator = ct_memory_a0.main_allocator()
    };
}

void CETECH_MODULE_UNLOAD (hashlib)(struct ct_api_a0 *api,
                                    int reload) {
    CT_UNUSED(api);
}
