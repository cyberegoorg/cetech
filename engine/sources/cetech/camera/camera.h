//! \addtogroup World
//! \{
#ifndef CETECH_CAMERA_H
#define CETECH_CAMERA_H

//==============================================================================
// Includes
//==============================================================================

#include <cetech/world/world.h>
#include <cetech/entity/entity.h>


//==============================================================================
// Typedefs
//==============================================================================

//! Camera struct
typedef struct {
    uint32_t idx;
} camera_t;


enum {
    CAMERA_API_ID = 21,
};

//==============================================================================
// Api
//==============================================================================

//! Camera API V0
struct camera_api_v0 {

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
                       float near,
                       float far,
                       float fov);
};

#endif //CETECH_CAMERA_H

//! |}