#include <string.h>

#include "cetech/kernel/hashlib/hashlib.h"
#include "cetech/kernel/config/config.h"
#include "cetech/kernel/memory/memory.h"
#include <cetech/kernel/module/module.h>
#include <cetech/kernel/yaml/ydb.h>
#include <cetech/kernel/math/fmath.h>
#include <cetech/kernel/ebus/ebus.h>
#include <cetech/kernel/macros.h>
#include "cetech/kernel/api/api_system.h"

#include "cetech/engine/resource/resource.h"
#include "cetech/engine/ecs/ecs.h"

#include "cetech/engine/transform/transform.h"
#include "cetech/engine/scenegraph/scenegraph.h"
#include <cetech/engine/renderer/renderer.h>
#include <cetech/engine/scene/scene.h>
#include <cetech/engine/material/material.h>
#include <cetech/engine/mesh_renderer/mesh_renderer.h>
#include <cetech/engine/debugdraw/debugdraw.h>


CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_scenegprah_a0);
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_material_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_scene_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_ecs_a0);
CETECH_DECL_API(ct_cdb_a0);
CETECH_DECL_API(ct_resource_a0);
CETECH_DECL_API(ct_ebus_a0);
CETECH_DECL_API(ct_dd_a0);
CETECH_DECL_API(ct_renderer_a0);


#define LOG_WHERE "mesh_renderer"

#define _G mesh_render_global

static struct _G {
    uint64_t type;
    struct ct_alloc *allocator;
} _G;

void _mesh_component_compiler(uint32_t ebus,
                              void *event) {
    struct ct_ecs_component_compile_ev *ev = event;

    uint64_t keys[ev->component_key_count + 3];
    memcpy(keys, ev->component_key, sizeof(uint64_t) * ev->component_key_count);

    keys[ev->component_key_count] = ct_yng_a0.key("scene");
    uint64_t scene = CT_ID32_0(ct_ydb_a0.get_str(ev->filename, keys,
                                                 ev->component_key_count + 1,
                                                 NULL));

    keys[ev->component_key_count] = ct_yng_a0.key("mesh");
    const char *mesh = ct_ydb_a0.get_str(ev->filename, keys,
                                         ev->component_key_count + 1, NULL);

    keys[ev->component_key_count] = ct_yng_a0.key("material");
    const char *mat = ct_ydb_a0.get_str(ev->filename, keys,
                                        ev->component_key_count + 1, NULL);

    keys[ev->component_key_count] = ct_yng_a0.key("node");
    const char *node = ct_ydb_a0.get_str(ev->filename, keys,
                                         ev->component_key_count + 1, NULL);

    ct_cdb_a0.set_uint64(ev->writer, PROP_MESH_ID, CT_ID64_0(mesh));
    ct_cdb_a0.set_uint64(ev->writer, PROP_NODE_ID, CT_ID64_0(node));
    ct_cdb_a0.set_uint64(ev->writer, PROP_MATERIAL_ID, CT_ID32_0(mat));
    ct_cdb_a0.set_uint64(ev->writer, PROP_SCENE, scene);
}

struct mesh_render_data {
    uint8_t viewid;
    uint64_t layer_name;
};

void foreach_mesh_renderer(struct ct_world world,
                           struct ct_entity *entities,
                           ct_entity_storage_t *item,
                           uint32_t n,
                           void *_data) {

    struct mesh_render_data *data = _data;

    struct ct_mesh_renderer *mesh_renderers;
    mesh_renderers = ct_ecs_a0.entities_data(MESH_RENDERER_COMPONENT, item);

    struct ct_transform_comp *transforms;
    transforms =  ct_ecs_a0.entities_data(TRANSFORM_COMPONENT, item);

    for (int i = 1; i < n; ++i) {
        struct ct_transform_comp t = transforms[i];
        struct ct_mesh_renderer m = mesh_renderers[i];

        uint64_t scene = m.scene;

        float final_w[16];
        ct_mat4_identity(final_w);
        ct_mat4_move(final_w, t.world);

        struct ct_resource_id rid = (struct ct_resource_id) {
                .type = CT_ID32_0("scene"),
                .name = (uint32_t) scene,
        };

        struct ct_cdb_obj_t *scene_obj = ct_resource_a0.get_obj(rid);

        uint64_t mesh = m.mesh_id;
        struct ct_cdb_obj_t *geom_obj = ct_cdb_a0.read_ref(scene_obj, mesh,
                                                           NULL);

        uint64_t size = ct_cdb_a0.read_uint64(geom_obj, SCENE_SIZE_PROP, 0);
        uint64_t ib = ct_cdb_a0.read_uint64(geom_obj, SCENE_IB_PROP, 0);
        uint64_t vb = ct_cdb_a0.read_uint64(geom_obj, SCENE_VB_PROP, 0);

        ct_render_index_buffer_handle_t ibh = {.idx = (uint16_t) ib};
        ct_render_vertex_buffer_handle_t vbh = {.idx = (uint16_t) vb};

        ct_renderer_a0.set_transform(&final_w, 1);
        ct_renderer_a0.set_vertex_buffer(0, vbh, 0, size);
        ct_renderer_a0.set_index_buffer(ibh, 0, size);

        ct_material_a0.submit(m.material,
                              data->layer_name, data->viewid);

        ct_dd_a0.draw_axis(t.position[0],
                           t.position[1],
                           t.position[2],
                           1.0f,
                           DD_AXIS_COUNT,
                           0.0f);
    }
}

