//==============================================================================
// includes
//==============================================================================

#include <cetech/core/math/math_types.h>
#include <cetech/core/memory/allocator.h>

#include <cetech/modules/camera/camera.h>
#include "../renderer.h"
#include <cetech/core/develop_system/develop.h>
#include <cetech/core/application.h>
#include <cetech/core/config.h>
#include <cetech/core/module.h>
#include <cetech/modules/world/world.h>
#include <cetech/core/os/window.h>
#include <cetech/core/api.h>

#include "bgfx/c99/platform.h"
#include "texture/texture.h"
#include "shader/shader.h"
#include "scene/scene.h"
#include "material/material.h"


IMPORT_API(cnsole_srv_api_v0);
IMPORT_API(mesh_renderer_api_v0);
IMPORT_API(camera_api_v0);
IMPORT_API(config_api_v0);
IMPORT_API(app_api_v0);
IMPORT_API(window_api_v0);

//==============================================================================
// GLobals
//==============================================================================

#define _G RendererGlobals
static struct G {
    uint64_t type;
    uint32_t size_width;
    uint32_t size_height;
    int capture;
    int vsync;
    int need_reset;
} _G = {0};


int material_init(struct api_v0 *api);

void material_shutdown();

//==============================================================================
// Private
//==============================================================================

static uint32_t _get_reset_flags() {
    return (_G.capture ? BGFX_RESET_CAPTURE : 0) |
           (_G.vsync ? BGFX_RESET_VSYNC : 0);
}

//==============================================================================
// Interface
//==============================================================================

static int _cmd_resize(mpack_node_t args,
                       mpack_writer_t *writer) {
    mpack_node_t width = mpack_node_map_cstr(args, "width");
    mpack_node_t height = mpack_node_map_cstr(args, "height");

    _G.size_width = (uint32_t) mpack_node_int(width);
    _G.size_height = (uint32_t) mpack_node_int(height);
    _G.need_reset = 1;

    return 0;
}


void renderer_create(window_t window) {
    bgfx_platform_data_t pd = {0};
    pd.nwh = window_api_v0.native_window_ptr(window);
    pd.ndt = window_api_v0.native_display_ptr(window);
    bgfx_set_platform_data(&pd);

    // TODO: from config
    bgfx_init(BGFX_RENDERER_TYPE_OPENGL, 0, 0, NULL, NULL);

    window_api_v0.size(window, &_G.size_width, &_G.size_height);

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
    if (CETECH_UNLIKELY(_G.need_reset)) {
        bgfx_reset(_G.size_width, _G.size_height, _get_reset_flags());
    }

    bgfx_set_view_clear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x66CCFFff,
                        1.0f, 0);

    mat44f_t view_matrix;
    mat44f_t proj_matrix;

    camera_api_v0.get_project_view(world, camera, &proj_matrix, &view_matrix);
    bgfx_set_view_transform(0, view_matrix.f, proj_matrix.f);

    bgfx_set_view_rect(0, 0, 0, (uint16_t) _G.size_width,
                       (uint16_t) _G.size_height);

    bgfx_touch(0);
    bgfx_dbg_text_clear(0, 0);

    mesh_renderer_api_v0.render_all(world);

    bgfx_frame(0);

    window_api_v0.update(app_api_v0.main_window());
}

vec2f_t renderer_get_size() {
    vec2f_t result;

    result.x = _G.size_width;
    result.y = _G.size_height;

    return result;
}

static void _init_api(struct api_v0 *api) {
    static struct renderer_api_v0 rendderer_api = {0};

    rendderer_api.create = renderer_create;
    rendderer_api.set_debug = renderer_set_debug;
    rendderer_api.get_size = renderer_get_size;
    rendderer_api.render_world = renderer_render_world;
    api->register_api("renderer_api_v0", &rendderer_api);

    static struct material_api_v0 material_api = {0};

    material_api.resource_create = material_create;
    material_api.get_texture_count = material_get_texture_count;
    material_api.set_texture = material_set_texture;
    material_api.set_vec4f = material_set_vec4f;
    material_api.set_mat33f = material_set_mat33f;
    material_api.set_mat44f = material_set_mat44f;
    material_api.use = material_use;
    material_api.submit = material_submit;

    api->register_api("material_api_v0", &material_api);
}


static void _init(struct api_v0 *api) {
    GET_API(api, cnsole_srv_api_v0);
    GET_API(api, mesh_renderer_api_v0);
    GET_API(api, camera_api_v0);
    GET_API(api, config_api_v0);
    GET_API(api, app_api_v0);
    GET_API(api, window_api_v0);

    _G = (struct G) {0};

    cvar_t daemon = config_api_v0.find("daemon");
    if (!config_api_v0.get_int(daemon)) {
        texture_init(api);
        shader_init(api);
        material_init(api);
        scene_init(api);

        cnsole_srv_api_v0.consolesrv_register_command("renderer.resize",
                                                      _cmd_resize);
    }
}

static void _shutdown() {
    cvar_t daemon = config_api_v0.find("daemon");
    if (!config_api_v0.get_int(daemon)) {
        texture_shutdown();
        shader_shutdown();
        material_shutdown();
        scene_shutdown();

        bgfx_shutdown();
    }

    _G = (struct G) {0};
}


void *renderer_get_module_api(int api) {

    switch (api) {
        case PLUGIN_EXPORT_API_ID: {
            static struct module_api_v0 module = {0};

            module.init = _init;
            module.init_api = _init_api;
            module.shutdown = _shutdown;

            return &module;
        }


        default:
            return NULL;
    }
}