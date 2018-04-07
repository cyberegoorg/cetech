#include <cetech/kernel/macros.h>

#include <cetech/kernel/log/log.h>
#include <cetech/kernel/config/config.h>
#include <cetech/kernel/module/module.h>
#include <cetech/kernel/api/api_system.h>
#include <cetech/kernel/hashlib/hashlib.h>

#include <cetech/engine/controlers/keyboard.h>
#include <cetech/kernel/ebus/ebus.h>
#include <cetech/engine/application/application.h>
#include <cetech/engine/ecs/ecs.h>

#include <cetech/playground//playground.h>
#include <cetech/engine/debugui/debugui.h>
#include <cetech/engine/renderer/renderer.h>
#include <cetech/engine/transform/transform.h>
#include <cetech/engine/viewport/viewport.h>
#include <cetech/engine/texture/texture.h>
#include <stdlib.h>
#include <cetech/engine/camera/camera.h>


CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_app_a0);
CETECH_DECL_API(ct_keyboard_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_hashlib_a0);

CETECH_DECL_API(ct_renderer_a0);

CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_ecs_a0);
//CETECH_DECL_API(ct_camera_a0);
CETECH_DECL_API(ct_texture_a0);
CETECH_DECL_API(ct_ebus_a0);
CETECH_DECL_API(ct_viewport_a0);


static struct G {
    struct ct_viewport viewport;
    struct ct_world world;
    struct ct_entity camera_ent;
    float dt;
} _G;



struct PosTexCoord0Vertex {
    float m_x;
    float m_y;
    float m_z;
    float m_u;
    float m_v;
};

static ct_render_vertex_decl_t ms_decl;

static void init_decl() {
    ct_renderer_a0.vertex_decl_begin(&ms_decl, CT_RENDER_RENDERER_TYPE_NOOP);
    ct_renderer_a0.vertex_decl_add(&ms_decl, CT_RENDER_ATTRIB_POSITION, 3, CT_RENDER_ATTRIB_TYPE_FLOAT, false, false);
    ct_renderer_a0.vertex_decl_add(&ms_decl, CT_RENDER_ATTRIB_TEXCOORD0, 2, CT_RENDER_ATTRIB_TYPE_FLOAT, false, false);
    ct_renderer_a0.vertex_decl_end(&ms_decl);
}


void screenspace_quad(float _textureWidth,
                      float _textureHeight,
                      float _texelHalf,
                      bool _originBottomLeft,
                      float _width,
                      float _height) {
    if (3 ==
            ct_renderer_a0.get_avail_transient_vertex_buffer(3, &ms_decl)) {
        ct_render_transient_vertex_buffer_t vb;
        ct_renderer_a0.alloc_transient_vertex_buffer(&vb, 3, &ms_decl);
        struct PosTexCoord0Vertex *vertex = (struct PosTexCoord0Vertex *) vb.data;

        const float minx = -_width;
        const float maxx = _width;
        const float miny = 0.0f;
        const float maxy = _height * 2.0f;

        const float texelHalfW = _texelHalf / _textureWidth;
        const float texelHalfH = _texelHalf / _textureHeight;
        const float minu = -1.0f + texelHalfW;
        const float maxu = 1.0f + texelHalfH;

        const float zz = 0.0f;

        float minv = texelHalfH;
        float maxv = 2.0f + texelHalfH;

        if (_originBottomLeft) {
            float temp = minv;
            minv = maxv;
            maxv = temp;

            minv -= 1.0f;
            maxv -= 1.0f;
        }

        vertex[0].m_x = minx;
        vertex[0].m_y = miny;
        vertex[0].m_z = zz;
        vertex[0].m_u = minu;
        vertex[0].m_v = minv;

        vertex[1].m_x = maxx;
        vertex[1].m_y = miny;
        vertex[1].m_z = zz;
        vertex[1].m_u = maxu;
        vertex[1].m_v = minv;

        vertex[2].m_x = maxx;
        vertex[2].m_y = maxy;
        vertex[2].m_z = zz;
        vertex[2].m_u = maxu;
        vertex[2].m_v = maxv;

        ct_renderer_a0.set_transient_vertex_buffer(0, &vb, 0, 0);
    }
}



void init(uint32_t bus_name, void *event) {
    _G.world = ct_ecs_a0.create_world();

    _G.camera_ent = ct_ecs_a0.spawn_entity(_G.world,
                                           CT_ID32_0("content/camera"));

    ct_ecs_a0.add_simulation(_G.world, CT_ID64_0("render"));


}

void shutdown(uint32_t bus_name, void *event) {
}

void update(uint32_t bus_name, void *event) {
    struct ct_app_update_ev *ev = event;

    _G.dt = ev->dt;
    if (ct_keyboard_a0.button_state(0, ct_keyboard_a0.button_index("v"))) {
        ct_log_a0.info("example", "PO");
        ct_log_a0.error("example", "LICE");
    }
    ///ct_log_a0.debug("example", "%f", dt);


    ct_ecs_a0.simulate(_G.world, _G.dt);

    struct ct_camera_component *camera_data;
    camera_data = ct_ecs_a0.entity_data(_G.world, CAMERA_COMPONENT,
                                        _G.camera_ent);

//    struct ct_render_texture_handle th = ct_viewport_a0.get_local_resource(
//            camera_data->viewport,
//            CT_ID64_0("bb_color"));

//    uint32_t w, h;
//    w = h = 0;
//
//    ct_renderer_a0.get_size(&w, &h);
//
//    screenspace_quad(w,h, 0.0f, ct_renderer_a0.get_caps()->originBottomLeft, 1.0f, 1.0f);
}

//==============================================================================
// Module def
//==============================================================================
CETECH_MODULE_DEF(
        example_develop,

//==============================================================================
// Init api
//==============================================================================
        {
            CETECH_GET_API(api, ct_keyboard_a0);
            CETECH_GET_API(api, ct_log_a0);
            CETECH_GET_API(api, ct_app_a0);
            CETECH_GET_API(api, ct_debugui_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_renderer_a0);
            CETECH_GET_API(api, ct_ebus_a0);

            CETECH_GET_API(api, ct_transform_a0);
            CETECH_GET_API(api, ct_ecs_a0);
//            CETECH_GET_API(api, ct_camera_a0);
            CETECH_GET_API(api, ct_texture_a0);
            CETECH_GET_API(api, ct_viewport_a0);
        },

//==============================================================================
// Load
//==============================================================================
        {
            CT_UNUSED(api);

            ct_log_a0.info("example", "Init %d", reload);

            init_decl();

            ct_ebus_a0.connect(APPLICATION_EBUS,
                               APP_GAME_UPDATE_EVENT, update, 0);

            ct_ebus_a0.connect(APPLICATION_EBUS,
                               APP_GAME_INIT_EVENT, init, 0);

            ct_ebus_a0.connect(APPLICATION_EBUS,
                               APP_GAME_SHUTDOWN_EVENT, shutdown, 0);
        },

//==============================================================================
// Unload
//==============================================================================
        {
            CT_UNUSED(api);

            ct_log_a0.info("example", "Shutdown %d", reload);

//            ct_debugui_a0.unregister_on_debugui(module1);
//            ct_debugui_a0.unregister_on_debugui(module2);
        }
)


