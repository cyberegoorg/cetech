#include "celib/config.h"
#include "cetech/resource/resource.h"
#include "celib/hashlib.h"
#include "celib/memory.h"
#include "celib/api_system.h"
#include <celib/ydb.h>
#include <celib/ydb.h>
#include <celib/macros.h>
#include <celib/array.inl>
#include <celib/fmath.inl>
#include <celib/ebus.h>
#include <celib/log.h>
#include <celib/cdb.h>

#include "celib/module.h"
#include "cetech/ecs/ecs.h"
#include <cetech/transform/transform.h>
#include <cetech/debugui/debugui.h>
#include <cetech/debugui/icons_font_awesome.h>

#include <cetech/asset_editor/sourcedb_ui.h>
#include <cetech/editor/property.h>

#define LOG_WHERE "transform"

static void transform_transform(uint64_t transform,
                                float *parent) {

    const ce_cdb_obj_o *reader = ce_cdb_a0->read(transform);

    float pos[3] ={
            ce_cdb_a0->read_float(reader, PROP_POSITION_X, 0.0f),
            ce_cdb_a0->read_float(reader, PROP_POSITION_Y, 0.0f),
            ce_cdb_a0->read_float(reader, PROP_POSITION_Z, 0.0f),
    };

    float rot[3] = {
            ce_cdb_a0->read_float(reader, PROP_ROTATION_X, 0.0f),
            ce_cdb_a0->read_float(reader, PROP_ROTATION_Y, 0.0f),
            ce_cdb_a0->read_float(reader, PROP_ROTATION_Z, 0.0f),
    };

    float sca[3] =  {
            ce_cdb_a0->read_float(reader, PROP_SCALE_X, 1.0f),
            ce_cdb_a0->read_float(reader, PROP_SCALE_Y, 1.0f),
            ce_cdb_a0->read_float(reader, PROP_SCALE_Z, 1.0f),
    };

    float rot_rad[3];
    ce_vec3_mul_s(rot_rad, rot, CE_DEG_TO_RAD);

    float world[16] = {};
    ce_mat4_srt(world,
                sca[0], sca[1], sca[2],
                rot_rad[0], rot_rad[1], rot_rad[2],
                pos[0], pos[1], pos[2]);

    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(transform);
    ce_cdb_a0->set_blob(w, PROP_WORLD, world, sizeof(world));
    ce_cdb_a0->write_commit(w);
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
    const ce_cdb_obj_o *reader = ce_cdb_a0->read(obj);

    float pos[3] = {
            ce_cdb_a0->read_float(reader, PROP_POSITION_X, 0.0f),
            ce_cdb_a0->read_float(reader, PROP_POSITION_Y, 0.0f),
            ce_cdb_a0->read_float(reader, PROP_POSITION_Z, 0.0f),
    };

    float rot[3] = {
            ce_cdb_a0->read_float(reader, PROP_ROTATION_X, 0.0f),
            ce_cdb_a0->read_float(reader, PROP_ROTATION_Y, 0.0f),
            ce_cdb_a0->read_float(reader, PROP_ROTATION_Z, 0.0f),
    };
    float sca[3] = {
            ce_cdb_a0->read_float(reader, PROP_SCALE_X, 1.0f),
            ce_cdb_a0->read_float(reader, PROP_SCALE_Y, 1.0f),
            ce_cdb_a0->read_float(reader, PROP_SCALE_Z, 1.0f),
    };


    float rot_rad[3];
    ce_vec3_mul_s(rot_rad, rot, CE_DEG_TO_RAD);

    ce_mat4_srt(world,
                sca[0], sca[1], sca[2],
                rot_rad[0], rot_rad[1], rot_rad[2],
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

    struct ct_cdb_obj_t *w = ce_cdb_a0->write_begin(obj);

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

    ce_cdb_obj_o *w = ce_cdb_a0->write_begin(component);
    ce_cdb_a0->set_float(w, PROP_SCALE_X, 1.0f);
    ce_cdb_a0->set_float(w, PROP_SCALE_Y, 1.0f);
    ce_cdb_a0->set_float(w, PROP_SCALE_Z, 1.0f);
    ce_cdb_a0->write_commit(w);

    return component;
}

static void property_editor(uint64_t obj) {

    ct_sourcedb_ui_a0->ui_vec3(obj,
                               (uint64_t[3]) {PROP_POSITION_X,
                                              PROP_POSITION_Y,
                                              PROP_POSITION_Z},
                               "Position",
                               (struct ui_vec3_p0) {});

    ct_sourcedb_ui_a0->ui_vec3(obj,
                               (uint64_t[3]) {PROP_ROTATION_X,
                                              PROP_ROTATION_Y,
                                              PROP_ROTATION_Z},
                               "Rotation",
                               (struct ui_vec3_p0) {});


    ct_sourcedb_ui_a0->ui_vec3(obj,
                               (uint64_t[3]) {PROP_SCALE_X,
                                              PROP_SCALE_Y,
                                              PROP_SCALE_Z},
                               "Scale",
                               (struct ui_vec3_p0) {});
}

static struct ct_property_editor_i0 ct_property_editor_i0 = {
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

//static uint64_t size() {
//    return sizeof(struct ct_transform_comp);
//}

static void transform_spawner(struct ct_world world,
                              uint64_t obj) {
//    struct ct_transform_comp *t = data;
//
//    *t = (struct ct_transform_comp) {
//            .position = {
//                    ce_cdb_a0->read_float(obj, PROP_POSITION_X, 0.0f),
//                    ce_cdb_a0->read_float(obj, PROP_POSITION_Y, 0.0f),
//                    ce_cdb_a0->read_float(obj, PROP_POSITION_Z, 0.0f),
//            },
//
//            .rotation = {
//                    ce_cdb_a0->read_float(obj, PROP_ROTATION_X, 0.0f),
//                    ce_cdb_a0->read_float(obj, PROP_ROTATION_Y, 0.0f),
//                    ce_cdb_a0->read_float(obj, PROP_ROTATION_Z, 0.0f),
//            },
//
//            .scale = {
//                    ce_cdb_a0->read_float(obj, PROP_SCALE_X, 1.0f),
//                    ce_cdb_a0->read_float(obj, PROP_SCALE_Y, 1.0f),
//                    ce_cdb_a0->read_float(obj, PROP_SCALE_Z, 1.0f),
//            },
//    };
}

void obj_change(struct ct_world world,
                uint64_t obj,
                const uint64_t *prop,
                uint32_t prop_count,
                struct ct_entity *ents,
                uint32_t n) {
//    for (int i = 0; i < n; ++i) {
//        struct ct_entity ent = ents[i];
//        struct ct_transform_comp *t = ct_ecs_a0->get_one(world,
//                                                         TRANSFORM_COMPONENT,
//                                                         ent);
//
//        *t = (struct ct_transform_comp) {
//                .position = {
//                        ce_cdb_a0->read_float(obj, PROP_POSITION_X, 0.0f),
//                        ce_cdb_a0->read_float(obj, PROP_POSITION_Y, 0.0f),
//                        ce_cdb_a0->read_float(obj, PROP_POSITION_Z, 0.0f),
//                },
//
//                .rotation = {
//                        ce_cdb_a0->read_float(obj, PROP_ROTATION_X, 0.0f),
//                        ce_cdb_a0->read_float(obj, PROP_ROTATION_Y, 0.0f),
//                        ce_cdb_a0->read_float(obj, PROP_ROTATION_Z, 0.0f),
//                },
//
//                .scale = {
//                        ce_cdb_a0->read_float(obj, PROP_SCALE_X, 1.0f),
//                        ce_cdb_a0->read_float(obj, PROP_SCALE_Y, 1.0f),
//                        ce_cdb_a0->read_float(obj, PROP_SCALE_Z, 1.0f),
//                },
//        };
//    }
}

static struct ct_component_i0 ct_component_i0 = {
        .cdb_type = cdb_type,
        .get_interface = get_interface,
        .spawner = transform_spawner,
        .obj_change = obj_change,
};


static void foreach_transform(struct ct_world world,
                              struct ct_entity *ent,
                              ct_entity_storage_t *item,
                              uint32_t n,
                              void *data) {
    for (uint32_t i = 1; i < n; ++i) {
        uint64_t transform = ct_ecs_a0->get_one(world, TRANSFORM_COMPONENT, ent[i]);
        transform_transform(transform, NULL);
    }
}

static void transform_system(struct ct_world world,
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
    api->register_api(COMPONENT_INTERFACE_NAME, &ct_component_i0);
    api->register_api(SIMULATION_INTERFACE_NAME, &transform_simulation_i0);
    api->register_api(PROPERTY_EDITOR_INTERFACE_NAME, &ct_property_editor_i0);
}

static void _shutdown() {

}

CE_MODULE_DEF(
        transform,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ce_ydb_a0);
            CE_INIT_API(api, ce_ydb_a0);
            CE_INIT_API(api, ce_cdb_a0);
            CE_INIT_API(api, ct_ecs_a0);
            CE_INIT_API(api, ce_ebus_a0);
            CE_INIT_API(api, ce_log_a0);
        },
        {
            CE_UNUSED(reload);
            _init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);
            _shutdown();
        }
)