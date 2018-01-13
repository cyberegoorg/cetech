//==============================================================================
// Includes
//==============================================================================

#include <cetech/entity/entity.h>
#include "celib/array.inl"
#include "celib/handler.inl"
#include "cetech/os/memory.h"
#include "cetech/config/config.h"
#include "cetech/api/api_system.h"

#include "cetech/module/module.h"

CETECH_DECL_API(ct_memory_a0);

using namespace celib;

//==============================================================================
// Typedefs
//==============================================================================

//==============================================================================
// Globals
//==============================================================================


namespace {
#define _G WorldGlobals
    static struct WorldGlobals {
        ct_world_callbacks_t* callbacks;
        cel_alloc* allocator;

        Handler<uint32_t> world_handler;
    } WorldGlobals;
}

//==============================================================================
// Public interface
//==============================================================================

namespace world {
    void register_callback(ct_world_callbacks_t clb) {
        cel_array_push(_G.callbacks, clb, _G.allocator);
    }

    ct_world create() {
        ct_world w = {.h = handler::create(_G.world_handler)};

        for (uint32_t i = 0; i < cel_array_size(_G.callbacks); ++i) {
            _G.callbacks[i].on_created(w);
        }

        return w;
    }

    void destroy(ct_world world) {
        for (uint32_t i = 0; i < cel_array_size(_G.callbacks); ++i) {
            _G.callbacks[i].on_destroy(world);
        }

        handler::destroy(_G.world_handler, world.h);
    }

    void update(ct_world world,
                float dt) {
        for (uint32_t i = 0; i < cel_array_size(_G.callbacks); ++i) {
            if (_G.callbacks[i].on_update != NULL) {
                _G.callbacks[i].on_update(world, dt);
            }
        }
    }
}

//==============================================================================
// Modules interface
//==============================================================================

namespace world_module {
    static ct_world_a0 _api = {
            .register_callback = world::register_callback,
            .create = world::create,
            .destroy = world::destroy,
            .update = world::update
    };


    static void _init_api(ct_api_a0 *api) {
        api->register_api("ct_world_a0", &_api);
    }


    void _init(ct_api_a0 *api) {
        _init_api(api);


        _G = {
                .allocator = ct_memory_a0.main_allocator(),
        };

        _G.world_handler.init(ct_memory_a0.main_allocator());

    }

    void _shutdown() {
        cel_array_free(_G.callbacks, _G.allocator);

        _G.world_handler.destroy();
    }

}

CETECH_MODULE_DEF(
        world,
        {
            CETECH_GET_API(api, ct_memory_a0);
        },
        {
            CEL_UNUSED(reload);
            world_module::_init(api);
        },
        {
            CEL_UNUSED(reload);
            CEL_UNUSED(api);

            world_module::_shutdown();

        }
)