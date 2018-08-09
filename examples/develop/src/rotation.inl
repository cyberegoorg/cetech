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

    struct ce_yng_doc *d = ce_ydb_a0->get(filename);
    uint64_t keys[component_key_count + 1];
    memcpy(keys, component_key, sizeof(uint64_t) * component_key_count);

    uint64_t key;

    keys[component_key_count] = ce_yng_a0->key("speed");
    key = ce_yng_a0->combine_key(keys, CE_ARRAY_LEN(keys));
    if (d->has_key(d, key)) {
        t_data.speed = ce_ydb_a0->get_float(filename, keys, CE_ARRAY_LEN(keys),
                                            0.0f);

        ce_cdb_a0->set_float(writer, PROP_SPEED, t_data.speed);
    }
}

static void _on_component_obj_change(uint64_t obj,
                                     const uint64_t *prop,
                                     uint32_t prop_count,
                                     void *data) {

    uint64_t ent_obj = ce_cdb_a0->parent(ce_cdb_a0->parent(obj));

    struct ct_world world = {
            .h = ce_cdb_a0->read_uint64(ent_obj, ENTITY_WORLD, 0)
    };

    struct ct_entity ent = {.h = ent_obj};

    struct rotation_component *component;
    component = ct_ecs_a0->component->get_one(world, ROTATION_COMPONENT,
                                              ent);

    component->speed = ce_cdb_a0->read_float(obj, PROP_SPEED, 0.0f);
}


static void component_spawner(uint64_t obj,
                              void *data) {
    struct rotation_component *component = data;

    component->speed = ce_cdb_a0->read_float(obj, PROP_SPEED, 0);
    ce_cdb_a0->register_notify(obj, _on_component_obj_change, NULL);
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

    for (uint32_t i = 1; i < n; ++i) {
        uint64_t ent_obj = ent[i].h;
        uint64_t components = ce_cdb_a0->read_subobject(ent_obj,
                                                        ENTITY_COMPONENTS, 0);

        uint64_t component = ce_cdb_a0->read_subobject(components,
                                                       TRANSFORM_COMPONENT, 0);

        float rot[3] = {0};
        ce_cdb_a0->read_vec3(component, PROP_ROTATION, rot);

        ce_vec3_add_s(rot, rot, rotation[i].speed * dt);

        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(component);
        ce_cdb_a0->set_vec3(w, PROP_ROTATION, rot);
        ce_cdb_a0->write_commit(w);
    }
}

static void rotation_system(struct ct_world world,
                            float dt) {
    uint64_t mask = ct_ecs_a0->component->mask(ROTATION_COMPONENT)
                    | ct_ecs_a0->component->mask(TRANSFORM_COMPONENT);

    ct_ecs_a0->system->process(world, mask, foreach_rotation, &dt);
}