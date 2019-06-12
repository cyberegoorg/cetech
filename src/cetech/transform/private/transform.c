#include <sys/mman.h>

#include <celib/memory/allocator.h>
#include "celib/config.h"
#include "cetech/resource/resource.h"
#include "celib/id.h"
#include "celib/memory/memory.h"
#include "celib/api.h"
#include <celib/ydb.h>
#include <celib/macros.h>
#include <celib/containers/array.h>
#include <celib/math/math.h>

#include <celib/log.h>
#include <celib/cdb.h>
#include "celib/module.h"
#include <celib/containers/hash.h>
#include <celib/containers/spsc.h>

#include "cetech/ecs/ecs.h"
#include <cetech/transform/transform.h>
#include <cetech/debugui/icons_font_awesome.h>

#include <cetech/editor/editor_ui.h>
#include <cetech/property_editor/property_editor.h>
#include <cetech/renderer/gfx.h>
#include <cetech/debugui/debugui.h>
#include <cetech/ecs/ecs.h>

#include <cetech/renderer/renderer.h>
#include <cetech/kernel/kernel.h>

#include <cetech/parent/parent.h>

#define LOG_WHERE "transform"

static struct transform_global {
    ce_alloc_t0 *alloc;
} _G = {};

// local to world
static const char *local_to_world_display_name() {
    return ICON_FA_ARROWS " Local to world";
}

static void _local_to_world_on_spawn(ct_world_t0 world,
                                     ce_cdb_t0 db,
                                     uint64_t obj,
                                     void *data) {
    ct_local_to_world_c *t = data;
    t->world = CE_MAT4_IDENTITY;
}

static struct ct_ecs_component_i0 transform_c_api = {
        .display_name = local_to_world_display_name,
        .cdb_type = LOCAL_TO_WORLD_COMPONENT,
        .size = sizeof(ct_local_to_world_c),
        .from_cdb_obj = _local_to_world_on_spawn,
};

// local to parent
static const char *local_to_parent_display_name() {
    return ICON_FA_ARROWS " Local to parent";
}

static void _local_to_parent_on_spawn(ct_world_t0 world,
                                      ce_cdb_t0 db,
                                      uint64_t obj,
                                      void *data) {
    ct_local_to_parent_c *t = data;
    t->local = CE_MAT4_IDENTITY;
}

static struct ct_ecs_component_i0 local_to_parent_c_api = {
        .display_name = local_to_parent_display_name,
        .cdb_type = LOCAL_TO_PARENT_COMPONENT,
        .size = sizeof(ct_local_to_parent_c),
        .from_cdb_obj = _local_to_parent_on_spawn,
};


// POS COMP
static const char *position_display_name() {
    return ICON_FA_ARROWS " Position";
}

static void _position_on_spawn(ct_world_t0 world,
                               ce_cdb_t0 db,
                               uint64_t obj,
                               void *data) {
    ct_position_c *c = data;
    ce_cdb_a0->read_to(ce_cdb_a0->db(), obj, c, sizeof(ct_position_c));
}

static struct ct_ecs_component_i0 position_c_api = {
        .cdb_type = POSITION_COMPONENT,
        .display_name = position_display_name,
        .size = sizeof(ct_position_c),
        .from_cdb_obj = _position_on_spawn,
};

static const ce_cdb_prop_def_t0 position_c_prop[] = {
        {
                .name = "position",
                .type = CE_CDB_TYPE_SUBOBJECT,
                .obj_type = VEC3_CDB_TYPE,
                .flags = CE_CDB_PROP_FLAG_UNPACK,
        },
};


static uint64_t _position_cdb_type() {
    return POSITION_COMPONENT;
}

static void _position_property_editor(uint64_t obj,
                                      uint64_t context,
                                      const char *filter) {
    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    uint64_t pos = ce_cdb_a0->read_subobject(r, PROP_POSITION, 0);

    ct_editor_ui_a0->prop_vec3(pos, "Position", filter,
                               (uint64_t[]) {PROP_VEC_X, PROP_VEC_Y, PROP_VEC_Z},
                               (ui_vec3_p0) {});
}

static struct ct_property_editor_i0 position_property_editor_api = {
        .cdb_type = _position_cdb_type,
        .draw_ui = _position_property_editor,
};


//
// Rot COMP
static const char *rotation_display_name() {
    return ICON_FA_ARROWS " Rotation";
}

