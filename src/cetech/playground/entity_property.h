#ifndef CETECH_ENTITY_PROPERTY_H
#define CETECH_ENTITY_PROPERTY_H



//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>

struct ct_entity;
struct ct_world;
struct ct_cdb_obj_t;

//==============================================================================
// Typedefs
//==============================================================================
typedef void (*ct_ep_on_component)(struct ct_world world,
                                   struct ct_cdb_obj_t *obj);

//==============================================================================
// Api
//==============================================================================
struct ct_entity_property_a0 {
    void (*register_component)(uint64_t type,
                               ct_ep_on_component on_component);

    void (*unregister_component)(uint64_t type);
};


#endif //CETECH_ENTITY_PROPERTY_H
