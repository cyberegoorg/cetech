//! \addtogroup Renderer
//! \{
#ifndef CETECH_MESH_RENDERER_H
#define CETECH_MESH_RENDERER_H

#include "celib/types.h"
#include "engine/entcom/entcom.h"
#include "engine/world/world.h"
#include "engine/renderer/material.h"

//! Mesh typedef
typedef struct {
    u32 idx;
} mesh_t;

//! Mesh API V0
struct MeshApiV0 {

    //! Is mesh valid
    //! \param mesh Mesh
    //! \return 1 if is valid else 0
    int (*is_valid)(mesh_t mesh);

    //! Has entity mesh renderer?
    //! \param world World
    //! \param entity Entity
    //! \return 1 if entity has mesh rendere else 0
    int (*has)(world_t world,
               entity_t entity);

    //! Get mesh render for entity
    //! \param world World
    //! \param entity Entity
    //! \return Mesh renderer
    mesh_t (*get)(world_t world,
                  entity_t entity);

    //! Create new mesh render
    //! \param world World
    //! \param entity Entity
    //! \param scene Scene
    //! \param mesh Mesh
    //! \param node Node
    //! \param material Material
    //! \return Mesh renderer
    mesh_t (*create)(world_t world,
                     entity_t entity,
                     stringid64_t scene,
                     stringid64_t mesh,
                     stringid64_t node,
                     stringid64_t material);

    //! Get mesh renderer material
    //! \param world World
    //! \param mesh Mesh
    //! \return Material
    material_t (*get_material)(world_t world,
                               mesh_t mesh);

    //! Set material
    //! \param world World
    //! \param mesh Mesh
    //! \param material Material
    void (*set_material)(world_t world,
                         mesh_t mesh,
                         stringid64_t material);

    //! Render all mesh in world
    //! \param world Word
    void (*render_all)(world_t world);
};


#endif //CETECH_MESH_RENDERER_H
//! \}