//! \defgroup Renderer
//! Renderer system
//! \{
#ifndef CETECH_RENDERER_H
#define CETECH_RENDERER_H

//==============================================================================
// Includes
//==============================================================================

#include <cetech/camera/camera.h>
#include "cetech/os/window.h"
#include "cetech/math/math_types.h"


//==============================================================================
// Typedefs
//==============================================================================

//! Material typedef
typedef struct {
    union {
        handler32_t h;
        uint32_t idx;
    };
} material_t;

//==============================================================================
// Api
//==============================================================================

//! Material API V0
struct material_api_v0 {

    //! Create new material
    //! \param name Material resource name
    //! \return Material
    material_t (*resource_create)(stringid64_t name);

    //! Get texture count in material
    //! \param material Material
    //! \return Texture count
    uint32_t (*get_texture_count)(material_t material);

    //! Set texture value
    //! \param material Material
    //! \param slot Slot Name
    //! \param texture Texture name
    void (*set_texture)(material_t material,
                        const char *slot,
                        stringid64_t texture);

    //! Set vec4f value
    //! \param material Material
    //! \param slot Variable name
    //! \param v Value
    void (*set_vec4f)(material_t material,
                      const char *slot,
                      cel_vec4f_t v);

    //! Set mat33f value
    //! \param material Material
    //! \param slot Variable name
    //! \param v Value
    void (*set_mat33f)(material_t material,
                       const char *slot,
                       mat33f_t v);

    //! Set mat44f value
    //! \param material Material
    //! \param slot Variable name
    //! \param v Value
    void (*set_mat44f)(material_t material,
                       const char *slot,
                       cel_mat44f_t v);

    //! Use material for actual render
    void (*use)(material_t material);

    //! Submit material for actual render
    void (*submit)(material_t material);
};

//==============================================================================
// Typedefs
//==============================================================================

//! Mesh typedef
typedef struct {
    uint32_t idx;
} mesh_renderer_t;


//==============================================================================
// Api
//==============================================================================

//! Mesh API V0
struct mesh_renderer_api_v0 {

    //! Is mesh valid
    //! \param mesh Mesh
    //! \return 1 if is valid else 0
    int (*is_valid)(mesh_renderer_t mesh);

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
    mesh_renderer_t (*get)(world_t world,
                           entity_t entity);

    //! Create new mesh render
    //! \param world World
    //! \param entity Entity
    //! \param scene Scene
    //! \param mesh Mesh
    //! \param node Node
    //! \param material Material
    //! \return Mesh renderer
    mesh_renderer_t (*create)(world_t world,
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
                               mesh_renderer_t mesh);

    //! Set material
    //! \param world World
    //! \param mesh Mesh
    //! \param material Material
    void (*set_material)(world_t world,
                         mesh_renderer_t mesh,
                         stringid64_t material);

    //! Render all mesh in world
    //! \param world Word
    void (*render_all)(world_t world);
};


//==============================================================================
// Tyedefs
//==============================================================================

typedef int viewport_t;


//==============================================================================
// Api
//==============================================================================

//! Render API V0
struct RendererApiV0 {
    //! Create renderer.
    //! \param window Window
    void (*create)(cel_window_t window);

    //! Set debug mode on/off
    //! \param debug True/False
    void (*set_debug)(int debug);

    //! Get renderer window size
    //! \return Renderer window size
    cel_vec2f_t (*get_size)();

    //! Render world
    //! \param world World
    //! \param camera Camera
    //! \param viewport Viewport
    void (*render_world)(world_t world,
                         camera_t camera,
                         viewport_t viewport);
};


#endif //CETECH_RENDERER_H
//! \}
