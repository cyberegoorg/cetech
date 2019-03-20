
#include <cetech/editor/editor_ui.h>
#include <celib/ydb.h>
#include <stdlib.h>
#include <celib/math/math.h>
#include <cetech/controlers/gamepad.h>
#include <cetech/machine/machine.h>


#define PLAYER_INPUT_SYSTEM \
    CE_ID64_0("player_input", 0xd89eea239dbc4d7aULL)

// system

static void player_input_foreach_components(struct ct_world_t0 world,
                                            struct ct_entity_t0 *ent,
                                            ct_entity_storage_o0 *item,
                                            uint32_t n,
                                            void *data) {
    ct_controlers_i0 *gamepad_ci = ct_controlers_a0->get(CONTROLER_GAMEPAD);
    ct_controlers_i0 *keyboard_ci = ct_controlers_a0->get(CONTROLER_KEYBOARD);

    player_input_component *player_inputs = ct_ecs_a0->get_all(PLAYER_INPUT_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        player_input_component *pi = &player_inputs[i];

        gamepad_ci->axis(0, i==0 ? GAMEPAD_AXIS_LEFT: GAMEPAD_AXIS_RIGHT, &pi->move.x);

        bool keyboard_up = keyboard_ci->button_state(0, keyboard_ci->button_index("w")) != 0;
        bool keyboard_down = keyboard_ci->button_state(0, keyboard_ci->button_index("s")) != 0;

        if(keyboard_up || keyboard_down) {
            pi->move.y += 1.0f * (keyboard_up ? 1: -1);
        }

        ct_ecs_a0->component_changed(world, ent[i], PLAYER_INPUT_COMPONENT);
    }

}

static void player_input_system(struct ct_world_t0 world,
                                float dt) {
    uint64_t mask = ct_ecs_a0->mask(PLAYER_INPUT_COMPONENT);

    ct_ecs_a0->process(world, mask, player_input_foreach_components, &dt);
}

static uint64_t player_input_name() {
    return PLAYER_INPUT_SYSTEM;
}

static struct ct_simulation_i0 player_input_simulation_i0 = {
        .simulation = player_input_system,
        .name = player_input_name,
};

