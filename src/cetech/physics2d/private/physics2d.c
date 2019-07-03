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
#include <cetech/physics2d/physics2d.h>
#include <cetech/transform/transform.h>

#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>

// World
static uint64_t world_cdb_type() {
    return ce_id_a0->id64("physics_world2d");
}

static const char *wolrd2d_display_name() {
    return "PhysicsWorld2D";
}

static const ce_cdb_prop_def_t0 wolrd2d_component_prop[] = {
        {
                .name = "gravity",
                .type = CE_CDB_TYPE_SUBOBJECT,
                .obj_type = VEC2_CDB_TYPE,
                .flags = CE_CDB_PROP_FLAG_UNPACK,
        },
};

static void _world2d_from_cdb(ct_world_t0 world,
                              ce_cdb_t0 db,
                              uint64_t obj,
                              void *data) {
    ct_physics_world2d_c *c = (ct_physics_world2d_c *) data;

    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);

    uint64_t gravity = ce_cdb_a0->read_subobject(r, PHYSICS_WORLD2D_GRAVITY_PROP, 0);

    const ce_cdb_obj_o0 *gravity_r = ce_cdb_a0->read(db, gravity);

    c->gravity = (ce_vec2_t) {
            .x=ce_cdb_a0->read_float(gravity_r, PROP_VEC_X, 0.f),
            .y=ce_cdb_a0->read_float(gravity_r, PROP_VEC_Y, 0.f),
    };

}

static struct ct_ecs_component_i0 world2d_component_i = {
        .display_name = wolrd2d_display_name,
        .cdb_type = PHYSICS_WORLD2D_COMPONENT,
        .size = sizeof(ct_physics_world2d_c),
        .from_cdb_obj = _world2d_from_cdb,
};

static void _wolrd2d_property_editor(ce_cdb_t0 db,
                                     uint64_t obj,
                                     uint64_t context,
                                     const char *filter) {
    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);
    uint64_t gravity = ce_cdb_a0->read_subobject(r, PHYSICS_WORLD2D_GRAVITY_PROP, 0);

    ct_editor_ui_a0->prop_vec2(gravity, "Gravity", filter,
                               (uint64_t[]) {PROP_VEC_X, PROP_VEC_Y},
                               (ui_vec2_p0) {});
}

static struct ct_property_editor_i0 wolrd2_dproperty_editor_api = {
        .cdb_type = world_cdb_type,
        .draw_ui = _wolrd2d_property_editor,
};

// Velocity
static uint64_t velocity2d_cdb_type() {
    return ce_id_a0->id64("velocity2d");
}

static const char *velocity2d_display_name() {
    return "Velocity2D";
}

static const ce_cdb_prop_def_t0 body2d_component_prop[] = {
        {
                .name = "linear",
                .type = CE_CDB_TYPE_SUBOBJECT,
                .obj_type = VEC2_CDB_TYPE,
                .flags = CE_CDB_PROP_FLAG_UNPACK,
        },
        {
                .name = "angular",
                .type = CE_CDB_TYPE_FLOAT,
        },
};

static void _velocity_from_cdb(ct_world_t0 world,
                               ce_cdb_t0 db,
                               uint64_t obj,
                               void *data) {
    ct_velocity2d_c *c = (ct_velocity2d_c *) data;

    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);

    uint64_t linear = ce_cdb_a0->read_subobject(r, VELOCITY2D_LINEAR_PROP, 0);

    const ce_cdb_obj_o0 *linear_r = ce_cdb_a0->read(db, linear);

    c->linear = (ce_vec2_t) {
            .x=ce_cdb_a0->read_float(linear_r, PROP_VEC_X, 0.f),
            .y=ce_cdb_a0->read_float(linear_r, PROP_VEC_Y, 0.f),
    };

    c->angular = ce_cdb_a0->read_float(r, VELOCITY2D_ANGULAR_PROP, 0.0f);
}

static struct ct_ecs_component_i0 velocity2d_component_i = {
        .display_name = velocity2d_display_name,
        .cdb_type = VELOCITY2D_COMPONENT,
        .size = sizeof(ct_velocity2d_c),
        .from_cdb_obj = _velocity_from_cdb,
};

static void _velocity_property_editor(ce_cdb_t0 db,
                                      uint64_t obj,
                                      uint64_t context,
                                      const char *filter) {
    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);
    uint64_t linear = ce_cdb_a0->read_subobject(r, VELOCITY2D_LINEAR_PROP, 0);

    ct_editor_ui_a0->prop_vec2(linear, "Linear", filter,
                               (uint64_t[]) {PROP_VEC_X, PROP_VEC_Y},
                               (ui_vec2_p0) {});

    ct_editor_ui_a0->prop_float(obj, "Angular", filter, VELOCITY2D_ANGULAR_PROP, (ui_float_p0) {});
}

