#ifndef CETECH_MESH_RENDERER_H
#define CETECH_MESH_RENDERER_H



//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

#define MESH_RENDERER_COMPONENT \
    CT_ID64_0("mesh_renderer", 0x345b95f8df017893ULL)

#define PROP_SCENE_ID \
    CT_ID64_0("scene_id", 0x418627ebacb56f11ULL)

#define PROP_MESH_ID \
    (CT_ID64_0("mesh_id", 0xba7a0a8001255d3fULL))

#define PROP_NODE_ID \
    (CT_ID64_0("node_id", 0xb0c5444873f5fc06ULL))

#define PROP_MATERIAL_ID \
    (CT_ID64_0("material_id", 0xb6e42505bc1c2a3aULL))

#define PROP_MATERIAL_REF \
    (CT_ID64_0("material_ref", 0x1be2b88dca67f200ULL))

#define PROP_SCENE \
    CT_ID64_0("scene", 0x9d0a795bfe818d19ULL)

#define PROP_NODE \
    (CT_ID64_0("node", 0x5ae0930b5138a928ULL))

#define PROP_MESH \
    (CT_ID64_0("mesh", 0x48ff313713a997a1ULL))

#define PROP_MATERIAL \
    (CT_ID64_0("material", 0xeac0b497876adedfULL))


struct ct_mesh_renderer {
    uint64_t scene_id;
    uint64_t mesh_id;
    uint64_t node_id;
    uint64_t material;
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

CT_MODULE(ct_mesh_renderer_a0);

#endif //CETECH_MESH_RENDERER_H