#include <celib/cdb.h>
#include <cetech/ecs/ecs.h>
#include <cetech/transform/transform.h>
#include <celib/ydb.h>
#include <celib/ydb.h>
#include <cetech/camera/camera.h>

#include <celib/memory/allocator.h>
#include <celib/macros.h>
#include <celib/containers/array.h>
#include <celib/math/math.h>

#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/editor/editor_ui.h>
#include <cetech/editor/property.h>

#include "celib/id.h"
#include "celib/config.h"
#include "celib/memory/memory.h"
#include "celib/api.h"

#include "celib/module.h"

#define _G CameraGlobal
static struct CameraGlobal {
    ce_alloc_t0 *allocator;
} CameraGlobal;

static void get_project_view(ct_world_t0 world,
                             struct ct_entity_t0 _camera,
                             float *proj,
                             float *view,
                             int width,
                             int height) {

    uint64_t transform = ct_ecs_a0->get_one(world, TRANSFORM_COMPONENT, _camera);
    uint64_t camera = ct_ecs_a0->get_one(world, CAMERA_COMPONENT, _camera);

    if (!transform) {
        return;
    }

    if (!camera) {
        return;
    }

    ct_camera_component camera_c = {};
    ce_cdb_a0->read_to(ce_cdb_a0->db(), camera, &camera_c, sizeof(camera_c));

    const ce_cdb_obj_o0* c = ce_cdb_a0->read(ce_cdb_a0->db(), camera);
    float far = ce_cdb_a0->read_float(c, PROP_FAR, 0);

    CE_UNUSED(far);

    float identity[16];
    ce_mat4_identity(identity);

    const ce_cdb_obj_o0 *tr = ce_cdb_a0->read(ce_cdb_a0->db(), transform);

    float ratio = (float) (width) / (float) (height);

//    ce_mat4_look_at(view, transform->position,
//                    (float[]){0.0f, 0.0f, 1.0f},
//                    (float[]){0.0f, 1.0f, 0.0f});

    ce_mat4_identity(proj);
    ce_mat4_proj_fovy(proj,
                      camera_c.fov,
                      ratio,
                      camera_c.near,
                      camera_c.far,
                      ct_gfx_a0->bgfx_get_caps()->homogeneousDepth);

    float w[16] = CE_MAT4_IDENTITY;
    float *wworld = ce_cdb_a0->read_blob(tr, PROP_WORLD, NULL, identity);
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

static struct ct_component_i0 ct_component_api = {
        .cdb_type = cdb_type,
        .get_interface = get_interface,
};

static struct ct_property_editor_i0 property_editor_api = {
        .cdb_type = cdb_type,
};

static ce_cdb_prop_def_t0 camera_component_prop[] = {
        {.name = "near", .type = CDB_TYPE_FLOAT, .value.f = 0.1f},
        {.name = "far", .type = CDB_TYPE_FLOAT, .value.f = 1000.0f},
        {.name = "fov", .type = CDB_TYPE_FLOAT, .value.f = 60.0f},
};

void CE_MODULE_LOAD(camera)(struct ce_api_a0 *api,
                            int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_ydb_a0);
    CE_INIT_API(api, ce_ydb_a0);
    CE_INIT_API(api, ct_ecs_a0);
    CE_INIT_API(api, ce_cdb_a0);


    api->register_api(CT_CAMERA_API, &camera_api, sizeof(camera_api));

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    api->register_api(COMPONENT_INTERFACE, &ct_component_api, sizeof(ct_component_api));
    api->register_api(PROPERTY_EDITOR_INTERFACE, &property_editor_api, sizeof(property_editor_api));

    ce_cdb_a0->reg_obj_type(CAMERA_COMPONENT,
                            camera_component_prop, CE_ARRAY_LEN(camera_component_prop));
}

void CE_MODULE_UNLOAD(camera)(struct ce_api_a0 *api,
                              int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}
