//==============================================================================
// includes
//==============================================================================

#include <cetech/celib/math_types.h>
#include <cetech/celib/allocator.h>

#include <cetech/kernel/application.h>
#include <cetech/kernel/config.h>
#include <cetech/kernel/module.h>
#include <cetech/kernel/window.h>
#include <cetech/kernel/api.h>

#include <cetech/modules/camera.h>
#include "cetech/modules/renderer.h"
#include <cetech/kernel/develop.h>
#include <cetech/modules/world.h>
#include <cetech/modules/console_server.h>

#include <include/mpack/mpack.h>
#include "bgfx/platform.h"

#include "texture/texture.h"
#include "shader/shader.h"
#include "scene/scene.h"
#include "material/material.h"
#include "mesh_renderer/mesh_renderer_private.h"


CETECH_DECL_API(cnsole_srv_api_v0);
CETECH_DECL_API(mesh_renderer_api_v0);
CETECH_DECL_API(config_api_v0);
CETECH_DECL_API(app_api_v0);
CETECH_DECL_API(window_api_v0);
CETECH_DECL_API(api_v0);

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
    bgfx::PlatformData pd = {0};
    pd.nwh = window_api_v0.native_window_ptr(window);
    pd.ndt = window_api_v0.native_display_ptr(window);
    bgfx::setPlatformData(pd);

    // TODO: from config
    bgfx::init(bgfx::RendererType::OpenGL, 0, 0, NULL, NULL);

    window_api_v0.size(window, &_G.size_width, &_G.size_height);

    _G.need_reset = 1;
}

void renderer_set_debug(int debug) {
    if (debug) {
        bgfx::setDebug(BGFX_DEBUG_STATS);
    } else {
        bgfx::setDebug(BGFX_DEBUG_NONE);
    }
}

void renderer_render_world(world_t world,
                           camera_t camera,
                           viewport_t viewport) {
    camera_api_v0 *camera_api = (camera_api_v0 *) api_v0.first(
            "camera_api_v0").api; // TODO: SHIT !!!!

    if (CETECH_UNLIKELY(_G.need_reset)) {
        bgfx::reset(_G.size_width, _G.size_height, _get_reset_flags());
    }

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x66CCFFff, 1.0f,
                       0);

    mat44f_t view_matrix;
    mat44f_t proj_matrix;

    camera_api->get_project_view(world, camera, &proj_matrix, &view_matrix);
    bgfx::setViewTransform(0, view_matrix.f, proj_matrix.f);

    bgfx::setViewRect(0, 0, 0, (uint16_t) _G.size_width,
                      (uint16_t) _G.size_height);

    bgfx::touch(0);
    bgfx::dbgTextClear(0, 0);

    mesh_renderer_api_v0.render_all(world);

    bgfx::frame(0);

    window_api_v0.update(app_api_v0.main_window());
}

vec2f_t renderer_get_size() {
    vec2f_t result;

    result.x = _G.size_width;
    result.y = _G.size_height;

    return result;
}

namespace renderer_module {
    static struct renderer_api_v0 rendderer_api = {
            .create = renderer_create,
            .set_debug = renderer_set_debug,
            .get_size = renderer_get_size,
            .render_world = renderer_render_world
    };

    static struct material_api_v0 material_api = {
            .resource_create = material::create,
            .get_texture_count = material::get_texture_count,
            .set_texture = material::set_texture,
            .set_vec4f = material::set_vec4f,
            .set_mat33f = material::set_mat33f,
            .set_mat44f = material::set_mat44f,
            .use = material::use,
            .submit = material::submit
    };

    void _init_api(struct api_v0 *api) {
        api->register_api("renderer_api_v0", &rendderer_api);
        api->register_api("material_api_v0", &material_api);
    }


    void _init(struct api_v0 *api) {
        _init_api(api);

        api_v0 = *api;
        CETECH_GET_API(api, config_api_v0);
        CETECH_GET_API(api, config_api_v0);
        CETECH_GET_API(api, cnsole_srv_api_v0);

        _G = (struct G) {0};

        cvar_t daemon = config_api_v0.find("daemon");
        if (!config_api_v0.get_int(daemon)) {
            texture::texture_init(api);
            shader::shader_init(api);
            material::init(api);
            scene::init(api);
            mesh::init(api);

            CETECH_GET_API(api, mesh_renderer_api_v0);

            cnsole_srv_api_v0.register_command("renderer.resize",
                                               _cmd_resize);
        }

        CETECH_GET_API(api, app_api_v0);
        CETECH_GET_API(api, window_api_v0);
    }

    void _shutdown() {
        cvar_t daemon = config_api_v0.find("daemon");
        if (!config_api_v0.get_int(daemon)) {
            texture::texture_shutdown();
            shader::shader_shutdown();
            material::shutdown();
            scene::shutdown();
            mesh::shutdown();

            bgfx::shutdown();
        }

        _G = (struct G) {0};
    }


    extern "C" void *renderer_load_module(struct api_v0 *api) {
        _init(api);
        return nullptr;

//        switch (api) {
//            case PLUGIN_EXPORT_API_ID: {
//                static struct module_export_api_v0 module = {0};
//
//                module.init = _init;
//                module.shutdown = _shutdown;
//
//                return &module;
//            }
//
//
//            default:
//                return NULL;
//        }
    }

    extern "C" void renderer_unload_module(struct api_v0 *api) {
        _shutdown();
    }
}