void mesh_render_all(struct ct_world world,
                     uint8_t viewid,
                     uint64_t layer_name) {
    struct mesh_render_data render_data = {.viewid = viewid, .layer_name = layer_name};
    ct_ecs_a0.process(world,
                      ct_ecs_a0.component_mask(MESH_RENDERER_COMPONENT) |
                      ct_ecs_a0.component_mask(TRANSFORM_COMPONENT),
                      foreach_mesh_renderer, &render_data);
}


static struct ct_mesh_renderer_a0 _api = {
        .render_all = mesh_render_all,
};

static void _init_api(struct ct_api_a0 *api) {

    api->register_api("ct_mesh_renderer_a0", &_api);
}


static void _component_spawner(uint32_t ebus,
                               void *event) {
    struct ct_ecs_component_spawn_ev *ev = (event);

    struct ct_mesh_renderer *mesh = (ev->data);

    *mesh = (struct ct_mesh_renderer) {
            .material = ct_material_a0.resource_create(
                    ct_cdb_a0.read_uint64(ev->obj, PROP_MATERIAL_ID, 0)),

            .mesh_id = ct_cdb_a0.read_uint64(ev->obj, PROP_MESH_ID, 0),
            .node_id = ct_cdb_a0.read_uint64(ev->obj, PROP_NODE_ID, 0),
            .scene = ct_cdb_a0.read_uint64(ev->obj, PROP_SCENE, 0),
    };

}

static void _init(struct ct_api_a0 *api) {
    _init_api(api);

    _G = (struct _G){
            .allocator = ct_memory_a0.main_allocator(),
            .type = CT_ID64_0("mesh_renderer"),
    };

    ct_ecs_a0.register_component(
            (struct ct_component_info) {
                    .component_name = "mesh_renderer",
                    .size = sizeof(struct ct_mesh_renderer),
            });

    ct_ebus_a0.connect_addr(ECS_EBUS, ECS_COMPONENT_SPAWN,
                            CT_ID64_0("mesh_renderer"), _component_spawner, 0);

    ct_ebus_a0.connect_addr(ECS_EBUS, ECS_COMPONENT_COMPILE,
                            CT_ID64_0("mesh_renderer"),
                            _mesh_component_compiler, 0);

}

static void _shutdown() {
    ct_ebus_a0.disconnect_addr(ECS_EBUS, ECS_COMPONENT_SPAWN,
                               CT_ID64_0("mesh_renderer"), _component_spawner);

    ct_ebus_a0.disconnect_addr(ECS_EBUS, ECS_COMPONENT_COMPILE,
                               CT_ID64_0("mesh_renderer"),
                               _mesh_component_compiler);
}

static void init(struct ct_api_a0 *api) {
    _init(api);
}

static void shutdown() {
    _shutdown();
}


CETECH_MODULE_DEF(
        mesh_renderer,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_scenegprah_a0);
            CETECH_GET_API(api, ct_transform_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_material_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_scene_a0);
            CETECH_GET_API(api, ct_ecs_a0);
            CETECH_GET_API(api, ct_cdb_a0);
            CETECH_GET_API(api, ct_resource_a0);
            CETECH_GET_API(api, ct_ebus_a0);
            CETECH_GET_API(api, ct_dd_a0);
            CETECH_GET_API(api, ct_renderer_a0);

        },
        {
            CT_UNUSED(reload);
            init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);

            shutdown();
        }
)