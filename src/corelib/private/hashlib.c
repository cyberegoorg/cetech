#include <stdint.h>
#include <memory.h>
#include <corelib/hashlib.h>
#include <corelib/module.h>
#include <corelib/api_system.h>
#include <corelib/hash.inl>
#include <corelib/memory.h>
#include <corelib/private/allocator_core_private.h>
#include <corelib/murmur_hash.inl>

#define _G hashlib_global


struct _G {
    char *str_id64;
    struct ct_hash_t id64_to_str;

    char *str_id32;
    struct ct_hash_t id32_to_str;

    struct ct_alloc *allocator;
} _G;


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

    const uint64_t hash = ct_hash_murmur2_64(str, str_len, STRINGID64_SEED);
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

    const uint32_t hash = ct_hash_murmur2_32(str, str_len, STRINGID32_SEED);
    if (!ct_hash_contain(&_G.id32_to_str, hash)) {
        const uint32_t idx = ct_array_size(_G.str_id32);
        ct_array_push_n(_G.str_id32, str, str_len + 1, alloc);

        ct_hash_add(&_G.id32_to_str, hash, idx, alloc);
    }

    return hash;
}

const char *str_from_id64(uint64_t key) {
    uint32_t idx = ct_hash_lookup(&_G.id64_to_str, key, UINT32_MAX);

    if (UINT32_MAX != idx) {
        return &_G.str_id64[idx];
    }

    return NULL;
}

const char *str_from_id32(uint32_t key) {
    uint32_t idx = ct_hash_lookup(&_G.id32_to_str, key, UINT32_MAX);

    if (UINT32_MAX != idx) {
        return &_G.str_id32[idx];
    }

    return NULL;
}

static struct ct_hashlib_a0 hash_api = {
        .id64_from_str = stringid64_from_string,
        .id32_from_str = stringid32_from_string,

        .str_from_id32 = str_from_id32,
        .str_from_id64 = str_from_id64,
};

struct ct_hashlib_a0 *ct_hashlib_a0 = &hash_api;

void CETECH_MODULE_INITAPI(hashlib)(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_memory_a0);
}

void CETECH_MODULE_LOAD (hashlib)(struct ct_api_a0 *api,
                                  int reload) {
    CT_UNUSED(reload);

    api->register_api("ct_hashlib_a0", &hash_api);

    CETECH_GET_API(api, ct_memory_a0);

    _G = (struct _G) {
            .allocator = ct_memory_a0->main_allocator()
    };
}

void CETECH_MODULE_UNLOAD (hashlib)(struct ct_api_a0 *api,
                                    int reload) {
    CT_UNUSED(api);
}