static struct ct_property_editor_i0 property_editor_api = {
        .cdb_type = velocity2d_cdb_type,
        .draw_ui = _velocity_property_editor,
};

// collider
static uint64_t collider2d_cdb_type() {
    return ce_id_a0->id64("collider2d");
}

static const char *collider2d_display_name() {
    return "Collider2D";
}

static const ce_cdb_prop_def_t0 collider2d_component_prop[] = {
        {
                .name = "shape",
                .type = CE_CDB_TYPE_SUBOBJECT,
                .obj_type = COLLIDER2D_RECTANGLE,
        },
};

static const ce_cdb_prop_def_t0 collider2d_rectangle_prop[] = {
        {
                .name = "half_size",
                .type = CE_CDB_TYPE_SUBOBJECT,
                .obj_type = VEC2_CDB_TYPE,
                .flags = CE_CDB_PROP_FLAG_UNPACK,
        },
};

static void _collider2d_from_cdb(ct_world_t0 world,
                                 ce_cdb_t0 db,
                                 uint64_t obj,
                                 void *data) {
    ct_collider2d_c *c = (ct_collider2d_c *) data;

    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);
    uint64_t shape = ce_cdb_a0->read_subobject(r, PHYSICS_COLLIDER2D_SHAPE_PROP, 0);
    uint64_t shape_type = ce_cdb_a0->obj_type(db, shape);

    c->type = shape_type;

    switch (shape_type) {
        case COLLIDER2D_RECTANGLE: {
            const ce_cdb_obj_o0 *shape_r = ce_cdb_a0->read(db, shape);
            uint64_t half_size = ce_cdb_a0->read_subobject(shape_r,
                                                           PHYSICS_COLLIDER2D_RECTANGLE_HALF_SIZE,
                                                           0);
            ce_cdb_a0->read_to(db, half_size,
                               &c->rectangle.half_size, sizeof(c->rectangle.half_size));
        }
            break;
    }
}

static struct ct_ecs_component_i0 collider2d_component_i = {
        .display_name = collider2d_display_name,
        .cdb_type = COLLIDER2D_COMPONENT,
        .size = sizeof(ct_collider2d_c),
        .from_cdb_obj = _collider2d_from_cdb,
};
///

// mass
static uint64_t mass2d_cdb_type() {
    return ce_id_a0->id64("mass2d");
}

static const char *mass2d_display_name() {
    return "Mass2D";
}

static const ce_cdb_prop_def_t0 mass2d_component_prop[] = {
        {
                .name = "mass",
                .type = CE_CDB_TYPE_FLOAT,
                .value.f = 1.0f,
        },
};

static void _mass2d_from_cdb(ct_world_t0 world,
                             ce_cdb_t0 db,
                             uint64_t obj,
                             void *data) {
    ct_mass2d_c *c = (ct_mass2d_c *) data;

    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);
    c->mass = ce_cdb_a0->read_float(r, MASS2D_PROP, 0);
}

static struct ct_ecs_component_i0 mass2d_component_i = {
        .display_name = mass2d_display_name,
        .cdb_type = MASS2D_COMPONENT,
        .size = sizeof(ct_mass2d_c),
        .from_cdb_obj = _mass2d_from_cdb,
};

static void _mass2d_property_editor(ce_cdb_t0 db,
                                    uint64_t obj,
                                    uint64_t context,
                                    const char *filter) {
    ct_editor_ui_a0->prop_float(obj, "Mass", filter, MASS2D_PROP, (ui_float_p0) {});
}

static struct ct_property_editor_i0 mass2d_property_editor_api = {
        .cdb_type = mass2d_cdb_type,
        .draw_ui = _mass2d_property_editor,
};
//


static int _shape_to_id(uint64_t shape_type) {
    switch (shape_type) {
        case COLLIDER2D_RECTANGLE: {
            return 0;
        }
            break;
    }

    return 0;
}

static uint64_t _idx_to_shape(int id) {
    uint64_t shape[] = {
            COLLIDER2D_RECTANGLE,
    };
    return shape[id];
}

