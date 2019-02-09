#ifndef CETECH_TRANSFORM_H
#define CETECH_TRANSFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define TRANSFORMATION_COMPONENT_NAME "transform"

#define TRANSFORM_SYSTEM \
    CE_ID64_0("transform", 0x69e14b13ad9b5315ULL)

#define TRANSFORM_COMPONENT \
    CE_ID64_0("transform", 0x69e14b13ad9b5315ULL)

#define PROP_POSITION_X \
    CE_ID64_0("position.x", 0x46b9c080961061caULL)
#define PROP_POSITION_Y \
    CE_ID64_0("position.y", 0xca45b3734c0b3e81ULL)
#define PROP_POSITION_Z \
    CE_ID64_0("position.z", 0x1c9cfdf1ce5b0b7aULL)


#define PROP_ROTATION_X \
    CE_ID64_0("rotation.x", 0xffe4bc05dfc40eb0ULL)
#define PROP_ROTATION_Y \
    CE_ID64_0("rotation.y", 0x9a0853d1050cffeeULL)
#define PROP_ROTATION_Z \
    CE_ID64_0("rotation.z", 0x9e83ba75a85de403ULL)


#define PROP_SCALE_X \
    CE_ID64_0("scale.x", 0x184e8fae1ef2a589ULL)
#define PROP_SCALE_Y \
    CE_ID64_0("scale.y", 0x604950590277f4fULL)
#define PROP_SCALE_Z \
    CE_ID64_0("scale.z", 0x6cbb9d53bbfbb80fULL)

#define PROP_WORLD \
    CE_ID64_0("world", 0x4d46ae3bbc0fb0f7ULL)

typedef struct ct_transform_comp {
    ce_vec3_t pos;
    ce_vec3_t rot;
    ce_vec3_t scale;
    float world[16];
}ct_transform_comp;

#ifdef __cplusplus
};
#endif

#endif //CETECH_TRANSFORM_H