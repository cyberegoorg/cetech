#ifndef CETECH_ENTITY_PROPERTY_H
#define CETECH_ENTITY_PROPERTY_H

#include <cstdint>

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>

struct ct_entity;
struct ct_world;

//==============================================================================
// Typedefs
//==============================================================================
typedef void (*ct_ep_on_component)(struct ct_world world, struct ct_entity entity);

//==============================================================================
// Api
//==============================================================================
struct ct_entity_property_a0 {
    void (*register_component)(ct_ep_on_component on_component);
    void (*unregister_component)(ct_ep_on_component on_component);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_ENTITY_PROPERTY_H
