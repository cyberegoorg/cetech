//==============================================================================
// Includes
//==============================================================================

#include <cetech/celib/map.inl>

#include <cetech/core/hash.h>
#include <cetech/core/module.h>
#include <cetech/core/api.h>
#include <cetech/core/config.h>


#include "../os/hash.inl"

IMPORT_API(hash_api_v0)

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


namespace api {
    void register_api(const char *name,
                      void *api) {
        uint64_t name_id = stringid64_from_string(name);


        multi_map::insert(_G.api_map, name_id, api);
    }

    api_entry first(const char *name) {
        uint64_t name_id = stringid64_from_string(name);

        auto first = multi_map::find_first(_G.api_map, name_id);

        return {.api = first->value, .entry  = (void *) first};
    }

    api_entry next(struct api_entry *entry) {
        auto map_entry = (const Map<void *>::Entry *) entry->entry;
        auto next = multi_map::find_next(_G.api_map, map_entry);

        if (!next) {
            return {0};
        }

        return {.api = next->value, .entry  = (void *) entry};
    }

    static struct api_v0 api_v0 = {
            .register_api = api::register_api,
            .first = api::first,
            .next = api::next
    };

    void init(struct allocator *allocator) {
        _G = {0};

        _G.api_map.init(allocator);

        api::register_api("api_v0", &api_v0);
    }

    void shutdown() {
        _G.api_map.destroy();
    }

    struct api_v0 *v0() {
        return &api_v0;
    }
}