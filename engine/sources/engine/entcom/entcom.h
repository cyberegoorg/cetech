#ifndef CETECH_ENTITY_MANAGER_H
#define CETECH_ENTITY_MANAGER_H

//==============================================================================
// Includes
//==============================================================================

#include <engine/world/world.h>
#include "celib/yaml/yaml.h"
#include "celib/containers/array.h"
#include "celib/containers/map.h"

//==============================================================================
// Entity interface
//==============================================================================

entity_t entity_manager_create();

void entity_manager_destroy(entity_t entity);

int entity_manager_alive(entity_t entity);

//==============================================================================
// Entity interface
//==============================================================================

void component_register_compiler(stringid64_t type,
                                 component_compiler_t compiler,
                                 u32 spawn_order);

int component_compile(stringid64_t type,
                      yaml_node_t body,
                      ARRAY_T(u8) *data);

u32 component_get_spawn_order(stringid64_t type);


void component_register_type(stringid64_t type,
                             struct component_clb clb);

void component_spawn(world_t world,
                     stringid64_t type,
                     entity_t *ent_ids,
                     u32 *cent,
                     u32 *ents_parent,
                     u32 ent_count,
                     void *data);

void component_destroy(world_t world,
                       entity_t *ent,
                       u32 count);


#endif //CETECH_ENTITY_MANAGER_H
