#ifndef CETECH_ENTITY_MANAGER_TYPES_H
#define CETECH_ENTITY_MANAGER_TYPES_H

//==============================================================================
// Includes
//==============================================================================

#include "celib/os/vio_types.h"
#include "celib/handler/handlerid.h"
#include "celib/yaml/yaml.h"
#include "celib/types.h"

#include "engine/world_system/types.h"

//==============================================================================
// Typedef
//==============================================================================

typedef struct {
    handler_t h;
} entity_t;

struct component_data {
    stringid64_t type;
    u32 size;
};

typedef int (*component_compiler_t)(yaml_node_t body,
                                    ARRAY_T(u8) *data);

typedef void (*component_on_world_create_t)(world_t world);

typedef void (*component_on_world_destroy_t)(world_t world);

typedef void (*component_destroyer_t)(world_t world,
                                      entity_t *ents,
                                      size_t ent_count);

typedef void (*component_spawner_t)(world_t world,
                                    entity_t *ents,
                                    entity_t *ents_parent,
                                    size_t ent_count,
                                    void *data);

#endif //CETECH_ENTITY_MANAGER_TYPES_H
