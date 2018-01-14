//==============================================================================
// Includes
//==============================================================================

#include <cetech/api/api_system.h>
#include <celib/hash.h>

//CETECH_DECL_API(ct_hash_a0)

//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "api_system"

//==============================================================================
// Globals
//==============================================================================

static struct ApiSystemGlobals {
    struct cel_hash_t api_map;
    void **api;
    struct cel_alloc *allocator;
} _G;

//==============================================================================
// Private
//==============================================================================

uint64_t stringid64_from_string(const char *);

static void api_register_api(const char *name,
                      void *api) {
    uint64_t name_id = stringid64_from_string(name);

    cel_array_push(_G.api, api, _G.allocator);
    cel_hash_add(&_G.api_map, name_id, cel_array_size(_G.api) - 1,
                 _G.allocator);
}

static int api_exist(const char *name) {
    uint64_t name_id = stringid64_from_string(name);

    return cel_hash_contain(&_G.api_map, name_id);
}

static struct ct_api_entry api_first(const char *name) {
    uint64_t name_id = stringid64_from_string(name);

    uint64_t first = cel_hash_lookup(&_G.api_map, name_id, UINT64_MAX);

    if (first == UINT64_MAX) {
        return (struct ct_api_entry){0};
    }

    return (struct ct_api_entry){.api = _G.api[first], .entry  = (void *) first};
}

static struct ct_api_entry api_next(struct ct_api_entry *entry) {
//        auto map_entry = (const Map<void *>::Entry *) entry->entry;
//        auto next = multi_map::find_next(_G.api_map, map_entry);

//        if (!next) {
    return (struct ct_api_entry){0};
//        }

//        return {.api = next->value, .entry  = (void *) entry};
}

static struct ct_api_a0 a0 = {
        .register_api = api_register_api,
        .first = api_first,
        .next = api_next,
        .exist = api_exist
};

void api_init(struct cel_alloc *allocator) {
    _G = (struct ApiSystemGlobals){
            .allocator = allocator
    };

    api_register_api("ct_api_a0", &a0);
}

void api_shutdown() {
    cel_hash_free(&_G.api_map, _G.allocator);
}

struct ct_api_a0 *api_v0() {
    return &a0;
}

struct ct_api_a0 *ct_api_get() {
    return &a0;
}
