#ifndef CETECH_RENDERER_H
#define CETECH_RENDERER_H

//==============================================================================
// Includes
//==============================================================================

#include <engine/world/camera.h>
#include "celib/window/types.h"
#include "celib/math/types.h"

//==============================================================================
// Interface
//==============================================================================


typedef int viewport_t;

struct RendererApiV1 {
    void (*create)(cel_window_t window);

    void (*set_debug)(int debug);

    cel_vec2f_t (*get_size)();

    void (*render_world)(world_t world,
                         camera_t camera,
                         viewport_t viewport);
};


#endif //CETECH_RENDERER_H
