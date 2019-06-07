#ifndef CETECH_MESH_RENDERER_H
#define CETECH_MESH_RENDERER_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

typedef struct ct_mesh_component {
    uint64_t material;
    uint64_t scene;
    uint64_t node;
    uint64_t mesh;
} ct_mesh_component;

#define STATIC_MESH_COMPONENT \
    CE_ID64_0("static_mesh", 0x7445bff74058f566ULL)

#define PROP_SCENE_ID \
    CE_ID64_0("scene", 0x9d0a795bfe818d19ULL)

#define PROP_MATERIAL \
    CE_ID64_0("material", 0xeac0b497876adedfULL)

#define PROP_NODE \
    CE_ID64_0("node", 0x5ae0930b5138a928ULL)

#define PROP_MESH \
    CE_ID64_0("mesh", 0x48ff313713a997a1ULL)

#ifdef __cplusplus
};
#endif

#endif //CETECH_MESH_RENDERER_H