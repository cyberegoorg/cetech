//! \addtogroup World
//! \{
#ifndef CETECH_LEVEL_H
#define CETECH_LEVEL_H

//==============================================================================
// Includes
//==============================================================================

typedef struct world_s world_t;
typedef struct entity_s entity_t;
typedef struct stringid64_s stringid64_t;

enum {
    LEVEL_API_ID = 23,
};

//==============================================================================
// Typedefs
//==============================================================================

//! Level idx
typedef struct level_s {
    uint32_t idx;
} level_t;


//==============================================================================
// Api
//==============================================================================

//! Level API V0
struct level_api_v0 {

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

    //! Get entity in level by name
    //! \param level Level
    //! \param name Name
    //! \return Entity
    entity_t (*entity_by_id)(level_t level,
                             stringid64_t name);

    //! Get level entity
    //! \param level Level
    //! \return Level entity
    entity_t (*entity)(level_t level);
};


#endif //CETECH_LEVEL_H
//! |}