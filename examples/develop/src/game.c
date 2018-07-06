#define CT_DYNAMIC_MODULE

#include <corelib/macros.h>

#include <corelib/log.h>
#include <corelib/module.h>
#include <corelib/api_system.h>
#include <corelib/hashlib.h>

#include <cetech/controlers/keyboard.h>
#include <corelib/ebus.h>

#include <corelib/cdb.h>
#include <cetech/ecs/ecs.h>

#include <cetech/debugui/debugui.h>
#include <cetech/renderer/renderer.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/default_render_graph/default_render_graph.h>
#include <cetech/transform/transform.h>
#include <cetech/texture/texture.h>
#include <cetech/camera/camera.h>
#include <cetech/kernel/kernel.h>


#define MOUDLE_NAME example_develop

static struct G {
    struct ct_world world;
    struct ct_entity camera_ent;
    float dt;
} _G;


void init(uint64_t event) {
    CT_UNUSED(event)

    _G.world = ct_ecs_a0->entity->create_world();

    _G.camera_ent = ct_ecs_a0->entity->spawn(_G.world,
                                            CT_ID32_0("content/camera"));
}


void shutdown(uint64_t event) {
    CT_UNUSED(event)
}

void update(uint64_t event) {

    _G.dt = ct_cdb_a0->read_float(event, CT_ID64_0("dt"), 0.0f);
    if (ct_keyboard_a0->button_state(0, ct_keyboard_a0->button_index("v"))) {
        ct_log_a0->info("example", "PO");
        ct_log_a0->error("example", "LICE");
    }
    ///ct_log_a0->debug("example", "%f", dt);

    ct_ecs_a0->simulate(_G.world, _G.dt);

    struct ct_camera_component *camera_data;
    camera_data = ct_ecs_a0->component->entity_data(_G.world, CAMERA_COMPONENT,
                                                    _G.camera_ent);

//    struct ct_render_texture_handle th = ct_viewport_a0->get_local_resource(
//            camera_data->viewport,
//            CT_ID64_0("bb_color"));

    uint32_t w, h;
    w = h = 0;

    ct_renderer_a0->get_size(&w, &h);
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

    CETECH_GET_API(api, ct_ecs_a0);
//            CETECH_GET_API(api, ct_camera_a0);
    CETECH_GET_API(api, ct_texture_a0);

    CETECH_GET_API(api, ct_render_graph_a0);
    CETECH_GET_API(api, ct_default_render_graph_a0);
    CETECH_GET_API(api, ct_cdb_a0);
}

void CETECH_MODULE_LOAD (example_develop)(struct ct_api_a0 *api,
                                          int reload) {
    CT_UNUSED(api);

    ct_log_a0->info("example", "Init %d", reload);

//    init_decl();

    ct_ebus_a0->connect(KERNEL_EBUS,
                        KERNEL_UPDATE_EVENT, update, KERNEL_ORDER);

    ct_ebus_a0->connect(KERNEL_EBUS,
                        KERNEL_INIT_EVENT, init, GAME_ORDER);

    ct_ebus_a0->connect(KERNEL_EBUS,
                        KERNEL_SHUTDOWN_EVENT, shutdown, GAME_ORDER);
}

void CETECH_MODULE_UNLOAD (example_develop)(struct ct_api_a0 *api,
                                            int reload) {
    CT_UNUSED(api);

    ct_log_a0->info("example", "Shutdown %d", reload);

    ct_ebus_a0->disconnect(KERNEL_EBUS, KERNEL_UPDATE_EVENT, update);
    ct_ebus_a0->disconnect(KERNEL_EBUS, KERNEL_INIT_EVENT, init);
    ct_ebus_a0->disconnect(KERNEL_EBUS, KERNEL_SHUTDOWN_EVENT, shutdown);
}

