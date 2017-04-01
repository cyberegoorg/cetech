//! \addtogroup World
//! \{
#ifndef CETECH_CAMERA_H
#define CETECH_CAMERA_H

//==============================================================================
// Includes
//==============================================================================

#include "world.h"
#include "engine/entity/api.h"


//==============================================================================
// Typedefs
//==============================================================================

//! Camera struct
typedef struct {
    u32 idx;
} camera_t;


//==============================================================================
// Api
//==============================================================================

//! Camera API V0
struct CameraApiV0 {

    //! Has entity camera component?
    //! \param world World
    //! \param entity Entity
    //! \return 1 if has else 0
    int (*has)(world_t world,
               entity_t entity);

    //! Is camera valid?
    //! \param camera Camera
    //! \return 1 if is oK else 0
    int (*is_valid)(camera_t camera);


    //! Get camera project and view matrix
    //! \param world World
    //! \param camera Camera
    //! \param proj Project matrix
    //! \param view View Matrix
    void (*get_project_view)(world_t world,
                             camera_t camera,
                             cel_mat44f_t *proj,
                             cel_mat44f_t *view);


    //! Get camera component
    //! \param world World
    //! \param entity Entity
    //! \return Camera component
    camera_t (*get)(world_t world,
                    entity_t entity);

    //! Create camera
    //! \param world World
    //! \param entity Entity
    //! \param near Near
    //! \param far Far
    //! \param fov Fov
    //! \return New camera
    camera_t (*create)(world_t world,
                       entity_t entity,
                       f32 near,
                       f32 far,
                       f32 fov);
};

#endif //CETECH_CAMERA_H

//! |}