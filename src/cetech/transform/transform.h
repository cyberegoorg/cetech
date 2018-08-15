#ifndef CETECH_TRANSFORM_H
#define CETECH_TRANSFORM_H

#include <stdint.h>

#define TRANSFORMATION_COMPONENT_NAME "transform"

#define TRANSFORM_COMPONENT \
    CE_ID64_0("transform", 0x69e14b13ad9b5315ULL)

#define PROP_POSITION \
    CE_ID64_0("position", 0x8bbeb160190f613aULL)

#define PROP_ROTATION \
    CE_ID64_0("rotation", 0x2060566242789baaULL)

#define PROP_SCALE \
    CE_ID64_0("scale", 0xeec8c5fba3c8bc0bULL)

struct ct_transform_comp {
    float position[3];
    float rotation[3];
    float scale[3];
    float world[16];
};

#endif //CETECH_TRANSFORM_H