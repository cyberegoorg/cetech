#ifndef CETECH_MESH_RENDERER_H
#define CETECH_MESH_RENDERER_H

#include <stdint.h>

#define MESH_RENDERER_COMPONENT \
    CE_ID64_0("mesh_renderer", 0x345b95f8df017893ULL)

#define PROP_SCENE_ID \
    CE_ID64_0("scene", 0x9d0a795bfe818d19ULL)

#define PROP_MATERIAL \
    (CE_ID64_0("material", 0xeac0b497876adedfULL))

#define PROP_NODE \
    (CE_ID64_0("node", 0x5ae0930b5138a928ULL))

#define PROP_MESH \
    (CE_ID64_0("mesh", 0x48ff313713a997a1ULL))

struct ct_world;
struct ct_entity;

//! Mesh API V0
struct ct_mesh_renderer_a0 {
    void (*_)();
};

CE_MODULE(ct_mesh_renderer_a0);

#endif //CETECH_MESH_RENDERER_H