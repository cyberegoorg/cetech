//==============================================================================
// Includes
//==============================================================================

#include <engine/world_system/types.h>
#include <engine/world_system/world_system.h>
#include "engine/memory_system/memory_system.h"
#include "celib/containers/map.h"

//==============================================================================
// Typedefs
//==============================================================================

ARRAY_T(world_t);
ARRAY_PROTOTYPE(world_callbacks_t);

MAP_T(world_t);

//==============================================================================
// GLobals
//==============================================================================


#define _G WorldGlobals
static struct G {
    struct handlerid world_handler;
    ARRAY_T(world_callbacks_t) callbacks;
} _G = {0};


//==============================================================================
// Public interface
//==============================================================================

int world_init() {
    _G = (struct G) {0};

    ARRAY_INIT(world_callbacks_t, &_G.callbacks, memsys_main_allocator());

    handlerid_init(&_G.world_handler, memsys_main_allocator());
    return 1;
}

void world_shutdown() {
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
