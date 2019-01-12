
#include <cetech/editor/editor_ui.h>
#include <celib/ydb.h>
#include <stdlib.h>


// component

struct rotation_component {
    float speed;
};

#define ROTATION_COMPONENT \
    CE_ID64_0("rotation", 0x2060566242789baaULL)

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

static uint64_t size() {
    return sizeof(struct rotation_component);
}

static float _rnd_speed(uint32_t max) {
    return (((float)rand())/RAND_MAX) * max;
}

static void spawner(struct ct_world world,
                    uint64_t obj,
                    void *data) {

//    const ce_cdb_obj_o *r = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    struct rotation_component *c = data;



    *c = (struct rotation_component) {
            .speed = _rnd_speed(100.0f),
    };
}


static struct ct_component_i0 rotation_component_i = {
        .size = size,
        .cdb_type = cdb_type,
        .get_interface = get_interface,
        .spawner = spawner,
};


// system


static void foreach_rotation(struct ct_world world,
                             struct ct_entity *ent,
                             ct_entity_storage_t *item,
                             uint32_t n,
                             void *data) {
    float dt = *(float *) (data);

    struct rotation_component *rotations = ct_ecs_a0->get_all(
            ROTATION_COMPONENT, item);
    struct ct_transform_comp *transforms = ct_ecs_a0->get_all(
            TRANSFORM_COMPONENT, item);

    for (uint32_t i = 0; i < n; ++i) {
        struct rotation_component *rotation = &rotations[i];
        struct ct_transform_comp *transform = &transforms[i];

        float rot[3] = {};
        ce_vec3_add_s(rot, transform->rot, rotation->speed * dt);
        ce_vec3_move(transform->rot, rot);
    }
}

static void rotation_system(struct ct_world world,
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

