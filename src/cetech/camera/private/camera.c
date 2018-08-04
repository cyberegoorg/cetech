#include <celib/cdb.h>
#include <cetech/ecs/ecs.h>
#include <cetech/transform/transform.h>
#include <celib/yng.h>
#include <celib/ydb.h>
#include <cetech/camera/camera.h>

#include <celib/macros.h>
#include <celib/array.inl>
#include <celib/fmath.inl>
#include <celib/ebus.h>
#include <cetech/gfx/renderer.h>
#include <cetech/gfx/private/iconfontheaders/icons_font_awesome.h>
#include <cetech/editor/editor_ui.h>

#include "celib/hashlib.h"
#include "celib/config.h"
#include "celib/memory.h"
#include "celib/api_system.h"

#include "celib/module.h"

#define _G CameraGlobal
static struct CameraGlobal {
    struct ce_alloc *allocator;
} CameraGlobal;

static void _camera_compiler(const char *filename,
                             uint64_t *component_key,
                             uint32_t component_key_count,
                             ce_cdb_obj_o *writer) {


    uint64_t keys[component_key_count + 1];
    memcpy(keys, component_key, sizeof(uint64_t) * component_key_count);

    keys[component_key_count] = ce_yng_a0->key("near");
    float near = ce_ydb_a0->get_float(filename, CE_ARR_ARG(keys), 0.0f);

    keys[component_key_count] = ce_yng_a0->key("far");
    float far = ce_ydb_a0->get_float(filename, CE_ARR_ARG(keys), 0.0f);

    keys[component_key_count] = ce_yng_a0->key("fov");
    float fov = ce_ydb_a0->get_float(filename, CE_ARR_ARG(keys), 0.0f);

    ce_cdb_a0->set_float(writer, PROP_NEAR, near);
    ce_cdb_a0->set_float(writer, PROP_FAR, far);
    ce_cdb_a0->set_float(writer, PROP_FOV, fov);
}


static void get_project_view(struct ct_world world,
                             struct ct_entity camera,
                             float *proj,
                             float *view,
                             int width,
                             int height) {

    struct ct_transform_comp *transform;
    struct ct_camera_component *camera_data;

    transform = ct_ecs_a0->component->get_one(world, TRANSFORM_COMPONENT,
                                                  camera);
    camera_data = ct_ecs_a0->component->get_one(world, CAMERA_COMPONENT,
                                                    camera);

    float ratio = (float) (width) / (float) (height);

//    ce_mat4_look_at(view, transform->position,
//                    (float[]){0.0f, 0.0f, 1.0f},
//                    (float[]){0.0f, 1.0f, 0.0f});

    ce_mat4_proj_fovy(proj,
                      camera_data->fov,
                      ratio,
                      camera_data->near,
                      camera_data->far,
                      ct_renderer_a0->get_caps()->homogeneousDepth);

    float w[16];
    ce_mat4_move(w, transform->world);

    w[12] *= -1.0f;
    w[13] *= -1.0f;
    w[14] *= -1.0f;

    ce_mat4_move(view, w);
}

static struct ct_camera_a0 camera_api = {
        .get_project_view = get_project_view,
};


struct ct_camera_a0 *ct_camera_a0 = &camera_api;

static void _on_obj_change(uint64_t obj,
                           const uint64_t *prop,
                           uint32_t prop_count,
                           void *data) {

    uint64_t ent_obj = ce_cdb_a0->parent(ce_cdb_a0->parent(obj));

    struct ct_world world = {
            .h = ce_cdb_a0->read_uint64(ent_obj, ENTITY_WORLD, 0)
    };

    struct ct_entity ent = {
            .h = ent_obj
    };

    struct ct_camera_component *camera;
    camera = ct_ecs_a0->component->get_one(world, CAMERA_COMPONENT, ent);

    for (int k = 0; k < prop_count; ++k) {
        if (prop[k] == PROP_FOV) {
            camera->fov = ce_cdb_a0->read_float(obj, PROP_FOV, 0.0f);
        } else if (prop[k] == PROP_NEAR) {
            camera->near = ce_cdb_a0->read_float(obj, PROP_NEAR, 0.0f);
        } else if (prop[k] == PROP_FAR) {
            camera->far = ce_cdb_a0->read_float(obj, PROP_FAR, 0.0f);
        }
    }
}

static void _component_spawner(uint64_t obj, void* data) {
    struct ct_camera_component *camera = data;

    *camera = (struct ct_camera_component) {
            .fov = ce_cdb_a0->read_float(obj, PROP_FOV, 0.0f),
            .near = ce_cdb_a0->read_float(obj, PROP_NEAR, 0.0f),
            .far = ce_cdb_a0->read_float(obj, PROP_FAR, 0.0f),
    };

    ce_cdb_a0->register_notify(obj, _on_obj_change, NULL);
}


static uint64_t cdb_type() {
    return CAMERA_COMPONENT;
}

static const char *display_name() {
    return ICON_FA_CAMERA " Camera";
}

static void property_editor(uint64_t obj) {
    ct_editor_ui_a0->ui_float(obj, PROP_NEAR, "Near", 0, 0);
    ct_editor_ui_a0->ui_float(obj, PROP_FAR, "Far", 0, 0);
    ct_editor_ui_a0->ui_float(obj, PROP_FOV, "Fov", 0, 0);
}

static void *get_interface(uint64_t name_hash) {
    if (EDITOR_COMPONENT == name_hash) {
        static struct ct_editor_component_i0 ct_editor_component_i0 = {
                .display_name = display_name,
                .property_editor = property_editor,
        };

        return &ct_editor_component_i0;
    }

    return NULL;
}


static uint64_t size() {
    return sizeof(struct ct_camera_component);
}

static struct ct_component_i0 ct_component_i0 = {
        .size = size,
        .cdb_type = cdb_type,
        .get_interface = get_interface,
        .compiler = _camera_compiler,
                .spawner = _component_spawner,
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
            CE_INIT_API(api, ce_yng_a0);
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