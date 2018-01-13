//==============================================================================
// Includes
//==============================================================================

#include <cetech/api/api_system.h>
#include <celib/hash.h>
#include "celib/map.inl"

//CETECH_DECL_API(ct_hash_a0)

using namespace celib;

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "api_system"

//==============================================================================
// Globals
//==============================================================================

static struct ApiSystemGlobals {
    cel_hash_t api_map;
    void** api;
    cel_alloc* allocator;
} _G;

//==============================================================================
// Private
//==============================================================================

uint64_t stringid64_from_string(const char *);

namespace api {
    void register_api(const char *name,
                      void *api) {
        uint64_t name_id = stringid64_from_string(name);

        cel_array_push(_G.api, api, _G.allocator);
        cel_hash_add(&_G.api_map, name_id, cel_array_size(_G.api) - 1, _G.allocator);
    }

    int exist(const char *name) {
        uint64_t name_id = stringid64_from_string(name);

        return cel_hash_contain(&_G.api_map, name_id);
    }

    ct_api_entry first(const char *name) {
        uint64_t name_id = stringid64_from_string(name);

        uint64_t first = cel_hash_lookup(&_G.api_map, name_id, UINT64_MAX);

        if (first == UINT64_MAX) {
            return {};
        }

        return {.api = _G.api[first], .entry  = (void *) first};
    }

    ct_api_entry next(ct_api_entry *entry) {
//        auto map_entry = (const Map<void *>::Entry *) entry->entry;
//        auto next = multi_map::find_next(_G.api_map, map_entry);

//        if (!next) {
            return {};
//        }

//        return {.api = next->value, .entry  = (void *) entry};
    }

    static ct_api_a0 a0 = {
            .register_api = api::register_api,
            .first = api::first,
            .next = api::next,
            .exist = api::exist
    };

    void init(cel_alloc *allocator) {
        _G = {.allocator = allocator};

        api::register_api("ct_api_a0", &a0);
    }

    void shutdown() {
        cel_hash_free(&_G.api_map, _G.allocator);
    }

    ct_api_a0 *v0() {
        return &a0;
    }
}

extern "C" ct_api_a0 *ct_api_get() {
    return &api::a0;
}
