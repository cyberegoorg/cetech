//==============================================================================
// Includes
//==============================================================================

#include "celib/containers/map.h"
#include <engine/memory/memory.h>
#include <engine/module/module.h>

#include "engine/world/world.h"
#include "engine/entity/entity.h"

//==============================================================================
// Globals
//==============================================================================

#define _G EntityMaagerGlobals
static struct G {
    struct handlerid entity_handler;
} _G = {0};

IMPORT_API(MemSysApi, 0);

static void _init(get_api_fce_t get_engine_api) {
    INIT_API(MemSysApi, MEMORY_API_ID, 0);

    _G = (struct G) {0};

    handlerid_init(&_G.entity_handler, MemSysApiV0.main_allocator());
}

static void _shutdown() {
    handlerid_destroy(&_G.entity_handler);
    _G = (struct G) {0};
}

//==============================================================================
// Public interface
//==============================================================================

entity_t entity_manager_create() {
    return (entity_t) {.idx = handlerid_handler_create(&_G.entity_handler).h};
}

void entity_manager_destroy(entity_t entity) {
    handlerid_handler_destroy(&_G.entity_handler, entity.h);
}

int entity_manager_alive(entity_t entity) {
    return handlerid_handler_alive(&_G.entity_handler, entity.h);
}

void *entity_get_module_api(int api,
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
        case ENTITY_API_ID:
            switch (version) {
                case 0: {
                    static struct EntitySystemApiV0 api = {0};

                    api.entity_manager_create = entity_manager_create;
                    api.entity_manager_destroy = entity_manager_destroy;
                    api.entity_manager_alive = entity_manager_alive;

                    return &api;
                }

                default:
                    return NULL;
            };

        default:
            return NULL;
    }
}