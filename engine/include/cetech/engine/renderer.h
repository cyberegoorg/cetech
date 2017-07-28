//! \defgroup Renderer
//! Renderer system
//! \{
#ifndef CETECH_RENDERER_H
#define CETECH_RENDERER_H

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

struct ct_camera;
struct ct_world;
struct ct_entity;
struct ct_window;

typedef void ct_window_t;

//! Material typedef
struct ct_material {
    uint32_t idx;
};


//==============================================================================
// Api
//==============================================================================

//! Material API V0
struct ct_material_a0 {

    //! Create new material
    //! \param name Material resource name
    //! \return Material
    struct ct_material (*resource_create)(uint64_t name);

    //! Get texture count in material
    //! \param material Material
    //! \return Texture count
    uint32_t (*get_texture_count)(struct ct_material material);

    //! Set texture value
    //! \param material Material
    //! \param slot Slot Name
    //! \param texture Texture name
    void (*set_texture)(struct ct_material material,
                        const char *slot,
                        uint64_t texture);

    //! Set mat44f value
    //! \param material Material
    //! \param slot Variable name
    //! \param v Value
    void (*set_mat44f)(struct ct_material material,
                       const char *slot,
                       float* value);

    //! Use material for actual render
    void (*use)(struct ct_material material);

    //! Submit material for actual render
    void (*submit)(struct ct_material material);
};

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
                                      uint64_t mesh,
                                      uint64_t node,
                                      uint64_t material);

    //! Get mesh renderer material
    //! \param world World
    //! \param mesh Mesh
    //! \return Material
    struct ct_material (*get_material)(struct ct_mesh_renderer mesh);

    //! Set material
    //! \param world World
    //! \param mesh Mesh
    //! \param material Material
    void (*set_material)(struct ct_mesh_renderer mesh,
                         uint64_t material);

    //! Render all mesh in world
    //! \param world Word
    void (*render_all)(struct ct_world world);
};


//==============================================================================
// Tyedefs
//==============================================================================

typedef int viewport_t;

//==============================================================================
// Api
//==============================================================================

//! Render API V0
struct ct_renderer_a0 {
    //! Create renderer.
    //! \param window Window
    void (*create)(ct_window *window);

    //! Set debug mode on/off
    //! \param debug True/False
    void (*set_debug)(int debug);

    //! Get renderer window size
    //! \return Renderer window size
    void  (*get_size)(int *width, int *height);

    //! Render world
    //! \param world World
    //! \param camera Camera
    //! \param viewport Viewport
    void (*render_world)(struct ct_world world,
                         struct ct_camera camera,
                         viewport_t viewport);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_RENDERER_H
//! \}
