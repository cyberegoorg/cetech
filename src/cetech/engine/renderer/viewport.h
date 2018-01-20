#ifndef CETECH_VIEWPORT_H
#define CETECH_VIEWPORT_H

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
struct ct_texture;


struct ct_viewport {
    uint32_t idx;
};

//==============================================================================
// Tyedefs
//==============================================================================

typedef struct {
    uint64_t name;
    uint64_t type;
    uint8_t output_count;
    uint64_t output[8];
} layer_entry_t;

#define MAX_LOCAL_RESOURCE 64
#define MAX_FRAME_BUFFERS 64

struct viewport_instance {
    uint16_t local_resource[MAX_LOCAL_RESOURCE];
    uint64_t local_resource_name[MAX_LOCAL_RESOURCE];

    uint16_t framebuffers[MAX_FRAME_BUFFERS];

    float size[2];

    layer_entry_t *layers;

    uint32_t *layers_data_offset;
    uint8_t *layers_data;

    uint64_t viewport;
    uint32_t layer_count;

    uint32_t fb_count;
    uint32_t resource_count;
};

typedef int (*ct_viewport_pass_compiler)(struct ct_yamlng_node body,
                                         char **data);

typedef void(*ct_viewport_on_pass_t)(viewport_instance *viewport,
                                     ct_viewport viewport_id,
                                     uint8_t viewid,
                                     uint8_t layeridx,
                                     struct ct_world world,
                                     struct ct_camera camera);

//==============================================================================
// Api
//==============================================================================

//! Viewport API V0
struct ct_viewport_a0 {
    struct ct_viewport (*create)(uint64_t name,
                                 float width,
                                 float height);

    void (*register_pass_compiler)(uint64_t type,
                                   ct_viewport_pass_compiler compiler);

    void (*register_layer_pass)(uint64_t type,
                                ct_viewport_on_pass_t on_pass);

    struct ct_texture (*get_global_resource)(uint64_t name);

    struct ct_texture (*get_local_resource)(ct_viewport viewport,
                                            uint64_t name);

    void (*resize)(struct ct_viewport viewport,
                   float width,
                   float height);

    //! Render world
    //! \param world World
    //! \param camera Camera
    //! \param viewport Viewport
    void (*render_world)(struct ct_world world,
                         struct ct_camera camera,
                         struct ct_viewport viewport);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_VIEWPORT_H
