#ifndef CETECH_LEVEL_H
#define CETECH_LEVEL_H

//==============================================================================
// Includes
//==============================================================================

#include <engine/entcom/types.h>
#include <celib/string/stringid.h>
#include "types.h"

//==============================================================================
// Interface
//==============================================================================

int level_init(int stage);

void level_shutdown();

level_t world_load_level(world_t world,
                         stringid64_t name);

void level_destroy(world_t world,
                   level_t level);

entity_t level_unit_by_id(level_t level,
                          stringid64_t name);

entity_t level_unit(level_t level);

#endif //CETECH_LEVEL_H
