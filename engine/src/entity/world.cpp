//==============================================================================
// Includes
//==============================================================================

#include <celib/array.inl>
#include <celib/handler.inl>
#include <cetech/memory.h>
#include <cetech/config.h>
#include <cetech/resource.h>
#include <cetech/api_system.h>

#include <cetech/entity.h>

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
        Array<ct_world_callbacks_t> callbacks;
        Handler<uint32_t> world_handler;
    } WorldGlobals;
}

//==============================================================================
// Public interface
//==============================================================================

namespace world {
    void register_callback(ct_world_callbacks_t clb) {
        array::push_back(_G.callbacks, clb);
    }

    ct_world create() {
        ct_world w = {.h = handler::create(_G.world_handler)};

        for (int i = 0; i < array::size(_G.callbacks); ++i) {
            _G.callbacks[i].on_created(w);
        }

        return w;
    }

    void destroy(ct_world world) {
        for (int i = 0; i < array::size(_G.callbacks); ++i) {
            _G.callbacks[i].on_destroy(world);
        }

        handler::destroy(_G.world_handler, world.h);
    }

    void update(ct_world world,
                float dt) {
        for (int i = 0; i < array::size(_G.callbacks); ++i) {
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

        CETECH_GET_API(api, ct_memory_a0);

        _G = {0};

        _G.callbacks.init(ct_memory_a0.main_allocator());
        _G.world_handler.init(ct_memory_a0.main_allocator());

    }

    void _shutdown() {
        _G.callbacks.destroy();
        _G.world_handler.destroy();
    }


    extern "C" void world_load_module(ct_api_a0 *api) {
        _init(api);
    }

    extern "C" void world_unload_module(ct_api_a0 *api) {
        _shutdown();
    }
}