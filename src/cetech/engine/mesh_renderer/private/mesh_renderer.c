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
#include <cetech/macros.h>

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

void _mesh_component_compiler(struct ct_cdb_obj_t *event) {
    const char *filename = ct_cdb_a0.read_str(event, CT_ID64_0("filename"), "");
    ct_cdb_obj_o *writer = ct_cdb_a0.read_ref(event, CT_ID64_0("writer"),
                                                     NULL);
    uint64_t *component_key = ct_cdb_a0.read_ptr(event,
                                                 CT_ID64_0("component_key"),
                                                 NULL);
    uint32_t component_key_count = ct_cdb_a0.read_uint64(event, CT_ID64_0(
            "component_key_count"), 0);

    uint64_t keys[component_key_count + 3];
    memcpy(keys, component_key, sizeof(uint64_t) * component_key_count);

    keys[component_key_count] = ct_yng_a0.key("scene");
    const char *scene = ct_ydb_a0.get_str(filename, keys,
                                          component_key_count + 1,
                                          "");

    keys[component_key_count] = ct_yng_a0.key("mesh");
    const char *mesh = ct_ydb_a0.get_str(filename, keys,
                                         component_key_count + 1, "");

    keys[component_key_count] = ct_yng_a0.key("material");
    const char *mat = ct_ydb_a0.get_str(filename, keys,
                                        component_key_count + 1, "");

    keys[component_key_count] = ct_yng_a0.key("node");
    const char *node = ct_ydb_a0.get_str(filename, keys,
                                         component_key_count + 1, "");

    ct_cdb_a0.set_uint64(writer, PROP_MESH_ID, CT_ID64_0(mesh));
    ct_cdb_a0.set_uint64(writer, PROP_NODE_ID, CT_ID64_0(node));
    ct_cdb_a0.set_uint64(writer, PROP_MATERIAL_ID, CT_ID32_0(mat));
    ct_cdb_a0.set_uint64(writer, PROP_SCENE_ID, CT_ID32_0(scene));

    ct_cdb_a0.set_str(writer, PROP_MESH, mesh);
    ct_cdb_a0.set_str(writer, PROP_NODE, node);
    ct_cdb_a0.set_str(writer, PROP_MATERIAL, mat);
    ct_cdb_a0.set_str(writer, PROP_SCENE, scene);

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
    transforms = ct_ecs_a0.entities_data(TRANSFORM_COMPONENT, item);

    for (int i = 1; i < n; ++i) {
        struct ct_transform_comp t = transforms[i];
        struct ct_mesh_renderer m = mesh_renderers[i];

        uint64_t scene = m.scene_id;

//        if(!scene) {
//            continue;
//        }

        float final_w[16];
        ct_mat4_identity(final_w);
        ct_mat4_move(final_w, t.world);

        struct ct_resource_id rid = (struct ct_resource_id) {
                .type = CT_ID32_0("scene"),
                .name = (uint32_t) scene,
        };

        struct ct_cdb_obj_t *scene_obj = ct_resource_a0.get(rid);

        uint64_t mesh = m.mesh_id;
        struct ct_cdb_obj_t *geom_obj = ct_cdb_a0.read_ref(scene_obj,
                                                           mesh,
                                                           NULL);

        if (!geom_obj) {
            continue;
        }

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

        ct_dd_a0.set_transform_mtx(t.world);
        ct_dd_a0.draw_axis(0, 0, 0,
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

static void _on_obj_change(struct ct_cdb_obj_t *obj,
                           uint64_t *prop,
                           uint32_t prop_count) {

    ct_cdb_obj_o *writer = NULL;
    for (int k = 0; k < prop_count; ++k) {
        if (prop[k] == PROP_SCENE) {
            const char *str = ct_cdb_a0.read_str(obj, PROP_SCENE, "");

            if (!writer) {
                writer = ct_cdb_a0.write_begin(obj);
            }

            ct_cdb_a0.set_uint64(writer, PROP_SCENE_ID, CT_ID32_0(str));


        } else if (prop[k] == PROP_NODE) {
            const char *str = ct_cdb_a0.read_str(obj, PROP_NODE, "");

            if (!writer) {
                writer = ct_cdb_a0.write_begin(obj);
            }

            ct_cdb_a0.set_uint64(writer, PROP_NODE_ID, CT_ID64_0(str));

        } else if (prop[k] == PROP_MESH) {
            const char *str = ct_cdb_a0.read_str(obj, PROP_MESH, "");

            if (!writer) {
                writer = ct_cdb_a0.write_begin(obj);
            }

            ct_cdb_a0.set_uint64(writer, PROP_MESH_ID, CT_ID64_0(str));


        } else if (prop[k] == PROP_MATERIAL) {
            const char *str = ct_cdb_a0.read_str(obj, PROP_MATERIAL, "");

            if (!writer) {
                writer = ct_cdb_a0.write_begin(obj);
            }

            ct_cdb_a0.set_uint64(writer, PROP_MATERIAL_ID, CT_ID32_0(str));

        } else if (prop[k] == PROP_MATERIAL_ID) {
            uint32_t material_id = ct_cdb_a0.read_uint64(obj,
                                                         PROP_MATERIAL_ID, 0);


            if (!writer) {
                writer = ct_cdb_a0.write_begin(obj);
            }

            ct_cdb_a0.set_ref(writer,
                              PROP_MATERIAL_REF,
                              ct_material_a0.resource_create(material_id));

        }
    }

    if (writer) {
        ct_cdb_a0.write_commit(writer);
    }
}

static void _component_spawner(struct ct_cdb_obj_t *event) {
    struct ct_cdb_obj_t *obj = ct_cdb_a0.read_ref(event,
                                                  CT_ID64_0("obj"),
                                                  NULL);

    struct ct_mesh_renderer *mesh = ct_cdb_a0.read_ptr(event,
                                                       CT_ID64_0("data"),
                                                       NULL);

    *mesh = (struct ct_mesh_renderer) {
            .material = ct_material_a0.resource_create(
                    ct_cdb_a0.read_uint64(obj, PROP_MATERIAL_ID, 0)),

            .mesh_id = ct_cdb_a0.read_uint64(obj, PROP_MESH_ID, 0),
            .node_id = ct_cdb_a0.read_uint64(obj, PROP_NODE_ID, 0),
            .scene_id = ct_cdb_a0.read_uint64(obj, PROP_SCENE_ID, 0),
    };

    ct_cdb_a0.register_notify(obj, (ct_cdb_notify) _on_obj_change, NULL);
}


void mesh_combo_items(struct ct_cdb_obj_t *obj,
                      char **items,
                      uint32_t *items_count) {
    uint32_t scene_id = ct_cdb_a0.read_uint64(obj, PROP_SCENE_ID, 0);
    ct_scene_a0.get_all_geometries(scene_id, items, items_count);
}

void node_combo_items(struct ct_cdb_obj_t *obj,
                      char **items,
                      uint32_t *items_count) {
    uint32_t scene_id = ct_cdb_a0.read_uint64(obj, PROP_SCENE_ID, 0);
    ct_scene_a0.get_all_nodes(scene_id, items, items_count);
}

static void _init(struct ct_api_a0 *api) {
    _init_api(api);

    _G = (struct _G) {
            .allocator = ct_memory_a0.main_allocator(),
            .type = MESH_RENDERER_COMPONENT,
    };

    static struct ct_component_prop_map prop_map[] = {
            {
                    .key = "scene",
                    .ui_name = "scene",
                    .type = CDB_TYPE_STR,
                    .offset = UINT64_MAX,
                    .resource.type = "scene",
            },
            {
                    .key = "scene_id",
                    .ui_name = "scene_id",
                    .type = CDB_TYPE_UINT64,
                    .offset = offsetof(struct ct_mesh_renderer, scene_id),
            },
            {
                    .key = "mesh",
                    .ui_name = "mesh",
                    .type = CDB_TYPE_STR,
                    .offset = UINT64_MAX,
                    .combo.combo_items = mesh_combo_items
            },
            {
                    .key = "mesh_id",
                    .ui_name = "mesh_id",
                    .type = CDB_TYPE_UINT64,
                    .offset = offsetof(struct ct_mesh_renderer, mesh_id),
            },
            {
                    .key = "node",
                    .ui_name = "node",
                    .type = CDB_TYPE_STR,
                    .offset = UINT64_MAX,
                    .combo.combo_items = node_combo_items
            },
            {
                    .key = "node_id",
                    .ui_name = "node_id",
                    .type = CDB_TYPE_UINT64,
                    .offset = offsetof(struct ct_mesh_renderer, node_id),
            },
            {
                    .key = "material",
                    .ui_name = "material",
                    .type = CDB_TYPE_STR,
                    .offset = UINT64_MAX,
                    .resource.type = "material",
            },
            {
                    .key = "material_id",
                    .ui_name = "material id",
                    .type = CDB_TYPE_UINT64,
                    .offset = UINT64_MAX,
            },
            {
                    .key = "material_ref",
                    .ui_name = "material ref",
                    .type = CDB_TYPE_REF,
                    .offset = offsetof(struct ct_mesh_renderer, material),
            },

    };

    ct_ecs_a0.register_component(
            (struct ct_component_info) {
                    .component_name = "mesh_renderer",
                    .size = sizeof(struct ct_mesh_renderer),
                    .prop_map = prop_map,
                    .prop_count = CT_ARRAY_LEN(prop_map),
            });

    ct_ebus_a0.connect_addr(ECS_EBUS, ECS_COMPONENT_SPAWN,
                            MESH_RENDERER_COMPONENT, _component_spawner, 0);

    ct_ebus_a0.connect_addr(ECS_EBUS, ECS_COMPONENT_COMPILE,
                            MESH_RENDERER_COMPONENT,
                            _mesh_component_compiler, 0);

}

static void _shutdown() {
    ct_ebus_a0.disconnect_addr(ECS_EBUS, ECS_COMPONENT_SPAWN,
                               MESH_RENDERER_COMPONENT, _component_spawner);

    ct_ebus_a0.disconnect_addr(ECS_EBUS, ECS_COMPONENT_COMPILE,
                               MESH_RENDERER_COMPONENT,
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