static void _collider_property_editor(ce_cdb_t0 db,
                                      uint64_t obj,
                                      uint64_t context,
                                      const char *filter) {
    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);
    uint64_t shape = ce_cdb_a0->read_subobject(r, PHYSICS_COLLIDER2D_SHAPE_PROP, 0);
    uint64_t shape_type = ce_cdb_a0->obj_type(db, shape);

    const char *shape_str[] = {
            "Rectangle",
    };

    int cur_item = _shape_to_id(shape_type);
    if (ct_debugui_a0->Combo("Type", &cur_item, shape_str, CE_ARRAY_LEN(shape_str), -1)) {
        shape_type = _idx_to_shape(cur_item);
        uint64_t new_shape = ce_cdb_a0->create_object(db, shape_type);

        ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(db, obj);
        ce_cdb_a0->set_subobject(w, PHYSICS_COLLIDER2D_SHAPE_PROP, new_shape);
        ce_cdb_a0->write_commit(w);
    }

    switch (shape_type) {
        case COLLIDER2D_RECTANGLE: {
            const ce_cdb_obj_o0 *shape_r = ce_cdb_a0->read(db, shape);
            uint64_t half_size = ce_cdb_a0->read_subobject(shape_r,
                                                           PHYSICS_COLLIDER2D_RECTANGLE_HALF_SIZE,
                                                           0);

            ct_editor_ui_a0->prop_vec2(half_size, "Half size", filter,
                                       (uint64_t[]) {PROP_VEC_X, PROP_VEC_Y}, (ui_vec2_p0) {});
        }
            break;
    }
}

static struct ct_property_editor_i0 _collider_property_editor_api = {
        .cdb_type = collider2d_cdb_type,
        .draw_ui = _collider_property_editor,
};
//

static struct ct_system_group_i0 physics_group = {
        .name = PHYSICS2D_GROUP,
        .group = CT_ECS_SIMULATION_GROUP,
        .before = CT_ECS_BEFORE(TRANSFORM_SYSTEM),
};

static struct ct_physics2d_a0 api = {
};

struct ct_physics2d_a0 *ct_physics2d_a0 = &api;

void CE_MODULE_LOAD(physics2d)(struct ce_api_a0 *api,
                               int reload) {
    api->add_api(CT_PHYSICS_A0, ct_physics2d_a0, sizeof(struct ct_physics2d_a0));

    ce_id_a0->id64("collider2d");
    ce_id_a0->id64("collider2d_shape_rectangle");
    ce_id_a0->id64("mass2d");
    ce_id_a0->id64("physics_world2d");

    api->add_impl(CT_ECS_SYSTEM_GROUP_I,
                  &physics_group, sizeof(physics_group));

    api->add_impl(CT_ECS_COMPONENT_I,
                  &world2d_component_i, sizeof(world2d_component_i));

    api->add_impl(CT_ECS_COMPONENT_I,
                  &velocity2d_component_i, sizeof(velocity2d_component_i));

    api->add_impl(CT_ECS_COMPONENT_I,
                  &collider2d_component_i, sizeof(collider2d_component_i));

    api->add_impl(CT_ECS_COMPONENT_I,
                  &mass2d_component_i, sizeof(mass2d_component_i));

    api->add_impl(CT_PROPERTY_EDITOR_I,
                  &wolrd2_dproperty_editor_api, sizeof(wolrd2_dproperty_editor_api));

    api->add_impl(CT_PROPERTY_EDITOR_I,
                  &property_editor_api, sizeof(property_editor_api));

    api->add_impl(CT_PROPERTY_EDITOR_I,
                  &_collider_property_editor_api, sizeof(_collider_property_editor_api));

    api->add_impl(CT_PROPERTY_EDITOR_I,
                  &mass2d_property_editor_api, sizeof(mass2d_property_editor_api));

    ce_cdb_a0->reg_obj_type(PHYSICS_WORLD2D_COMPONENT,
                            wolrd2d_component_prop, CE_ARRAY_LEN(wolrd2d_component_prop));

    ce_cdb_a0->reg_obj_type(VELOCITY2D_COMPONENT,
                            body2d_component_prop, CE_ARRAY_LEN(body2d_component_prop));

    ce_cdb_a0->reg_obj_type(MASS2D_COMPONENT,
                            mass2d_component_prop, CE_ARRAY_LEN(mass2d_component_prop));

    ce_cdb_a0->reg_obj_type(COLLIDER2D_COMPONENT,
                            collider2d_component_prop,
                            CE_ARRAY_LEN(collider2d_component_prop));

    ce_cdb_a0->reg_obj_type(COLLIDER2D_RECTANGLE,
                            collider2d_rectangle_prop,
                            CE_ARRAY_LEN(collider2d_rectangle_prop));
}

void CE_MODULE_UNLOAD(physics2d)(struct ce_api_a0 *api,
                                 int reload) {
}
