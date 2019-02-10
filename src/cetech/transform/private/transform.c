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

static void guizmo_get_transform(uint64_t obj,
                                 float *world,
                                 float *local) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    float pos[3] = {
            ce_cdb_a0->read_float(reader, PROP_POSITION_X, 0.0f),
            ce_cdb_a0->read_float(reader, PROP_POSITION_Y, 0.0f),
            ce_cdb_a0->read_float(reader, PROP_POSITION_Z, 0.0f),
    };

    ce_vec3_t rot = {
            .x = ce_cdb_a0->read_float(reader, PROP_ROTATION_X, 0.0f),
            .y = ce_cdb_a0->read_float(reader, PROP_ROTATION_Y, 0.0f),
            .z = ce_cdb_a0->read_float(reader, PROP_ROTATION_Z, 0.0f),
    };

    float sca[3] = {
            ce_cdb_a0->read_float(reader, PROP_SCALE_X, 1.0f),
            ce_cdb_a0->read_float(reader, PROP_SCALE_Y, 1.0f),
            ce_cdb_a0->read_float(reader, PROP_SCALE_Z, 1.0f),
    };


    ce_vec3_t rot_rad = ce_vec3_mul_s(rot, CE_DEG_TO_RAD);

    ce_mat4_srt(world,
                sca[0], sca[1], sca[2],
                rot_rad.x, rot_rad.y, rot_rad.z,
                pos[0], pos[1], pos[2]);
}

static void guizmo_set_transform(uint64_t obj,
                                 uint8_t operation,
                                 float *world,
                                 float *local) {
    float pos[3] = {};
    float rot_deg[3] = {};
    float scale[3] = {};
    ct_debugui_a0->guizmo_decompose_matrix(world, pos, rot_deg, scale);

    struct ct_cdb_obj_t *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), obj);

    switch (operation) {
        case TRANSLATE:
            ce_cdb_a0->set_float(w, PROP_POSITION_X, pos[0]);
            ce_cdb_a0->set_float(w, PROP_POSITION_Y, pos[1]);
            ce_cdb_a0->set_float(w, PROP_POSITION_Z, pos[2]);
            break;

        case ROTATE:
            ce_cdb_a0->set_float(w, PROP_ROTATION_X, rot_deg[0]);
            ce_cdb_a0->set_float(w, PROP_ROTATION_Y, rot_deg[1]);
            ce_cdb_a0->set_float(w, PROP_ROTATION_Z, rot_deg[2]);
            break;

        case SCALE:
            ce_cdb_a0->set_float(w, PROP_SCALE_X, scale[0]);
            ce_cdb_a0->set_float(w, PROP_SCALE_Y, scale[1]);
            ce_cdb_a0->set_float(w, PROP_SCALE_Z, scale[2]);
            break;

        default:
            break;
    }

    ce_cdb_a0->write_commit(w);
}

static uint64_t create_new() {
    uint64_t component = ce_cdb_a0->create_object(ce_cdb_a0->db(),
                                                  TRANSFORM_COMPONENT);

    ce_cdb_obj_o0 *w = ce_cdb_a0->write_begin(ce_cdb_a0->db(), component);
    ce_cdb_a0->set_float(w, PROP_SCALE_X, 1.0f);
    ce_cdb_a0->set_float(w, PROP_SCALE_Y, 1.0f);
    ce_cdb_a0->set_float(w, PROP_SCALE_Z, 1.0f);
    ce_cdb_a0->write_commit(w);

    return component;
}

static void property_editor(uint64_t obj,
                            uint64_t context) {

    ct_editor_ui_a0->prop_vec3(obj,
                               (uint64_t[3]) {PROP_POSITION_X,
                                              PROP_POSITION_Y,
                                              PROP_POSITION_Z},
                               "Position",
                               (ui_vec3_p0) {});

    ct_editor_ui_a0->prop_vec3(obj,
                               (uint64_t[3]) {PROP_ROTATION_X,
                                              PROP_ROTATION_Y,
                                              PROP_ROTATION_Z},
                               "Rotation",
                               (ui_vec3_p0) {});


    ct_editor_ui_a0->prop_vec3(obj,
                               (uint64_t[3]) {PROP_SCALE_X,
                                              PROP_SCALE_Y,
                                              PROP_SCALE_Z},
                               "Scale",
                               (ui_vec3_p0) {});
}

static struct ct_property_editor_i0 property_editor_api = {
        .cdb_type = cdb_type,
        .draw_ui = property_editor,
};


static void *get_interface(uint64_t name_hash) {
    if (EDITOR_COMPONENT == name_hash) {
        static struct ct_editor_component_i0 ct_editor_component_i0 = {
                .display_name = display_name,
                .guizmo_get_transform = guizmo_get_transform,
                .guizmo_set_transform = guizmo_set_transform,
                .create_new = create_new,
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


    ce_vec3_t rotation = {
            ce_cdb_a0->read_float(r, PROP_ROTATION_X, 0.0f),
            ce_cdb_a0->read_float(r, PROP_ROTATION_Y, 0.0f),
            ce_cdb_a0->read_float(r, PROP_ROTATION_Z, 0.0f),
    };
    rotation = ce_vec3_mul_s(rotation, CE_DEG_TO_RAD);

    *t = (ct_transform_comp) {
            .t.pos = {
                    ce_cdb_a0->read_float(r, PROP_POSITION_X, 0.0f),
                    ce_cdb_a0->read_float(r, PROP_POSITION_Y, 0.0f),
                    ce_cdb_a0->read_float(r, PROP_POSITION_Z, 0.0f),
            },
            .t.rot = ce_quat_from_euler(rotation.x, rotation.y, rotation.z),
            .t.scl = {
                    ce_cdb_a0->read_float(r, PROP_SCALE_X, 1.0f),
                    ce_cdb_a0->read_float(r, PROP_SCALE_Y, 1.0f),
                    ce_cdb_a0->read_float(r, PROP_SCALE_Z, 1.0f),
            },
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
    struct ct_transform_comp *root_t = ct_ecs_a0->get_one(world,
                                                          TRANSFORM_COMPONENT,
                                                          ent);

    float *rootw = w;
    if (root_t) {
        rootw = root_t->world;
        transform_transform(root_t, w);
    }

    struct ct_entity_t0 ent_it = ct_ecs_a0->first_child(world, ent);
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
    struct ct_entity_t0 *roots = NULL;
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

static void _init(struct ce_api_a0 *api) {
    api->register_api(COMPONENT_INTERFACE, &ct_component_api, sizeof(ct_component_api));
    api->register_api(SIMULATION_INTERFACE, &transform_simulation_i0,
                      sizeof(transform_simulation_i0));
    api->register_api(PROPERTY_EDITOR_INTERFACE, &property_editor_api, sizeof(property_editor_api));
}

static void _shutdown() {

}

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
    _init(api);
}

void CE_MODULE_UNLOAD(transform)(struct ce_api_a0 *api,
                                 int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
    _shutdown();
}
