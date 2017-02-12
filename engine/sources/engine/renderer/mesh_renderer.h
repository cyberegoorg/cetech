#ifndef CETECH_MESH_RENDERER_H
#define CETECH_MESH_RENDERER_H

#include "celib/types.h"
#include "engine/entcom/entcom.h"
#include "engine/world/world.h"
#include "engine/renderer/material.h"

typedef struct {
    u32 idx;
} mesh_t;

struct MeshApiV1 {
    int (*is_valid)(mesh_t mesh);
    int (*has)(world_t world,entity_t entity);
    mesh_t (*get)(world_t world, entity_t entity);
    mesh_t (*create)(world_t world,entity_t entity, stringid64_t scene, stringid64_t mesh, stringid64_t node, stringid64_t material);
    material_t (*get_material)(world_t world,mesh_t mesh);
    void (*set_material)(world_t world,  mesh_t mesh, stringid64_t material);
    void (*render_all)(world_t world);
};


#endif //CETECH_MESH_RENDERER_H
