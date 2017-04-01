//==============================================================================
// Includes
//==============================================================================

#include "celib/containers/map.h"
#include <engine/world/world.h>
#include <engine/memory/memory.h>
#include <engine/module/module.h>
#include <celib/string/stringid.h>


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
    struct handlerid world_handler;
} _G = {0};

IMPORT_API(MemSysApi, 0);

//==============================================================================
// Public interface
//==============================================================================
static void _init(get_api_fce_t get_engine_api) {
    _G = (struct G) {0};

    INIT_API(MemSysApi, MEMORY_API_ID, 0);

    ARRAY_INIT(world_callbacks_t, &_G.callbacks, MemSysApiV0.main_allocator());

    handlerid_init(&_G.world_handler, MemSysApiV0.main_allocator());

}

static void _shutdown() {
    handlerid_destroy(&_G.world_handler);
    ARRAY_DESTROY(world_callbacks_t, &_G.callbacks);
    _G = (struct G) {0};
}


void world_register_callback(world_callbacks_t clb) {
    ARRAY_PUSH_BACK(world_callbacks_t, &_G.callbacks, clb);
}

world_t world_create() {
    world_t w = {.h = handlerid_handler_create(&_G.world_handler)};

    for (int i = 0; i < ARRAY_SIZE(&_G.callbacks); ++i) {
        ARRAY_AT(&_G.callbacks, i).on_created(w);
    }

    return w;
}

void world_destroy(world_t world) {
    for (int i = 0; i < ARRAY_SIZE(&_G.callbacks); ++i) {
        ARRAY_AT(&_G.callbacks, i).on_destroy(world);
    }

    handlerid_handler_destroy(&_G.world_handler, world.h);
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


void *world_get_module_api(int api,
                           int version) {

    switch (api) {
        case PLUGIN_EXPORT_API_ID:
            switch (version) {
                case 0: {
                    static struct module_api_v0 module = {0};

                    module.init = _init;
                    module.shutdown = _shutdown;

                    return &module;
                }

                default:
                    return NULL;
            };
        case WORLD_API_ID:
            switch (version) {
                case 0: {
                    static struct WorldApiV0 api = {0};

                    api.register_callback = world_register_callback;
                    api.create = world_create;
                    api.destroy = world_destroy;
                    api.update = world_update;
//                    api.load_level = world_load_level;
//                    api.unit_by_id = level_unit_by_id;
//                    api.unit = level_unit ;

                    return &api;
                }

                default:
                    return NULL;
            };

        default:
            return NULL;
    }
}