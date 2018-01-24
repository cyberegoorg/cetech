//==============================================================================
// Includes
//==============================================================================

#include <cetech/engine/entity/entity.h>
#include "cetech/core/containers/handler.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/config/config.h"
#include "cetech/core/api/api_system.h"

#include "cetech/core/module/module.h"

CETECH_DECL_API(ct_memory_a0);

//==============================================================================
// Typedefs
//==============================================================================

//==============================================================================
// Globals
//==============================================================================

#define _G WorldGlobals
static struct WorldGlobals {
    ct_world_callbacks_t *callbacks;
    ct_alloc *allocator;

    ct_handler_t world_handler;
} WorldGlobals;

//==============================================================================
// Public interface
//==============================================================================

static void register_callback(ct_world_callbacks_t clb) {
    ct_array_push(_G.callbacks, clb, _G.allocator);
}

static ct_world create() {
    ct_world w = {.h = ct_handler_create(&_G.world_handler, _G.allocator)};

    for (uint32_t i = 0; i < ct_array_size(_G.callbacks); ++i) {
        _G.callbacks[i].on_created(w);
    }

    return w;
}

static void destroy(ct_world world) {
    for (uint32_t i = 0; i < ct_array_size(_G.callbacks); ++i) {
        _G.callbacks[i].on_destroy(world);
    }

    ct_handler_destroy(&_G.world_handler, world.h, _G.allocator);
}

static void update(ct_world world,
                   float dt) {
    for (uint32_t i = 0; i < ct_array_size(_G.callbacks); ++i) {
        if (_G.callbacks[i].on_update != NULL) {
            _G.callbacks[i].on_update(world, dt);
        }
    }
}

//==============================================================================
// Modules interface
//==============================================================================

static ct_world_a0 _api = {
        .register_callback = register_callback,
        .create = create,
        .destroy = destroy,
        .update = update
};


static void _init_api(ct_api_a0 *api) {
    api->register_api("ct_world_a0", &_api);
}


static void _init(ct_api_a0 *api) {
    _init_api(api);


    _G = {
            .allocator = ct_memory_a0.main_allocator(),
    };

}

static void _shutdown() {
    ct_array_free(_G.callbacks, _G.allocator);

    ct_handler_free(&_G.world_handler, _G.allocator);
}

CETECH_MODULE_DEF(
        world,
        {
            CETECH_GET_API(api, ct_memory_a0);
        },
        {
            CT_UNUSED(reload);
            _init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);

            _shutdown();

        }
)