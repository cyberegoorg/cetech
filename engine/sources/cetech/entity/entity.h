//! \defgroup Entity
//! Entity system
//! \{
#ifndef CETECH_ENTITY_MANAGER_H
#define CETECH_ENTITY_MANAGER_H

//==============================================================================
// Includes
//==============================================================================

#include <celib/containers/map.h>
#include <celib/yaml/yaml.h>
#include <celib/string/stringid.h>
#include <celib/handler/handlerid.h>
#include <cetech/world/world.h>


//==============================================================================
// Typedefs
//==============================================================================

//! Entity typedef
typedef struct {
    union {
        handler_t h;
        u32 idx;
    };
} entity_t;

ARRAY_PROTOTYPE(entity_t);
MAP_PROTOTYPE(entity_t);

//==============================================================================
// Api
//==============================================================================

//! Entity system API V0
struct EntitySystemApiV0 {
    //! Create new entity
    //! \return New entity
    entity_t (*entity_manager_create)();

    //! Destoy entity
    //! \param entity Entity to destroy
    void (*entity_manager_destroy)(entity_t entity);

    //! Is entity alive?
    //! \param entity Entity
    //! \return 1 if entity is alive else 0
    int (*entity_manager_alive)(entity_t entity);
};

#endif //CETECH_ENTITY_MANAGER_H
//! \}