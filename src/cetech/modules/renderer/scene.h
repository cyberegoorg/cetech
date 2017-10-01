#ifndef CETECH_SCENE_H
#define CETECH_SCENE_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

//==============================================================================
// Typedefs
//==============================================================================

struct ct_world;
struct ct_entity;

//==============================================================================
// Api
//==============================================================================

//! Texture API V0
struct ct_scene_a0 {
    void (*setVBIB)(uint64_t scene,
                    uint64_t geom_name);

    void (*create_graph)(struct ct_world world,
                         struct ct_entity entity,
                         uint64_t scene);

    uint64_t (*get_mesh_node)(uint64_t scene,
                              uint64_t mesh);

    void (*get_all_geometries)(uint64_t scene, char** geometries, uint32_t* count);
    void (*get_all_nodes)(uint64_t scene, char** geometries, uint32_t* count);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_SCENE_H