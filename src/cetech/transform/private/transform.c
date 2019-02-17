#include <celib/memory/allocator.h>
#include "celib/config.h"
#include "cetech/resource/resource.h"
#include "celib/id.h"
#include "celib/memory/memory.h"
#include "celib/api.h"
#include <celib/ydb.h>
#include <celib/ydb.h>
#include <celib/macros.h>
#include <celib/containers/array.h>
#include <celib/math/math.h>

#include <celib/log.h>
#include <celib/cdb.h>

#include "celib/module.h"
#include "cetech/ecs/ecs.h"
#include <cetech/transform/transform.h>
#include <cetech/debugui/icons_font_awesome.h>

#include <cetech/editor/editor_ui.h>
#include <cetech/editor/property.h>
#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>

#define LOG_WHERE "transform"

static void transform_transform(ct_transform_comp *transform,
                                float *parent) {
    ce_vec3_t rot = ce_quat_to_euler(transform->t.rot);

    float obj[16] = {};

    ce_mat4_srt(obj,
                transform->t.scl.x, transform->t.scl.y, transform->t.scl.z,
                rot.x, rot.y, rot.z,
                transform->t.pos.x, transform->t.pos.y, transform->t.pos.z);


    if (parent) {
        ce_mat4_mul(transform->world, obj, parent);
    } else {
        ce_mat4_move(transform->world, obj);
    }
}

static uint64_t cdb_type() {
    return TRANSFORM_COMPONENT;
}

static const char *display_name() {
    return ICON_FA_ARROWS " Transform";
}

static uint64_t _position_cdb_type() {
    return PROP_POSITION;
}

static uint64_t _rotation_cdb_type() {
    return PROP_ROTATION;
}

static uint64_t _scale_cdb_type() {
    return PROP_SCALE;
}

static void _position_property_editor(uint64_t obj,
                                      uint64_t context) {

    ct_editor_ui_a0->prop_vec3(obj,
                               (uint64_t[3]) {PROP_POSITION_X,
                                              PROP_POSITION_Y,
                                              PROP_POSITION_Z},
                               "Position",
                               (ui_vec3_p0) {});

}

static void _rotation_property_editor(uint64_t obj,
                                      uint64_t context) {

    ct_editor_ui_a0->prop_vec3(obj,
                               (uint64_t[3]) {PROP_ROTATION_X,
                                              PROP_ROTATION_Y,
                                              PROP_ROTATION_Z},
                               "Rotation",
                               (ui_vec3_p0) {});

}

static void _scale_property_editor(uint64_t obj,
                                   uint64_t context) {

    ct_editor_ui_a0->prop_vec3(obj,
                               (uint64_t[3]) {PROP_SCALE_X,
                                              PROP_SCALE_Y,
                                              PROP_SCALE_Z},
                               "Scale",
                               (ui_vec3_p0) {});
}


static struct ct_property_editor_i0 position_property_editor_api = {
        .cdb_type = _position_cdb_type,
        .draw_ui = _position_property_editor,
};

static struct ct_property_editor_i0 rotation_property_editor_api = {
        .cdb_type = _rotation_cdb_type,
        .draw_ui = _rotation_property_editor,
};

static struct ct_property_editor_i0 scale_property_editor_api = {
        .cdb_type = _scale_cdb_type,
        .draw_ui = _scale_property_editor,
};

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
    return sizeof(ct_transform_comp);
}

