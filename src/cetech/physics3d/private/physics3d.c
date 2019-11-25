#include <celib/api.h>
#include <celib/module.h>
#include <celib/macros.h>
#include <celib/memory/allocator.h>
#include <celib/memory/memory.h>
#include <celib/containers/hash.h>
#include <celib/cdb.h>
#include <celib/id.h>
#include <celib/log.h>

#include <cetech/ecs/ecs.h>
#include <cetech/editor/editor_ui.h>
#include <cetech/property_editor/property_editor.h>
#include <cetech/physics3d/physics3d.h>
#include <cetech/transform/transform.h>
#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>

// World
static uint64_t world_cdb_type() {
    return ce_id_a0->id64("physics_world3d");
}

static const char *wolrd3d_display_name() {
    return "PhysicsWorld3D";
}

static const ce_cdb_prop_def_t0 wolrd3d_component_prop[] = {
        {
                .name = "gravity",
                .type = CE_CDB_TYPE_SUBOBJECT,
                .obj_type = VEC3_CDB_TYPE,
                .flags = CE_CDB_PROP_FLAG_UNPACK,
        },
};

static void _world3d_from_cdb(ct_world_t0 world,
                              ce_cdb_t0 db,
                              uint64_t obj,
                              void *data) {
    ct_physics_world3d_c *c = (ct_physics_world3d_c *) data;

    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);

    uint64_t gravity = ce_cdb_a0->read_subobject(r, PHYSICS_WORLD3D_GRAVITY_PROP, 0);

    const ce_cdb_obj_o0 *gravity_r = ce_cdb_a0->read(db, gravity);

    c->gravity = (ce_vec3_t) {
            .x=ce_cdb_a0->read_float(gravity_r, PROP_VEC_X, 0.f),
            .y=ce_cdb_a0->read_float(gravity_r, PROP_VEC_Y, 0.f),
            .z=ce_cdb_a0->read_float(gravity_r, PROP_VEC_Z, 0.f),
    };

}

static struct ct_ecs_component_i0 world3d_component_i = {
        .display_name = wolrd3d_display_name,
        .cdb_type = PHYSICS3D_WORLD_COMPONENT,
        .size = sizeof(ct_physics_world3d_c),
        .from_cdb_obj = _world3d_from_cdb,
};

static void _wolrd3d_property_editor(ce_cdb_t0 db,
                                     uint64_t obj,
                                     uint64_t context,
                                     const char *filter) {
    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);
    uint64_t gravity = ce_cdb_a0->read_subobject(r, PHYSICS_WORLD3D_GRAVITY_PROP, 0);

    ct_editor_ui_a0->prop_vec3(gravity, "Gravity", filter,
                               (uint64_t[]) {PROP_VEC_X, PROP_VEC_Y, PROP_VEC_Z},
                               (ui_vec3_p0) {});
}

static struct ct_property_editor_i0 wolrd2_dproperty_editor_api = {
        .cdb_type = world_cdb_type,
        .draw_ui = _wolrd3d_property_editor,
};

// Velocity
static uint64_t velocity3d_cdb_type() {
    return ce_id_a0->id64("velocity3d");
}

static const char *velocity3d_display_name() {
    return "Velocity3D";
}

static const ce_cdb_prop_def_t0 body3d_component_prop[] = {
        {
                .name = "linear",
                .type = CE_CDB_TYPE_SUBOBJECT,
                .obj_type = VEC3_CDB_TYPE,
                .flags = CE_CDB_PROP_FLAG_UNPACK,
        },
        {
                .name = "angular",
                .type = CE_CDB_TYPE_SUBOBJECT,
                .obj_type = VEC3_CDB_TYPE,
                .flags = CE_CDB_PROP_FLAG_UNPACK,
        },
};

