//==============================================================================
// includes
//==============================================================================

#include <celib/math/types.h>
#include <cetech/camera/camera.h>
#include <cetech/renderer/renderer.h>
#include <cetech/develop/develop.h>
#include <cetech/application/application.h>
#include <cetech/config/config.h>
#include <cetech/module/module.h>
#include "celib/window/window.h"

#include "bgfx/c99/platform.h"
#include <cetech/renderer/private/texture/texture.h>
#include <cetech/renderer/private/shader/shader.h>
#include <cetech/renderer/private/scene/scene.h>
#include <cetech/renderer/private/material/material.h>


IMPORT_API(ConsoleServerApi, 0);
IMPORT_API(MeshRendererApi, 0);
IMPORT_API(CameraApi, 0);
IMPORT_API(ConfigApi, 0);
IMPORT_API(ApplicationApi, 0);

//==============================================================================
// GLobals
//==============================================================================

#define _G RendererGlobals
static struct G {
    stringid64_t type;
    u32 size_width;
    u32 size_height;
    int capture;
    int vsync;
    int need_reset;
} _G = {0};


int material_init(get_api_fce_t get_engine_api);

void material_shutdown();

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

static int _cmd_resize(mpack_node_t args,
                       mpack_writer_t *writer) {
    mpack_node_t width = mpack_node_map_cstr(args, "width");
    mpack_node_t height = mpack_node_map_cstr(args, "height");

    _G.size_width = (u32) mpack_node_int(width);
    _G.size_height = (u32) mpack_node_int(height);
    _G.need_reset = 1;

    return 0;
}

static void _init(get_api_fce_t get_engine_api) {
    INIT_API(ConsoleServerApi, CONSOLE_SERVER_API_ID, 0);
    INIT_API(MeshRendererApi, MESH_API_ID, 0);
    INIT_API(CameraApi, CAMERA_API_ID, 0);
    INIT_API(ConfigApi, CONFIG_API_ID, 0);
    INIT_API(ApplicationApi, APPLICATION_API_ID, 0);

    _G = (struct G) {0};

    cvar_t daemon = ConfigApiV0.find("daemon");
    if (!ConfigApiV0.get_int(daemon)) {
        texture_init();
        shader_init();
        material_init(get_engine_api);
        scene_init();

        ConsoleServerApiV0.consolesrv_register_command("renderer.resize",
                                                       _cmd_resize);
    }
}

static void _shutdown() {
    cvar_t daemon = ConfigApiV0.find("daemon");
    if (!ConfigApiV0.get_int(daemon)) {
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
    pd.nwh = cel_window_native_cel_window_ptr(window);
    pd.ndt = cel_window_native_display_ptr(window);
    bgfx_set_platform_data(&pd);

    // TODO: from config
    bgfx_init(BGFX_RENDERER_TYPE_OPENGL, 0, 0, NULL, NULL);

    cel_window_get_size(window, &_G.size_width, &_G.size_height);

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

    CameraApiV0.get_project_view(world, camera, &proj_matrix, &view_matrix);
    bgfx_set_view_transform(0, view_matrix.f, proj_matrix.f);

    bgfx_set_view_rect(0, 0, 0, (uint16_t) _G.size_width,
                       (uint16_t) _G.size_height);

    bgfx_touch(0);
    bgfx_dbg_text_clear(0, 0);

    MeshRendererApiV0.render_all(world);

    bgfx_frame(0);
    cel_window_update(ApplicationApiV0.main_window());
}

cel_vec2f_t renderer_get_size() {
    cel_vec2f_t result;

    result.x = _G.size_width;
    result.y = _G.size_height;

    return result;
}


void *renderer_get_module_api(int api,
                              int version) {

    switch (api) {
        case PLUGIN_EXPORT_API_ID:
            switch (version) {
                case 0: {
                    static struct module_api_v0 module = {0};

                    module.init = _init;
                    module.shutdown = _shutdown;

                    return &module;
                }

                default:
                    return NULL;
            };

        case RENDERER_API_ID:
            switch (version) {
                case 0: {
                    static struct RendererApiV0 api = {0};

                    api.create = renderer_create;
                    api.set_debug = renderer_set_debug;
                    api.get_size = renderer_get_size;
                    api.render_world = renderer_render_world;

                    return &api;
                }

                default:
                    return NULL;
            };

        case MATERIAL_API_ID:
            switch (version) {
                case 0: {
                    static struct MaterialApiV0 api = {0};

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
            };

        default:
            return NULL;
    }
}