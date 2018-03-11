//! \addtogroup World
//! \{
#ifndef CETECH_TRANSFORM_H
#define CETECH_TRANSFORM_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

#define TRANSFORMATION_COMPONENT_NAME "transform"
#define TRANSFORM_COMPONENT CT_ID64_0(TRANSFORMATION_COMPONENT_NAME)

#define PROP_POSITION CT_ID64_0("position")
#define PROP_ROTATION CT_ID64_0("rotation")
#define PROP_SCALE CT_ID64_0("scale")
#define PROP_WORLD CT_ID64_0("world")

//==============================================================================
// Typedefs
//==============================================================================

struct ct_world;
struct ct_entity;

struct ct_transform_comp {
    float position[3];
    float scale[3];
    float rotation[3];
    float world[16];
};

//==============================================================================
// Api
//==============================================================================

//! Transform API V0
struct ct_transform_a0 {
    void (*_)();
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_TRANSFORM_H
//! |}