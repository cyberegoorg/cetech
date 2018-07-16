#include <corelib/cdb.h>
#include <cetech/ecs/ecs.h>
#include <cetech/transform/transform.h>
#include <corelib/yng.h>
#include <corelib/ydb.h>
#include <cetech/camera/camera.h>

#include <corelib/macros.h>
#include <corelib/array.inl>
#include <corelib/fmath.inl>
#include <corelib/ebus.h>
#include <cetech/ecs/entity_property.h>
#include <cetech/renderer/renderer.h>
#include <cetech/debugui/private/iconfontheaders/icons_font_awesome.h>
#include <cetech/editor_ui/editor_ui.h>

#include "corelib/hashlib.h"
#include "corelib/config.h"
#include "corelib/memory.h"
#include "corelib/api_system.h"

#include "corelib/module.h"

#define _G CameraGlobal
static struct CameraGlobal {
    struct ct_alloc *allocator;
} CameraGlobal;

static void _camera_compiler(const char *filename,
                             uint64_t *component_key,
                             uint32_t component_key_count,
                             ct_cdb_obj_o *writer) {


    uint64_t keys[component_key_count + 1];
    memcpy(keys, component_key, sizeof(uint64_t) * component_key_count);

    keys[component_key_count] = ct_yng_a0->key("near");
    float near = ct_ydb_a0->get_float(filename, CETECH_ARR_ARG(keys), 0.0f);

    keys[component_key_count] = ct_yng_a0->key("far");
    float far = ct_ydb_a0->get_float(filename, CETECH_ARR_ARG(keys), 0.0f);

    keys[component_key_count] = ct_yng_a0->key("fov");
    float fov = ct_ydb_a0->get_float(filename, CETECH_ARR_ARG(keys), 0.0f);

    ct_cdb_a0->set_float(writer, PROP_NEAR, near);
    ct_cdb_a0->set_float(writer, PROP_FAR, far);
    ct_cdb_a0->set_float(writer, PROP_FOV, fov);
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

//    ct_mat4_look_at(view, transform->position,
//                    (float[]){0.0f, 0.0f, 1.0f},
//                    (float[]){0.0f, 1.0f, 0.0f});

    ct_mat4_proj_fovy(proj,
                      camera_data->fov,
                      ratio,
                      camera_data->near,
                      camera_data->far,
                      ct_renderer_a0->get_caps()->homogeneousDepth);

    float w[16];
    ct_mat4_move(w, transform->world);

    w[12] *= -1.0f;
    w[13] *= -1.0f;
    w[14] *= -1.0f;

    ct_mat4_move(view, w);
}

static struct ct_camera_a0 camera_api = {
        .get_project_view = get_project_view,
};


struct ct_camera_a0 *ct_camera_a0 = &camera_api;

static void _on_obj_change(uint64_t obj,
                           const uint64_t *prop,
                           uint32_t prop_count,
                           void *data) {

    uint64_t ent_obj = ct_cdb_a0->parent(ct_cdb_a0->parent(obj));

    struct ct_world world = {
            .h = ct_cdb_a0->read_uint64(ent_obj, ENTITY_WORLD, 0)
    };

    struct ct_entity ent = {
            .h = ent_obj
    };

    struct ct_camera_component *camera;
    camera = ct_ecs_a0->component->get_one(world, CAMERA_COMPONENT, ent);

    for (int k = 0; k < prop_count; ++k) {
        if (prop[k] == PROP_FOV) {
            camera->fov = ct_cdb_a0->read_float(obj, PROP_FOV, 0.0f);
        } else if (prop[k] == PROP_NEAR) {
            camera->near = ct_cdb_a0->read_float(obj, PROP_NEAR, 0.0f);
        } else if (prop[k] == PROP_FAR) {
            camera->far = ct_cdb_a0->read_float(obj, PROP_FAR, 0.0f);
        }
    }
}

static void _component_spawner(uint64_t obj, void* data) {
    struct ct_camera_component *camera = data;

    *camera = (struct ct_camera_component) {
            .fov = ct_cdb_a0->read_float(obj, PROP_FOV, 0.0f),
            .near = ct_cdb_a0->read_float(obj, PROP_NEAR, 0.0f),
            .far = ct_cdb_a0->read_float(obj, PROP_FAR, 0.0f),
    };

    ct_cdb_a0->register_notify(obj, _on_obj_change, NULL);
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


static void _init(struct ct_api_a0 *api) {
    api->register_api("ct_camera_a0", &camera_api);

    _G = (struct _G) {
            .allocator = ct_memory_a0->system,
    };

    api->register_api("ct_component_i0", &ct_component_i0);
}

static void _shutdown() {
}


CETECH_MODULE_DEF(
        camera,
        {
            CT_INIT_API(api, ct_memory_a0);
            CT_INIT_API(api, ct_hashlib_a0);
            CT_INIT_API(api, ct_yng_a0);
            CT_INIT_API(api, ct_ydb_a0);
            CT_INIT_API(api, ct_ecs_a0);
            CT_INIT_API(api, ct_cdb_a0);
            CT_INIT_API(api, ct_ebus_a0);
        },
        {
            CT_UNUSED(reload);
            _init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);
            _shutdown();
        }
)