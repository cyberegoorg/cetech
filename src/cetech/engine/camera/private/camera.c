#include <cetech/core/cdb/cdb.h>
#include <cetech/engine/ecs/ecs.h>
#include <cetech/engine/renderer/renderer.h>
#include <cetech/engine/transform/transform.h>
#include <cetech/core/yaml/yng.h>
#include <cetech/core/yaml/ydb.h>
#include <cetech/engine/renderer/viewport.h>
#include <cetech/engine/camera/camera.h>

#include <cetech/macros.h>
#include <cetech/core/containers/array.h>
#include <cetech/core/math/fmath.h>
#include <cetech/core/ebus/ebus.h>

#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/config/config.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"

#include "cetech/core/module/module.h"

#include <bgfx/c99/bgfx.h>

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_ecs_a0);
CETECH_DECL_API(ct_cdb_a0);
CETECH_DECL_API(ct_viewport_a0);
CETECH_DECL_API(ct_ebus_a0);


#define _G CameraGlobal
static struct CameraGlobal {
    uint64_t type;
    struct ct_alloc *allocator;
} CameraGlobal;

static void _camera_compiler(uint32_t ebus,void *event) {
    struct ct_ecs_component_compile_ev* ev = event;

    uint64_t keys[ev->component_key_count + 1];
    memcpy(keys, ev->component_key, sizeof(uint64_t) * ev->component_key_count);

    keys[ev->component_key_count] = ct_yng_a0.key("near");
    float near = ct_ydb_a0.get_float(ev->filename, CETECH_ARR_ARG(keys), 0.0f);

    keys[ev->component_key_count] = ct_yng_a0.key("far");
    float far = ct_ydb_a0.get_float(ev->filename, CETECH_ARR_ARG(keys), 0.0f);

    keys[ev->component_key_count] = ct_yng_a0.key("fov");
    float fov = ct_ydb_a0.get_float(ev->filename, CETECH_ARR_ARG(keys), 0.0f);

    ct_cdb_a0.set_float(ev->writer, PROP_NEAR, near);
    ct_cdb_a0.set_float(ev->writer, PROP_FAR, far);
    ct_cdb_a0.set_float(ev->writer, PROP_FOV, fov);
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


struct cameras {
    struct ct_camera_component camera_data[32];
    struct ct_entity ent[32];
    uint32_t n;
};

void foreach_camera(struct ct_world world,
                        struct ct_entity *ent,
                        ct_entity_storage_t *item,
                        uint32_t n,
                        void *data) {
    struct cameras* cameras = data;

    struct ct_camera_component *camera_data;
    camera_data = ct_ecs_a0.component_data(CAMERA_COMPONENT, item);

    for (int i = 1; i < n; ++i) {
        uint32_t idx = cameras->n++;

        cameras->ent[idx].h = ent[i].h;

        memcpy(cameras->camera_data+idx,
               camera_data+i,
               sizeof(struct ct_camera_component));
    }
}


static void render_simu(struct ct_world world,
                        float dt) {

    struct cameras cameras = {{{0}}};

    ct_ecs_a0.process(world,
                         ct_ecs_a0.component_mask(CAMERA_COMPONENT),
                         foreach_camera, &cameras);


    for (int i = 0; i < cameras.n; ++i) {
        ct_viewport_a0.render_world(world, cameras.ent[i],
                                    cameras.camera_data[i].viewport);
    }
}

static void _component_spawner(uint32_t ebus,
                               void *event) {
    struct ct_ecs_component_spawn_ev *ev = event;
    struct ct_camera_component *camera = ev->data;

    struct ct_viewport v = ct_viewport_a0.create(CT_ID64_0("default"), 0, 0);

    *camera = (struct ct_camera_component) {
            .fov = ct_cdb_a0.read_float(ev->obj, PROP_FOV, 0.0f),
            .near = ct_cdb_a0.read_float(ev->obj, PROP_NEAR, 0.0f),
            .far = ct_cdb_a0.read_float(ev->obj, PROP_FAR, 0.0f),
            .viewport = v,
    };
}

static void _init(struct ct_api_a0 *api) {
    api->register_api("ct_camera_a0", &camera_api);

    _G = (struct _G) {
            .allocator = ct_memory_a0.main_allocator(),
            .type = CAMERA_COMPONENT,
    };

    struct ct_component_prop_map prop_map[] = {
            {.key = PROP_FAR, .offset = offsetof(struct ct_camera_component, far)},
            {.key = PROP_NEAR, .offset = offsetof(struct ct_camera_component, near)},
            {.key = PROP_FOV, .offset = offsetof(struct ct_camera_component, fov)}
    };

    ct_ecs_a0.register_component((struct ct_component_info) {
            .component_name = "camera",
            .size = sizeof(struct ct_camera_component),
            .prop_map = prop_map,
            .prop_count = CT_ARRAY_LEN(prop_map)
    });

    ct_ebus_a0.connect_addr(ECS_EBUS, ECS_COMPONENT_SPAWN,
                            CT_ID64_0("camera"), _component_spawner, 0);


    ct_ebus_a0.connect_addr(ECS_EBUS, ECS_COMPONENT_COMPILE,
                            CT_ID64_0("camera"), _camera_compiler, 0);


    ct_ecs_a0.register_simulation("render", render_simu);
}

static void _shutdown() {
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
            CETECH_GET_API(api, ct_viewport_a0);
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