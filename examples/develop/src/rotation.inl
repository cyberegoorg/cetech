
#include <cetech/editor/resource_ui.h>
#include <celib/ydb.h>


// component

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

//static uint64_t size() {
//    return sizeof(struct rotation_component);
//}

static struct ct_component_i0 rotation_component_i = {
//        .size = size,
        .cdb_type = cdb_type,
        .get_interface = get_interface,
};


// system


static void foreach_rotation(struct ct_world world,
                             struct ct_entity *ent,
                             ct_entity_storage_t *item,
                             uint32_t n,
                             void *data) {
    float dt = *(float *) (data);

    for (uint32_t i = 1; i < n; ++i) {
        uint64_t rotation = ct_ecs_a0->get_one(world, ROTATION_COMPONENT,
                                               ent[i]);
        uint64_t transform = ct_ecs_a0->get_one(world, TRANSFORM_COMPONENT,
                                                ent[i]);

        const ce_cdb_obj_o *t_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                       transform);
        const ce_cdb_obj_o *r_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                       rotation);

        float speed = ce_cdb_a0->read_float(r_reader, PROP_SPEED, 0);

        float rotc[3] = {
                ce_cdb_a0->read_float(t_reader, PROP_ROTATION_X, 0),
                ce_cdb_a0->read_float(t_reader, PROP_ROTATION_Y, 0),
                ce_cdb_a0->read_float(t_reader, PROP_ROTATION_Z, 0),
        };

        float rot[3] = {};
        ce_vec3_add_s(rot, rotc, speed * dt);

        ce_cdb_obj_o *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), transform);
        ce_cdb_a0->set_float(w, PROP_ROTATION_X, rot[0]);
        ce_cdb_a0->set_float(w, PROP_ROTATION_Y, rot[1]);
        ce_cdb_a0->set_float(w, PROP_ROTATION_Z, rot[2]);
        ce_cdb_a0->write_commit(w);
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

