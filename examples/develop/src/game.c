#include <cetech/kernel/macros.h>

#include <cetech/kernel/log/log.h>
#include <cetech/kernel/module/module.h>
#include <cetech/kernel/api/api_system.h>
#include <cetech/kernel/hashlib/hashlib.h>

#include <cetech/engine/controlers/keyboard.h>
#include <cetech/kernel/ebus/ebus.h>
#include <cetech/engine/ecs/ecs.h>

#include <cetech/engine/debugui/debugui.h>
#include <cetech/engine/renderer/renderer.h>
#include <cetech/engine/render_graph/render_graph.h>
#include <cetech/engine/default_render_graph/default_render_graph.h>
#include <cetech/engine/transform/transform.h>
#include <cetech/engine/texture/texture.h>
#include <cetech/engine/camera/camera.h>
#include <cetech/kernel/kernel.h>


CETECH_DECL_API(ct_log_a0);
CETECH_DECL_API(ct_keyboard_a0);
CETECH_DECL_API(ct_debugui_a0);
CETECH_DECL_API(ct_hashlib_a0);

CETECH_DECL_API(ct_renderer_a0);

CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_ecs_a0);
//CETECH_DECL_API(ct_camera_a0);
CETECH_DECL_API(ct_texture_a0);
CETECH_DECL_API(ct_ebus_a0);
CETECH_DECL_API(ct_render_graph_a0);
CETECH_DECL_API(ct_default_render_graph_a0);

#define MOUDLE_NAME example_develop

static struct G {
    struct ct_world world;
    struct ct_entity camera_ent;
    float dt;
} _G;



void init(uint32_t bus_name,
          void *event) {
    _G.world = ct_ecs_a0.create_world();

    _G.camera_ent = ct_ecs_a0.spawn_entity(_G.world,
                                           CT_ID32_0("content/camera"));

}


void shutdown(uint32_t bus_name,
              void *event) {
}

void update(uint32_t bus_name,
            void *event) {
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

    uint32_t w, h;
    w = h = 0;

    ct_renderer_a0.get_size(&w, &h);
}

//==============================================================================
// Module def
//==============================================================================

//==============================================================================
// Init api
//==============================================================================
void CETECH_MODULE_INITAPI(example_develop)(struct ct_api_a0 *api) {
    CETECH_GET_API(api, ct_keyboard_a0);
    CETECH_GET_API(api, ct_log_a0);
    CETECH_GET_API(api, ct_debugui_a0);
    CETECH_GET_API(api, ct_hashlib_a0);
    CETECH_GET_API(api, ct_renderer_a0);
    CETECH_GET_API(api, ct_ebus_a0);

    CETECH_GET_API(api, ct_transform_a0);
    CETECH_GET_API(api, ct_ecs_a0);
//            CETECH_GET_API(api, ct_camera_a0);
    CETECH_GET_API(api, ct_texture_a0);

    CETECH_GET_API(api, ct_render_graph_a0);
    CETECH_GET_API(api, ct_default_render_graph_a0);
}

void CETECH_MODULE_LOAD (example_develop)(struct ct_api_a0 *api,
                                          int reload) {
    CT_UNUSED(api);

    ct_log_a0.info("example", "Init %d", reload);

//    init_decl();

    ct_ebus_a0.connect(KERNEL_EBUS,
                       KERNEL_UPDATE_EVENT, update, KERNEL_ORDER);

    ct_ebus_a0.connect(KERNEL_EBUS,
                       KERNEL_INIT_EVENT, init, GAME_ORDER);

    ct_ebus_a0.connect(KERNEL_EBUS,
                       KERNEL_SHUTDOWN_EVENT, shutdown, GAME_ORDER);
}

void CETECH_MODULE_UNLOAD (example_develop)(struct ct_api_a0 *api,
                                            int reload) {
    CT_UNUSED(api);

    ct_log_a0.info("example", "Shutdown %d", reload);

    ct_ebus_a0.disconnect(KERNEL_EBUS, KERNEL_UPDATE_EVENT, update);
    ct_ebus_a0.disconnect(KERNEL_EBUS, KERNEL_INIT_EVENT, init);
    ct_ebus_a0.disconnect(KERNEL_EBUS, KERNEL_SHUTDOWN_EVENT, shutdown);
}

