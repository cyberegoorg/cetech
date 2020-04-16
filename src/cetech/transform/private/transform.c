
#include <celib/memory/allocator.h>
#include "cetech/asset/asset.h"
#include "celib/id.h"
#include "celib/memory/memory.h"
#include "celib/api.h"
#include <celib/yaml_cdb.h>
#include <celib/macros.h>
#include <celib/math/math.h>

#include <celib/log.h>
#include <celib/cdb.h>
#include <celib/module.h>
#include <celib/containers/hash.h>
#include <celib/containers/bagraph.h>

#include "cetech/ecs/ecs.h"
#include <cetech/transform/transform.h>
#include <cetech/ui/icons_font_awesome.h>

#include <cetech/property_editor/property_editor.h>
#include <cetech/property_editor/property_editor.h>

#include <cetech/renderer/renderer.h>

#include <cetech/parent/parent.h>
#include <cetech/cdb_types/cdb_types.h>

#define LOG_WHERE "transform"

static struct transform_global {
    ce_alloc_t0 *alloc;
} _G = {};

// local to world
static void _local_to_world_on_spawn(ct_world_t0 world,
                                     ce_cdb_t0 db,
                                     uint64_t obj,
                                     void *data) {
    ct_local_to_world_c *t = data;
    t->world = CE_MAT4_IDENTITY;
}

static struct ct_ecs_component_i0 transform_c_api = {
        .icon = ICON_FA_ARROWS,
        .display_name = "Local to world",
        .cdb_type = LOCAL_TO_WORLD_COMPONENT,
        .size = sizeof(ct_local_to_world_c),
        .from_cdb_obj = _local_to_world_on_spawn,
};

// local to parent
static void _local_to_parent_on_spawn(ct_world_t0 world,
                                      ce_cdb_t0 db,
                                      uint64_t obj,
                                      void *data) {
    ct_local_to_parent_c *t = data;
    t->local = CE_MAT4_IDENTITY;
}

static struct ct_ecs_component_i0 local_to_parent_c_api = {
        .icon = ICON_FA_ARROWS,
        .display_name = "Local to parent",
        .cdb_type = LOCAL_TO_PARENT_COMPONENT,
        .size = sizeof(ct_local_to_parent_c),
        .from_cdb_obj = _local_to_parent_on_spawn,
};


// POS COMP
static void _position_on_spawn(ct_world_t0 world,
                               ce_cdb_t0 db,
                               uint64_t obj,
                               void *data) {
    ct_position_c *c = data;
    ce_cdb_a0->read_to(db, obj, c, sizeof(ct_position_c));
}

static struct ct_ecs_component_i0 position_c_api = {
        .cdb_type = POSITION_COMPONENT,
        .icon  = ICON_FA_ARROWS,
        .display_name = "Position",
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


//
// Rot COMP


static void _rotation_on_spawn(ct_world_t0 world,
                               ce_cdb_t0 db,
                               uint64_t obj,
                               void *data) {
    ct_rotation_c *c = data;
    ce_vec3_t rot;
    ce_cdb_a0->read_to(db, obj, &rot, sizeof(ce_vec3_t));

    c->rot = ce_quat_from_euler(rot.x * CE_DEG_TO_RAD,
                                rot.y * CE_DEG_TO_RAD,
                                rot.z * CE_DEG_TO_RAD);
}

static struct ct_ecs_component_i0 rotation_c_api = {
        .icon = ICON_FA_ARROWS,
        .cdb_type = ROTATION_COMPONENT,
        .display_name = "Rotation",
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

//
// SCALE COMP
static void _scale_on_spawn(ct_world_t0 world,
                            ce_cdb_t0 db,
                            uint64_t obj,
                            void *data) {
    ct_scale_c *c = data;
    ce_cdb_a0->read_to(db, obj, c, sizeof(ct_scale_c));
}

static struct ct_ecs_component_i0 scale_c_api = {
        .icon = ICON_FA_ARROWS,
        .cdb_type = SCALE_COMPONENT,
        .display_name = "Scale",
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
        ce_mat4_t t = CE_MAT4_IDENTITY;
        ce_mat4_t r = CE_MAT4_IDENTITY;
        ce_mat4_t s = CE_MAT4_IDENTITY;
        if (position) {
            ce_vec3_t pos = position[i].pos;
            ce_mat4_translate(t.m, pos.x, pos.y, pos.z);
        }

        if (rotaiton) {
            ce_mat4_quat(r.m, rotaiton[i].rot);
        }

        if (scale) {
            ce_vec3_t scl = scale[i].scl;
            ce_mat4_scale(s.m, scl.x, scl.y, scl.z);
        }

        ct_local_to_parent_c *t_c = &transforms[i];

        ce_mat4_t sr;
        ce_mat4_mul(sr.m, s.m, r.m);
        ce_mat4_mul(t_c->local.m, sr.m, t.m);
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
        ce_mat4_t t = CE_MAT4_IDENTITY;
        ce_mat4_t r = CE_MAT4_IDENTITY;
        ce_mat4_t s = CE_MAT4_IDENTITY;
        if (position) {
            ce_vec3_t pos = position[i].pos;
            ce_mat4_translate(t.m, pos.x, pos.y, pos.z);
        }

        if (rotaiton) {
            ce_mat4_quat(r.m, rotaiton[i].rot);
        }

