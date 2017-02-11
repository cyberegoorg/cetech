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


void renderer_create(cel_window_t window);

void renderer_set_debug(int debug);

cel_vec2f_t renderer_get_size();

typedef int viewport_t;

void renderer_render_world(world_t world,
                           camera_t camera,
                           viewport_t viewport);

#endif //CETECH_RENDERER_H
