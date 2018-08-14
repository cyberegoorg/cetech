#include <celib/ydb.h>
#include <celib/yng.h>
#include <cetech/editor/editor_ui.h>


// component

#define ROTATION_COMPONENT \
    CE_ID64_0("rotation", 0x2060566242789baaULL)

#define PROP_SPEED \
    CE_ID64_0("speed", 0x2c1c82c87303ec5fULL)


struct rotation_component {
    float speed;
};

static void component_compiler(const char *filename,
                               uint64_t *component_key,
                               uint32_t component_key_count,
                               ce_cdb_obj_o *writer) {
    struct rotation_component t_data;

//    struct ce_yng_doc *d = ce_ydb_a0->get(filename);
    uint64_t keys[component_key_count + 1];
    memcpy(keys, component_key, sizeof(uint64_t) * component_key_count);

//    uint64_t key;

    keys[component_key_count] = ce_yng_a0->key("speed");
//    key = ce_yng_a0->combine_key(keys, CE_ARRAY_LEN(keys));
//    if (d->has_key(d, key)) {
        t_data.speed = ce_ydb_a0->get_float(filename, keys, CE_ARRAY_LEN(keys),
                                            0.0f);

        ce_cdb_a0->set_float(writer, PROP_SPEED, t_data.speed);
//    }
}


static void _obj_change(struct ct_world world,
                        uint64_t obj,
                        const uint64_t *prop,
                        uint32_t prop_count,
                        struct ct_entity *ents,
                        uint32_t n) {
    for (int i = 0; i < n; ++i) {
        struct ct_entity ent = ents[i];

        struct rotation_component *component;
        component = ct_ecs_a0->component->get_one(world, ROTATION_COMPONENT,
                                                  ent);

        component->speed = ce_cdb_a0->read_float(obj, PROP_SPEED, 0.0f);
    }
}


static void component_spawner(struct ct_world world,
                              uint64_t obj,
                              void *data) {
    struct rotation_component *component = data;

    component->speed = ce_cdb_a0->read_float(obj, PROP_SPEED, 0);
}

static uint64_t cdb_type() {
    return ROTATION_COMPONENT;
}

static const char *display_name() {
    return "Rotation";
}

static void property_editor(uint64_t obj) {
    ct_editor_ui_a0->ui_float(obj, PROP_SPEED, "Speed", 0, 0);
}

static void *get_interface(uint64_t name_hash) {
    if (EDITOR_COMPONENT == name_hash) {
        static struct ct_editor_component_i0 ct_editor_component_i0 = {
                .display_name = display_name,
                .property_editor = property_editor,
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
        .spawner = component_spawner,
        .get_interface = get_interface,
        .obj_change = _obj_change,
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