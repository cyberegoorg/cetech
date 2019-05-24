
#include <stdlib.h>

#include <celib/ydb.h>
#include <celib/math/math.h>
#include <celib/math/bounds.h>
#include <cetech/controlers/gamepad.h>
#include <cetech/machine/machine.h>
#include <celib/memory/allocator.h>
#include <celib/memory/memory.h>
#include <celib/containers/array.h>

#include <cetech/editor/editor_ui.h>
#include <float.h>

#define COLLISION_SYSTEM \
    CE_ID64_0("collision", 0xa3d491d11371d699ULL)

// system


typedef struct rect_t {
    ce_vec2_t points[4];
} rect_t;

static inline ce_vec2_t _get_normal(ce_vec2_t v) {
    return (ce_vec2_t) {.x = -v.y, .y = v.x};
}

static inline ce_vec2_t get_perpendicular_axis(rect_t o1,
                                               uint32_t idx) {
    ce_vec2_t v1 = o1.points[idx + 1];
    ce_vec2_t v2 = o1.points[idx];
    return _get_normal(ce_vec2_norm(ce_vec2_sub(v1, v2)));
}

static inline void get_perpendicular_axes(rect_t o1,
                                          rect_t o2,
                                          ce_vec2_t axis[4]) {

    axis[0] = get_perpendicular_axis(o1, 0);
    axis[1] = get_perpendicular_axis(o1, 1);

    axis[2] = get_perpendicular_axis(o2, 0);
    axis[3] = get_perpendicular_axis(o2, 1);
}

static inline ce_vec2_t _project_on_axis(rect_t o1,
                                         ce_vec2_t axis) {
    float min = INFINITY;
    float max = -INFINITY;

    for (int i = 0; i < 4; ++i) {
        ce_vec2_t v1 = o1.points[i];

        float proj = ce_vec2_dot(v1, axis);
        if (proj < min) { min = proj; }
        if (proj > max) { max = proj; }
    }

    return (ce_vec2_t) {.x = min, .y = max};
}


static float _get_overlap_length(ce_vec2_t a,
                                 ce_vec2_t b) {
    if (!(a.x <= b.y && a.y >= b.x)) {
        return 0.0f;
    }

    return ce_fmin(a.y, b.y) - ce_fmax(a.x, b.x);
}


static inline bool _test_collision(ce_vec2_t c1,
                                   rect_t o1,
                                   ce_vec2_t c2,
                                   rect_t o2,
                                   ce_vec2_t *_mtv) {
    ce_vec2_t axis[4];
    get_perpendicular_axes(o1, o2, axis);

    float min_overlap = INFINITY;
    ce_vec2_t mtv = {};
    for (int i = 0; i < 4; ++i) {
        ce_vec2_t proj1 = _project_on_axis(o1, axis[i]);
        ce_vec2_t proj2 = _project_on_axis(o2, axis[i]);

        float l = _get_overlap_length(proj1, proj2);

        if (l == 0.0f) {
            return false;
        }

        if (l < min_overlap) {
            min_overlap = l;
            mtv = ce_vec2_mul_s(axis[i], min_overlap);
        }

    }

    if (ce_vec2_dot(ce_vec2_sub(c1, c2), mtv) < 0) {
        mtv = ce_vec2_neg(mtv);
    }

    *_mtv = mtv;

    return true;
}

typedef struct coll_t {
    float dt;
    rect_t *obbs;
    ce_vec2_t *centers;
    ct_entity_t0 *ents;
} coll_t;

static void collision_foreach_components(struct ct_world_t0 world,
                                         struct ct_entity_t0 *ent,
                                         ct_entity_storage_o0 *item,
                                         uint32_t n,
                                         ct_ecs_cmd_buffer_t *cmd_buff,
                                         void *data) {
    ct_transform_comp *transforms = ct_ecs_a0->get_all(TRANSFORM_COMPONENT, item);
    rectangle_component *rectangles = ct_ecs_a0->get_all(RECTANGLE_COMPONENT, item);

    coll_t *output = (coll_t *) data;

    for (uint32_t i = 0; i < n; ++i) {
        ct_transform_comp *transform_o = &transforms[i];
        rectangle_component *rectangle = &rectangles[i];

        ce_vec2_t center = {.x = transform_o->pos.x, .y = transform_o->pos.y};

        ce_array_push(output->ents, ent[i], ce_memory_a0->system);
        ce_array_push(output->centers, center, ce_memory_a0->system);

        rect_t r = {
                .points={
                        {center.x - rectangle->half_size.x, center.y - rectangle->half_size.y},
                        {center.x + rectangle->half_size.x, center.y - rectangle->half_size.y},
                        {center.x + rectangle->half_size.x, center.y + rectangle->half_size.y},
                        {center.x - rectangle->half_size.x, center.y + rectangle->half_size.y},
                }
        };

        ce_array_push(output->obbs, r, ce_memory_a0->system);
    }
}


static void collision_system(struct ct_world_t0 world,
                             float dt) {
    uint64_t mask = ct_ecs_a0->mask(TRANSFORM_COMPONENT)
                    | ct_ecs_a0->mask(RECTANGLE_COMPONENT);

    coll_t output = {.dt=dt};
    ct_ecs_a0->process_serial(world, mask, collision_foreach_components, &output);

    uint32_t n = ce_array_size(output.obbs);

    for (uint32_t i = 0; i < n; ++i) {
        rect_t obb1 = output.obbs[i];
        ce_vec2_t c1 = output.centers[i];

        for (uint32_t j = 0; j < n; ++j) {
            if (i == j) {
                continue;
            }

            rect_t obb2 = output.obbs[j];
            ce_vec2_t c2 = output.centers[j];

            ce_vec2_t mtv = {};
            if (_test_collision(c1, obb1, c2, obb2, &mtv)) {
                ct_ecs_a0->add(world, output.ents[i], (ct_component_pair_t0[]) {
                        {
                                .data = &(hit_component) {
                                        .ent2=output.ents[j],
                                        .normal = mtv,
                                },
                                .type = HIT_COMPONENT
                        }
                }, 1);
            }
        }
    }

    ce_array_free(output.obbs, ce_memory_a0->system);
    ce_array_free(output.ents, ce_memory_a0->system);
    ce_array_free(output.centers, ce_memory_a0->system);
}

static uint64_t collision_name() {
    return COLLISION_SYSTEM;
}

static const uint64_t *collision_before(uint32_t *n) {
    static uint64_t _before[] = {TRANSFORM_SYSTEM};
    *n = CE_ARRAY_LEN(_before);
    return _before;
}

static const uint64_t *collision_after(uint32_t *n) {
    static uint64_t _before[] = {PLAYER_INPUT_SYSTEM};
    *n = CE_ARRAY_LEN(_before);
    return _before;
}

static struct ct_system_i0 collision_system_i0 = {
        .simulation = collision_system,
        .name = collision_name,
        .before = collision_before,
        .after = collision_after,
};

