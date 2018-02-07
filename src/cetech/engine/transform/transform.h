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

#define PROP_POSITION CT_ID64_0("position")
#define PROP_ROTATION CT_ID64_0("rotation")
#define PROP_SCALE CT_ID64_0("scale")
#define PROP_WORLD CT_ID64_0("world")

//==============================================================================
// Typedefs
//==============================================================================

struct ct_world;
struct ct_entity;

//==============================================================================
// Api
//==============================================================================

//! Transform API V0
struct ct_transform_a0 {
    //! Get world matrix
    //! \param world World
    //! \param transform transform
    //! \return World matrix
    void (*get_world_matrix)(struct ct_entity transform,
                             float *matrix);


    //! Link two transform
    //! \param world World
    //! \param parent Parent entity
    //! \param child Child entity
    void (*link)(struct ct_world world,
                 struct ct_entity parent,
                 struct ct_entity child);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_TRANSFORM_H
//! |}