static void _rotation_on_spawn(ct_world_t0 world,
                               ce_cdb_t0 db,
                               uint64_t obj,
                               void *data) {
    ct_rotation_c *c = data;
    ce_cdb_a0->read_to(ce_cdb_a0->db(), obj, c, sizeof(ct_rotation_c));
}

static struct ct_ecs_component_i0 rotation_c_api = {
        .cdb_type = ROTATION_COMPONENT,
        .display_name = rotation_display_name,
        .size = sizeof(ct_rotation_c),
        .from_cdb_obj = _rotation_on_spawn,
};

static const ce_cdb_prop_def_t0 rotation_c_prop[] = {
        {
                .name = "rotation",
                .type = CE_CDB_TYPE_SUBOBJECT,
                .obj_type = VEC3_CDB_TYPE,
                .flags = CE_CDB_PROP_FLAG_UNPACK,
        },
};

static uint64_t _rotation_cdb_type() {
    return ROTATION_COMPONENT;
}

static void _rotation_property_editor(uint64_t obj,
                                      uint64_t context,
                                      const char *filter) {
    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    uint64_t pos = ce_cdb_a0->read_subobject(r, PROP_ROTATION, 0);

    ct_editor_ui_a0->prop_vec3(pos, "Rotation", filter,
                               (uint64_t[]) {PROP_VEC_X, PROP_VEC_Y, PROP_VEC_Z},
                               (ui_vec3_p0) {});
}

static struct ct_property_editor_i0 rotation_property_editor_api = {
        .cdb_type = _rotation_cdb_type,
        .draw_ui = _rotation_property_editor,
};

//
// SCALE COMP
static const char *scale_display_name() {
    return ICON_FA_ARROWS " Scale";
}

static void _scale_on_spawn(ct_world_t0 world,
                            ce_cdb_t0 db,
                            uint64_t obj,
                            void *data) {
    ct_scale_c *c = data;
    ce_cdb_a0->read_to(ce_cdb_a0->db(), obj, c, sizeof(ct_scale_c));
}

static struct ct_ecs_component_i0 scale_c_api = {
        .cdb_type = SCALE_COMPONENT,
        .display_name = scale_display_name,
        .size = sizeof(ct_scale_c),
        .from_cdb_obj = _scale_on_spawn,
};

static const ce_cdb_prop_def_t0 scale_c_prop[] = {
        {
                .name = "scale",
                .type = CE_CDB_TYPE_SUBOBJECT,
                .obj_type = VEC3_CDB_TYPE,
                .flags = CE_CDB_PROP_FLAG_UNPACK,
        },
};


static uint64_t _scale_cdb_type() {
    return SCALE_COMPONENT;
}

static void _scale_property_editor(uint64_t obj,
                                   uint64_t context,
                                   const char *filter) {
    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    uint64_t pos = ce_cdb_a0->read_subobject(r, PROP_SCALE, 0);

    ct_editor_ui_a0->prop_vec3(pos, "Scale", filter,
                               (uint64_t[]) {PROP_VEC_X, PROP_VEC_Y, PROP_VEC_Z},
                               (ui_vec3_p0) {});
}

static struct ct_property_editor_i0 scale_property_editor_api = {
        .cdb_type = _scale_cdb_type,
        .draw_ui = _scale_property_editor,
};

//

//
void _trs_to_local_to_parent(ct_world_t0 world,
                             struct ct_entity_t0 *entities,
                             ct_ecs_ent_chunk_o0 *item,
                             uint32_t n,
                             void *_data) {

    ct_position_c *position = ct_ecs_c_a0->get_all(world, POSITION_COMPONENT, item);
    ct_rotation_c *rotaiton = ct_ecs_c_a0->get_all(world, ROTATION_COMPONENT, item);
    ct_scale_c *scale = ct_ecs_c_a0->get_all(world, SCALE_COMPONENT, item);

    ct_local_to_parent_c *transforms = ct_ecs_c_a0->get_all(world, LOCAL_TO_PARENT_COMPONENT, item);

    for (int i = 0; i < n; ++i) {
        ce_vec3_t pos = position ? position[i].pos : CE_VEC3_ZERO;
        ce_vec3_t rot = rotaiton ? ce_vec3_mul_s(rotaiton[i].rot, CE_DEG_TO_RAD) : CE_VEC3_ZERO;
        ce_vec3_t scl = scale ? scale[i].scl : CE_VEC3_UNIT;

        ct_local_to_parent_c *t_c = &transforms[i];

        ce_mat4_srt(t_c->local.m,
                    scl.x, scl.y, scl.z,
                    rot.x, rot.y, rot.z,
                    pos.x, pos.y, pos.z);
    }
}