static void _velocity_from_cdb(ct_world_t0 world,
                               ce_cdb_t0 db,
                               uint64_t obj,
                               void *data) {
    ct_velocity3d_c *c = (ct_velocity3d_c *) data;

    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);

    uint64_t linear = ce_cdb_a0->read_subobject(r, VELOCITY3D_LINEAR_PROP, 0);
    const ce_cdb_obj_o0 *linear_r = ce_cdb_a0->read(db, linear);
    c->linear = (ce_vec3_t) {
            .x=ce_cdb_a0->read_float(linear_r, PROP_VEC_X, 0.f),
            .y=ce_cdb_a0->read_float(linear_r, PROP_VEC_Y, 0.f),
            .z=ce_cdb_a0->read_float(linear_r, PROP_VEC_Z, 0.f),
    };

    uint64_t angular = ce_cdb_a0->read_subobject(r, VELOCITY3D_ANGULAR_PROP, 0);
    const ce_cdb_obj_o0 *angular_r = ce_cdb_a0->read(db, angular);
    c->angular = (ce_vec3_t) {
            .x=ce_cdb_a0->read_float(angular_r, PROP_VEC_X, 0.f),
            .y=ce_cdb_a0->read_float(angular_r, PROP_VEC_Y, 0.f),
            .z=ce_cdb_a0->read_float(angular_r, PROP_VEC_Z, 0.f),
    };
}

static struct ct_ecs_component_i0 velocity3d_component_i = {
        .display_name = velocity3d_display_name,
        .cdb_type = VELOCITY3D_COMPONENT,
        .size = sizeof(ct_velocity3d_c),
        .from_cdb_obj = _velocity_from_cdb,
};

static void _velocity_property_editor(ce_cdb_t0 db,
                                      uint64_t obj,
                                      uint64_t context,
                                      const char *filter) {
    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);

    uint64_t linear = ce_cdb_a0->read_subobject(r, VELOCITY3D_LINEAR_PROP, 0);
    ct_editor_ui_a0->prop_vec3(linear, "Linear", filter,
                               (uint64_t[]) {PROP_VEC_X, PROP_VEC_Y, PROP_VEC_Z},
                               (ui_vec3_p0) {});

    uint64_t angular = ce_cdb_a0->read_subobject(r, VELOCITY3D_ANGULAR_PROP, 0);
    ct_editor_ui_a0->prop_vec3(angular, "Angular", filter,
                               (uint64_t[]) {PROP_VEC_X, PROP_VEC_Y, PROP_VEC_Z},
                               (ui_vec3_p0) {});
}

static struct ct_property_editor_i0 property_editor_api = {
        .cdb_type = velocity3d_cdb_type,
        .draw_ui = _velocity_property_editor,
};

// collider
static uint64_t collider3d_cdb_type() {
    return ce_id_a0->id64("collider3d");
}

static const char *collider3d_display_name() {
    return "Collider3D";
}

static const ce_cdb_prop_def_t0 collider3d_component_prop[] = {
        {
                .name = "shape",
                .type = CE_CDB_TYPE_SUBOBJECT,
                .obj_type = COLLIDER3D_BOX,
        },
};

static const ce_cdb_prop_def_t0 collider3d_box_prop[] = {
        {
                .name = "half_size",
                .type = CE_CDB_TYPE_SUBOBJECT,
                .obj_type = VEC3_CDB_TYPE,
                .flags = CE_CDB_PROP_FLAG_UNPACK,
        },
};

static const ce_cdb_prop_def_t0 collider3d_sphere_prop[] = {
        {
                .name = "radius",
                .type = CE_CDB_TYPE_FLOAT,
        },
};

static void _collider3d_from_cdb(ct_world_t0 world,
                                 ce_cdb_t0 db,
                                 uint64_t obj,
                                 void *data) {
    ct_collider3d_c *c = (ct_collider3d_c *) data;

    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);
    uint64_t shape = ce_cdb_a0->read_subobject(r, COLLIDER3D_SHAPE_PROP, 0);
    uint64_t shape_type = ce_cdb_a0->obj_type(db, shape);

    c->type = shape_type;

    switch (shape_type) {
        case COLLIDER3D_BOX: {
            const ce_cdb_obj_o0 *shape_r = ce_cdb_a0->read(db, shape);
            uint64_t half_size = ce_cdb_a0->read_subobject(shape_r,
                                                           COLLIDER3D_BOX_HALF,
                                                           0);
            ce_cdb_a0->read_to(db, half_size,
                               &c->shape.box.half_size, sizeof(c->shape.box.half_size));
        }
            break;

        case COLLIDER3D_SPHERE: {
            ce_cdb_a0->read_to(db, shape,
                               &c->shape.sphere.radius, sizeof(c->shape.sphere.radius));
        }
            break;

    }
}

