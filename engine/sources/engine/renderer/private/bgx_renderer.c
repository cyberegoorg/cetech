//==============================================================================
// includes
//==============================================================================

#include <celib/stringid/types.h>
#include <celib/math/types.h>
#include <engine/world_system/camera.h>
#include <celib/stringid/stringid.h>
#include <engine/renderer/mesh_renderer.h>
#include "celib/window/window.h"
#include "../renderer.h"

#include "bgfx/c99/bgfxplatform.h"
#include "bgfx_texture_resource.h"
#include "bgfx_shader_resource.h"
#include "engine/renderer/material.h"

//==============================================================================
// GLobals
//==============================================================================

#define _G RendererGlobals
static struct G {
    stringid64_t shader_type;
    u32 size_width;
    u32 size_height;
    int capture;
    int vsync;
    int need_reset;
} _G = {0};


//==============================================================================
// Private
//==============================================================================

static u32 _get_reset_flags() {
    return (_G.capture ? BGFX_RESET_CAPTURE : 0) |
           (_G.vsync ? BGFX_RESET_VSYNC : 0);
}

//==============================================================================
// Interface
//==============================================================================

int renderer_init(int stage) {
    if (stage == 0) {
        return 1;
    }


    _G = (struct G) {0};

    texture_resource_init();
    shader_resource_init();
    material_resource_init();

    return 1;
}

void renderer_shutdown() {
    texture_resource_shutdown();
    shader_resource_shutdown();
    material_resource_shutdown();

    bgfx_shutdown();

    _G = (struct G) {0};
}

void renderer_create(window_t window) {
    bgfx_platform_data_t pd = {0};
    pd.nwh = window_native_window_ptr(window);
    pd.ndt = window_native_display_ptr(window);
    bgfx_set_platform_data(&pd);

    // TODO: from config
    bgfx_init(BGFX_RENDERER_TYPE_OPENGL, 0, 0, NULL, NULL);

    window_get_size(window, &_G.size_width, &_G.size_height);

    _G.need_reset = 1;
}

void renderer_set_debug(int debug) {
    if (debug) {
        bgfx_set_debug(BGFX_DEBUG_STATS);
    } else {
        bgfx_set_debug(BGFX_DEBUG_NONE);
    }
}

void renderer_render_world(world_t world,
                           camera_t camera,
                           viewport_t viewport) {
    if (CE_UNLIKELY(_G.need_reset)) {
        bgfx_reset(_G.size_width, _G.size_height, _get_reset_flags());
    }

    bgfx_set_view_clear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x66CCFFff, 1.0f, 0);

    mat44f_t view_matrix;
    mat44f_t proj_matrix;

    camera_get_project_view(world, camera, &proj_matrix, &view_matrix);
    bgfx_set_view_transform(0, view_matrix.f, proj_matrix.f);

    bgfx_set_view_rect(0, 0, 0, (uint16_t) _G.size_width, (uint16_t) _G.size_height);

    bgfx_touch(0);
    bgfx_dbg_text_clear(0, 0);

    mesh_render_all(world);

    bgfx_frame(0);
}

vec2f_t renderer_get_size() {
    vec2f_t result;

    result.x = _G.size_width;
    result.y = _G.size_height;

    return result;
}
