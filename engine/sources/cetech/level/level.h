//! \addtogroup World
//! \{
#ifndef CETECH_LEVEL_H
#define CETECH_LEVEL_H

//==============================================================================
// Includes
//==============================================================================

#include <celib/types.h>
#include <celib/string/stringid.h>
#include <cetech/world/world.h>
#include <cetech/entity/entity.h>


//==============================================================================
// Typedefs
//==============================================================================

//! Level idx
typedef struct {
    u32 idx;
} level_t;


//==============================================================================
// Api
//==============================================================================

//! Level API V0
struct LevelApiV0 {

    //! Load level from resource
    //! \param world World
    //! \param name Resource name
    //! \return New level
    level_t (*load_level)(world_t world,
                          stringid64_t name);

    //! Destroy level
    //! \param world World
    //! \param level Level
    void (*destroy)(world_t world,
                    level_t level);

    //! Get unit in level by name
    //! \param level Level
    //! \param name Name
    //! \return Entity
    entity_t (*unit_by_id)(level_t level,
                           stringid64_t name);

    //! Get level unit
    //! \param level Level
    //! \return Level unit
    entity_t (*unit)(level_t level);
};


#endif //CETECH_LEVEL_H
//! |}