//==============================================================================
// Includes
//==============================================================================

#include <celib/api_system.h>
#include <celib/hash.inl>
#include <celib/hashlib.h>
#include <celib/ebus.h>

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
    struct ce_hash_t api_map;

    struct impl_list *impl_list;

    struct ce_hash_t api_on_add_map;
    ce_api_on_add_t ***on_add;

    struct ce_alloc *allocator;
} _G;

//==============================================================================
// Private
//==============================================================================

static void api_register_api(const char *name,
                             void *api) {
    uint64_t name_id = ce_id_a0->id64(name);

    uint64_t idx = ce_hash_lookup(&_G.api_map, name_id, UINT64_MAX);

    if (idx == UINT64_MAX) {
        idx = ce_array_size(_G.impl_list);
        ce_array_push(_G.impl_list, (struct impl_list) {}, _G.allocator);

        ce_hash_add(&_G.api_map, name_id, idx, _G.allocator);
    }

    ce_array_push(_G.impl_list[idx].api, api, _G.allocator);

    uint64_t on_add_idx = ce_hash_lookup(&_G.api_on_add_map, name_id,
                                         UINT64_MAX);

    if (UINT64_MAX != on_add_idx) {
        ce_api_on_add_t **on_add = _G.on_add[on_add_idx];
        const uint32_t on_add_n = ce_array_size(on_add);
        for (int i = 0; i < on_add_n; ++i) {
            on_add[i](name_id, api);
        }
    }
}

static int api_exist(const char *name) {
    uint64_t name_id = ce_id_a0->id64(name);

    return ce_hash_contain(&_G.api_map, name_id);
}

static struct ce_api_entry api_first(uint64_t name) {
    uint64_t first = ce_hash_lookup(&_G.api_map, name, UINT64_MAX);

    if (first == UINT64_MAX) {
        return (struct ce_api_entry) {};
    }

    return (struct ce_api_entry) {
            .api = _G.impl_list[first].api[0],
            .idx = 0,
            .entry  = &_G.impl_list[first]
    };
}

static struct ce_api_entry api_next(struct ce_api_entry entry) {
    struct impl_list *impl_list = entry.entry;

    const uint32_t n = ce_array_size(impl_list->api) - 1;

    if (entry.idx >= n) {
        return (struct ce_api_entry) {};
    }

    return (struct ce_api_entry) {
            .api = impl_list->api[entry.idx + 1],
            .idx = entry.idx + 1,
            .entry  = impl_list
    };
}

void register_on_add(uint64_t name,
                     ce_api_on_add_t *on_add) {
    uint64_t idx = ce_hash_lookup(&_G.api_map, name, UINT64_MAX);

    if (UINT64_MAX == idx) {
        idx = ce_array_size(_G.on_add);
        ce_array_push(_G.on_add, 0, _G.allocator);
    }

    ce_hash_add(&_G.api_on_add_map, name, idx, _G.allocator);
    ce_array_push(_G.on_add[idx], on_add, _G.allocator);
}

static struct ce_api_a0 a0 = {
        .register_api = api_register_api,
        .first = api_first,
        .next = api_next,
        .exist = api_exist,
        .register_on_add = register_on_add,
};

struct ce_api_a0 *ce_api_a0 = &a0;

void api_init(struct ce_alloc *allocator) {
    _G = (struct ApiSystemGlobals) {
            .allocator = allocator
    };

    api_register_api("ce_api_a0", &a0);
}

void api_shutdown() {
    ce_hash_free(&_G.api_map, _G.allocator);
}

