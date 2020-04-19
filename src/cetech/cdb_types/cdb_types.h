#ifndef CETECH_CDB_TYPES_H
#define CETECH_CDB_TYPES_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define VEC2_CDB_TYPE \
    CE_ID64_0("vec2_t", 0x9363184a58efecc1ULL)

#define VEC3_CDB_TYPE \
    CE_ID64_0("vec3_t", 0x6112fca24c71dadULL)

#define VEC4_CDB_TYPE \
    CE_ID64_0("vec4_t", 0x406710bee42b16a2ULL)

#define PROP_VEC_X \
    CE_ID64_0("x", 0x8295151af8224269ULL)

#define PROP_VEC_Y \
    CE_ID64_0("y", 0x9a5db2cd2c1fd6ceULL)

#define PROP_VEC_Z \
    CE_ID64_0("z", 0x88a824e868c7c5efULL)

#define PROP_VEC_W \
    CE_ID64_0("w", 0x22727cb14c3bb41dULL)

#define COLOR4_CDB_TYPE \
    CE_ID64_0("color4_t", 0x683d352f1c973c7cULL)

#define PROP_COLOR_R \
    CE_ID64_0("r", 0xeb9e71988f8c8e3dULL)

#define PROP_COLOR_G \
    CE_ID64_0("g", 0xb5f2c769562d9a23ULL)

#define PROP_COLOR_B \
    CE_ID64_0("b", 0xea8bfc7d922a2a37ULL)

#define PROP_COLOR_A \
    CE_ID64_0("a", 0x71717d2d36b6b11ULL)

#ifdef __cplusplus
};
#endif

#endif //CETECH_CDB_TYPES_H