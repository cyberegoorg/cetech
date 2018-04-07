#ifndef CETECH_MESH_RENDERER_H
#define CETECH_MESH_RENDERER_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

#define MESH_RENDERER_COMPONENT CT_ID64_0("mesh_renderer")

#define PROP_SCENE CT_ID64_0("scene")
#define PROP_GEOM_COUNT CT_ID64_0("geom_count")

#define PROP_MESH_ID (CT_ID64_0("mesh_id") << 32)
#define PROP_NODE_ID (CT_ID64_0("node_id") << 32)
#define PROP_NODE (CT_ID64_0("node_id") << 32)
#define PROP_MATERIAL_ID (CT_ID64_0("material_id")<< 32)

struct ct_mesh_renderer {
    uint64_t scene;
    uint8_t geom_n;

    struct {
        uint64_t mesh_id;
        uint64_t node_id;
        ct_cdb_obj_t* material;
    } geometries[8];
};

//==============================================================================
// Typedefs
//==============================================================================

struct ct_world;
struct ct_entity;


//==============================================================================
// Api
//==============================================================================

//! Mesh API V0
struct ct_mesh_renderer_a0 {
    //! Render all mesh in world
    //! \param world Word
    void (*render_all)(struct ct_world world,
                       uint8_t viewid,
                       uint64_t layer_name);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_MESH_RENDERER_H