void _trs_to_local_to_world(ct_world_t0 world,
                            struct ct_entity_t0 *entities,
                            ct_ecs_ent_chunk_o0 *item,
                            uint32_t n,
                            void *_data) {

    ct_position_c *position = ct_ecs_c_a0->get_all(world, POSITION_COMPONENT, item);
    ct_rotation_c *rotaiton = ct_ecs_c_a0->get_all(world, ROTATION_COMPONENT, item);
    ct_scale_c *scale = ct_ecs_c_a0->get_all(world, SCALE_COMPONENT, item);

    ct_local_to_world_c *transforms = ct_ecs_c_a0->get_all(world, LOCAL_TO_WORLD_COMPONENT, item);

    for (int i = 0; i < n; ++i) {
        ce_vec3_t pos = position ? position[i].pos : CE_VEC3_ZERO;
        ce_vec3_t rot = rotaiton ? ce_vec3_mul_s(rotaiton[i].rot, CE_DEG_TO_RAD) : CE_VEC3_ZERO;
        ce_vec3_t scl = scale ? scale[i].scl : CE_VEC3_UNIT;

        ct_local_to_world_c *t_c = &transforms[i];

        ce_mat4_srt(t_c->world.m,
                    scl.x, scl.y, scl.z,
                    rot.x, rot.y, rot.z,
                    pos.x, pos.y, pos.z);
    }
}

void _local_to_parent(ct_world_t0 world,
                      struct ct_entity_t0 *entities,
                      ct_ecs_ent_chunk_o0 *item,
                      uint32_t n,
                      void *_data) {

    ct_local_to_world_c *transforms = ct_ecs_c_a0->get_all(world, LOCAL_TO_WORLD_COMPONENT, item);
    ct_local_to_parent_c *local = ct_ecs_c_a0->get_all(world, LOCAL_TO_PARENT_COMPONENT, item);
    ct_parent_c *parent = ct_ecs_c_a0->get_all(world, CT_PARENT_COMPONENT, item);

    for (int i = 0; i < n; ++i) {
        ct_local_to_parent_c *l = &local[i];
        ct_entity_t0 p = parent[i].parent;

        ct_local_to_world_c *ltp = ct_ecs_c_a0->get_one(world, LOCAL_TO_WORLD_COMPONENT, p, false);
        if (!ltp) {
            continue;
        }

        ce_mat4_mul(transforms[i].world.m, ltp->world.m, l->local.m);
    }
}

static void transform_system(ct_world_t0 world,
                             float dt,
                             uint32_t rq_version,
                             ct_ecs_cmd_buffer_t *cmd) {

    ct_ecs_q_a0->foreach(world,
                         (ct_ecs_query_t0) {
                                 .all = CT_ECS_ARCHETYPE(LOCAL_TO_PARENT_COMPONENT),

                                 .any = CT_ECS_ARCHETYPE(POSITION_COMPONENT,
                                                         ROTATION_COMPONENT,
                                                         SCALE_COMPONENT),

                                 .write = CT_ECS_ARCHETYPE(LOCAL_TO_PARENT_COMPONENT),

                                 .only_changed = true,
                         }, rq_version,
                         _trs_to_local_to_parent, NULL);

    ct_ecs_q_a0->foreach(world,
                         (ct_ecs_query_t0) {
                                 .all = CT_ECS_ARCHETYPE(LOCAL_TO_WORLD_COMPONENT),

                                 .any = CT_ECS_ARCHETYPE(POSITION_COMPONENT,
                                                         ROTATION_COMPONENT,
                                                         SCALE_COMPONENT),

                                 .none = CT_ECS_ARCHETYPE(LOCAL_TO_PARENT_COMPONENT),

                                 .write = CT_ECS_ARCHETYPE(LOCAL_TO_WORLD_COMPONENT),

                                 .only_changed = true,
                         }, rq_version,
                         _trs_to_local_to_world, NULL);

    ct_ecs_q_a0->foreach(world,
                         (ct_ecs_query_t0) {
                                 .all =  CT_ECS_ARCHETYPE(LOCAL_TO_PARENT_COMPONENT,
                                                          CT_PARENT_COMPONENT),

                                 .write = CT_ECS_ARCHETYPE(LOCAL_TO_WORLD_COMPONENT),

                                 .only_changed = true,
                         }, rq_version,
                         _local_to_parent, NULL);

}

