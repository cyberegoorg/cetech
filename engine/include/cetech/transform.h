//! \addtogroup World
//! \{
#ifndef CETECH_TRANSFORM_H
#define CETECH_TRANSFORM_H

//==============================================================================
// Includes
//==============================================================================

#include "math_types.h"
#include <cetech/entity.h>

enum {
    TRANSFORM_API_ID = 25,
};

//==============================================================================
// Typedefs
//==============================================================================

//! Transform component
typedef struct {
    uint32_t idx;
} transform_t;


//==============================================================================
// Api
//==============================================================================

//! Transform API V0
struct transform_api_v0 {
    //! Is transform valid?
    //! \param transform transform
    //! \return 1 if is oK else 0
    int (*is_valid)(transform_t transform);

    //! Calc world matrix
    //! \param world World
    //! \param transform Transform
    //! \param parent Parent matrix
    void (*transform)(world_t world,
                      transform_t transform,
                      mat44f_t *parent);

    //! Get transform position
    //! \param world World
    //! \param transform transform
    //! \return Position
    vec3f_t (*get_position)(world_t world,
                                transform_t transform);

    //! Get transform rotation
    //! \param world World
    //! \param transform transform
    //! \return Rotation
    quatf_t (*get_rotation)(world_t world,
                                transform_t transform);

    //! Get transform scale
    //! \param world World
    //! \param transform transform
    //! \return Scale
    vec3f_t (*get_scale)(world_t world,
                             transform_t transform);

    //! Get world matrix
    //! \param world World
    //! \param transform transform
    //! \return World matrix
    mat44f_t *(*get_world_matrix)(world_t world,
                                      transform_t transform);

    //! Set position
    //! \param world World
    //! \param transform transform
    //! \param pos Position
    void (*set_position)(world_t world,
                         transform_t transform,
                         vec3f_t pos);

    //! Set rotation
    //! \param world World
    //! \param transform transform
    //! \param rot Rotation
    void (*set_rotation)(world_t world,
                         transform_t transform,
                         quatf_t rot);

    //! Set scale
    //! \param world World
    //! \param transform transform
    //! \param scale Scale
    void (*set_scale)(world_t world,
                      transform_t transform,
                      vec3f_t scale);

    //! Has entity scene-graph component?
    //! \param world World
    //! \param entity Entity
    //! \return 1 if has else 0
    int (*has)(world_t world,
               entity_t entity);

    //! Get transformation
    //! \param world World
    //! \param entity Entity
    //! \return Transformation
    transform_t (*get)(world_t world,
                       entity_t entity);

    //! Create transforms
    //! \param world World
    //! \param entity Entity
    //! \param names Names map
    //! \param parent Parent map
    //! \param pose Pose map
    //! \param count transform count
    //! \return Root scene transform
    transform_t (*create)(world_t world,
                          entity_t entity,
                          entity_t parent,
                          vec3f_t position,
                          quatf_t rotation,
                          vec3f_t scale);

    //! Link two transform
    //! \param world World
    //! \param parent Parent entity
    //! \param child Child entity
    void (*link)(world_t world,
                 entity_t parent,
                 entity_t child);
};

#endif //CETECH_TRANSFORM_H
//! |}