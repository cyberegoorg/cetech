#ifndef CETECH_WORLD_SYSTEM_H
#define CETECH_WORLD_SYSTEM_H

//==============================================================================
// Includes
//==============================================================================

#include <celib/stringid/types.h>
#include "engine/components/types.h"
#include "engine/core/types.h"


//==============================================================================
// Interface
//==============================================================================

typedef void (*world_on_created_t)(world_t world);

typedef void (*world_on_destroy_t)(world_t world);

typedef struct {
    world_on_created_t on_created;
    world_on_destroy_t on_destroy;
} world_callbacks_t;



int world_init(int stage);

void world_shutdown();

void world_register_callback(world_callbacks_t clb);

world_t world_create();

void world_destroy(world_t world);


level_t world_load_level(world_t world,
                         stringid64_t name);

entity_t level_unit_by_id(level_t level,
                          stringid64_t name);

entity_t level_unit(level_t level);

#endif //CETECH_WORLD_SYSTEM_H