static void transform_spawner(ct_world_t0 world,
                              uint64_t obj,
                              void *data) {
    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    ct_transform_comp *t = data;

    ce_vec3_t pos = {};
    ce_vec3_t rot = {};
    ce_vec3_t scl = CE_VEC3_UNIT;

    uint64_t pos_obj = ce_cdb_a0->read_subobject(r, PROP_POSITION, 0);
    if (pos_obj) {
        const ce_cdb_obj_o0 *pos_r = ce_cdb_a0->read(ce_cdb_a0->db(), pos_obj);

        pos = (ce_vec3_t) {
                ce_cdb_a0->read_float(pos_r, PROP_POSITION_X, 0.0f),
                ce_cdb_a0->read_float(pos_r, PROP_POSITION_Y, 0.0f),
                ce_cdb_a0->read_float(pos_r, PROP_POSITION_Z, 0.0f),
        };
    }

    uint64_t rot_obj = ce_cdb_a0->read_subobject(r, PROP_ROTATION, 0);
    if (rot_obj) {
        const ce_cdb_obj_o0 *rot_r = ce_cdb_a0->read(ce_cdb_a0->db(), rot_obj);

        rot = (ce_vec3_t) {
                ce_cdb_a0->read_float(rot_r, PROP_ROTATION_X, 0.0f),
                ce_cdb_a0->read_float(rot_r, PROP_ROTATION_Y, 0.0f),
                ce_cdb_a0->read_float(rot_r, PROP_ROTATION_Z, 0.0f),
        };

        rot = ce_vec3_mul_s(rot, CE_DEG_TO_RAD);
    }

    uint64_t scl_obj = ce_cdb_a0->read_subobject(r, PROP_SCALE, 0);
    if (scl_obj) {
        const ce_cdb_obj_o0 *scl_r = ce_cdb_a0->read(ce_cdb_a0->db(), scl_obj);

        scl = (ce_vec3_t) {
                ce_cdb_a0->read_float(scl_r, PROP_SCALE_X, 1.0f),
                ce_cdb_a0->read_float(scl_r, PROP_SCALE_Y, 1.0f),
                ce_cdb_a0->read_float(scl_r, PROP_SCALE_Z, 1.0f),
        };
    }

    *t = (ct_transform_comp) {
            .t.pos = pos,
            .t.rot = ce_quat_from_euler(rot.x, rot.y, rot.z),
            .t.scl = scl,
    };
}

static struct ct_component_i0 ct_component_api = {
        .cdb_type = cdb_type,
        .size = size,
        .get_interface = get_interface,
        .spawner = transform_spawner,
};


static void _transform_root_naive(ct_world_t0 world,
                                  struct ct_entity_t0 ent,
                                  float *w) {
    ct_transform_comp *root_t = ct_ecs_a0->get_one(world,
                                                   TRANSFORM_COMPONENT,
                                                   ent);

    float *rootw = w;
    if (root_t) {
        rootw = root_t->world;
        transform_transform(root_t, w);
    }

    ct_entity_t0 ent_it = ct_ecs_a0->first_child(world, ent);
    while (ent_it.h) {
        _transform_root_naive(world, ent_it, rootw);
        ent_it = ct_ecs_a0->next_sibling(world, ent_it);
    }
}

static void foreach_transform(ct_world_t0 world,
                              struct ct_entity_t0 *ents,
                              ct_entity_storage_t *item,
                              uint32_t n,
                              void *data) {
    ct_entity_t0 *roots = NULL;
    for (uint32_t i = 0; i < n; ++i) {
        struct ct_entity_t0 ent = ents[i];

        struct ct_entity_t0 parent_ent = ct_ecs_a0->parent(world, ent);

        if (!parent_ent.h) {
            ce_array_push(roots, ent, ce_memory_a0->system);
        }

        if (!ct_ecs_a0->has(world, parent_ent,
                            (uint64_t[]) {TRANSFORM_COMPONENT}, 1)) {
            ce_array_push(roots, ent, ce_memory_a0->system);
        }
    }

    uint32_t roots_n = ce_array_size(roots);
    for (int i = 0; i < roots_n; ++i) {
        struct ct_entity_t0 ent = roots[i];

        _transform_root_naive(world, ent, NULL);
    }


    ce_array_free(roots, ce_memory_a0->system);
}

