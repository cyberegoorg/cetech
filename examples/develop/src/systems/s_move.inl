
#include <stdlib.h>

#include <celib/ydb.h>
#include <celib/math/math.h>
#include <cetech/controlers/gamepad.h>
#include <cetech/machine/machine.h>
#include <celib/memory/allocator.h>
#include <celib/memory/memory.h>
#include <celib/containers/array.h>

#include <cetech/editor/editor_ui.h>

#define BALL_SYSTEM \
    CE_ID64_0("move", 0x33603ac62788b5c5ULL)

// system
static void move_foreach_components(struct ct_world_t0 world,
                                         struct ct_entity_t0 *ent,
                                         ct_entity_storage_o0 *item,
                                         uint32_t n,
                                         ct_ecs_cmd_buffer_t *cmd_buff,
                                         void *data) {
    float dt = *(float *) (data);

    ct_transform_comp *transforms = ct_ecs_a0->get_all(TRANSFORM_COMPONENT, item);
    velocity_component *speds = ct_ecs_a0->get_all(VELOCITY_COMPONENT, item);
    
    for (uint32_t i = 0; i < n; ++i) {
        ct_transform_comp *transform_o = &transforms[i];
        velocity_component *speed_o = &speds[i];

        ce_vec2_t dt_pos = ce_vec2_mul_s(speed_o->velocity, dt);
        transform_o->pos.x += dt_pos.x;
        transform_o->pos.y += dt_pos.y;

        ct_ecs_a0->component_changed(world, ent[i], TRANSFORM_COMPONENT);
    }
}

static void move_system(struct ct_world_t0 world,
                             float dt) {
    uint64_t mask = ct_ecs_a0->mask(TRANSFORM_COMPONENT)
                    | ct_ecs_a0->mask(VELOCITY_COMPONENT);

    ct_ecs_a0->process(world, mask, move_foreach_components, &dt);
}

static uint64_t move_name() {
    return BALL_SYSTEM;
}

static const uint64_t *move_before(uint32_t *n) {
    static uint64_t _before[] = {TRANSFORM_SYSTEM};
    *n = CE_ARRAY_LEN(_before);
    return _before;
}

static const uint64_t *move_after(uint32_t *n) {
    static uint64_t _before[] = {PLAYER_INPUT_SYSTEM};
    *n = CE_ARRAY_LEN(_before);
    return _before;
}

static struct ct_system_i0 move_system_i0 = {
        .simulation = move_system,
        .name = move_name,
        .before = move_before,
        .after = move_after,
};

