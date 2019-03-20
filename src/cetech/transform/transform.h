#ifndef CETECH_TRANSFORM_H
#define CETECH_TRANSFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define TRANSFORMATION_COMPONENT_NAME "transform"

#define VEC2_CDB_TYPE \
    CE_ID64_0("vec2_t", 0x9363184a58efecc1ULL)

#define PROP_VEC2_X \
    CE_ID64_0("x", 0x8295151af8224269ULL)

#define PROP_VEC2_Y \
    CE_ID64_0("y", 0x9a5db2cd2c1fd6ceULL)

#define TRANSFORM_SYSTEM \
    CE_ID64_0("transform", 0x69e14b13ad9b5315ULL)

#define TRANSFORM_COMPONENT \
    CE_ID64_0("transform", 0x69e14b13ad9b5315ULL)

#define PROP_POSITION \
    CE_ID64_0("position", 0x8bbeb160190f613aULL)

#define PROP_ROTATION \
    CE_ID64_0("rotation", 0x2060566242789baaULL)

#define PROP_SCALE \
    CE_ID64_0("scale", 0xeec8c5fba3c8bc0bULL)

#define PROP_POSITION_X \
    CE_ID64_0("x", 0x8295151af8224269ULL)
#define PROP_POSITION_Y \
    CE_ID64_0("y", 0x9a5db2cd2c1fd6ceULL)
#define PROP_POSITION_Z \
    CE_ID64_0("z", 0x88a824e868c7c5efULL)

#define PROP_ROTATION_X \
    CE_ID64_0("x", 0x8295151af8224269ULL)
#define PROP_ROTATION_Y \
    CE_ID64_0("y", 0x9a5db2cd2c1fd6ceULL)
#define PROP_ROTATION_Z \
    CE_ID64_0("z", 0x88a824e868c7c5efULL)

#define PROP_SCALE_X \
    CE_ID64_0("x", 0x8295151af8224269ULL)
#define PROP_SCALE_Y \
    CE_ID64_0("y", 0x9a5db2cd2c1fd6ceULL)
#define PROP_SCALE_Z \
    CE_ID64_0("z", 0x88a824e868c7c5efULL)

typedef struct ct_transform_comp {
    ce_vec3_t pos;
    ce_vec3_t rot;
    ce_vec3_t scl;
    ce_mat4_t world;
}ct_transform_comp;

#ifdef __cplusplus
};
#endif

#endif //CETECH_TRANSFORM_H