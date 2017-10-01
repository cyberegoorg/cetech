#ifndef CETECH_MESH_RENDERER_H
#define CETECH_MESH_RENDERER_H

#ifdef __cplusplus
extern "C" {
#endif

//==============================================================================
// Includes
//==============================================================================

#include <stdint.h>

//==============================================================================
// Typedefs
//==============================================================================

struct ct_world;
struct ct_entity;

//==============================================================================
// Typedefs
//==============================================================================

//! Mesh typedef
struct ct_mesh_renderer {
    struct ct_world world;
    uint32_t idx;
};

//==============================================================================
// Api
//==============================================================================

//! Mesh API V0
struct ct_mesh_renderer_a0 {

    //! Is mesh valid
    //! \param mesh Mesh
    //! \return 1 if is valid else 0
    int (*is_valid)(struct ct_mesh_renderer mesh);

    //! Has entity mesh renderer?
    //! \param world World
    //! \param entity Entity
    //! \return 1 if entity has mesh rendere else 0
    int (*has)(struct ct_world world,
               struct ct_entity entity);

    //! Get mesh render for entity
    //! \param world World
    //! \param entity Entity
    //! \return Mesh renderer
    struct ct_mesh_renderer (*get)(struct ct_world world,
                                   struct ct_entity entity);

    //! Create new mesh render
    //! \param world World
    //! \param entity Entity
    //! \param scene Scene
    //! \param mesh Mesh
    //! \param node Node
    //! \param material Material
    //! \return Mesh renderer
    struct ct_mesh_renderer (*create)(struct ct_world world,
                                      struct ct_entity entity,
                                      uint64_t scene,
                                      uint64_t *mesh,
                                      uint64_t *node,
                                      uint64_t *material,
                                      uint32_t geom_count);

    //! Get mesh renderer material
    //! \param world World
    //! \param mesh Mesh
    //! \return Material
    struct ct_material (*get_material)(struct ct_mesh_renderer mesh, uint32_t idx);

    //! Set material
    //! \param world World
    //! \param mesh Mesh
    //! \param material Material
    void (*set_material)(struct ct_mesh_renderer mesh,
                         uint32_t idx,
                         uint64_t material);

    //! Render all mesh in world
    //! \param world Word
    void (*render_all)(struct ct_world world,
                       uint8_t viewid,
                       uint64_t layer_name);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_MESH_RENDERER_H