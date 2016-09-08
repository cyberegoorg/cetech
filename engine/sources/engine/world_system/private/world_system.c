#include <engine/world_system/types.h>
#include "celib/handler/handlerid.h"
#include "engine/memory_system/memory_system.h"
#include "celib/containers/map.h"

ARRAY_T(world_t);
MAP_T(world_t);

#define _G WorldGlobals
static struct G {
    struct handlerid world_handler;
} _G = {0};

int world_init() {
    _G = (struct G) {0};

    handlerid_init(&_G.world_handler, memsys_main_allocator());
    return 1;
}

void world_shutdown() {
    handlerid_destroy(&_G.world_handler);

    _G = (struct G) {0};
}

world_t world_create() {
    return (world_t) {.h = handlerid_handler_create(&_G.world_handler)};
}

void world_destroy(world_t world) {
    handlerid_handler_destroy(&_G.world_handler, world.h);
}
