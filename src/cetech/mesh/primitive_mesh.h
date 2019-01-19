#ifndef CETECH_PRIMITIVE_MESH_H
#define CETECH_PRIMITIVE_MESH_H

#include <stdint.h>

struct ct_primitive_mesh{
    uint64_t material;
};


#define PRIMITIVE_MESH_COMPONENT \
    CE_ID64_0("primitive_mesh", 0x41994de0b780f885ULL)


#endif //CETECH_PRIMITIVE_MESH_H