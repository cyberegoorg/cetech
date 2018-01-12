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

//==============================================================================
// Typedefs
//==============================================================================

struct ct_world;
struct ct_entity;

//==============================================================================
// Structs
//==============================================================================

//! Camera struct
struct ct_camera {
    struct ct_world world;
    uint32_t idx;
};


//==============================================================================
// Api
//==============================================================================

//! Camera API V0
struct ct_camera_a0 {

    //! Has entity camera component?
    //! \param world World
    //! \param entity Entity
    //! \return 1 if has else 0
    int (*has)(struct ct_world world,
               struct ct_entity entity);

    //! Is camera valid?
    //! \param camera Camera
    //! \return 1 if is oK else 0
    int (*is_valid)(struct ct_camera camera);

    //! Get camera project and view matrix
    //! \param world World
    //! \param camera Camera
    //! \param proj Project matrix
    //! \param view View Matrix
    void (*get_project_view)(struct ct_camera camera,
                             float *proj,
                             float *view,
                             int width,
                             int height);


    //! Get camera component
    //! \param world World
    //! \param entity Entity
    //! \return Camera component
    struct ct_camera (*get)(struct ct_world world,
                            struct ct_entity entity);

    //! Create camera
    //! \param world World
    //! \param entity Entity
    //! \param near Near
    //! \param far Far
    //! \param fov Fov
    //! \return New camera
    struct ct_camera (*create)(struct ct_world world,
                               struct ct_entity entity,
                               float near,
                               float far,
                               float fov);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_CAMERA_H

//! |}