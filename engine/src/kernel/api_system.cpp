//==============================================================================
// Includes
//==============================================================================

#include <cetech/celib/map.inl>

#include <cetech/kernel/hash.h>
#include <cetech/kernel/module.h>
#include <cetech/kernel/api_system.h>
#include <cetech/kernel/config.h>

CETECH_DECL_API(ct_hash_api_v0)

using namespace cetech;

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "api_system"

//==============================================================================
// Globals
//==============================================================================

static struct ApiSystemGlobals {
    Map<void *> api_map;
} _G;

//==============================================================================
// Private
//==============================================================================

uint64_t stringid64_from_string(const char *);

namespace api {
    void register_api(const char *name,
                      void *api) {
        uint64_t name_id = stringid64_from_string(name);

        multi_map::insert(_G.api_map, name_id, api);
    }

    int exist(const char *name) {
        uint64_t name_id = stringid64_from_string(name);

        return map::has(_G.api_map, name_id);
    }

    ct_api_entry_v0 first(const char *name) {
        uint64_t name_id = stringid64_from_string(name);

        auto first = multi_map::find_first(_G.api_map, name_id);

        if (!first) {
            return {0};
        }

        return {.api = first->value, .entry  = (void *) first};
    }

    ct_api_entry_v0 next(struct ct_api_entry_v0 *entry) {
        auto map_entry = (const Map<void *>::Entry *) entry->entry;
        auto next = multi_map::find_next(_G.api_map, map_entry);

        if (!next) {
            return {0};
        }

        return {.api = next->value, .entry  = (void *) entry};
    }

    static struct ct_api_v0 api_v0 = {
            .register_api = api::register_api,
            .first = api::first,
            .next = api::next,
            .exist = api::exist
    };

    void init(struct ct_allocator *allocator) {
        _G = {0};

        _G.api_map.init(allocator);

        api::register_api("ct_api_v0", &api_v0);
    }

    void shutdown() {
        _G.api_map.destroy();
    }

    struct ct_api_v0 *v0() {
        return &api_v0;
    }
}

extern "C" struct ct_api_v0 *ce_api_get_v0() {
    return &api::api_v0;
}