#ifndef CETECH_ENTITY_MANAGER_H
#define CETECH_ENTITY_MANAGER_H

//==============================================================================
// Includes
//==============================================================================
#include <celib/containers/map.h>
#include <celib/yaml/yaml.h>
#include <celib/string/stringid.h>
#include <celib/handler/handlerid.h>
#include <engine/world/world.h>

typedef struct {
    union {
        handler_t h;
        u32 idx;
    };
} entity_t;

ARRAY_PROTOTYPE(entity_t);
MAP_PROTOTYPE(entity_t);

typedef int (*component_compiler_t)(yaml_node_t body,
                                    ARRAY_T(u8) *data);

typedef void (*component_destroyer_t)(world_t world,
                                      entity_t *ents,
                                      size_t ent_count);

typedef void (*component_spawner_t)(world_t world,
                                    entity_t *ents,
                                    u32 *cent,
                                    u32 *ents_parent,
                                    size_t ent_count,
                                    void *data);

static struct component_clb {
    component_spawner_t spawner;
    component_destroyer_t destroyer;
    world_on_created_t on_world_create;
    world_on_destroy_t on_world_destroy;
    world_on_update_t on_world_update;
} component_clb_null = {0};



struct EntComSystemApiV1 {
    entity_t (*entity_manager_create)();
    void (*entity_manager_destroy)(entity_t entity);
    int (*entity_manager_alive)(entity_t entity);
    void (*component_register_compiler)(stringid64_t type, component_compiler_t compiler,u32 spawn_order);
    int (*component_compile)(stringid64_t type, yaml_node_t body, ARRAY_T(u8) *data);
    u32 (*component_get_spawn_order)(stringid64_t type);
    void (*component_register_type)(stringid64_t type, struct component_clb clb);
    void (*component_spawn)(world_t world, stringid64_t type, entity_t *ent_ids, u32 *cent, u32 *ents_parent, u32 ent_count, void *data);
    void (*component_destroy)(world_t world, entity_t *ent, u32 count);
};


#endif //CETECH_ENTITY_MANAGER_H
