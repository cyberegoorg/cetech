#define CE_DYNAMIC_MODULE 1


#include <celib/macros.h>
#include <celib/log.h>
#include <celib/module.h>
#include <celib/api.h>
#include <celib/id.h>
#include <cetech/controlers/keyboard.h>

#include <celib/cdb.h>
#include <celib/math/math.h>

#include <cetech/ecs/ecs.h>

#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/default_rg/default_rg.h>
#include <cetech/transform/transform.h>
#include <cetech/texture/texture.h>
#include <cetech/camera/camera.h>
#include <cetech/kernel/kernel.h>
#include <cetech/controlers/controlers.h>
#include <cetech/game/game_system.h>

static struct G {
    struct ct_world_t0 world;
    struct ct_entity_t0 camera_ent;
    struct ct_entity_t0 level;
    struct ct_viewport_t0 viewport;
} _G;

#define _CAMERA_ASSET 0x57899875c4457313

#define _LEVEL_ASSET 0x588f56dc4e82f7b2

#include "rotation.inl"


void init() {
    _G.world = ct_ecs_a0->create_world();

    _G.camera_ent = ct_ecs_a0->spawn(_G.world, _CAMERA_ASSET);
    _G.level = ct_ecs_a0->spawn(_G.world, _LEVEL_ASSET);

    _G.viewport = ct_renderer_a0->create_viewport(_G.world, _G.camera_ent);
}


void shutdown() {
    ct_ecs_a0->destroy_world(_G.world);
}

void update(float dt) {
    struct ct_controlers_i0 *keyboard;
    keyboard = ct_controlers_a0->get(CONTROLER_KEYBOARD);
//
//    if (keyboard->button_state(0, keyboard->button_index("c"))) {
//        ce_log_a0->info("example", "PO");
//        ce_log_a0->error("example", "LICE");
//    }

    ct_ecs_a0->simulate(_G.world, dt);
}

static uint64_t game_name() {
    return ce_id_a0->id64("default");
}

static struct ct_viewport_t0 render_graph_builder() {
    return _G.viewport;
}

struct ct_game_i0 game_i0 = {
        .init = init,
        .shutdown = shutdown,
        .update = update,
        .name = game_name,
        .viewport = render_graph_builder
};

//==============================================================================
// Module def
//==============================================================================

void CE_MODULE_LOAD (example_develop)(struct ce_api_a0 *api,
                                      int reload) {
    CE_INIT_API(api, ct_controlers_a0);
    CE_INIT_API(api, ce_log_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ct_renderer_a0);
    CE_INIT_API(api, ct_ecs_a0);
    CE_INIT_API(api, ct_rg_a0);
    CE_INIT_API(api, ct_default_rg_a0);
    CE_INIT_API(api, ce_cdb_a0);
    CE_INIT_API(api, ct_game_system_a0);
    CE_INIT_API(api, ce_ydb_a0);
    CE_INIT_API(api, ct_editor_ui_a0);

    CE_UNUSED(api);

    ce_log_a0->info("example", "Init %d", reload);

    api->register_api(GAME_INTERFACE,
                      &game_i0, sizeof(game_i0));
    api->register_api(COMPONENT_INTERFACE,
                      &rotation_component_i, sizeof(rotation_component_i));

    api->register_api(SIMULATION_INTERFACE,
                      &rotation_simulation_i0, sizeof(rotation_simulation_i0));
}

void CE_MODULE_UNLOAD (example_develop)(struct ce_api_a0 *api,
                                        int reload) {
    CE_UNUSED(api);

    ce_log_a0->info("example", "Shutdown %d", reload);
}

