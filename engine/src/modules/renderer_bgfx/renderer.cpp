//==============================================================================
// includes
//==============================================================================

#include <cetech/celib/math_types.h>
#include <cetech/celib/allocator.h>

#include <cetech/modules/application.h>
#include <cetech/kernel/config.h>
#include <cetech/kernel/module.h>
#include <cetech/kernel/os.h>
#include <cetech/kernel/api_system.h>

#include <cetech/modules/camera.h>
#include "cetech/modules/renderer.h"
#include <cetech/kernel/develop.h>

#include <cetech/kernel/console_server.h>

#include <include/mpack/mpack.h>
#include <cetech/modules/entity.h>
#include "bgfx/platform.h"

#include "texture/texture.h"
#include "shader/shader.h"
#include "scene/scene.h"
#include "material/material.h"
#include "mesh_renderer/mesh_renderer_private.h"


CETECH_DECL_API(ct_console_srv_api_v0);
CETECH_DECL_API(ct_mesh_renderer_api_v0);
CETECH_DECL_API(ct_config_api_v0);
CETECH_DECL_API(ct_app_api_v0);
CETECH_DECL_API(ct_window_api_v0);
CETECH_DECL_API(ct_api_v0);

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


void renderer_create(os_window_t *window) {
    bgfx::PlatformData pd = {0};
    pd.nwh = ct_window_api_v0.native_window_ptr(window);
    pd.ndt = ct_window_api_v0.native_display_ptr(window);
    bgfx::setPlatformData(pd);

    // TODO: from config
    bgfx::init(bgfx::RendererType::OpenGL, 0, 0, NULL, NULL);

    ct_window_api_v0.size(window, &_G.size_width, &_G.size_height);

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
                           struct ct_camera camera,
                           viewport_t viewport) {
    ct_camera_api_v0 *camera_api = (ct_camera_api_v0 *) ct_api_v0.first(
            "ct_camera_api_v0").api; // TODO: SHIT !!!!

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

    ct_mesh_renderer_api_v0.render_all(world);

    bgfx::frame(0);

    ct_window_api_v0.update(ct_app_api_v0.main_window());
}

vec2f_t renderer_get_size() {
    vec2f_t result;

    result.x = _G.size_width;
    result.y = _G.size_height;

    return result;
}

namespace renderer_module {
    static struct ct_renderer_api_v0 rendderer_api = {
            .create = renderer_create,
            .set_debug = renderer_set_debug,
            .get_size = renderer_get_size,
            .render_world = renderer_render_world
    };

    static struct ct_material_api_v0 material_api = {
            .resource_create = material::create,
            .get_texture_count = material::get_texture_count,
            .set_texture = material::set_texture,
            .set_vec4f = material::set_vec4f,
            .set_mat33f = material::set_mat33f,
            .set_mat44f = material::set_mat44f,
            .use = material::use,
            .submit = material::submit
    };

    void _init_api(struct ct_api_v0 *api) {
        api->register_api("ct_renderer_api_v0", &rendderer_api);
        api->register_api("ct_material_api_v0", &material_api);
    }


    void _init(struct ct_api_v0 *api) {
        _init_api(api);

        ct_api_v0 = *api;
        CETECH_GET_API(api, ct_config_api_v0);
        CETECH_GET_API(api, ct_config_api_v0);
        CETECH_GET_API(api, ct_console_srv_api_v0);

        _G = (struct G) {0};

        ct_cvar_t daemon = ct_config_api_v0.find("daemon");
        if (!ct_config_api_v0.get_int(daemon)) {
            texture::texture_init(api);
            shader::shader_init(api);
            material::init(api);
            scene::init(api);
            mesh::init(api);

            CETECH_GET_API(api, ct_mesh_renderer_api_v0);

            ct_console_srv_api_v0.register_command("renderer.resize",
                                               _cmd_resize);
        }

        CETECH_GET_API(api, ct_app_api_v0);
        CETECH_GET_API(api, ct_window_api_v0);
    }

    void _shutdown() {
        ct_cvar_t daemon = ct_config_api_v0.find("daemon");
        if (!ct_config_api_v0.get_int(daemon)) {
            texture::texture_shutdown();
            shader::shader_shutdown();
            material::shutdown();
            scene::shutdown();
            mesh::shutdown();

            bgfx::shutdown();
        }

        _G = (struct G) {0};
    }


    extern "C" void renderer_load_module(struct ct_api_v0 *api) {
        _init(api);
    }

    extern "C" void renderer_unload_module(struct ct_api_v0 *api) {
        _shutdown();
    }
}