        if (scale) {
            ce_vec3_t scl = scale[i].scl;
            ce_mat4_scale(s.m, scl.x, scl.y, scl.z);
        }

        ct_local_to_world_c *t_c = &transforms[i];

        ce_mat4_t sr;
        ce_mat4_mul(sr.m, s.m, r.m);
        ce_mat4_mul(t_c->world.m, sr.m, t.m);
    }
}

void _local_to_parent(ct_world_t0 world,
                      struct ct_entity_t0 *entities,
                      ct_ecs_ent_chunk_o0 *item,
                      uint32_t n,
                      void *_data) {
    ce_ba_graph_t *graph = _data;

    ct_parent_c *parent = ct_ecs_c_a0->get_all(world, CT_PARENT_COMPONENT, item);

    for (int i = 0; i < n; ++i) {
        ct_entity_t0 p = parent[i].parent;

        ct_local_to_world_c *ltp = ct_ecs_c_a0->get_one(world, LOCAL_TO_WORLD_COMPONENT, p, false);
        if (!ltp) {
            continue;
        }

        ce_bag_add(graph,
                   entities[i].h, &p.h, 1, NULL, 0, _G.alloc);
    }
}

static void transform_system(ct_world_t0 world,
                             float dt,
                             uint32_t rq_version,
                             ct_ecs_cmd_buffer_t *cmd) {

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
                                 .all = CT_ECS_ARCHETYPE(LOCAL_TO_PARENT_COMPONENT),

                                 .any = CT_ECS_ARCHETYPE(POSITION_COMPONENT,
                                                         ROTATION_COMPONENT,
                                                         SCALE_COMPONENT),

                                 .write = CT_ECS_ARCHETYPE(LOCAL_TO_PARENT_COMPONENT),

                                 .only_changed = true,
                         }, rq_version,
                         _trs_to_local_to_parent, NULL);

    ce_ba_graph_t graph = {};
    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all =  CT_ECS_ARCHETYPE(LOCAL_TO_PARENT_COMPONENT,
                                                                 LOCAL_TO_WORLD_COMPONENT,
                                                                 CT_PARENT_COMPONENT),

                                        .write = CT_ECS_ARCHETYPE(LOCAL_TO_WORLD_COMPONENT),
                                }, rq_version,
                                _local_to_parent, &graph);

    ce_bag_build(&graph, _G.alloc);
    uint32_t n = ce_array_size(graph.output);
    for (int i = 0; i < n; ++i) {
        ct_entity_t0 ent = {graph.output[i]};

        ct_parent_c *parent = ct_ecs_c_a0->get_one(world, CT_PARENT_COMPONENT, ent, false);
        if (!parent) {
            continue;
        }

        ct_entity_t0 par_ent = parent->parent;

        ct_local_to_world_c *ltp = ct_ecs_c_a0->get_one(world, LOCAL_TO_WORLD_COMPONENT, par_ent,
                                                        false);
        if (!ltp) {
            continue;
        }

        ct_local_to_world_c *ltw = ct_ecs_c_a0->get_one(world, LOCAL_TO_WORLD_COMPONENT, ent,
                                                        false);

        ct_local_to_parent_c *local = ct_ecs_c_a0->get_one(world, LOCAL_TO_PARENT_COMPONENT, ent,
                                                           false);

        if (!local) {
            continue;
        }

        ce_mat4_mul(ltw->world.m, local->local.m, ltp->world.m);
    }
    ce_bag_free(&graph, _G.alloc);

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

void CE_MODULE_LOAD(transform)(struct ce_api_a0 *api,
                               int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_yaml_cdb_a0);
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

    api->add_impl(CT_ECS_COMPONENT_I0_STR,
                  &transform_c_api, sizeof(transform_c_api));

    api->add_impl(CT_ECS_COMPONENT_I0_STR,
                  &local_to_parent_c_api, sizeof(local_to_parent_c_api));

    api->add_impl(CT_ECS_SYSTEM_I0_STR,
                  &transform_system_i0, sizeof(transform_system_i0));

    api->add_impl(CT_ECS_SYSTEM_GROUP_I0_STR,
                  &transform_group, sizeof(transform_group));

    ///
    api->add_impl(CT_ECS_COMPONENT_I0_STR, &position_c_api, sizeof(position_c_api));
    api->add_impl(CT_ECS_COMPONENT_I0_STR, &rotation_c_api, sizeof(rotation_c_api));
    api->add_impl(CT_ECS_COMPONENT_I0_STR, &scale_c_api, sizeof(scale_c_api));

    ce_cdb_a0->reg_obj_type(POSITION_COMPONENT, position_c_prop, CE_ARRAY_LEN(position_c_prop));
    ce_cdb_a0->reg_obj_type(ROTATION_COMPONENT, rotation_c_prop, CE_ARRAY_LEN(rotation_c_prop));
    ce_cdb_a0->reg_obj_type(SCALE_COMPONENT, scale_c_prop, CE_ARRAY_LEN(scale_c_prop));

    ce_cdb_a0->reg_obj_type(LOCAL_TO_WORLD_COMPONENT,
                            local_to_world_prop, CE_ARRAY_LEN(local_to_world_prop));

    ce_cdb_a0->reg_obj_type(LOCAL_TO_PARENT_COMPONENT,
                            local_to_world_prop, CE_ARRAY_LEN(local_to_world_prop));

}

void CE_MODULE_UNLOAD(transform)(struct ce_api_a0 *api,
                                 int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}
