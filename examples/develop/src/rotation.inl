
#include <cetech/editor/editor_ui.h>
#include <celib/ydb.h>
#include <stdlib.h>
#include <celib/math/math.h>

// component

#define ROTATION_COMPONENT \
    CE_ID64_0("rotation_component", 0x775af3c84c1fa8efULL)

#define ROTATION_SYSTEM \
    CE_ID64_0("rotation", 0x2060566242789baaULL)

#define PROP_SPEED \
    CE_ID64_0("velocity", 0xa5d1f45b42234278ULL)

typedef struct rotation_component {
    float speed;
}rotation_component;

static uint64_t cdb_type() {
    return ce_id_a0->id64("rotation_component");
}

static const char *display_name() {
    return "Rotation";
}

float _rnd_speed(uint32_t max) {
    return (((float) rand()) / RAND_MAX) * max;
}

static uint64_t rotation_size() {
    return sizeof(struct rotation_component);
}

static void _rotation_on_spawn(uint64_t obj,
                             void *data) {
    rotation_component *c = data;
    ce_cdb_a0->read_to(ce_cdb_a0->db(), obj, c, sizeof(rotation_component));
}


static struct ct_ecs_component_i0 rotation_component_i = {
        .display_name = display_name,
        .cdb_type = cdb_type,
        .size = rotation_size,
        .on_spawn = _rotation_on_spawn,
        .on_change = _rotation_on_spawn,
};


// system
static void foreach_rotation(struct ct_world_t0 world,
                             struct ct_entity_t0 *ent,
                             ct_entity_storage_o0 *item,
                             uint32_t n,
                             ct_ecs_cmd_buffer_t* cmd_buff,
                             void *data) {
    float dt = *(float *) (data);

    rotation_component *rotations = ct_ecs_a0->get_all(ROTATION_COMPONENT, item);
    ct_transform_comp *transforms = ct_ecs_a0->get_all(TRANSFORM_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        ct_transform_comp *t = &transforms[i];
        rotation_component *r = &rotations[i];

        t->rot.z += r->speed * 0.1f * dt;

        ct_ecs_a0->component_changed(world, ent[i], TRANSFORM_COMPONENT);
    }
}

static void rotation_system(struct ct_world_t0 world,
                            float dt) {
    uint64_t mask = ct_ecs_a0->mask(ROTATION_COMPONENT)
                    | ct_ecs_a0->mask(TRANSFORM_COMPONENT);

    ct_ecs_a0->process(world, mask, foreach_rotation, &dt);
}

static uint64_t rotation_name() {
    return ROTATION_SYSTEM;
}

static const uint64_t *rotation_before(uint32_t *n) {
    static uint64_t _before[] = {TRANSFORM_SYSTEM};
    *n = CE_ARRAY_LEN(_before);
    return _before;
}

static struct ct_system_i0 rotation_system_i0 = {
        .simulation = rotation_system,
        .name = rotation_name,
        .before = rotation_before,
};

static const ce_cdb_prop_def_t0 rotaton_component_prop[] = {
        {
                .name = "velocity",
                .type = CE_CDB_TYPE_FLOAT,
                .value.f = 10.0f,
        },
};
