//==============================================================================
// Includes
//==============================================================================

#include <celib/memory/allocator.h>
#include <celib/api.h>
#include <celib/macros.h>
#include <celib/containers/hash.h>
#include <celib/id.h>
#include <stdatomic.h>
#include <sys/mman.h>
#include <celib/memory/memory.h>


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

typedef struct api_block_t api_block_t;
struct api_block_t {
    uint64_t block[256];
};

static struct _G {
    ce_hash_t api_map;

    impl_list *impl_list;

    ce_hash_t api_on_add_map;
    ce_api_on_add_t0 ***on_add;

    api_block_t *api_blocks;
    atomic_int num_blocks;

    ce_alloc_t0 *allocator;
} _G;

//==============================================================================
// Private
//==============================================================================
static void *_add_block() {
    uint64_t idx = atomic_fetch_add(&_G.num_blocks, 1);
    void *block = &_G.api_blocks[idx];
    memset(block, 0, sizeof(api_block_t));
    return block;
}


static void api_register_api(uint64_t name_id,
                             void *api,
                             uint32_t size) {
    uint64_t idx = ce_hash_lookup(&_G.api_map, name_id, UINT64_MAX);

    void *block = NULL;
    // create entry for api
    if (idx == UINT64_MAX) {
        idx = ce_array_size(_G.impl_list);

        ce_array_push(_G.impl_list, (impl_list) {},
                      _G.allocator);
        ce_hash_add(&_G.api_map, name_id, idx, _G.allocator);
    } else {
        //void *a = _G.impl_list[idx].api[0];
        //CE_ASSERT(LOG_WHERE, ((char *) a)[0] == 0);
    }

    void **apis = _G.impl_list[idx].api;
    uint32_t api_n = ce_array_size(apis);

    if (api_n == 1) {
        void *a = _G.impl_list[idx].api[0];
        uint64_t *au = a;

        if (!(*au)) {
            block = a;
        }
    }

    if (!block) {
        block = _add_block();
        ce_array_push(_G.impl_list[idx].api, block, _G.allocator);
    }

    if (api) {
        memcpy(block, api, size);
    }


    uint64_t on_add_idx = ce_hash_lookup(&_G.api_on_add_map, name_id,
                                         UINT64_MAX);

    if (UINT64_MAX != on_add_idx) {
        ce_api_on_add_t0 **on_add = _G.on_add[on_add_idx];
        const uint32_t on_add_n = ce_array_size(on_add);
        for (int i = 0; i < on_add_n; ++i) {
            on_add[i](name_id, block);
        }
    }
}

static void api_add_impl(uint64_t name_id,
                         void *api,
                         uint32_t size) {
    uint64_t idx = ce_hash_lookup(&_G.api_map, name_id, UINT64_MAX);

    void *block = NULL;
    // create entry for api
    if (idx == UINT64_MAX) {
        idx = ce_array_size(_G.impl_list);

        ce_array_push(_G.impl_list, (impl_list) {},
                      _G.allocator);
        ce_hash_add(&_G.api_map, name_id, idx, _G.allocator);
    }

    void **apis = _G.impl_list[idx].api;
    uint32_t api_n = ce_array_size(apis);

    if (api_n == 1) {
        void *a = _G.impl_list[idx].api[0];
        uint64_t *au = a;

        if (!(*au)) {
            block = a;
        }
    }

    if (!block) {
        block = _add_block();
        ce_array_push(_G.impl_list[idx].api, block, _G.allocator);
    }

    if (api) {
        memcpy(block, api, size);
    }


    uint64_t on_add_idx = ce_hash_lookup(&_G.api_on_add_map, name_id,
                                         UINT64_MAX);

    if (UINT64_MAX != on_add_idx) {
        ce_api_on_add_t0 **on_add = _G.on_add[on_add_idx];
        const uint32_t on_add_n = ce_array_size(on_add);
        for (int i = 0; i < on_add_n; ++i) {
            on_add[i](name_id, block);
        }
    }
}

static int api_exist(const char *name) {
    uint64_t name_id = ce_id_a0->id64(name);

    return ce_hash_contain(&_G.api_map, name_id);
}

static struct ce_api_entry_t0 api_first(uint64_t name) {
    uint64_t first = ce_hash_lookup(&_G.api_map, name, UINT64_MAX);

    if (first == UINT64_MAX) {
        api_register_api(name, NULL, 0);
        first = ce_hash_lookup(&_G.api_map, name, UINT64_MAX);
    }

    return (ce_api_entry_t0) {
            .api = _G.impl_list[first].api[0],
            .idx = 0,
            .entry  = &_G.impl_list[first]
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

void register_on_add(uint64_t name,
                     ce_api_on_add_t0 *on_add) {
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
        .add_impl = api_add_impl,
        .first = api_first,
        .next = api_next,
        .exist = api_exist,
        .register_on_add = register_on_add,
};

struct ce_api_a0 *ce_api_a0 = &a0;


void api_init(ce_alloc_t0 *allocator) {
    _G = (struct _G) {
            .allocator = allocator,
            .api_blocks = CE_ALLOC(ce_memory_a0->virt_system,
                                   api_block_t, sizeof(api_block_t) * 256),

    };


    api_register_api(CE_API_API, &a0, sizeof(a0));
}

void api_shutdown() {
    ce_hash_free(&_G.api_map, _G.allocator);
}

