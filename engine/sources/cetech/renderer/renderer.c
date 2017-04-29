//==============================================================================
// includes
//==============================================================================

#include <cetech/math_types.h>
#include <cetech/allocator.h>

#include <cetech/camera.h>
#include <cetech/renderer.h>
#include <cetech/develop.h>
#include <cetech/application.h>
#include <cetech/config.h>
#include <cetech/module.h>
#include "cetech/window.h"

#include "bgfx/c99/platform.h"
#include <cetech/renderer/texture/texture.h>
#include <cetech/renderer/shader/shader.h>
#include <cetech/renderer/scene/scene.h>
#include <cetech/renderer/material/material.h>


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
    stringid64_t type;
    uint32_t size_width;
    uint32_t size_height;
    int capture;
    int vsync;
    int need_reset;
} _G = {0};


int material_init(get_api_fce_t get_engine_api);

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

static void _init(get_api_fce_t get_engine_api) {
    INIT_API(get_engine_api, cnsole_srv_api_v0, CONSOLE_SERVER_API_ID);
    INIT_API(get_engine_api, mesh_renderer_api_v0, MESH_API_ID);
    INIT_API(get_engine_api, camera_api_v0, CAMERA_API_ID);
    INIT_API(get_engine_api, config_api_v0, CONFIG_API_ID);
    INIT_API(get_engine_api, app_api_v0, APPLICATION_API_ID);
    INIT_API(get_engine_api, window_api_v0, WINDOW_API_ID);

    _G = (struct G) {0};

    cvar_t daemon = config_api_v0.find("daemon");
    if (!config_api_v0.get_int(daemon)) {
        texture_init();
        shader_init();
        material_init(get_engine_api);
        scene_init();

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


void renderer_create(cel_window_t window) {
    bgfx_platform_data_t pd = {0};
    pd.nwh = window_api_v0.native_window_ptr(window);
    pd.ndt = window_api_v0.native_display_ptr(window);
    bgfx_set_platform_data(&pd);

    // TODO: from config
    bgfx_init(BGFX_RENDERER_TYPE_OPENGL, 0, 0, NULL, NULL);

    window_api_v0.get_size(window, &_G.size_width, &_G.size_height);

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
    if (CEL_UNLIKELY(_G.need_reset)) {
        bgfx_reset(_G.size_width, _G.size_height, _get_reset_flags());
    }

    bgfx_set_view_clear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x66CCFFff,
                        1.0f, 0);

    cel_mat44f_t view_matrix;
    cel_mat44f_t proj_matrix;

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

cel_vec2f_t renderer_get_size() {
    cel_vec2f_t result;

    result.x = _G.size_width;
    result.y = _G.size_height;

    return result;
}


void *renderer_get_module_api(int api) {

    switch (api) {
        case PLUGIN_EXPORT_API_ID:
                {
                    static struct module_api_v0 module = {0};

                    module.init = _init;
                    module.shutdown = _shutdown;

                    return &module;
                }

        case RENDERER_API_ID:
                {
                    static struct renderer_api_v0 api = {0};

                    api.create = renderer_create;
                    api.set_debug = renderer_set_debug;
                    api.get_size = renderer_get_size;
                    api.render_world = renderer_render_world;

                    return &api;
                }


        case MATERIAL_API_ID:
                {
                    static struct material_api_v0 api = {0};

                    api.resource_create = material_create;
                    api.get_texture_count = material_get_texture_count;
                    api.set_texture = material_set_texture;
                    api.set_vec4f = material_set_vec4f;
                    api.set_mat33f = material_set_mat33f;
                    api.set_mat44f = material_set_mat44f;
                    api.use = material_use;
                    api.submit = material_submit;

                    return &api;
                }


        default:
            return NULL;
    }
}