#ifndef CETECH_TRANSFORM_H
#define CETECH_TRANSFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define VEC2_CDB_TYPE \
    CE_ID64_0("vec2_t", 0x9363184a58efecc1ULL)

#define VEC3_CDB_TYPE \
    CE_ID64_0("vec3_t", 0x6112fca24c71dadULL)

#define VEC4_CDB_TYPE \
    CE_ID64_0("vec3_t", 0x6112fca24c71dadULL)

#define PROP_VEC_X \
    CE_ID64_0("x", 0x8295151af8224269ULL)
#define PROP_VEC_Y \
    CE_ID64_0("y", 0x9a5db2cd2c1fd6ceULL)

#define PROP_VEC_Z \
    CE_ID64_0("z", 0x88a824e868c7c5efULL)

#define PROP_VEC_W \
    CE_ID64_0("w", 0x22727cb14c3bb41dULL)

#define TRANSFORM_SYSTEM \
    CE_ID64_0("transform", 0x69e14b13ad9b5315ULL)

#define TRANSFORM_GROUP \
    CE_ID64_0("transform_group", 0xa091ac1b0c1e8522ULL)

#define PROP_POSITION \
    CE_ID64_0("position", 0x8bbeb160190f613aULL)

#define PROP_ROTATION \
    CE_ID64_0("rotation", 0x2060566242789baaULL)

#define PROP_SCALE \
    CE_ID64_0("scale", 0xeec8c5fba3c8bc0bULL)

#define POSITION_COMPONENT \
    CE_ID64_0("position", 0x8bbeb160190f613aULL)

#define ROTATION_COMPONENT \
    CE_ID64_0("rotation", 0x2060566242789baaULL)

#define SCALE_COMPONENT \
    CE_ID64_0("scale", 0xeec8c5fba3c8bc0bULL)

#define LOCAL_TO_WORLD_COMPONENT \
    CE_ID64_0("local_to_world", 0x7681f4b3f07eea41ULL)

#define LOCAL_TO_PARENT_COMPONENT \
    CE_ID64_0("local_to_parent", 0xdcaec9a80da958a0ULL)

typedef struct ct_position_c {
    ce_vec3_t pos;
} ct_position_c;

typedef struct ct_rotation_c {
    ce_vec4_t rot;
} ct_rotation_c;

typedef struct ct_scale_c {
    ce_vec3_t scl;
} ct_scale_c;

typedef struct ct_local_to_world_c {
    ce_mat4_t world;
} ct_local_to_world_c;

typedef struct ct_local_to_parent_c {
    ce_mat4_t local;
} ct_local_to_parent_c;


#ifdef __cplusplus
};
#endif

#endif //CETECH_TRANSFORM_H