static void transform_system(ct_world_t0 world,
                             float dt) {
    uint64_t mask = ct_ecs_a0->mask(TRANSFORM_COMPONENT);

    ct_ecs_a0->process(world, mask, foreach_transform, &dt);
}


static uint64_t name() {
    return TRANSFORM_SYSTEM;
}


static struct ct_simulation_i0 transform_simulation_i0 = {
        .simulation = transform_system,
        .name = name,
};

static const ce_cdb_prop_def_t0 transform_component_prop[] = {
        {.name = "position", .type = CDB_TYPE_SUBOBJECT, .obj_type = PROP_POSITION},
        {.name = "rotation", .type = CDB_TYPE_SUBOBJECT, .obj_type = PROP_ROTATION},
        {.name = "scale", .type = CDB_TYPE_SUBOBJECT, .obj_type = PROP_SCALE},
};

static const ce_cdb_prop_def_t0 position_prop[] = {
        {.name = "x", .type = CDB_TYPE_FLOAT, .obj_type = PROP_POSITION_X},
        {.name = "y", .type = CDB_TYPE_FLOAT, .obj_type = PROP_POSITION_Y},
        {.name = "z", .type = CDB_TYPE_FLOAT, .obj_type = PROP_POSITION_Z},
};

static const ce_cdb_prop_def_t0 rotation_prop[] = {
        {.name = "x", .type = CDB_TYPE_FLOAT, .obj_type = PROP_ROTATION_X},
        {.name = "y", .type = CDB_TYPE_FLOAT, .obj_type = PROP_ROTATION_Y},
        {.name = "z", .type = CDB_TYPE_FLOAT, .obj_type = PROP_ROTATION_Z},
};

static const ce_cdb_prop_def_t0 scale_prop[] = {
        {.name = "x", .type = CDB_TYPE_FLOAT, .obj_type = PROP_SCALE_X, .value.f = 1.0f},
        {.name = "y", .type = CDB_TYPE_FLOAT, .obj_type = PROP_SCALE_Y, .value.f = 1.0f},
        {.name = "z", .type = CDB_TYPE_FLOAT, .obj_type = PROP_SCALE_Z, .value.f = 1.0f},
};

void CE_MODULE_LOAD(transform)(struct ce_api_a0 *api,
                               int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_ydb_a0);
    CE_INIT_API(api, ce_ydb_a0);
    CE_INIT_API(api, ce_cdb_a0);
    CE_INIT_API(api, ct_ecs_a0);
    CE_INIT_API(api, ce_log_a0);

    api->register_api(COMPONENT_INTERFACE,
                      &ct_component_api, sizeof(ct_component_api));

    api->register_api(SIMULATION_INTERFACE,
                      &transform_simulation_i0, sizeof(transform_simulation_i0));

    api->register_api(PROPERTY_EDITOR_INTERFACE,
                      &position_property_editor_api, sizeof(position_property_editor_api));

    api->register_api(PROPERTY_EDITOR_INTERFACE,
                      &rotation_property_editor_api, sizeof(rotation_property_editor_api));

    api->register_api(PROPERTY_EDITOR_INTERFACE,
                      &scale_property_editor_api, sizeof(scale_property_editor_api));

    ce_cdb_a0->reg_obj_type(PROP_POSITION,
                            position_prop, CE_ARRAY_LEN(position_prop));

    ce_cdb_a0->reg_obj_type(PROP_ROTATION,
                            rotation_prop, CE_ARRAY_LEN(rotation_prop));

    ce_cdb_a0->reg_obj_type(PROP_SCALE,
                            scale_prop, CE_ARRAY_LEN(scale_prop));

    ce_cdb_a0->reg_obj_type(TRANSFORM_COMPONENT,
                            transform_component_prop, CE_ARRAY_LEN(transform_component_prop));


}

void CE_MODULE_UNLOAD(transform)(struct ce_api_a0 *api,
                                 int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}
