//==============================================================================
// Includes
//==============================================================================

#include <celib/memory/allocator.h>
#include <celib/api.h>
#include <celib/containers/hash.h>
#include <celib/id.h>
#include <stdatomic.h>
#include <celib/memory/memory.h>
#include <celib/log.h>


//==============================================================================
// Defines
//==============================================================================

#define LOG_WHERE "api_system"

//==============================================================================
// Globals
//==============================================================================

#define _G ApiSystemGlobals
typedef struct impl_list {
    void **api;
} impl_list;

static struct _G {
    ce_hash_t api_map;

    ce_hash_t impl_map;
    impl_list *impl_list;

    ce_api_on_add_t0 **on_add;

    ce_alloc_t0 *allocator;
} _G;

//==============================================================================
// Private
//==============================================================================


static void api_register_api(const char *name,
                             void *api,
                             uint32_t size) {
    ce_log_a0->debug(LOG_WHERE, "Add api %s", name);

    uint64_t name_id = ce_id_a0->id64(name);

    void *mem = (void *) ce_hash_lookup(&_G.api_map, name_id, 0);

    if (!mem) {
        mem = CE_ALLOC(ce_memory_a0->system, char, size);
        ce_hash_add(&_G.api_map, name_id, (uint64_t) mem, _G.allocator);
    }

    if (api) {
        memcpy(mem, api, size);
    }

    ce_api_on_add_t0 **on_add = _G.on_add;
    const uint32_t on_add_n = ce_array_size(on_add);
    for (int i = 0; i < on_add_n; ++i) {
        on_add[i](name_id, mem);
    }

}

static void *get_api(const char* name) {
    uint64_t name_id = ce_id_a0->id64(name);
    void *mem = (void *) ce_hash_lookup(&_G.api_map, name_id, 0);

    if (!mem) {
        api_register_api(name, NULL, 0);
        return get_api(name);
    }

    return mem;
}

static void api_add_impl(const char *name,
                         void *api,
                         uint32_t size) {
    ce_log_a0->debug(LOG_WHERE, "Add impl %s", name);

    uint64_t name_id = ce_id_a0->id64(name);

    uint64_t idx = ce_hash_lookup(&_G.impl_map, name_id, UINT64_MAX);

    if (idx == UINT64_MAX) {
        idx = ce_array_size(_G.impl_list);
        ce_array_push(_G.impl_list, (impl_list) {}, ce_memory_a0->system);
        ce_hash_add(&_G.impl_map, name_id, idx, _G.allocator);
    }

    impl_list *il = &_G.impl_list[idx];

    ce_array_push(il->api, api, _G.allocator);

    ce_api_on_add_t0 **on_add = _G.on_add;
    const uint32_t on_add_n = ce_array_size(on_add);
    for (int i = 0; i < on_add_n; ++i) {
        on_add[i](name_id, api);
    }
}

static int api_exist(const char *name) {
    uint64_t name_id = ce_id_a0->id64(name);

    return ce_hash_contain(&_G.api_map, name_id);
}

static struct ce_api_entry_t0 api_first(uint64_t name) {
    uint64_t idx = ce_hash_lookup(&_G.impl_map, name, UINT64_MAX);

    if (idx == UINT64_MAX) {
        return (ce_api_entry_t0) {
                .api = NULL,
                .idx = 0,
                .entry  = NULL
        };
    }

    return (ce_api_entry_t0) {
            .api = _G.impl_list[idx].api[0],
            .idx = 0,
            .entry = &_G.impl_list[idx]
    };
}

static struct ce_api_entry_t0 api_next(ce_api_entry_t0 entry) {
    impl_list *impl_list = entry.entry;

    const uint32_t n = ce_array_size(impl_list->api) - 1;

    if (entry.idx >= n) {
        return (ce_api_entry_t0) {};
    }

    return (ce_api_entry_t0) {
            .api = impl_list->api[entry.idx + 1],
            .idx = entry.idx + 1,
            .entry  = impl_list
    };
}

void register_on_add(ce_api_on_add_t0 *on_add) {
    ce_array_push(_G.on_add, on_add, _G.allocator);
}

void remove_api(uint64_t name_id,
                void *api) {
    ce_log_a0->debug(LOG_WHERE, "Remove api %s", ce_id_a0->str_from_id64(name_id));

    ce_hash_remove(&_G.api_map, name_id);
}

void remove_impl(uint64_t name_id,
                 void *api) {
    ce_log_a0->debug(LOG_WHERE, "Remove impl %s", ce_id_a0->str_from_id64(name_id));

    uint64_t idx = ce_hash_lookup(&_G.impl_map, name_id, UINT64_MAX);
    impl_list *il = &_G.impl_list[idx];

    uint64_t n = ce_array_size(il->api);
    for (int i = 0; i < n; ++i) {
        if (il->api[i] != api) {
            continue;
        }

        il->api[i] = il->api[n - 1];
        ce_array_pop_back(il->api);
        break;
    }
}


static struct ce_api_a0 a0 = {
        .add_api = api_register_api,
        .remove_api = remove_api,
        .add_impl = api_add_impl,
        .remove_impl = remove_impl,
        .first = api_first,
        .next = api_next,
        .exist = api_exist,
        .get = get_api,
        .register_on_add = register_on_add,
};

struct ce_api_a0 *ce_api_a0 = &a0;

void api_init(ce_alloc_t0 *allocator) {
    _G = (struct _G) {
            .allocator = allocator,
    };


    api_register_api("ce_api_a0", &a0, sizeof(a0));
}

void api_shutdown() {
    ce_hash_free(&_G.api_map, _G.allocator);
}

