#include <cetech/kernel/cdb/cdb.h>
#include <cetech/engine/ecs/ecs.h>
#include <cetech/engine/transform/transform.h>
#include <cetech/kernel/yaml/yng.h>
#include <cetech/kernel/yaml/ydb.h>
#include <cetech/engine/camera/camera.h>

#include <cetech/macros.h>
#include <cetech/kernel/containers/array.h>
#include <cetech/kernel/math/fmath.h>
#include <cetech/kernel/ebus/ebus.h>

#include "cetech/kernel/hashlib/hashlib.h"
#include "cetech/kernel/config/config.h"
#include "cetech/kernel/memory/memory.h"
#include "cetech/kernel/api/api_system.h"

#include "cetech/kernel/module/module.h"

#include "bgfx/c99/bgfx.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_ecs_a0);
CETECH_DECL_API(ct_cdb_a0);
CETECH_DECL_API(ct_ebus_a0);


#define _G CameraGlobal
static struct CameraGlobal {
    uint64_t type;
    struct ct_alloc *allocator;
} CameraGlobal;

static void _camera_compiler(struct ct_cdb_obj_t *event) {
    const char *filename = ct_cdb_a0.read_str(event, CT_ID64_0("filename"), "");
    struct ct_cdb_obj_t *writer  = ct_cdb_a0.read_ref(event, CT_ID64_0("writer"), NULL);
    uint64_t *component_key = ct_cdb_a0.read_ptr(event, CT_ID64_0("component_key"), NULL);
    uint32_t component_key_count = ct_cdb_a0.read_uint32(event, CT_ID64_0("component_key_count"), 0);
    
    
    uint64_t keys[component_key_count + 1];
    memcpy(keys, component_key, sizeof(uint64_t) * component_key_count);

    keys[component_key_count] = ct_yng_a0.key("near");
    float near = ct_ydb_a0.get_float(filename, CETECH_ARR_ARG(keys), 0.0f);

    keys[component_key_count] = ct_yng_a0.key("far");
    float far = ct_ydb_a0.get_float(filename, CETECH_ARR_ARG(keys), 0.0f);

    keys[component_key_count] = ct_yng_a0.key("fov");
    float fov = ct_ydb_a0.get_float(filename, CETECH_ARR_ARG(keys), 0.0f);

    ct_cdb_a0.set_float(writer, PROP_NEAR, near);
    ct_cdb_a0.set_float(writer, PROP_FAR, far);
    ct_cdb_a0.set_float(writer, PROP_FOV, fov);
}

static void get_project_view(struct ct_world world,
                             struct ct_entity camera,
                             float *proj,
                             float *view,
                             int width,
                             int height) {

    struct ct_transform_comp *transform;
    struct ct_camera_component *camera_data;


    transform = ct_ecs_a0.entity_data(world, TRANSFORM_COMPONENT, camera);
    camera_data = ct_ecs_a0.entity_data(world, CAMERA_COMPONENT, camera);

    float ratio = (float) (width) / (float) (height);

//    ct_mat4_look_at(view, transform->position,
//                    (float[]){0.0f, 0.0f, -1.0f},
//                    (float[]){0.0f, 1.0f, 0.0f});

    ct_mat4_proj_fovy(proj,
                      camera_data->fov, ratio,
                      camera_data->near, camera_data->far,
                      bgfx_get_caps()->homogeneousDepth);

    ct_mat4_inverse(view, transform->world);
}

static struct ct_camera_a0 camera_api = {
        .get_project_view = get_project_view,
};


static void _component_spawner(struct ct_cdb_obj_t *event) {
    struct ct_cdb_obj_t*obj = ct_cdb_a0.read_ref(event, CT_ID64_0("obj"), NULL);
    struct ct_camera_component *camera= ct_cdb_a0.read_ptr(event, CT_ID64_0("data"), NULL);


    *camera = (struct ct_camera_component) {
            .fov = ct_cdb_a0.read_float(obj, PROP_FOV, 0.0f),
            .near = ct_cdb_a0.read_float(obj, PROP_NEAR, 0.0f),
            .far = ct_cdb_a0.read_float(obj, PROP_FAR, 0.0f),
    };
}

static void _init(struct ct_api_a0 *api) {
    api->register_api("ct_camera_a0", &camera_api);

    _G = (struct _G) {
            .allocator = ct_memory_a0.main_allocator(),
            .type = CAMERA_COMPONENT,
    };

    static struct ct_component_prop_map prop_map[] = {
            {
                    .key = "far",
                    .ui_name = "far",
                    .type = CDB_TYPE_FLOAT,
                    .offset = offsetof(struct ct_camera_component, far)
            },
            {
                    .key = "near",
                    .ui_name = "near",
                    .type = CDB_TYPE_FLOAT,
                    .offset = offsetof(struct ct_camera_component, near)
            },
            {
                    .key = "fov",
                    .ui_name = "fov",
                    .type = CDB_TYPE_FLOAT,
                    .offset = offsetof(struct ct_camera_component, fov)
            }
    };

    ct_ecs_a0.register_component((struct ct_component_info) {
            .component_name = "camera",
            .size = sizeof(struct ct_camera_component),
            .prop_map = prop_map,
            .prop_count = CT_ARRAY_LEN(prop_map)
    });

    ct_ebus_a0.connect_addr(ECS_EBUS,
                            ECS_COMPONENT_SPAWN,
                            _G.type, _component_spawner, 0);

    ct_ebus_a0.connect_addr(ECS_EBUS,
                            ECS_COMPONENT_COMPILE,
                            _G.type, _camera_compiler, 0);


//    ct_ecs_a0.register_simulation("render", render_simu);
}

static void _shutdown() {
    ct_ebus_a0.disconnect_addr(ECS_EBUS,
                               ECS_COMPONENT_SPAWN,
                               _G.type,
                               _component_spawner);


    ct_ebus_a0.disconnect_addr(ECS_EBUS,
                               ECS_COMPONENT_COMPILE,
                               _G.type,
                               _camera_compiler);
}


CETECH_MODULE_DEF(
        camera,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_transform_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_ecs_a0);
            CETECH_GET_API(api, ct_cdb_a0);
            CETECH_GET_API(api, ct_ebus_a0);
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