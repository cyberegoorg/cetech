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
struct ct_texture;

//==============================================================================
// Api
//==============================================================================

typedef void (*ct_render_on_render)();

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

    void (*register_on_render)(ct_render_on_render on_render);
    void (*unregister_on_render)(ct_render_on_render on_render);
};

#ifdef __cplusplus
}
#endif

#endif //CETECH_RENDERER_H
//! \}
