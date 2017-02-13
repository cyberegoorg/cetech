#ifndef CETECH_LEVEL_H
#define CETECH_LEVEL_H

//==============================================================================
// Includes
//==============================================================================

#include <celib/types.h>
#include <celib/string/stringid.h>
#include "world.h"
#include "engine/entcom/entcom.h"


//==============================================================================
// Interface
//==============================================================================


typedef struct {
    u32 idx;
} level_t;

struct LevelApiV1 {
    level_t (*load_level)(world_t world,
                          stringid64_t name);

    void (*destroy)(world_t world,
                    level_t level);

    entity_t (*unit_by_id)(level_t level,
                           stringid64_t name);

    entity_t (*unit)(level_t level);
};


#endif //CETECH_LEVEL_H
