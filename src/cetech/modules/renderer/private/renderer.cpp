//==============================================================================
// includes
//==============================================================================

#include "celib/allocator.h"

#include <cetech/modules/application/application.h>

#include <include/mpack/mpack.h>
#include <cetech/core/api/api_system.h>
#include <cetech/engine/console_server/console_server.h>
#include <cetech/core/config/config.h>
#include <cetech/engine/machine/window.h>
#include <cetech/engine/entity/entity.h>
#include <cetech/modules/renderer/renderer.h>
#include <cetech/modules/camera/camera.h>
#include <cetech/core/macros.h>
#include <cetech/core/module/module.h>
#include <cetech/core/memory/memory.h>

#include "bgfx/platform.h"

#include "texture/texture.h"
#include "shader/shader.h"
#include "scene/scene.h"
#include "material/material.h"
#include "mesh_renderer/mesh_renderer_private.h"


CETECH_DECL_API(ct_console_srv_a0);
CETECH_DECL_API(ct_mesh_renderer_a0);
CETECH_DECL_API(ct_config_a0);
CETECH_DECL_API(ct_window_a0);
CETECH_DECL_API(ct_api_a0);
CETECH_DECL_API(ct_memory_a0);

//==============================================================================
// GLobals
//==============================================================================

#define _G RendererGlobals
static struct G {
    ct_window *main_window;
    uint64_t type;
    uint32_t size_width;
    uint32_t size_height;
    int capture;
    int vsync;
    int need_reset;
} _G = {};

struct GConfig {
    ct_cvar screen_x;
    ct_cvar screen_y;
    ct_cvar fullscreen;
    ct_cvar daemon;
    ct_cvar wid;
} GConfig;

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
    CEL_UNUSED(writer);

    mpack_node_t width = mpack_node_map_cstr(args, "width");
    mpack_node_t height = mpack_node_map_cstr(args, "height");

    _G.size_width = (uint32_t) mpack_node_int(width);
    _G.size_height = (uint32_t) mpack_node_int(height);
    _G.need_reset = 1;

    return 0;
}


void renderer_create() {
    if (!ct_config_a0.get_int(GConfig.daemon)) {
        intptr_t wid = ct_config_a0.get_int(GConfig.wid);

        char title[128] = {};
        snprintf(title, CETECH_ARRAY_LEN(title), "cetech");

        if (wid == 0) {
            _G.main_window = ct_window_a0.create(
                    ct_memory_a0.main_allocator(),
                    title,
                    WINDOWPOS_UNDEFINED,
                    WINDOWPOS_UNDEFINED,
                    ct_config_a0.get_int(GConfig.screen_x),
                    ct_config_a0.get_int(GConfig.screen_y),
                    ct_config_a0.get_int(GConfig.fullscreen)
                    ? WINDOW_FULLSCREEN : WINDOW_NOFLAG
            );
        } else {
            _G.main_window = ct_window_a0.create_from(
                    ct_memory_a0.main_allocator(), (void *) wid);
        }
    }

    bgfx::PlatformData pd = {};
    pd.nwh = _G.main_window->native_window_ptr(_G.main_window->inst);
    pd.ndt = _G.main_window->native_display_ptr(_G.main_window->inst);
    bgfx::setPlatformData(pd);

    // TODO: from config
    bgfx::init(bgfx::RendererType::OpenGL, 0, 0, NULL, NULL);

    _G.main_window->size(_G.main_window->inst, &_G.size_width, &_G.size_height);

    _G.need_reset = 1;
}

void renderer_set_debug(int debug) {
    if (debug) {
        bgfx::setDebug(BGFX_DEBUG_STATS);
    } else {
        bgfx::setDebug(BGFX_DEBUG_NONE);
    }
}

void renderer_render_world(ct_world world,
                           ct_camera camera,
                           viewport_t viewport) {
    CEL_UNUSED(viewport);

    ct_camera_a0 *camera_api = (ct_camera_a0 *) ct_api_a0.first(
            "ct_camera_a0").api; // TODO: SHIT !!!!

    if (CETECH_UNLIKELY(_G.need_reset)) {
        bgfx::reset(_G.size_width, _G.size_height, _get_reset_flags());
    }

    bgfx::setViewClear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x66CCFFff, 1.0f,
                       0);

    float view_matrix[16];
    float proj_matrix[16];

    camera_api->get_project_view(camera, proj_matrix, view_matrix,
                                 _G.size_width, _G.size_height);

    bgfx::setViewTransform(0, view_matrix, proj_matrix);

    bgfx::setViewRect(0, 0, 0, (uint16_t) _G.size_width,
                      (uint16_t) _G.size_height);

    bgfx::touch(0);
    bgfx::dbgTextClear(0, 0);

    ct_mesh_renderer_a0.render_all(world);

    bgfx::frame(0);

    _G.main_window->update(_G.main_window);
}

void renderer_get_size(uint32_t *width,
                       uint32_t *height) {
    *width = _G.size_width;
    *height = _G.size_height;
}

namespace renderer_module {
    static ct_renderer_a0 rendderer_api = {
            .create = renderer_create,
            .set_debug = renderer_set_debug,
            .get_size = renderer_get_size,
            .render_world = renderer_render_world
    };

    static ct_material_a0 material_api = {
            .resource_create = material::create,
            .get_texture_count = material::get_texture_count,
            .set_texture = material::set_texture,
            .set_mat44f = material::set_mat44f,
            .use = material::use,
            .submit = material::submit
    };

    void _init_api(struct ct_api_a0 *api) {
        api->register_api("ct_renderer_a0", &rendderer_api);
        api->register_api("ct_material_a0", &material_api);
    }


    void _init(struct ct_api_a0 *api) {
        _init_api(api);

        ct_api_a0 = *api;

        _G = (struct G) {};

        GConfig = {
                .screen_x = ct_config_a0.new_int("screen.x", "Screen width", 1024),
                .screen_y = ct_config_a0.new_int("screen.y", "Screen height", 768),
                .fullscreen = ct_config_a0.new_int("screen.fullscreen",
                                                   "Fullscreen", 0),

                .daemon = ct_config_a0.new_int("daemon", "Daemon mode", 0),
                .wid = ct_config_a0.new_int("wid", "Wid", 0)
        };

        if (!ct_config_a0.get_int(GConfig.daemon)) {
            texture::texture_init(api);
            shader::shader_init(api);
            material::init(api);
            scene::init(api);
            mesh::init(api);

            CETECH_GET_API(api, ct_mesh_renderer_a0);

            ct_console_srv_a0.register_command("renderer.resize",
                                               _cmd_resize);
        }

        CETECH_GET_API(api, ct_window_a0);
    }

    void _shutdown() {
        ct_cvar daemon = ct_config_a0.find("daemon");
        if (!ct_config_a0.get_int(daemon)) {
            texture::texture_shutdown();
            shader::shader_shutdown();
            material::shutdown();
            scene::shutdown();
            mesh::shutdown();

            bgfx::shutdown();
        }

        _G = (struct G) {};
    }

}

CETECH_MODULE_DEF(
        renderer,
        {
            CETECH_GET_API(api, ct_config_a0);
            CETECH_GET_API(api, ct_config_a0);
            CETECH_GET_API(api, ct_console_srv_a0);
            CETECH_GET_API(api, ct_memory_a0);
        },
        {
            renderer_module::_init(api);
        },
        {
            CEL_UNUSED(api);

            renderer_module::_shutdown();

        }
)