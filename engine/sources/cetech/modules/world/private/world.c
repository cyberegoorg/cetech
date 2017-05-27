//==============================================================================
// Includes
//==============================================================================

#include <cetech/core/map.inl>
#include <cetech/modules/world/world.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/config.h>
#include <cetech/modules/resource/resource.h>
#include <cetech/kernel/module.h>
#include <cetech/kernel/hash.h>
#include <cetech/core/handler.h>
#include <cetech/kernel/api.h>


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
    struct handler32gen *world_handler;
} _G = {0};

IMPORT_API(memory_api_v0);
IMPORT_API(handler_api_v0);

//==============================================================================
// Public interface
//==============================================================================

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

static void _init_api(struct api_v0* api){
    static struct world_api_v0 _api = {0};

    _api.register_callback = world_register_callback;
    _api.create = world_create;
    _api.destroy = world_destroy;
    _api.update = world_update;
//                    api.load_level = world_load_level;
//                    api.entity_by_id = level_entity_by_id;
//                    api.entity = level_entity ;

    api->register_api("world_api_v0", &_api);
}


static void _init( struct api_v0* api) {
    _G = (struct G) {0};

    GET_API(api, memory_api_v0 );
    GET_API(api, handler_api_v0 );



    ARRAY_INIT(world_callbacks_t, &_G.callbacks,
               memory_api_v0.main_allocator());

    _G.world_handler = handler_api_v0.handler32gen_create(
            memory_api_v0.main_allocator());
}

static void _shutdown() {
    handler_api_v0.handler32gen_destroy(_G.world_handler);
    ARRAY_DESTROY(world_callbacks_t, &_G.callbacks);
    _G = (struct G) {0};
}


void *world_get_module_api(int api) {

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