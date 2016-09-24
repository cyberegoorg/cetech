#ifndef CETECH_LEVEL_H
#define CETECH_LEVEL_H

//==============================================================================
// Includes
//==============================================================================

#include "celib/stringid/types.h"
#include "engine/core/types.h"

//==============================================================================
// Interface
//==============================================================================

int level_init(int stage);

void level_shutdown();

level_t world_load_level(world_t world,
                         stringid64_t name);

entity_t level_unit_by_id(level_t level,
                          stringid64_t name);

entity_t level_unit(level_t level);

#endif //CETECH_LEVEL_H
