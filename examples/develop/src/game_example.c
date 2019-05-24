#define CE_DYNAMIC_MODULE 1

#include <celib/macros.h>
#include <celib/log.h>
#include <celib/module.h>
#include <celib/api.h>
#include <celib/id.h>
#include <celib/cdb.h>
#include <celib/math/math.h>

#include <cetech/ecs/ecs.h>
#include <cetech/controlers/keyboard.h>
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/transform/transform.h>
#include <cetech/texture/texture.h>
#include <cetech/camera/camera.h>
#include <cetech/kernel/kernel.h>
#include <cetech/controlers/controlers.h>
#include <cetech/game/game_system.h>

#include "rotation.inl"
#include "components/c_player_input.inl"
#include "components/c_player_speed.inl"
#include "systems/s_player_input.inl"
#include "systems/s_player_move.inl"


void init() {
}


void shutdown() {
}

void update(float dt) {
//    struct ct_controlers_i0 *keyboard;
//    keyboard = ct_controlers_a0->get(CONTROLER_KEYBOARD);
//
//    if (keyboard->button_state(0, keyboard->button_index("c"))) {
//        ce_log_a0->info("example", "PO");
//        ce_log_a0->error("example", "LICE");
//    }
}

static uint64_t game_name() {
    return ce_id_a0->id64("default");
}

struct ct_game_i0 game_i0 = {
        .init = init,
        .shutdown = shutdown,
        .update = update,
        .name = game_name,
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
    CE_INIT_API(api, ce_cdb_a0);
    CE_INIT_API(api, ct_game_system_a0);
    CE_INIT_API(api, ce_ydb_a0);
    CE_INIT_API(api, ct_editor_ui_a0);

    CE_UNUSED(api);

    ce_log_a0->info("example", "Init %d", reload);

    api->add_impl(CT_GAME_I,
                  &game_i0, sizeof(game_i0));

    api->add_impl(CT_ECS_COMPONENT_I,
                  &rotation_component_i, sizeof(rotation_component_i));

    api->add_impl(CT_ECS_COMPONENT_I,
                  &player_input_component_i, sizeof(player_input_component_i));

    api->register_api(CT_COMPONENT_INTERFACE,
                      &player_speed_component_i, sizeof(player_speed_component_i));

    api->add_impl(CT_ECS_SYSTEM_I,
                  &rotation_system_i0, sizeof(rotation_system_i0));

    api->add_impl(CT_ECS_SYSTEM_I,
                  &player_input_system_i0, sizeof(player_input_system_i0));

    api->register_api(SIMULATION_INTERFACE,
                      &player_move_simulation_i0, sizeof(player_move_simulation_i0));

    ce_cdb_a0->reg_obj_type(PLAYER_INPUT_COMPONENT,
                            plyer_input_component_prop, CE_ARRAY_LEN(plyer_input_component_prop));

    ce_cdb_a0->reg_obj_type(PLAYER_SPEED_COMPONENT,
                            plyer_speed_component_prop, CE_ARRAY_LEN(plyer_speed_component_prop));

    ce_cdb_a0->reg_obj_type(ROTATION_COMPONENT,
                            rotaton_component_prop, CE_ARRAY_LEN(rotaton_component_prop));

}

void CE_MODULE_UNLOAD (example_develop)(struct ce_api_a0 *api,
                                        int reload) {
    CE_UNUSED(api);

    ce_log_a0->info("example", "Shutdown %d", reload);
}

