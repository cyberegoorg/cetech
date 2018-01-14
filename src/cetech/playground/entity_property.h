#ifndef CETECH_ENTITY_PROPERTY_H
#define CETECH_ENTITY_PROPERTY_H

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
typedef void (*ct_ep_on_component)(struct ct_world world,
                                   struct ct_entity entity,
                                   const char *filename,
                                   uint64_t *keys,
                                   uint32_t keys_count);

//==============================================================================
// Api
//==============================================================================
struct ct_entity_property_a0 {
    void (*register_component)(uint64_t type, ct_ep_on_component on_component);

    void (*unregister_component)(uint64_t type);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_ENTITY_PROPERTY_H
