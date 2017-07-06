//! \addtogroup World
//! \{
#ifndef CETECH_SCENE_GAPH_H
#define CETECH_SCENE_GAPH_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================


#include "stdint.h"
#include <cetech/celib/math_types.h>

//==============================================================================
// Typedefs
//==============================================================================

struct ct_world;
struct ct_entity;


//! Scene node struct
struct ct_scene_node {
    uint32_t idx;
};


//==============================================================================
// Api
//==============================================================================

//! Scene-graph API V0
struct ct_scenegprah_a0 {

    //! Is node valid?
    //! \param node Node
    //! \return 1 if is oK else 0
    int (*is_valid)(struct ct_scene_node node);

    //! Get node position
    //! \param world World
    //! \param node Node
    //! \return Position
    vec3f_t (*get_position)(struct ct_world world,
                            struct ct_scene_node node);

    //! Get node rotation
    //! \param world World
    //! \param node Node
    //! \return Rotation
    quatf_t (*get_rotation)(struct ct_world world,
                            struct ct_scene_node node);

    //! Get node scale
    //! \param world World
    //! \param node Node
    //! \return Scale
    vec3f_t (*get_scale)(struct ct_world world,
                         struct ct_scene_node node);

    //! Get world matrix
    //! \param world World
    //! \param node Node
    //! \return World matrix
    mat44f_t *(*get_world_matrix)(struct ct_world world,
                                  struct ct_scene_node node);

    //! Set position
    //! \param world World
    //! \param node Node
    //! \param pos Position
    void (*set_position)(struct ct_world world,
                         struct ct_scene_node node,
                         vec3f_t pos);

    //! Set rotation
    //! \param world World
    //! \param node Node
    //! \param rot Rotation
    void (*set_rotation)(struct ct_world world,
                         struct ct_scene_node node,
                         quatf_t rot);

    //! Set scale
    //! \param world World
    //! \param node Node
    //! \param scale Scale
    void (*set_scale)(struct ct_world world,
                      struct ct_scene_node node,
                      vec3f_t scale);

    //! Has entity scene-graph component?
    //! \param world World
    //! \param entity Entity
    //! \return 1 if has else 0
    int (*has)(struct ct_world world,
               struct ct_entity entity);

    //! Get root node
    //! \param world World
    //! \param entity Entity
    //! \return Root node
    struct ct_scene_node (*get_root)(struct ct_world world,
                             struct ct_entity entity);

    //! Create nodes
    //! \param world World
    //! \param entity Entity
    //! \param names Names map
    //! \param parent Parent map
    //! \param pose Pose map
    //! \param count Node count
    //! \return Root scene node
    struct ct_scene_node (*create)(struct ct_world world,
                           struct ct_entity entity,
                           uint64_t *names,
                           uint32_t *parent,
                           mat44f_t *pose,
                           uint32_t count);

    //! Link two node
    //! \param world World
    //! \param parent Parent node
    //! \param child Child node
    void (*link)(struct ct_world world,
                 struct ct_scene_node parent,
                 struct ct_scene_node child);

    //! Get node by name
    //! \param world World
    //! \param entity Entity
    //! \param name Name
    //! \return Node
    struct ct_scene_node (*node_by_name)(struct ct_world world,
                                 struct ct_entity entity,
                                 uint64_t name);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_SCENE_GAPH_H
//! |}