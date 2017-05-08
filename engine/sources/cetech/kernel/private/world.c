//==============================================================================
// Includes
//==============================================================================

#include "../../core/map.inl"
#include "../world.h"
#include "../../core/memory.h"
#include "../application.h"
#include "../config.h"
#include "../resource.h"
#include "../../core/module.h"
#include "../../core/hash.h"
#include "../../core/handler.h"


//==============================================================================
// Typedefs
//==============================================================================

ARRAY_T(world_t);
ARRAY_PROTOTYPE(world_callbacks_t);
ARRAY_PROTOTYPE(stringid64_t);


//==============================================================================
// Globals
//==============================================================================

#define _G WorldGlobals
static struct G {
    ARRAY_T(world_callbacks_t) callbacks;
    struct handler32gen* world_handler;
} _G = {0};

IMPORT_API(memory_api_v0);
IMPORT_API(handler_api_v0);

//==============================================================================
// Public interface
//==============================================================================
static void _init(get_api_fce_t get_engine_api) {
    _G = (struct G) {0};

    INIT_API(get_engine_api, memory_api_v0, MEMORY_API_ID);
    INIT_API(get_engine_api, handler_api_v0, HANDLER_API_ID);

    ARRAY_INIT(world_callbacks_t, &_G.callbacks, memory_api_v0.main_allocator());

    _G.world_handler = handler_api_v0.handler32gen_create(memory_api_v0.main_allocator());
}

static void _shutdown() {
    handler_api_v0.handler32gen_destroy(_G.world_handler);
    ARRAY_DESTROY(world_callbacks_t, &_G.callbacks);
    _G = (struct G) {0};
}


void world_register_callback(world_callbacks_t clb) {
    ARRAY_PUSH_BACK(world_callbacks_t, &_G.callbacks, clb);
}

world_t world_create() {
    world_t w = {.h = handler_api_v0.handler32_create(_G.world_handler)};

    for (int i = 0; i < ARRAY_SIZE(&_G.callbacks); ++i) {
        ARRAY_AT(&_G.callbacks, i).on_created(w);
    }

    return w;
}

void world_destroy(world_t world) {
    for (int i = 0; i < ARRAY_SIZE(&_G.callbacks); ++i) {
        ARRAY_AT(&_G.callbacks, i).on_destroy(world);
    }

    handler_api_v0.handler32_destroy(_G.world_handler, world.h);
}

void world_update(world_t world,
                  float dt) {
    for (int i = 0; i < ARRAY_SIZE(&_G.callbacks); ++i) {
        if (ARRAY_AT(&_G.callbacks, i).on_update != NULL) {
            ARRAY_AT(&_G.callbacks, i).on_update(world, dt);
            return;
        }
    }
}


void *world_get_module_api(int api) {

    switch (api) {
        case PLUGIN_EXPORT_API_ID:
                {
                    static struct module_api_v0 module = {0};

                    module.init = _init;
                    module.shutdown = _shutdown;

                    return &module;
                }


        case WORLD_API_ID:
            {
                    static struct world_api_v0 api = {0};

                    api.register_callback = world_register_callback;
                    api.create = world_create;
                    api.destroy = world_destroy;
                    api.update = world_update;
//                    api.load_level = world_load_level;
//                    api.entity_by_id = level_entity_by_id;
//                    api.entity = level_entity ;

                    return &api;
                }


        default:
            return NULL;
    }
}