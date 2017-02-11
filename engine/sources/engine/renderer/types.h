#ifndef CETECH_RENDERER_TYPES_H
#define CETECH_RENDERER_TYPES_H

//==============================================================================
// Includes
//==============================================================================

#include <engine/world/camera.h>
#include "celib/window/types.h"
#include "celib/math/types.h"

//==============================================================================
// Interface
//==============================================================================
typedef int viewport_t;

struct RendererApiV1 {
    void (*create)(cel_window_t window);
    void (*set_debug)(int debug);
    cel_vec2f_t (*get_size)();
    void (*render_world)(world_t world, camera_t camera, viewport_t viewport);
};

struct MaterialApiV1 {
    material_t (*resource_create)(stringid64_t name);
    u32 (*get_texture_count)(material_t material);
    void (*set_texture)(material_t material, const char *slot, stringid64_t texture);
    void (*set_vec4f)(material_t material, const char *slot, cel_vec4f_t v);
    void (*set_mat33f)(material_t material, const char *slot, mat33f_t v);
    void (*set_mat44f)(material_t material, const char *slot, cel_mat44f_t v);
    void (*use)(material_t material);
    void (*submit)(material_t material);
};

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

#endif //CETECH_RENDERER_TYPES_H
