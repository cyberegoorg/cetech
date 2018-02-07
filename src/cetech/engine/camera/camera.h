//! \addtogroup World
//! \{
#ifndef CETECH_CAMERA_H
#define CETECH_CAMERA_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stddef.h>

#define PROP_FOV CT_ID64_0("fov")
#define PROP_NEAR CT_ID64_0("near")
#define PROP_FAR CT_ID64_0("far")
#define PROP_CAMERA_VIEWPORT CT_ID64_0("camera.viewport")
#define PROP_PROJECTION CT_ID64_0("projection")
#define PROP_VIEW CT_ID64_0("view")


//==============================================================================
// Typedefs
//==============================================================================

struct ct_world;
struct ct_entity;

//==============================================================================
// Api
//==============================================================================

//! Camera API V0
struct ct_camera_a0 {
    //! Get camera project and view matrix
    //! \param world World
    //! \param camera Camera
    //! \param proj Project matrix
    //! \param view View Matrix
    void (*get_project_view)(struct ct_world world,
                             struct ct_entity entity,
                             float *proj,
                             float *view,
                             int width,
                             int height);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_CAMERA_H

//! |}