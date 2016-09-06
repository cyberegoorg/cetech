#ifndef CETECH_RENDERER_H
#define CETECH_RENDERER_H

//==============================================================================
// Includes
//==============================================================================

#include "celib/window/types.h"


//==============================================================================
// Interface
//==============================================================================

int renderer_init();

void renderer_shutdown();

void renderer_create(window_t window);

void renderer_set_debug(int debug);

typedef int world_t;
typedef int camera_t;
typedef int viewport_t;

void renderer_render_world(world_t world, camera_t camera, viewport_t viewport);

#endif //CETECH_RENDERER_H
