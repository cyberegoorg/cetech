#ifndef CETECH_RENDERER_H
#define CETECH_RENDERER_H

//==============================================================================
// Includes
//==============================================================================

#include "engine/world_system/types.h"
#include "celib/window/types.h"
#include "celib/math/types.h"

//==============================================================================
// Interface
//==============================================================================

int renderer_init(int stage);

void renderer_shutdown();

void renderer_create(window_t window);

void renderer_set_debug(int debug);

vec2f_t renderer_get_size();

typedef int viewport_t;

void renderer_render_world(world_t world,
                           camera_t camera,
                           viewport_t viewport);

#endif //CETECH_RENDERER_H
