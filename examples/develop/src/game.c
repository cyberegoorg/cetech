#define CT_DYNAMIC_MODULE 1

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
#include <cetech/controlers/controlers.h>
#include <string.h>


#define MOUDLE_NAME example_develop

static struct G {
    struct ct_world world;
    struct ct_entity camera_ent;
    float dt;
} _G;

#define _CAMERA_ASSET \
    CT_ID64_0("content/camera", 0x2d0dc3c05bc23f4fULL)

void init(uint64_t event) {
    CT_UNUSED(event)

    _G.world = ct_ecs_a0->entity->create_world();

    _G.camera_ent = ct_ecs_a0->entity->spawn(_G.world,_CAMERA_ASSET);
}


void shutdown(uint64_t event) {
    CT_UNUSED(event)
}

void update(uint64_t event) {
    _G.dt = ct_cdb_a0->read_float(event, ct_hashlib_a0->id64("dt"), 0.0f);

    struct ct_controlers_i0* keyboard;
    keyboard = ct_controlers_a0->get(CONTROLER_KEYBOARD);


    if (keyboard->button_state(0, keyboard->button_index("v"))) {
        ct_log_a0->info("example", "PO");
        ct_log_a0->error("example", "LICE");
    }

    ///ct_log_a0->debug("example", "%f", dt);

    ct_ecs_a0->system->simulate(_G.world, _G.dt);

    struct ct_camera_component *camera_data;
    camera_data = ct_ecs_a0->component->get_one(_G.world, CAMERA_COMPONENT,
                                                    _G.camera_ent);

//    struct ct_render_texture_handle th = ct_viewport_a0->get_local_resource(
//            camera_data->viewport,
//            CT_ID64_0("bb_color", 0xbc7954e9095ce3c9ULL));

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
    if(CT_DYNAMIC_MODULE) {
        CT_INIT_API(api, ct_controlers_a0);
        CT_INIT_API(api, ct_log_a0);
        CT_INIT_API(api, ct_debugui_a0);
        CT_INIT_API(api, ct_hashlib_a0);
        CT_INIT_API(api, ct_renderer_a0);
        CT_INIT_API(api, ct_ebus_a0);
        CT_INIT_API(api, ct_ecs_a0);
        CT_INIT_API(api, ct_texture_a0);
        CT_INIT_API(api, ct_render_graph_a0);
        CT_INIT_API(api, ct_default_rg_a0);
        CT_INIT_API(api, ct_cdb_a0);
    }
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

