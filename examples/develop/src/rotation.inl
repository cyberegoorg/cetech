
#include <cetech/editor/editor_ui.h>
#include <celib/ydb.h>
#include <stdlib.h>
#include <celib/math/math.h>

// component

struct rotation_component {
    float speed;
};

#define ROTATION_COMPONENT \
    CE_ID64_0("rotation_component", 0x775af3c84c1fa8efULL)

#define ROTATION_SYSTEM \
    CE_ID64_0("rotation", 0x2060566242789baaULL)

#define PROP_SPEED \
    CE_ID64_0("speed", 0x2c1c82c87303ec5fULL)


static uint64_t cdb_type() {
    return ROTATION_COMPONENT;
}

static const char *display_name() {
    return "Rotation";
}

static void *get_interface(uint64_t name_hash) {
    if (EDITOR_COMPONENT == name_hash) {
        static struct ct_editor_component_i0 ct_editor_component_i0 = {
                .display_name = display_name,
        };
        return &ct_editor_component_i0;
    }

    return NULL;
}


float _rnd_speed(uint32_t max) {
    return (((float) rand()) / RAND_MAX) * max;
}

static struct ct_component_i0 rotation_component_i = {
        .cdb_type = cdb_type,
        .get_interface = get_interface,
};


// system


static void foreach_rotation(struct ct_world_t0 world,
                             struct ct_entity_t0 *ent,
                             ct_entity_storage_o0 *item,
                             uint32_t n,
                             void *data) {
    float dt = *(float *) (data);

    uint64_t *rotations = ct_ecs_a0->get_all(ROTATION_COMPONENT, item);
    uint64_t *transforms = ct_ecs_a0->get_all(TRANSFORM_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        const ce_cdb_obj_o0 *transform = ce_cdb_a0->read(ce_cdb_a0->db(), transforms[i]);
        const ce_cdb_obj_o0 *rotation = ce_cdb_a0->read(ce_cdb_a0->db(), rotations[i]);


        uint64_t rot_o = ce_cdb_a0->read_subobject(transform, PROP_ROTATION, 0);
        ce_vec3_t rot = {};
        ce_cdb_a0->read_to(ce_cdb_a0->db(), rot_o, &rot, sizeof(rot));


        float speed = ce_cdb_a0->read_float(rotation, PROP_SPEED, 100.0f);
        rot.y += speed * 0.1f * dt;


        ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), rot_o);
        ce_cdb_a0->set_float(w, PROP_ROTATION_Y, rot.y);
        ce_cdb_a0->write_commit(w);
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

static struct ct_simulation_i0 rotation_simulation_i0 = {
        .simulation = rotation_system,
        .name = rotation_name,
        .before = rotation_before,
};

