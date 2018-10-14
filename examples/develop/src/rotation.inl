#include <celib/ydb.h>
#include <celib/yng.h>
#include <cetech/editor/editor_ui.h>


// component

#define ROTATION_COMPONENT \
    CE_ID64_0("rotation", 0x2060566242789baaULL)

#define ROTATION_SYSTEM \
    CE_ID64_0("rotation", 0x2060566242789baaULL)

#define PROP_SPEED \
    CE_ID64_0("speed", 0x2c1c82c87303ec5fULL)


struct rotation_component {
    float speed;
};

static void component_compiler(const char *filename,
                               uint64_t component_key,
                               ce_cdb_obj_o *writer) {
    struct rotation_component t_data;

    t_data.speed = ce_cdb_a0->read_float(component_key, ce_yng_a0->key("speed"), 0.0f);
    ce_cdb_a0->set_float(writer, PROP_SPEED, t_data.speed);
}


static struct ct_comp_prop_decs ct_comp_prop_decs = {
        .prop_decs = (struct ct_prop_decs[]) {
                {
                        .type = ECS_PROP_FLOAT,
                        .name = PROP_SPEED,
                        .offset = offsetof(struct rotation_component, speed),
                },
        },
        .prop_n = 1,

};

static const struct ct_comp_prop_decs *prop_desc() {
    return &ct_comp_prop_decs;
}

static const char *prop_display_name(uint64_t prop) {
    switch (prop) {
        case PROP_SPEED:
            return "Speed";
        default:
            return NULL;
    }
}

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
                .prop_display_name = prop_display_name,
        };
        return &ct_editor_component_i0;
    }

    return NULL;
}

static uint64_t size() {
    return sizeof(struct rotation_component);
}

static struct ct_component_i0 rotation_component_i = {
        .size = size,
        .cdb_type = cdb_type,
        .compiler = component_compiler,
        .get_interface = get_interface,
        .prop_desc = prop_desc,
};


// system


static void foreach_rotation(struct ct_world world,
                             struct ct_entity *ent,
                             ct_entity_storage_t *item,
                             uint32_t n,
                             void *data) {
    float dt = *(float *) (data);

    struct rotation_component *rotation;
    rotation = ct_ecs_a0->component->get_all(ROTATION_COMPONENT, item);

    struct ct_transform_comp *transform;
    transform = ct_ecs_a0->component->get_all(TRANSFORM_COMPONENT, item);

    for (uint32_t i = 1; i < n; ++i) {
        float rot[3] = {0};
        ce_vec3_add_s(rot, transform[i].rotation, rotation[i].speed * dt);
        ce_vec3_move(transform[i].rotation, rot);
    }
}

static void rotation_system(struct ct_world world,
                            float dt) {
    uint64_t mask = ct_ecs_a0->component->mask(ROTATION_COMPONENT)
                    | ct_ecs_a0->component->mask(TRANSFORM_COMPONENT);

    ct_ecs_a0->system->process(world, mask, foreach_rotation, &dt);
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

