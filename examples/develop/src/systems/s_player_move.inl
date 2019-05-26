
#include <cetech/editor/editor_ui.h>
#include <celib/ydb.h>
#include <stdlib.h>
#include <celib/math/math.h>
#include <cetech/controlers/gamepad.h>
#include <cetech/machine/machine.h>


#define PLAYER_MOVE_SYSTEM \
    CE_ID64_0("player_move", 0xe77c42029af60d02ULL)

// system

static void player_move_foreach_components(struct ct_world_t0 world,
                                           struct ct_entity_t0 *ent,
                                           ct_entity_storage_o0 *item,
                                           uint32_t n,
                                           ct_ecs_cmd_buffer_t *cmd_buff,
                                           void *data) {
    float dt = *(float *) (data);

    player_input_component *player_inputs = ct_ecs_a0->get_all(PLAYER_INPUT_COMPONENT, item);
    velocity_component *speds = ct_ecs_a0->get_all(VELOCITY_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        player_input_component *pi_o = &player_inputs[i];
        velocity_component *speed_o = &speds[i];

        speed_o->velocity.y = pi_o->move * pi_o->move_speed * dt;

        ct_ecs_a0->component_changed(world, ent[i], VELOCITY_COMPONENT);
    }
}

static void player_move_system(struct ct_world_t0 world,
                               float dt) {
    uint64_t mask = ct_ecs_a0->mask(PLAYER_INPUT_COMPONENT)
                    | ct_ecs_a0->mask(VELOCITY_COMPONENT);

    ct_ecs_a0->process(world, mask, player_move_foreach_components, &dt);
}

static uint64_t player_move_name() {
    return PLAYER_MOVE_SYSTEM;
}

static const uint64_t *player_move_before(uint32_t *n) {
    static uint64_t _before[] = {TRANSFORM_SYSTEM};
    *n = CE_ARRAY_LEN(_before);
    return _before;
}

static const uint64_t *player_move_after(uint32_t *n) {
    static uint64_t _before[] = {PLAYER_INPUT_SYSTEM, };
    *n = CE_ARRAY_LEN(_before);
    return _before;
}

static struct ct_system_i0 player_move_system_i0 = {
        .simulation = player_move_system,
        .name = player_move_name,
        .before = player_move_before,
        .after = player_move_after,
};

