//==============================================================================
// Includes
//==============================================================================

#include <engine/memory_system/memory_system.h>
#include "../types.h"


//==============================================================================
// GLobals
//==============================================================================

#define _G EntityMaagerGlobals
static struct G {
    struct handlerid entity_handler;
} _G = {0};


//==============================================================================
// Public interface
//==============================================================================

int entity_manager_init() {
    _G = (struct G) {0};

    handlerid_init(&_G.entity_handler, memsys_main_allocator());

    return 1;
}

void entity_manager_shutdown() {
    handlerid_destroy(&_G.entity_handler);

    _G = (struct G) {0};
}

entity_t entity_manager_create() {
    return (entity_t) {.h = handlerid_handler_create(&_G.entity_handler)};
}

void entity_manager_destroy(entity_t entity) {
    handlerid_handler_destroy(&_G.entity_handler, entity.h);
}

int entity_manager_alive(entity_t entity) {
    return handlerid_handler_alive(&_G.entity_handler, entity.h);
}