//! \addtogroup World
//! \{
#ifndef CETECH_SCENE_GAPH_H
#define CETECH_SCENE_GAPH_H

//==============================================================================
// Includes
//==============================================================================


#include "stdint.h"
#include <cetech/core/math_types.h>

//==============================================================================
// Typedefs
//==============================================================================

typedef struct world_s world_t;
typedef struct entity_s entity_t;
typedef struct stringid64_s stringid64_t;

//! Scene node struct
typedef struct {
    uint32_t idx;
} scene_node_t;


//==============================================================================
// Api
//==============================================================================

//! Scene-graph API V0
struct scenegprah_api_v0 {

    //! Is node valid?
    //! \param node Node
    //! \return 1 if is oK else 0
    int (*is_valid)(scene_node_t node);

    //! Get node position
    //! \param world World
    //! \param node Node
    //! \return Position
    vec3f_t (*get_position)(world_t world,
                            scene_node_t node);

    //! Get node rotation
    //! \param world World
    //! \param node Node
    //! \return Rotation
    quatf_t (*get_rotation)(world_t world,
                            scene_node_t node);

    //! Get node scale
    //! \param world World
    //! \param node Node
    //! \return Scale
    vec3f_t (*get_scale)(world_t world,
                         scene_node_t node);

    //! Get world matrix
    //! \param world World
    //! \param node Node
    //! \return World matrix
    mat44f_t *(*get_world_matrix)(world_t world,
                                  scene_node_t node);

    //! Set position
    //! \param world World
    //! \param node Node
    //! \param pos Position
    void (*set_position)(world_t world,
                         scene_node_t node,
                         vec3f_t pos);

    //! Set rotation
    //! \param world World
    //! \param node Node
    //! \param rot Rotation
    void (*set_rotation)(world_t world,
                         scene_node_t node,
                         quatf_t rot);

    //! Set scale
    //! \param world World
    //! \param node Node
    //! \param scale Scale
    void (*set_scale)(world_t world,
                      scene_node_t node,
                      vec3f_t scale);

    //! Has entity scene-graph component?
    //! \param world World
    //! \param entity Entity
    //! \return 1 if has else 0
    int (*has)(world_t world,
               entity_t entity);

    //! Get root node
    //! \param world World
    //! \param entity Entity
    //! \return Root node
    scene_node_t (*get_root)(world_t world,
                             entity_t entity);

    //! Create nodes
    //! \param world World
    //! \param entity Entity
    //! \param names Names map
    //! \param parent Parent map
    //! \param pose Pose map
    //! \param count Node count
    //! \return Root scene node
    scene_node_t (*create)(world_t world,
                           entity_t entity,
                           stringid64_t *names,
                           uint32_t *parent,
                           mat44f_t *pose,
                           uint32_t count);

    //! Link two node
    //! \param world World
    //! \param parent Parent node
    //! \param child Child node
    void (*link)(world_t world,
                 scene_node_t parent,
                 scene_node_t child);

    //! Get node by name
    //! \param world World
    //! \param entity Entity
    //! \param name Name
    //! \return Node
    scene_node_t (*node_by_name)(world_t world,
                                 entity_t entity,
                                 stringid64_t name);
};


#endif //CETECH_SCENE_GAPH_H
//! |}