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

#include "components/c_player_input.inl"
#include "components/c_ball.inl"
#include "components/c_pad.inl"
#include "components/c_hit.inl"
#include "components/c_rectangle.inl"
#include "components/c_gamepad_controler.inl"

#include "systems/s_player_input.inl"
#include "systems/rectangle_renderer.inl"


void init(ct_world_t0 world) {
    uint64_t boot_ent = ce_config_a0->read_uint(CONFIG_BOOT_ENT, 0);
    ct_ecs_e_a0->spawn_entity(world, boot_ent);
}

void shutdown(ct_world_t0 world) {
}

void update(ct_world_t0 world,
            float dt) {
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
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_config_a0);
    CE_INIT_API(api, ct_dd_a0);

    ce_log_a0->info("example", "Init %d", reload);

    api->add_impl(CT_GAME_I,
                  &game_i0, sizeof(game_i0));

    api->add_impl(CT_ECS_COMPONENT_I,
                  &player_input_component_i, sizeof(player_input_component_i));

    api->add_impl(CT_ECS_COMPONENT_I,
                  &ball_component_i, sizeof(ball_component_i));

    api->add_impl(CT_ECS_COMPONENT_I,
                  &pad_component_i, sizeof(pad_component_i));

    api->add_impl(CT_ECS_COMPONENT_I,
                  &hit_component_i, sizeof(hit_component_i));

    api->add_impl(CT_ECS_COMPONENT_I,
                  &rectangle_component_i, sizeof(rectangle_component_i));

    api->add_impl(CT_ECS_COMPONENT_I,
                  &gamepad_controler_component_i, sizeof(gamepad_controler_component_i));

    api->add_impl(CT_ECS_SYSTEM_I,
                  &player_input_system_i0, sizeof(player_input_system_i0));

    ce_cdb_a0->reg_obj_type(GAMEPAD_COMPONENT,
                            gamepad_controler_compo_prop,
                            CE_ARRAY_LEN(gamepad_controler_compo_prop));

    ce_cdb_a0->reg_obj_type(RECTANGLE_COMPONENT,
                            rectangle_component_prop,
                            CE_ARRAY_LEN(rectangle_component_prop));

    ce_cdb_a0->reg_obj_type(PLAYER_INPUT_COMPONENT, CE_ARR_ARG(player_input_component_prop));

    ce_cdb_a0->reg_obj_type(BALL_COMPONENT, NULL, 0);
    ce_cdb_a0->reg_obj_type(PAD_COMPONENT, NULL, 0);

    rectangle_render_load_module(api, reload);
}

void CE_MODULE_UNLOAD (example_develop)(struct ce_api_a0 *api,
                                        int reload) {
    CE_UNUSED(api);

    ce_log_a0->info("example", "Shutdown %d", reload);
}

