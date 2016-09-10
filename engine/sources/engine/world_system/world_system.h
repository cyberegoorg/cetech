#ifndef CETECH_WORLD_SYSTEM_H
#define CETECH_WORLD_SYSTEM_H

//==============================================================================
// Includes
//==============================================================================

#include "types.h"


//==============================================================================
// Interface
//==============================================================================

typedef void (*world_on_created_t)(world_t world);

typedef void (*world_on_destroy_t)(world_t world);

typedef struct {
    world_on_created_t on_created;
    world_on_destroy_t on_destroy;
} world_callbacks_t;

int world_init();

void world_shutdown();

void world_register_callback(world_callbacks_t clb);

world_t world_create();

void world_destroy(world_t world);

#endif //CETECH_WORLD_SYSTEM_H
