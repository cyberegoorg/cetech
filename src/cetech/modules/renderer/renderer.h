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

//! Material typedef
struct ct_material {
    uint32_t idx;
};

struct ct_viewport {
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
                       float *value);

    //! Submit material for actual render
    void (*submit)(struct ct_material material,  uint64_t layer, uint8_t viewid);
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
    void (*render_all)(struct ct_world world, uint8_t viewid);
};


//==============================================================================
// Tyedefs
//==============================================================================

typedef struct {
    uint64_t name;
    uint64_t type;
    uint8_t input_count;
    uint8_t output_count;
    uint64_t input[8];
    uint64_t output[8];
} layer_entry_t;

struct ct_texture {
    uint16_t idx;
};

#define MAX_LOCAL_RESOURCE 64
#define MAX_FRAME_BUFFERS 64

struct viewport_instance {
    uint16_t local_resource[MAX_LOCAL_RESOURCE];
    uint64_t local_resource_name[MAX_LOCAL_RESOURCE];

    uint16_t framebuffers[MAX_FRAME_BUFFERS];

    float size[2];

    layer_entry_t *layers;

    uint64_t layers_name;
    uint32_t layer_count;

    uint32_t fb_count;
    uint32_t resource_count;
};

typedef void (*ct_render_on_render)();
typedef void(*ct_renderer_on_pass_t)(viewport_instance* viewport, uint8_t viewid, struct ct_world world, struct ct_camera camera);

//==============================================================================
// Api
//==============================================================================

//! Render API V0
struct ct_renderer_a0 {
    //! Create renderer.
    void (*create)();

    //! Set debug mode on/off
    //! \param debug True/False
    void (*set_debug)(int debug);

    //! Get renderer window size
    //! \return Renderer window size
    void (*get_size)(uint32_t *width,
                     uint32_t *height);


    struct ct_texture (*get_global_resource)(uint64_t name);

    //! Render world
    //! \param world World
    //! \param camera Camera
    //! \param viewport Viewport
    void (*render_world)(struct ct_world world,
                         struct ct_camera camera,
                         struct ct_viewport viewport);

    void (*register_layer_pass)(uint64_t  type, ct_renderer_on_pass_t on_pass);

    void (*register_on_render)(ct_render_on_render on_render);
    void (*unregister_on_render)(ct_render_on_render on_render);

    struct ct_viewport (*create_viewport)(uint64_t name, float width, float height);
    uint16_t (*viewport_get_local_resource)(ct_viewport viewport,uint64_t name);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_RENDERER_H
//! \}
