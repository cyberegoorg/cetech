//! \addtogroup World
//! \{
#ifndef CETECH_TRANSFORM_H
#define CETECH_TRANSFORM_H

#include "celib/math/types.h"
#include "engine/entcom/entcom.h"

//! Transform component
typedef struct {
    u32 idx;
} transform_t;

//! Transform API V1
struct TransformApiV1 {
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
                      cel_mat44f_t *parent);

    //! Get transform position
    //! \param world World
    //! \param transform transform
    //! \return Position
    cel_vec3f_t (*get_position)(world_t world,
                                transform_t transform);

    //! Get transform rotation
    //! \param world World
    //! \param transform transform
    //! \return Rotation
    cel_quatf_t (*get_rotation)(world_t world,
                                transform_t transform);

    //! Get transform scale
    //! \param world World
    //! \param transform transform
    //! \return Scale
    cel_vec3f_t (*get_scale)(world_t world,
                             transform_t transform);

    //! Get world matrix
    //! \param world World
    //! \param transform transform
    //! \return World matrix
    cel_mat44f_t *(*get_world_matrix)(world_t world,
                                      transform_t transform);

    //! Set position
    //! \param world World
    //! \param transform transform
    //! \param pos Position
    void (*set_position)(world_t world,
                         transform_t transform,
                         cel_vec3f_t pos);

    //! Set rotation
    //! \param world World
    //! \param transform transform
    //! \param rot Rotation
    void (*set_rotation)(world_t world,
                         transform_t transform,
                         cel_quatf_t rot);

    //! Set scale
    //! \param world World
    //! \param transform transform
    //! \param scale Scale
    void (*set_scale)(world_t world,
                      transform_t transform,
                      cel_vec3f_t scale);

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
                          cel_vec3f_t position,
                          cel_quatf_t rotation,
                          cel_vec3f_t scale);

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