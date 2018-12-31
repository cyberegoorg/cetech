#include <celib/cdb.h>
#include <cetech/ecs/ecs.h>
#include <cetech/transform/transform.h>
#include <celib/ydb.h>
#include <celib/ydb.h>
#include <cetech/camera/camera.h>

#include <celib/macros.h>
#include <celib/array.inl>
#include <celib/fmath.inl>
#include <celib/ebus.h>
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/editor/resource_ui.h>

#include "celib/hashlib.h"
#include "celib/config.h"
#include "celib/memory.h"
#include "celib/api_system.h"

#include "celib/module.h"

#define _G CameraGlobal
static struct CameraGlobal {
    struct ce_alloc *allocator;
} CameraGlobal;

static void get_project_view(struct ct_world world,
                             struct ct_entity camera,
                             float *proj,
                             float *view,
                             int width,
                             int height) {

    uint64_t transform = ct_ecs_a0->get_one(world, TRANSFORM_COMPONENT,
                                              camera);

    uint64_t camera_data = ct_ecs_a0->get_one(world, CAMERA_COMPONENT,
                                              camera);

    const ce_cdb_obj_o *t_reader = ce_cdb_a0->read(ce_cdb_a0->db(),transform);
    const ce_cdb_obj_o *c_reader = ce_cdb_a0->read(ce_cdb_a0->db(),camera_data);


    float fov = ce_cdb_a0->read_float(c_reader, PROP_FOV, 0);
    float near = ce_cdb_a0->read_float(c_reader, PROP_NEAR, 0);
    float far = ce_cdb_a0->read_float(c_reader, PROP_FAR, 0);
    float *wworld = ce_cdb_a0->read_blob(t_reader, PROP_WORLD, NULL, 0);

    float ratio = (float) (width) / (float) (height);

//    ce_mat4_look_at(view, transform->position,
//                    (float[]){0.0f, 0.0f, 1.0f},
//                    (float[]){0.0f, 1.0f, 0.0f});

    ce_mat4_proj_fovy(proj,fov,
                      ratio,
                      near,
                      far,
                      ct_gfx_a0->get_caps()->homogeneousDepth);

    float w[16] = {};
    if(wworld) {
        ce_mat4_move(w, wworld);
    }

    w[12] *= -1.0f;
    w[13] *= -1.0f;
    w[14] *= -1.0f;

    ce_mat4_move(view, w);
}

static struct ct_camera_a0 camera_api = {
        .get_project_view = get_project_view,
};


struct ct_camera_a0 *ct_camera_a0 = &camera_api;

static uint64_t cdb_type() {
    return CAMERA_COMPONENT;
}

static const char *display_name() {
    return ICON_FA_CAMERA " Camera";
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
//    return sizeof(struct ct_camera_component);
//}

static void camera_spawner(struct ct_world world,
                              uint64_t obj) {
//    struct ct_camera_component *c = data;
//
//    *c = (struct ct_camera_component) {
//            .far = ce_cdb_a0->read_float(obj, PROP_FAR, 100.0f),
//            .near = ce_cdb_a0->read_float(obj, PROP_NEAR, 0.0f),
//            .fov = ce_cdb_a0->read_float(obj, PROP_FOV, 0.0f),
//    };
}


static struct ct_component_i0 ct_component_i0 = {
        .cdb_type = cdb_type,
        .get_interface = get_interface,
        .spawner = camera_spawner,
};


static void _init(struct ce_api_a0 *api) {
    api->register_api("ct_camera_a0", &camera_api);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    api->register_api("ct_component_i0", &ct_component_i0);
}

static void _shutdown() {
}


CE_MODULE_DEF(
        camera,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ce_ydb_a0);
            CE_INIT_API(api, ce_ydb_a0);
            CE_INIT_API(api, ct_ecs_a0);
            CE_INIT_API(api, ce_cdb_a0);
            CE_INIT_API(api, ce_ebus_a0);
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