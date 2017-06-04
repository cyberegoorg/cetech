//==============================================================================
// Includes
//==============================================================================

#include <cetech/celib/array2.inl>
#include <cetech/celib/handler.inl>
#include <cetech/core/memory.h>
#include <cetech/core/config.h>
#include <cetech/modules/resource.h>
#include <cetech/core/module.h>
#include <cetech/core/api.h>

#include <cetech/modules/world.h>

IMPORT_API(memory_api_v0);

using namespace cetech;

//==============================================================================
// Typedefs
//==============================================================================

//==============================================================================
// Globals
//==============================================================================


namespace {
    static struct WorldGlobals {
        Array<world_callbacks_t> callbacks;
        Handler<uint32_t> world_handler;
    } _G = {0};
}

//==============================================================================
// Public interface
//==============================================================================

namespace world {
    void register_callback(world_callbacks_t clb) {
        array::push_back(_G.callbacks, clb);
    }

    world_t create() {
        world_t w = {.h = handler::create(_G.world_handler)};

        for (int i = 0; i < array::size(_G.callbacks); ++i) {
            _G.callbacks[i].on_created(w);
        }

        return w;
    }

    void destroy(world_t world) {
        for (int i = 0; i < array::size(_G.callbacks); ++i) {
            _G.callbacks[i].on_destroy(world);
        }

        handler::destroy(_G.world_handler, world.h);
    }

    void update(world_t world,
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
    static struct world_api_v0 _api = {
            .register_callback = world::register_callback,
            .create = world::create,
            .destroy = world::destroy,
            .update = world::update
    };


    static void _init_api(struct api_v0 *api) {
        api->register_api("world_api_v0", &_api);
    }


    void _init(struct api_v0 *api) {
        GET_API(api, memory_api_v0);

        _G = {0};

        _G.callbacks.init(memory_api_v0.main_allocator());
        _G.world_handler.init(memory_api_v0.main_allocator());

    }

    void _shutdown() {
        _G = {0};
    }


    extern "C" void *world_get_module_api(int api) {

        switch (api) {
            case PLUGIN_EXPORT_API_ID: {
                static struct module_api_v0 module = {0};

                module.init = _init;
                module.init_api = _init_api;
                module.shutdown = _shutdown;

                return &module;
            }

            default:
                return NULL;
        }
    }
}