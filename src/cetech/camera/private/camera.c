#include <celib/cdb.h>
#include <cetech/ecs/ecs.h>
#include <cetech/transform/transform.h>
#include <celib/cdb_yaml.h>
#include <cetech/camera/camera.h>

#include <celib/memory/allocator.h>
#include <celib/macros.h>
#include <celib/math/math.h>

#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/ui/icons_font_awesome.h>
#include <cetech/property_editor/property_editor.h>
#include <cetech/property_editor/property_editor.h>

#include "celib/id.h"
#include "celib/config.h"
#include "celib/memory/memory.h"
#include "celib/api.h"

#include "celib/module.h"

#define _G CameraGlobal
static struct _G {
    ce_alloc_t0 *allocator;
} _G;

static void get_project_view(ce_mat4_t world,
                             ct_camera_component camera,
                             float *proj,
                             float *view,
                             int width,
                             int height) {

    float identity[16];
    ce_mat4_identity(identity);

    float ratio = (float) (width) / (float) (height);

//    ce_mat4_look_at(view, transform->position,
//                    (float[]){0.0f, 0.0f, 1.0f},
//                    (float[]){0.0f, 1.0f, 0.0f});


    ce_mat4_identity(proj);

    if (camera.camera_type == CAMERA_TYPE_PERSPECTIVE) {
        ce_mat4_proj_fovy(proj,
                          camera.fov,
                          ratio,
                          camera.near,
                          camera.far,
                          ct_gfx_a0->bgfx_get_caps()->homogeneousDepth);

    } else if (camera.camera_type == CAMERA_TYPE_ORTHO) {
        ce_mat4_ortho(proj, 0, width, 0, height, camera.near, camera.far, 0,
                      ct_gfx_a0->bgfx_get_caps()->homogeneousDepth);
    }

    ce_mat4_t w = CE_MAT4_IDENTITY;
    ce_mat4_inverse(w.m, world.m);
    ce_mat4_move(view, w.m);
}

static struct ct_camera_a0 camera_api = {
        .get_project_view = get_project_view,
};


struct ct_camera_a0 *ct_camera_a0 = &camera_api;

///
static void _camera_on_spawn(ct_world_t0 world,
                             ce_cdb_t0 db,
                             uint64_t obj,
                             void *data) {
    ct_camera_component *c = data;

    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);
    const char *camera_type = ce_cdb_a0->read_str(r, PROP_CAMERA_TYPE, "perspective");

    *c = (ct_camera_component) {
            .camera_type = ce_id_a0->id64(camera_type),
            .fov = ce_cdb_a0->read_float(r, PROP_FOV, 60),
            .near = ce_cdb_a0->read_float(r, PROP_NEAR, 60),
            .far = ce_cdb_a0->read_float(r, PROP_FAR, 60),
    };
}

static struct ct_ecs_component_i0 ct_component_api = {
        .icon = ICON_FA_CAMERA,
        .display_name = "Camera",
        .cdb_type = CT_CAMERA_COMPONENT,
        .size = sizeof(ct_camera_component),
        .from_cdb_obj = _camera_on_spawn,
};

////
static struct ct_ecs_component_i0 ct_active_camera_component = {
        .icon = ICON_FA_CAMERA,
        .display_name = "Active camera",
        .cdb_type = CT_ACTIVE_CAMERA_COMPONENT,
};

///
static void _draw_camera_property(ce_cdb_t0 db,
                                  uint64_t obj,
                                  const char *title,
                                  uint64_t context) {

    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(db, obj);
    const char *camea_type_str = ce_cdb_a0->read_str(r, PROP_CAMERA_TYPE, "");
    uint64_t camera_type = ce_id_a0->id64(camea_type_str);

    ct_property_editor_a0->ui_str_combo2(obj, "Camera type",
                                         PROP_CAMERA_TYPE,
                                         (const char *[]) {"perspective", "ortho"}, 2, obj);

    ct_property_editor_a0->ui_float(obj, "Near", PROP_NEAR, (ui_float_p0) {});
    ct_property_editor_a0->ui_float(obj, "Far", PROP_FAR, (ui_float_p0) {});

    if (camera_type == CAMERA_TYPE_PERSPECTIVE) {
        ct_property_editor_a0->ui_float(obj, "Fov", PROP_FOV, (ui_float_p0) {});
    }
}

static ce_cdb_prop_def_t0 camera_component_prop[] = {
        {.name = "camera_type", .type = CE_CDB_TYPE_STR, .value.str = "perspective"},
        {.name = "near", .type = CE_CDB_TYPE_FLOAT, .value.f = 0.1f},
        {.name = "far", .type = CE_CDB_TYPE_FLOAT, .value.f = 1000.0f},
        {.name = "fov", .type = CE_CDB_TYPE_FLOAT, .value.f = 60.0f},
};


void CE_MODULE_LOAD(camera)(struct ce_api_a0 *api,
                            int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ce_cdb_yaml_a0);
    CE_INIT_API(api, ce_cdb_yaml_a0);
    CE_INIT_API(api, ct_ecs_a0);
    CE_INIT_API(api, ce_cdb_a0);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    api->add_api(CT_CAMERA_A0_STR, &camera_api, sizeof(camera_api));

    api->add_impl(CT_ECS_COMPONENT_I0_STR,
                  &ct_component_api,
                  sizeof(ct_component_api));

    api->add_impl(CT_ECS_COMPONENT_I0_STR,
                  &ct_active_camera_component,
                  sizeof(ct_active_camera_component));

    ce_cdb_a0->reg_obj_type(CT_CAMERA_COMPONENT,
                            camera_component_prop,
                            CE_ARRAY_LEN(camera_component_prop));
    ce_cdb_a0->set_aspect(CT_CAMERA_COMPONENT, CT_PROPERTY_EDITOR_ASPECT, _draw_camera_property);


    ce_cdb_a0->reg_obj_type(CT_ACTIVE_CAMERA_COMPONENT, NULL, 0);
}

void CE_MODULE_UNLOAD(camera)(struct ce_api_a0 *api,
                              int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}
