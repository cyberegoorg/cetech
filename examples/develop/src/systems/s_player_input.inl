
#include <cetech/editor/editor_ui.h>
#include <celib/ydb.h>
#include <stdlib.h>
#include <celib/math/math.h>
#include <cetech/controlers/gamepad.h>
#include <cetech/machine/machine.h>


#define PLAYER_INPUT_SYSTEM \
    CE_ID64_0("player_input", 0xd89eea239dbc4d7aULL)

// system

//static void player_input_foreach_components(struct ct_world_t0 world,
//                                            struct ct_entity_t0 *ent,
//                                            ct_entity_storage_o0 *item,
//                                            uint32_t n,
//                                            ct_ecs_cmd_buffer_t *cmd_buff,
//                                            void *data) {
//    ct_controler_i0 *keyboard_ci = ct_controlers_a0->get(CONTROLER_KEYBOARD);
//
//    player_input_component *player_inputs = ct_ecs_a0->get_all(PLAYER_INPUT_COMPONENT, item);
//
//    for (uint32_t i = 0; i < n; ++i) {
//        player_input_component *pi = &player_inputs[i];
//
//        bool keyboard_up = keyboard_ci->button_state(0, keyboard_ci->button_index("w")) != 0;
//        bool keyboard_down = keyboard_ci->button_state(0, keyboard_ci->button_index("s")) != 0;
//        pi->move = (keyboard_up ? 1.0f : (keyboard_down ? -1.0f : 0.0f));
//
//        ct_ecs_a0->component_changed(world, ent[i], PLAYER_INPUT_COMPONENT);
//    }
//
//}

static float _angle(ce_vec2_t v) {
    ce_vec2_t n = ce_vec2_norm(v);
    float angle = ce_fasin(n.x) * CE_RAD_TO_DEG;

    if (v.y < 0) {
        angle = 180 - angle;
    } else if (v.x < 0) {
        angle = 360 + angle;
    }

    return angle;
}

static void _gamepad_controler(struct ct_world_t0 world,
                               struct ct_entity_t0 *ent,
                               ct_entity_storage_o0 *item,
                               uint32_t n,
                               void *data) {
    ct_controler_i0 *gamepad_ci = ct_controlers_a0->get(CONTROLER_GAMEPAD);
    player_input_component *player_inputs = ct_ecs_a0->get_all(PLAYER_INPUT_COMPONENT, item);
    gamepad_controler_component *gamepad = ct_ecs_a0->get_all(GAMEPAD_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        player_input_component *pi = &player_inputs[i];

        ce_vec3_t left_axis = {};
        ce_vec3_t right_axis = {};

        gamepad_ci->axis(gamepad[i].gamepad_id, GAMEPAD_AXIS_LEFT, &left_axis.x);
        gamepad_ci->axis(gamepad[i].gamepad_id, GAMEPAD_AXIS_RIGHT, &right_axis.x);

        if (left_axis.y > 0) {
            pi->move = 1.0f;
        } else if (left_axis.y < 0) {
            pi->move = -1.0f;
        } else {
            pi->move = 0;
        }

        float l = ce_vec2_length((ce_vec2_t) {.x = right_axis.x, .y = right_axis.y});
        float d = _angle((ce_vec2_t) {.x = right_axis.x, .y = right_axis.y});

        if (l != 0) {
            ce_log_a0->debug("ddd", "%f", d);

            if (d < 45) {
                d = 45;
            }

            if (d > 135) {
                d = 135;
            }

            ce_vec2_t dir = {.x = ce_fsin(d * CE_DEG_TO_RAD), .y = ce_fcos(d * CE_DEG_TO_RAD)};


            pi->shoot_dir = dir;
        }

        ct_ecs_a0->component_changed(world, ent[i], PLAYER_INPUT_COMPONENT);
    }
}

static void player_input_system(struct ct_world_t0 world,
                                float dt,
                                uint32_t rq_version,
                                ct_ecs_cmd_buffer_t *cmd) {
    ct_ecs_q_a0->foreach(world,
                         (ct_ecs_query_t0) {
                                 .all =  CT_ECS_ARCHETYPE(PLAYER_INPUT_COMPONENT,
                                                          GAMEPAD_COMPONENT),
                                 .write = CT_ECS_ARCHETYPE(PLAYER_INPUT_COMPONENT),
                         },
                         rq_version,
                         _gamepad_controler, NULL);
}

static uint64_t player_input_name() {
    return PLAYER_INPUT_SYSTEM;
}

static struct ct_system_i0 player_input_system_i0 = {
        .simulation = player_input_system,
        .name = player_input_name,
};