static struct ct_system_group_i0 transform_group = {
        .name = TRANSFORM_GROUP,
};

static struct ct_system_i0 transform_system_i0 = {
        .name = TRANSFORM_SYSTEM,
        .group = TRANSFORM_GROUP,
        .process = transform_system,
        .after = CT_ECS_AFTER(CT_PARENT_SYSTEM),
};



static const ce_cdb_prop_def_t0 local_to_world_prop[] = {
};

static const ce_cdb_prop_def_t0 vec3_prop[] = {
        {.name = "x", .type = CE_CDB_TYPE_FLOAT},
        {.name = "y", .type = CE_CDB_TYPE_FLOAT},
        {.name = "z", .type = CE_CDB_TYPE_FLOAT},
};

//static const ce_cdb_prop_def_t0 scale_prop[] = {
//        {.name = "x", .type = CE_CDB_TYPE_FLOAT, .value.f = 1.0f},
//        {.name = "y", .type = CE_CDB_TYPE_FLOAT, .value.f = 1.0f},
//        {.name = "z", .type = CE_CDB_TYPE_FLOAT, .value.f = 1.0f},
//};

static const ce_cdb_prop_def_t0 vec2_prop[] = {
        {.name = "x", .type = CE_CDB_TYPE_FLOAT},
        {.name = "y", .type = CE_CDB_TYPE_FLOAT},
};


void CE_MODULE_LOAD(transform)(struct ce_api_a0 *api,
                               int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_ydb_a0);
    CE_INIT_API(api, ce_cdb_a0);
    CE_INIT_API(api, ct_ecs_a0);
    CE_INIT_API(api, ce_log_a0);

    ce_id_a0->id64("position");
    ce_id_a0->id64("rotation");
    ce_id_a0->id64("scale");
    ce_id_a0->id64("local_to_world");
    ce_id_a0->id64("local_to_parent");
    ce_id_a0->id64("parent");

    _G = (struct transform_global) {
            .alloc = ce_memory_a0->system,
    };

    api->add_impl(CT_ECS_COMPONENT_I,
                  &transform_c_api, sizeof(transform_c_api));

    api->add_impl(CT_ECS_COMPONENT_I,
                  &local_to_parent_c_api, sizeof(local_to_parent_c_api));

    api->add_impl(CT_ECS_SYSTEM_I,
                  &transform_system_i0, sizeof(transform_system_i0));

    api->add_impl(CT_ECS_SYSTEM_GROUP_I,
                  &transform_group, sizeof(transform_group));

    ///
    api->add_impl(CT_ECS_COMPONENT_I, &position_c_api, sizeof(position_c_api));
    api->add_impl(CT_ECS_COMPONENT_I, &rotation_c_api, sizeof(rotation_c_api));
    api->add_impl(CT_ECS_COMPONENT_I, &scale_c_api, sizeof(scale_c_api));

    ce_cdb_a0->reg_obj_type(POSITION_COMPONENT, position_c_prop, CE_ARRAY_LEN(position_c_prop));
    ce_cdb_a0->reg_obj_type(ROTATION_COMPONENT, rotation_c_prop, CE_ARRAY_LEN(rotation_c_prop));
    ce_cdb_a0->reg_obj_type(SCALE_COMPONENT, scale_c_prop, CE_ARRAY_LEN(scale_c_prop));
    ///

    api->add_impl(CT_PROPERTY_EDITOR_I,
                  &position_property_editor_api, sizeof(position_property_editor_api));

    api->add_impl(CT_PROPERTY_EDITOR_I,
                  &rotation_property_editor_api, sizeof(rotation_property_editor_api));

    api->add_impl(CT_PROPERTY_EDITOR_I,
                  &scale_property_editor_api, sizeof(scale_property_editor_api));

    ce_cdb_a0->reg_obj_type(VEC2_CDB_TYPE,
                            vec2_prop, CE_ARRAY_LEN(vec2_prop));

    ce_cdb_a0->reg_obj_type(VEC3_CDB_TYPE,
                            vec3_prop, CE_ARRAY_LEN(vec3_prop));

    ce_cdb_a0->reg_obj_type(LOCAL_TO_WORLD_COMPONENT,
                            local_to_world_prop, CE_ARRAY_LEN(local_to_world_prop));

}

void CE_MODULE_UNLOAD(transform)(struct ce_api_a0 *api,
                                 int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}
