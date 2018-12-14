#ifndef CETECH_SCENE_H
#define CETECH_SCENE_H

#include <stdint.h>
#include <celib/module.inl>

struct ct_world;
struct ct_entity;


#define SCENE_TYPE \
    CE_ID64_0("scene", 0x9d0a795bfe818d19ULL)

#define SCENE_IB_PROP   \
    CE_ID64_0("ib", 0xecb387261d2c25a3ULL)

#define SCENE_VB_PROP   \
    CE_ID64_0("vb", 0x483690c3614b4c35ULL)

#define SCENE_GEOM_COUNT \
    CE_ID64_0("geom_count", 0x423934fe3be0af59ULL)

#define SCENE_VB_DECL \
    CE_ID64_0("vb_decl", 0x1d8a0b163d414dbaULL)

#define SCENE_GEOM_NAME \
    CE_ID64_0("geom_name", 0x24fc37fc38807bbbULL)

#define SCENE_IB_OFFSET \
    CE_ID64_0("ib_offset", 0x728cf737dbc1e768ULL)

#define SCENE_VB_OFFSET \
    CE_ID64_0("vb_offset", 0x1cf22db14206ad07ULL)

#define SCENE_IB_SIZE \
    CE_ID64_0("ib_size", 0x9dcfa3f7770638d8ULL)

#define SCENE_VB_SIZE \
    CE_ID64_0("vb_size", 0xc5729c2a2f3777f9ULL)

#define SCENE_IB_LEN \
    CE_ID64_0("ib_len", 0x484276e7d0d8a15cULL)

#define SCENE_VB_LEN \
    CE_ID64_0("vb_len", 0x55671b5ab72a5c3dULL)

#define SCENE_GEOM_STR \
    CE_ID64_0("geom_str", 0xe808ceb7d7398434ULL)

#define SCENE_NODE_COUNT \
    CE_ID64_0("node_count", 0x8289cc88f4fc1b8dULL)

#define SCENE_NODE_NAME \
    CE_ID64_0("node_name", 0x9c24bc4702c0d052ULL)

#define SCENE_NODE_PARENT \
    CE_ID64_0("node_parent", 0x9a6f751ba923814cULL)

#define SCENE_NODE_POSE \
    CE_ID64_0("node_pose", 0x367dd98b4afd211eULL)

#define SCENE_NODE_GEOM \
    CE_ID64_0("geom_node", 0xc8167579ff5f9ee5ULL)

#define SCENE_NODE_STR \
    CE_ID64_0("node_str", 0x8449734a8dc39415ULL)


//==============================================================================
// Api
//==============================================================================

struct ct_scene_a0 {
    uint64_t (*get_mesh_node)(uint64_t scene,
                              uint64_t mesh);

    void (*get_all_geometries)(uint64_t scene,
                               char **geometries,
                               uint32_t *count);

    void (*get_all_nodes)(uint64_t scene,
                          char **geometries,
                          uint32_t *count);
};

CE_MODULE(ct_scene_a0);

#endif //CETECH_SCENE_H