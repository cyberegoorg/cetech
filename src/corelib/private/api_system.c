//==============================================================================
// Includes
//==============================================================================

#include <corelib/api_system.h>
#include <corelib/hash.inl>
#include <corelib/hashlib.h>

//CETECH_DECL_API(ct_hashlib_a0)

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "api_system"

//==============================================================================
// Globals
//==============================================================================

#define _G ApiSystemGlobals
struct impl_list {
    void **api;
};

static struct _G {
    struct ct_hash_t api_map;
    struct impl_list *impl_list;
    struct ct_alloc *allocator;
} _G;

//==============================================================================
// Private
//==============================================================================

static void api_register_api(const char *name,
                             void *api) {

    uint64_t name_id = ct_hashlib_a0->id64_from_str(name);

    uint64_t idx = ct_hash_lookup(&_G.api_map, name_id, UINT64_MAX);

    if (idx == UINT64_MAX) {
        idx = ct_array_size(_G.impl_list);
        ct_array_push(_G.impl_list, (struct impl_list) {0}, _G.allocator);

        ct_hash_add(&_G.api_map, name_id, idx, _G.allocator);
    }

    ct_array_push(_G.impl_list[idx].api, api, _G.allocator);

}

static int api_exist(const char *name) {
    uint64_t name_id = ct_hashlib_a0->id64_from_str(name);

    return ct_hash_contain(&_G.api_map, name_id);
}

static struct ct_api_entry api_first(const char *name) {
    uint64_t name_id = ct_hashlib_a0->id64_from_str(name);

    uint64_t first = ct_hash_lookup(&_G.api_map, name_id, UINT64_MAX);

    if (first == UINT64_MAX) {
        return (struct ct_api_entry) {0};
    }

    return (struct ct_api_entry) {
            .api = _G.impl_list[first].api[0],
            .idx = 0,
            .entry  = &_G.impl_list[first]
    };
}

static struct ct_api_entry api_next(struct ct_api_entry entry) {
    struct impl_list *impl_list = entry.entry;

    const uint32_t n = ct_array_size(impl_list->api);

    if (entry.idx >= n) {
        return (struct ct_api_entry) {0};
    }

    return (struct ct_api_entry) {
            .api = impl_list->api[entry.idx + 1],
            .idx = entry.idx + 1,
            .entry  = impl_list
    };
}

static struct ct_api_a0 a0 = {
        .register_api = api_register_api,
        .first = api_first,
        .next = api_next,
        .exist = api_exist
};

struct ct_api_a0 *ct_api_a0 = &a0;

void api_init(struct ct_alloc *allocator) {
    _G = (struct ApiSystemGlobals) {
            .allocator = allocator
    };

    api_register_api("ct_api_a0", &a0);
}

void api_shutdown() {
    ct_hash_free(&_G.api_map, _G.allocator);
}

