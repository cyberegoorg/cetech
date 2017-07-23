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

struct ct_world;
struct ct_entity;

//==============================================================================
// Typedefs
//==============================================================================

//! Transform component
struct ct_transform {
    struct ct_world world;
    uint32_t idx;
};

//==============================================================================
// Api
//==============================================================================

//! Transform API V0
struct ct_transform_a0 {

    //! Is transform valid?
    //! \param transform transform
    //! \return 1 if is oK else 0
    int (*is_valid)(struct ct_transform transform);

    //! Calc world matrix
    //! \param world World
    //! \param transform Transform
    //! \param parent Parent matrix
    void (*transform)(struct ct_transform transform,
                      float *parent);

    //! Get transform position
    //! \param world World
    //! \param transform transform
    //! \return Position
    void (*get_position)(struct ct_transform transform, float* position);

    //! Get transform rotation
    //! \param world World
    //! \param transform transform
    //! \return Rotation
    void (*get_rotation)(struct ct_transform transform, float* rotation);

    //! Get transform scale
    //! \param world World
    //! \param transform transform
    //! \return Scale
    void (*get_scale)(struct ct_transform transform, float* scale);

    //! Get world matrix
    //! \param world World
    //! \param transform transform
    //! \return World matrix
    void (*get_world_matrix)(struct ct_transform transform, float* matrix);

    //! Set position
    //! \param world World
    //! \param transform transform
    //! \param pos Position
    void (*set_position)(struct ct_transform transform,
                         float* position);

    //! Set rotation
    //! \param world World
    //! \param transform transform
    //! \param rot Rotation
    void (*set_rotation)(struct ct_transform transform,
                         float* rotation);

    //! Set scale
    //! \param world World
    //! \param transform transform
    //! \param scale Scale
    void (*set_scale)(struct ct_transform transform,
                      float* scale);

    //! Has entity scene-graph component?
    //! \param world World
    //! \param entity Entity
    //! \return 1 if has else 0
    int (*has)(struct ct_world world,
               struct ct_entity entity);

    //! Get transformation
    //! \param world World
    //! \param entity Entity
    //! \return Transformation
    struct ct_transform (*get)(struct ct_world world,
                               struct ct_entity entity);

    //! Create transforms
    //! \param world World
    //! \param entity Entity
    //! \param names Names map
    //! \param parent Parent map
    //! \param pose Pose map
    //! \param count transform count
    //! \return Root scene transform
    struct ct_transform (*create)(struct ct_world world,
                                  struct ct_entity entity,
                                  struct ct_entity parent,
                                  float* position,
                                  float* rotation,
                                  float* scale);

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