static struct ct_ecs_component_i0 collider3d_component_i = {
        .display_name = collider3d_display_name,
        .cdb_type = COLLIDER3D_COMPONENT,
        .size = sizeof(ct_collider3d_c),
        .from_cdb_obj = _collider3d_from_cdb,
};
///

// mass
static uint64_t mass3d_cdb_type() {
    return ce_id_a0->id64("mass3d");
}

static const char *mass3d_display_name() {
    return "Mass3D";
}

static const ce_cdb_prop_def_t0 mass3d_component_prop[] = {
        {
                .name = "mass",
                .type = CE_CDB_TYPE_FLOAT,
                .value.f = 1.0f,
        },
};

static void _mass3d_from_cdb(ct_world_t0 world,
                             ce_cdb_t0 db,
                             uint64_t obj,
                             void *data) {
    ct_mass3d_c *c = (ct_mass3d_c *) data;

    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);
    c->mass = ce_cdb_a0->read_float(r, MASS3D_PROP, 0);
}

static struct ct_ecs_component_i0 mass3d_component_i = {
        .display_name = mass3d_display_name,
        .cdb_type = MASS3D_COMPONENT,
        .size = sizeof(ct_mass3d_c),
        .from_cdb_obj = _mass3d_from_cdb,
};

static void _mass3d_property_editor(ce_cdb_t0 db,
                                    uint64_t obj,
                                    uint64_t context,
                                    const char *filter) {
    ct_editor_ui_a0->prop_float(obj, "Mass", filter, MASS3D_PROP, (ui_float_p0) {});
}

static struct ct_property_editor_i0 mass3d_property_editor_api = {
        .cdb_type = mass3d_cdb_type,
        .draw_ui = _mass3d_property_editor,
};
//

static const char *SHAPE_STR[] = {
        "Box",
        "Sphere",
};

static uint64_t SHAPE_ID[] = {
        COLLIDER3D_BOX,
        COLLIDER3D_SPHERE,
};

static uint32_t _shape_to_idx(uint64_t shape_type) {
    for (int i = 0; i < CE_ARRAY_LEN(SHAPE_ID); ++i) {
        if (SHAPE_ID[i] == shape_type) {
            return i;
        }
    }
    return UINT32_MAX;
}

static uint64_t _idx_to_shape(int id) {
    return SHAPE_ID[id];
}

static void _collider_property_editor(ce_cdb_t0 db,
                                      uint64_t obj,
                                      uint64_t context,
                                      const char *filter) {
    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);
    uint64_t shape = ce_cdb_a0->read_subobject(r, COLLIDER3D_SHAPE_PROP, 0);
    uint64_t shape_type = ce_cdb_a0->obj_type(db, shape);

    int cur_item = _shape_to_idx(shape_type);
    if (ct_debugui_a0->Combo("Type", &cur_item, SHAPE_STR, CE_ARRAY_LEN(SHAPE_STR), -1)) {
        shape_type = _idx_to_shape(cur_item);
        uint64_t new_shape = ce_cdb_a0->create_object(db, shape_type);

        ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(db, obj);
        ce_cdb_a0->set_subobject(w, COLLIDER3D_SHAPE_PROP, new_shape);
        ce_cdb_a0->write_commit(w);
    }

    switch (shape_type) {
        case COLLIDER3D_BOX: {
            const ce_cdb_obj_o0 *shape_r = ce_cdb_a0->read(db, shape);
            uint64_t half_size = ce_cdb_a0->read_subobject(shape_r,
                                                           COLLIDER3D_BOX_HALF,
                                                           0);

            ct_editor_ui_a0->prop_vec3(half_size, "Half size", filter,
                                       (uint64_t[]) {PROP_VEC_X, PROP_VEC_Y, PROP_VEC_Z},
                                       (ui_vec3_p0) {});
        }
            break;

        case COLLIDER3D_SPHERE: {
            ct_editor_ui_a0->prop_float(shape, "Radius", filter,
                                        COLLIDER3D_SPHERE_RADIUS,
                                        (ui_float_p0) {});
        }
            break;
    }
}

