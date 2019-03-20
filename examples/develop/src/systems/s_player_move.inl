
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
                                           void *data) {
    float dt = *(float *) (data);

    ct_transform_comp *transforms = ct_ecs_a0->get_all(TRANSFORM_COMPONENT, item);

    player_input_component *player_inputs = ct_ecs_a0->get_all(PLAYER_INPUT_COMPONENT, item);
    player_speed_component *speds = ct_ecs_a0->get_all(PLAYER_SPEED_COMPONENT, item);

    ct_viewport_t0 vw = ct_game_system_a0->viewport(ce_id_a0->id64("default"));
    ct_rg_builder_t0*b = ct_renderer_a0->viewport_builder(vw);

    uint16_t size[2] = {};
    b->get_size(b, size);

    size[0] -= 5.0f;
    size[1] -= 5.0f;

    for (uint32_t i = 0; i < n; ++i) {
        player_input_component *pi_o = &player_inputs[i];
        ct_transform_comp *transform_o = &transforms[i];
        player_speed_component *speed_o = &speds[i];

        transform_o->pos.x += pi_o->move.x * speed_o->speed * dt;
        transform_o->pos.y += pi_o->move.y * speed_o->speed * dt;

        if (transform_o->pos.x < 5.0f) {
            transform_o->pos.x = 5.0f;
        }

        if (transform_o->pos.x > size[0]) {
            transform_o->pos.x = size[0];
        }

        if (transform_o->pos.y < 5.0f) {
            transform_o->pos.y = 5.0f;
        }

        if (transform_o->pos.y > size[1]) {
            transform_o->pos.y = size[1];
        }

        ct_ecs_a0->component_changed(world, ent[i], TRANSFORM_COMPONENT);
    }
}

static void player_move_system(struct ct_world_t0 world,
                               float dt) {
    uint64_t mask = ct_ecs_a0->mask(TRANSFORM_COMPONENT)
                    | ct_ecs_a0->mask(PLAYER_INPUT_COMPONENT)
                    | ct_ecs_a0->mask(PLAYER_SPEED_COMPONENT);

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
    static uint64_t _before[] = {PLAYER_INPUT_SYSTEM};
    *n = CE_ARRAY_LEN(_before);
    return _before;
}

static struct ct_simulation_i0 player_move_simulation_i0 = {
        .simulation = player_move_system,
        .name = player_move_name,
        .before = player_move_before,
        .after = player_move_after,
};

