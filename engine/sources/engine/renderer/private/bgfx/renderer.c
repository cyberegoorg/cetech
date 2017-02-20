//==============================================================================
// includes
//==============================================================================

#include <celib/math/types.h>
#include <engine/world/camera.h>
#include <engine/renderer/mesh_renderer.h>
#include <engine/develop/console_server.h>
#include <engine/application/application.h>
#include <engine/config/cvar.h>
#include <engine/plugin/plugin_api.h>
#include "celib/window/window.h"
#include "engine/renderer/renderer.h"

#include "bgfx/c99/platform.h"
#include "texture.h"
#include "shader.h"
#include "scene.h"
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

static struct ConsoleServerApiV1 ConsoleServerApiV1;
static struct MeshApiV1 MeshApiV1;
static struct CameraApiV1 CameraApiV1;
static struct ConfigApiV1 ConfigApiV1;
static struct ApplicationApiV1 ApplicationApiV1;

static void _init(get_api_fce_t get_engine_api) {
    ConsoleServerApiV1 = *((struct ConsoleServerApiV1 *) get_engine_api(CONSOLE_SERVER_API_ID, 0));
    MeshApiV1 = *((struct MeshApiV1 *) get_engine_api(MESH_API_ID, 0));
    CameraApiV1 = *((struct CameraApiV1 *) get_engine_api(CAMERA_API_ID, 0));
    ConfigApiV1 = *((struct ConfigApiV1 *) get_engine_api(CONFIG_API_ID, 0));
    ApplicationApiV1 = *((struct ApplicationApiV1 *) get_engine_api(APPLICATION_API_ID, 0));

    _G = (struct G) {0};

    cvar_t daemon = ConfigApiV1.find("daemon");
    if (!ConfigApiV1.get_int(daemon)) {
        texture_resource_init();
        shader_resource_init();
        material_resource_init();
        scene_resource_init();

        ConsoleServerApiV1.consolesrv_register_command("renderer.resize", _cmd_resize);
    }
}

static void _shutdown() {
    cvar_t daemon = ConfigApiV1.find("daemon");
    if (!ConfigApiV1.get_int(daemon)) {
        texture_resource_shutdown();
        shader_resource_shutdown();
        material_resource_shutdown();
        scene_resource_shutdown();

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

    bgfx_set_view_clear(0, BGFX_CLEAR_COLOR | BGFX_CLEAR_DEPTH, 0x66CCFFff, 1.0f, 0);

    cel_mat44f_t view_matrix;
    cel_mat44f_t proj_matrix;

    CameraApiV1.get_project_view(world, camera, &proj_matrix, &view_matrix);
    bgfx_set_view_transform(0, view_matrix.f, proj_matrix.f);

    bgfx_set_view_rect(0, 0, 0, (uint16_t) _G.size_width, (uint16_t) _G.size_height);

    bgfx_touch(0);
    bgfx_dbg_text_clear(0, 0);

    MeshApiV1.render_all(world);

    bgfx_frame(0);
    cel_window_update(ApplicationApiV1.main_window());
}

cel_vec2f_t renderer_get_size() {
    cel_vec2f_t result;

    result.x = _G.size_width;
    result.y = _G.size_height;

    return result;
}


void *renderer_get_plugin_api(int api,
                              int version) {

    switch (api) {
        case PLUGIN_EXPORT_API_ID:
            switch (version) {
                case 0: {
                    static struct plugin_api_v0 plugin = {0};

                    plugin.init = _init;
                    plugin.shutdown = _shutdown;

                    return &plugin;
                }

                default:
                    return NULL;
            };

        case RENDERER_API_ID:
            switch (version) {
                case 0: {
                    static struct RendererApiV1 api = {0};

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
                    static struct MaterialApiV1 api = {0};

                    material_t material_resource_create(stringid64_t name);

                    u32 material_get_texture_count(material_t material);

                    void material_set_texture(material_t material,
                                              const char *slot,
                                              stringid64_t texture);

                    void material_set_vec4f(material_t material,
                                            const char *slot,
                                            cel_vec4f_t v);

                    void material_set_mat33f(material_t material,
                                             const char *slot,
                                             mat33f_t v);

                    void material_set_mat44f(material_t material,
                                             const char *slot,
                                             cel_mat44f_t v);


                    void material_use(material_t material);

                    void material_submit(material_t material);

                    api.resource_create = material_resource_create;
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