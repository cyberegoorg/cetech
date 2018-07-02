#ifndef CETECH_TRANSFORM_H
#define CETECH_TRANSFORM_H



//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

#define TRANSFORMATION_COMPONENT_NAME "transform"
#define TRANSFORM_COMPONENT CT_ID64_0(TRANSFORMATION_COMPONENT_NAME)

#define PROP_POSITION CT_ID64_0("position")
#define PROP_ROTATION CT_ID64_0("rotation")
#define PROP_SCALE CT_ID64_0("scale")

//==============================================================================
// Typedefs
//==============================================================================

struct ct_transform_comp {
    float position[3];
    float rotation[3];
    float scale[3];
    float world[16];
};


#endif //CETECH_TRANSFORM_H