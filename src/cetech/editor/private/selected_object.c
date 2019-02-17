#include <celib/macros.h>
#include <celib/memory/allocator.h>
#include <celib/containers/hash.h>
#include <celib/memory/memory.h>
#include <celib/id.h>
#include "celib/macros.h"
#include "celib/module.h"
#include "celib/api.h"

#include "cetech/editor/selcted_object.h"

#define _G editor_globals

typedef struct slot_t {
    uint64_t *slots;
    uint64_t idx;
} slot_t;

static struct _G {
    ce_hash_t selected_object_map;
    slot_t *slots;
} _G;

static void set_selected_object(uint64_t context,
                                uint64_t obj) {
    uint64_t idx = ce_hash_lookup(&_G.selected_object_map, context, UINT64_MAX);

    if (idx == UINT64_MAX) {
        idx = ce_array_size(_G.slots);
        ce_array_push(_G.slots, (slot_t) {}, ce_memory_a0->system);

        ce_hash_add(&_G.selected_object_map, context, idx, ce_memory_a0->system);

        slot_t *slot = &_G.slots[idx];
        ce_array_push(slot->slots, 0, ce_memory_a0->system);
    }

    slot_t *slot = &_G.slots[idx];

    uint64_t last_idx = ce_array_size(slot->slots) - 1;

    if (slot->idx != last_idx) {
        slot->slots[slot->idx + 1] = obj;
    } else {
        ce_array_push(slot->slots, obj, ce_memory_a0->system);
    }

    ++slot->idx;
}

static uint64_t selected_object(uint64_t context) {
    uint64_t idx = ce_hash_lookup(&_G.selected_object_map, context, UINT64_MAX);

    if (idx == UINT64_MAX) {
        return 0;
    }

    slot_t *slot = &_G.slots[idx];
    return slot->slots[slot->idx];
}

void set_previous(uint64_t context) {
    uint64_t idx = ce_hash_lookup(&_G.selected_object_map, context, UINT64_MAX);

    if (idx == UINT64_MAX) {
        return;
    }

    slot_t *slot = &_G.slots[idx];

    if (!slot->idx) {
        return;
    }

    --slot->idx;
}

void set_next(uint64_t context) {
    uint64_t idx = ce_hash_lookup(&_G.selected_object_map, context, UINT64_MAX);

    if (idx == UINT64_MAX) {
        return;
    }

    slot_t *slot = &_G.slots[idx];

    uint64_t last_idx = ce_array_size(slot->slots) - 1;

    if (last_idx == slot->idx) {
        return;
    }

    ++slot->idx;
}

bool has_previous(uint64_t context) {
    uint64_t idx = ce_hash_lookup(&_G.selected_object_map, context, UINT64_MAX);

    if (idx == UINT64_MAX) {
        return false;
    }

    slot_t *slot = &_G.slots[idx];

    if (!slot->idx) {
        return false;
    }

    return true;

}

bool has_next(uint64_t context) {
    uint64_t idx = ce_hash_lookup(&_G.selected_object_map, context, UINT64_MAX);

    if (idx == UINT64_MAX) {
        return false;
    }

    slot_t *slot = &_G.slots[idx];
    uint64_t last_idx = ce_array_size(slot->slots) - 1;

    if (last_idx == slot->idx) {
        return false;
    }

    return true;
}

static struct ct_selected_object_a0 ct_selected_object_api0 = {
        .selected_object = selected_object,
        .set_selected_object = set_selected_object,
        .set_previous = set_previous,
        .set_next = set_next,
        .has_previous = has_previous,
        .has_next = has_next,
};

struct ct_selected_object_a0 *ct_selected_object_a0 = &ct_selected_object_api0;


static void _init(struct ce_api_a0 *api) {
    _G = (struct _G) {
    };

    api->register_api(CT_SELECTED_OBJECT_API,
                      &ct_selected_object_api0,
                      sizeof(ct_selected_object_api0));
}

static void _shutdown() {
    _G = (struct _G) {};
}

void CE_MODULE_LOAD(selected_object)(struct ce_api_a0 *api,
                                     int reload) {
    CE_UNUSED(reload);
    _init(api);
}

void CE_MODULE_UNLOAD(selected_object)(struct ce_api_a0 *api,
                                       int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
    _shutdown();
}

