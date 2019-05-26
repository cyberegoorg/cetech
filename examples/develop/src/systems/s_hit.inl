
#include <stdlib.h>

#include <celib/ydb.h>
#include <celib/math/math.h>
#include <cetech/controlers/gamepad.h>
#include <cetech/machine/machine.h>
#include <celib/memory/allocator.h>
#include <celib/memory/memory.h>
#include <celib/containers/array.h>

#include <cetech/editor/editor_ui.h>

#define HIT_SYSTEM \
    CE_ID64_0("hit_system", 0x48a646ace82d475fULL)

// system

static ce_vec2_t ce_vec2_reflect(ce_vec2_t dir,
                                 ce_vec2_t normal) {
    float s = 2.0f * ce_vec2_dot(dir, normal);
    ce_vec2_t n = ce_vec2_mul_s(normal, s);

    return ce_vec2_neg(ce_vec2_sub(n, dir));
}

static void hit_foreach_components(struct ct_world_t0 world,
                                   struct ct_entity_t0 *ent,
                                   ct_entity_storage_o0 *item,
                                   uint32_t n,
                                   ct_ecs_cmd_buffer_t *cmd_buff,
                                   void *data) {
    ct_transform_comp *transforms = ct_ecs_a0->get_all(TRANSFORM_COMPONENT, item);
    velocity_component *velocity = ct_ecs_a0->get_all(VELOCITY_COMPONENT, item);
    hit_component *hits = ct_ecs_a0->get_all(HIT_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        ct_transform_comp *t = &transforms[i];
        hit_component *hit = &hits[i];
        velocity_component *vel = &velocity[i];

        ce_vec2_t pos = {.x = t->pos.x, .y = t->pos.y};
        pos = ce_vec2_add(pos, hit->normal);

        t->pos.x = pos.x;
        t->pos.y = pos.y;

        bool with_pad = ct_ecs_a0->has(world, hit->ent2,
                                       (uint64_t[]) {PLAYER_INPUT_COMPONENT}, 1);


        if (with_pad) {
            float speed = ce_vec2_length(vel->velocity);

            player_input_component *pi = ct_ecs_a0->get_one(world,
                                                            PLAYER_INPUT_COMPONENT,
                                                            hit->ent2);

            if (ce_vec2_length(pi->shoot_dir) != 0.0) {
                vel->velocity = ce_vec2_mul_s(ce_vec2_norm(pi->shoot_dir), speed);
            } else {
                vel->velocity = ce_vec2_reflect(vel->velocity, ce_vec2_norm(hit->normal));
            }

            ce_vec2_t dir = ce_vec2_mul_s(ce_vec2_norm(vel->velocity), 1);

            vel->velocity = ce_vec2_add(vel->velocity, dir);

        } else {
            vel->velocity = ce_vec2_reflect(vel->velocity, ce_vec2_norm(hit->normal));
        }

        ct_ecs_a0->component_changed(world, ent[i], TRANSFORM_COMPONENT);

        ct_ecs_a0->remove_buff(cmd_buff, world, ent[i],
                               (uint64_t[]) {HIT_COMPONENT}, 1);

    }
}

static void hit_pad_foreach_components(struct ct_world_t0 world,
                                       struct ct_entity_t0 *ent,
                                       ct_entity_storage_o0 *item,
                                       uint32_t n,
                                       ct_ecs_cmd_buffer_t *cmd_buff,
                                       void *data) {
    ct_transform_comp *transforms = ct_ecs_a0->get_all(TRANSFORM_COMPONENT, item);
    hit_component *hits = ct_ecs_a0->get_all(HIT_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        ct_transform_comp *t = &transforms[i];
        hit_component *hit = &hits[i];

        bool with_wall = !ct_ecs_a0->has(world, hit->ent2,
                                         (uint64_t[]) {BALL_COMPONENT}, 1);

        if (with_wall) {
            ce_vec2_t pos = {.x = t->pos.x, .y = t->pos.y};
            pos = ce_vec2_add(pos, ce_vec2_mul_s(hit->normal, 1.2));

            t->pos.x = pos.x;
            t->pos.y = pos.y;

            ct_ecs_a0->component_changed(world, ent[i], TRANSFORM_COMPONENT);
        }

        ct_ecs_a0->remove_buff(cmd_buff, world, ent[i],
                               (uint64_t[]) {HIT_COMPONENT}, 1);

    }
}

static void hit_system(struct ct_world_t0 world,
                       float dt) {
    uint64_t mask = ct_ecs_a0->mask(TRANSFORM_COMPONENT)
                    | ct_ecs_a0->mask(BALL_COMPONENT)
                    | ct_ecs_a0->mask(VELOCITY_COMPONENT)
                    | ct_ecs_a0->mask(HIT_COMPONENT);

    ct_ecs_a0->process(world, mask, hit_foreach_components, &dt);

    mask = ct_ecs_a0->mask(TRANSFORM_COMPONENT)
           | ct_ecs_a0->mask(PAD_COMPONENT)
           | ct_ecs_a0->mask(HIT_COMPONENT);

    ct_ecs_a0->process(world, mask, hit_pad_foreach_components, &dt);
}

static uint64_t hit_name() {
    return HIT_SYSTEM;
}

static const uint64_t *hit_before(uint32_t *n) {
    static uint64_t _before[] = {TRANSFORM_SYSTEM};
    *n = CE_ARRAY_LEN(_before);
    return _before;
}

static const uint64_t *hit_after(uint32_t *n) {
    static uint64_t _after[] = {COLLISION_SYSTEM};
    *n = CE_ARRAY_LEN(_after);
    return _after;
}

static struct ct_system_i0 hit_system_i0 = {
        .simulation = hit_system,
        .name = hit_name,
        .before = hit_before,
        .after = hit_after,
};

