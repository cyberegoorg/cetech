
#include "cetech/core/containers/map.inl"

#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/config/config.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"

#include "cetech/engine/resource/resource.h"

#include "cetech/engine/ecs/ecs.h"

#include "cetech/engine/transform/transform.h"
#include "cetech/engine/scenegraph/scenegraph.h"

#include <bgfx/bgfx.h>
#include <cetech/macros.h>
#include <cetech/engine/renderer/renderer.h>
#include <cetech/engine/renderer/scene.h>
#include <cetech/engine/renderer/material.h>
#include <cetech/core/module/module.h>
#include <cetech/engine/renderer/mesh_renderer.h>
#include <cetech/core/yaml/ydb.h>
#include <cetech/core/math/fmath.h>
#include <cetech/core/ebus/ebus.h>



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


#define LOG_WHERE "mesh_renderer"

static struct MeshRendererGlobal {
    uint64_t type;
    struct ct_alloc *allocator;
} _G;

void _mesh_component_compiler(uint32_t ebus,void *event) {
    ct_ecs_component_compile_ev* ev = static_cast<ct_ecs_component_compile_ev *>(event);

    uint64_t keys[ev->component_key_count + 3];
    memcpy(keys, ev->component_key, sizeof(uint64_t) * ev->component_key_count);

    keys[ev->component_key_count] = ct_yng_a0.key("scene");
    uint64_t scene = CT_ID32_0(ct_ydb_a0.get_str(ev->filename, keys,
                                                 ev->component_key_count + 1, NULL));

    uint64_t geom[32] = {};
    uint32_t geom_keys_count = 0;

    keys[ev->component_key_count] = ct_yng_a0.key("geometries");
    ct_ydb_a0.get_map_keys(ev->filename,
                           keys, ev->component_key_count + 1,
                           geom, CT_ARRAY_LEN(geom),
                           &geom_keys_count);

    for (uint32_t i = 0; i < geom_keys_count; ++i) {
        keys[ev->component_key_count + 1] = geom[i];

        keys[ev->component_key_count + 2] = ct_yng_a0.key("mesh");
        const char *mesh = ct_ydb_a0.get_str(ev->filename, keys, ev->component_key_count + 3, NULL);

        keys[ev->component_key_count + 2] = ct_yng_a0.key("material");
        const char *mat = ct_ydb_a0.get_str(ev->filename, keys, ev->component_key_count + 3, NULL);

        keys[ev->component_key_count + 2] = ct_yng_a0.key("node");
        const char *node = ct_ydb_a0.get_str(ev->filename, keys, ev->component_key_count + 3, NULL);

        ct_cdb_a0.set_uint64(ev->writer, PROP_MESH_ID + i, CT_ID64_0(mesh));
        ct_cdb_a0.set_uint64(ev->writer, PROP_NODE_ID + i, CT_ID64_0(node));
        ct_cdb_a0.set_uint64(ev->writer, PROP_MATERIAL_ID + i, CT_ID32_0(mat));
    }

    ct_cdb_a0.set_uint64(ev->writer, PROP_SCENE, scene);
    ct_cdb_a0.set_uint64(ev->writer, PROP_GEOM_COUNT, geom_keys_count);
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

    struct mesh_render_data *data = static_cast<mesh_render_data *>(_data);

    ct_mesh_renderer *mesh_renderers;
    mesh_renderers = (ct_mesh_renderer *) ct_ecs_a0.component_data(
            MESH_RENDERER_COMPONENT, item);

    ct_transform_comp *transforms;
    transforms = (ct_transform_comp *) ct_ecs_a0.component_data(
            TRANSFORM_COMPONENT, item);

    for (int i = 1; i < n; ++i) {
        ct_transform_comp t = transforms[i];
        ct_mesh_renderer m = mesh_renderers[i];

        uint64_t scene = m.scene;
        uint64_t kcount = m.geom_n;

        float final_w[16];
        ct_mat4_identity(final_w);
        ct_mat4_move(final_w, t.world);

        struct ct_resource_id rid = (struct ct_resource_id) {
                .type = CT_ID32_0("scene"),
                .name = static_cast<uint32_t>(scene),
        };

        struct ct_cdb_obj_t *scene_obj = ct_resource_a0.get_obj(rid);

        for (int j = 0; j < kcount; ++j) {
            uint64_t mesh = m.geometries[j].mesh_id;
            struct ct_cdb_obj_t *geom_obj = ct_cdb_a0.read_ref(scene_obj, mesh,
                                                               NULL);

            uint64_t size = ct_cdb_a0.read_uint64(geom_obj, SCENE_SIZE_PROP, 0);
            uint64_t ib = ct_cdb_a0.read_uint64(geom_obj, SCENE_IB_PROP, 0);
            uint64_t vb = ct_cdb_a0.read_uint64(geom_obj, SCENE_VB_PROP, 0);

            bgfx::IndexBufferHandle ibh = {.idx = (uint16_t) ib};
            bgfx::VertexBufferHandle vbh = {.idx = (uint16_t) vb};

            bgfx::setTransform(&final_w, 1);
            bgfx::setVertexBuffer(0, vbh, 0, size);
            bgfx::setIndexBuffer(ibh, 0, size);

            ct_material_a0.submit(m.geometries[j].material,
                                  data->layer_name, data->viewid);
        }
    }
}

void mesh_render_all(struct ct_world world,
                     uint8_t viewid,
                     uint64_t layer_name) {
    mesh_render_data render_data = {.viewid = viewid, .layer_name = layer_name};
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
    ct_ecs_component_spawn_ev *ev = static_cast<ct_ecs_component_spawn_ev *>(event);

    struct ct_mesh_renderer *mesh = static_cast<ct_mesh_renderer *>(ev->data);

    *mesh = (struct ct_mesh_renderer) {
            .geom_n = static_cast<uint8_t>(ct_cdb_a0.read_uint64(ev->obj,
                                                                 PROP_GEOM_COUNT,
                                                                 0)),
            .scene = ct_cdb_a0.read_uint64(ev->obj, PROP_SCENE, 0),
    };

    for (int i = 0; i < mesh->geom_n; ++i) {
        mesh->geometries[i].material = ct_material_a0.resource_create(
                ct_cdb_a0.read_uint64(ev->obj,
                                      PROP_MATERIAL_ID +
                                      i, 0));

        mesh->geometries[i].mesh_id = ct_cdb_a0.read_uint64(ev->obj,
                                                            PROP_MESH_ID + i,
                                                            0);
        mesh->geometries[i].node_id = ct_cdb_a0.read_uint64(ev->obj,
                                                            PROP_NODE_ID + i,
                                                            0);
    }
}

static void _init(struct ct_api_a0 *api) {
    _init_api(api);

    _G = {
            .allocator = ct_memory_a0.main_allocator(),
            .type = CT_ID64_0("mesh_renderer"),
    };

    ct_ecs_a0.register_component(
            (struct ct_component_info) {
                    .component_name = "mesh_renderer",
                    .size = sizeof(ct_mesh_renderer),
            });

    ct_ebus_a0.connect_addr(ECS_EBUS, ECS_COMPONENT_SPAWN,
                            CT_ID64_0("mesh_renderer"), _component_spawner, 0);

    ct_ebus_a0.connect_addr(ECS_EBUS, ECS_COMPONENT_COMPILE,
                            CT_ID64_0("mesh_renderer"), _mesh_component_compiler, 0);

}

static void _shutdown() {

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