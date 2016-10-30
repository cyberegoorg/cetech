#ifndef CETECH_ENTCOM_TYPES_H
#define CETECH_ENTCOM_TYPES_H

#include <engine/world/types.h>
#include <celib/containers/map.h>
#include <celib/yaml/yaml.h>

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

typedef void (*component_on_world_create_t)(world_t world);

typedef void (*component_on_world_destroy_t)(world_t world);

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
    component_on_world_create_t on_world_create;
    component_on_world_destroy_t on_world_destroy;
} component_clb_null = {0};


#endif //CETECH_ENTCOM_TYPES_H