static struct ct_property_editor_i0 _collider_property_editor_api = {
        .cdb_type = collider3d_cdb_type,
        .draw_ui = _collider_property_editor,
};
//

static struct ct_system_group_i0 physics_group = {
        .name = PHYSICS3D_GROUP,
        .group = CT_ECS_SIMULATION_GROUP,
        .before = CT_ECS_BEFORE(TRANSFORM_SYSTEM),
};

static struct ct_physics3d_a0 api = {
};

struct ct_physics3d_a0 *ct_physics3d_a0 = &api;

void CE_MODULE_LOAD(physics3d)(struct ce_api_a0 *api,
                               int reload) {
    ce_id_a0->id64("collider3d");
    ce_id_a0->id64("collider3d_box");
    ce_id_a0->id64("mass3d");
    ce_id_a0->id64("physics_world3d");

    api->add_api(CT_PHYSICS3D_A0_STR, ct_physics3d_a0, sizeof(struct ct_physics3d_a0));

    api->add_impl(CT_ECS_SYSTEM_GROUP_I0_STR,
                  &physics_group, sizeof(physics_group));

    api->add_impl(CT_ECS_COMPONENT_I0_STR,
                  &world3d_component_i, sizeof(world3d_component_i));

    api->add_impl(CT_ECS_COMPONENT_I0_STR,
                  &velocity3d_component_i, sizeof(velocity3d_component_i));

    api->add_impl(CT_ECS_COMPONENT_I0_STR,
                  &collider3d_component_i, sizeof(collider3d_component_i));

    api->add_impl(CT_ECS_COMPONENT_I0_STR,
                  &mass3d_component_i, sizeof(mass3d_component_i));

    api->add_impl(CT_PROPERTY_EDITOR_I0_STR,
                  &wolrd2_dproperty_editor_api, sizeof(wolrd2_dproperty_editor_api));

    api->add_impl(CT_PROPERTY_EDITOR_I0_STR,
                  &property_editor_api, sizeof(property_editor_api));

    api->add_impl(CT_PROPERTY_EDITOR_I0_STR,
                  &_collider_property_editor_api, sizeof(_collider_property_editor_api));

    api->add_impl(CT_PROPERTY_EDITOR_I0_STR,
                  &mass3d_property_editor_api, sizeof(mass3d_property_editor_api));

    ce_cdb_a0->reg_obj_type(PHYSICS3D_WORLD_COMPONENT,
                            wolrd3d_component_prop, CE_ARRAY_LEN(wolrd3d_component_prop));

    ce_cdb_a0->reg_obj_type(VELOCITY3D_COMPONENT,
                            body3d_component_prop, CE_ARRAY_LEN(body3d_component_prop));

    ce_cdb_a0->reg_obj_type(MASS3D_COMPONENT,
                            mass3d_component_prop, CE_ARRAY_LEN(mass3d_component_prop));

    ce_cdb_a0->reg_obj_type(COLLIDER3D_COMPONENT,
                            collider3d_component_prop,
                            CE_ARRAY_LEN(collider3d_component_prop));

    ce_cdb_a0->reg_obj_type(COLLIDER3D_BOX,
                            collider3d_box_prop,
                            CE_ARRAY_LEN(collider3d_box_prop));

    ce_cdb_a0->reg_obj_type(COLLIDER3D_SPHERE,
                            collider3d_sphere_prop,
                            CE_ARRAY_LEN(collider3d_sphere_prop));
}

void CE_MODULE_UNLOAD(physics3d)(struct ce_api_a0 *api,
                                 int reload) {
}
