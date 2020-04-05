#include <string.h>

#include "celib/id.h"
#include "celib/memory/memory.h"
#include <celib/module.h>
#include <celib/yaml_cdb.h>

#include <celib/macros.h>
#include "celib/api.h"

#include "cetech/asset/asset.h"
#include "cetech/ecs/ecs.h"

#include "cetech/transform/transform.h"
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/scene/scene.h>
#include <cetech/material/material.h>
#include <cetech/mesh/static_mesh.h>

#include <celib/memory/allocator.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <cetech/editor/editor_ui.h>
#include <cetech/property_editor/property_editor.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/default_rg/default_rg.h>
#include <cetech/camera/camera.h>


#define LOG_WHERE "static_mesh"

#define _G mesh_render_global

static struct _G {
    ce_alloc_t0 *allocator;
} _G;

typedef struct mesh_render_data {
    uint8_t viewid;
    uint64_t layer_name;
} mesh_render_data;

void render_static_mesh(ct_world_t0 world,
                        struct ct_entity_t0 *entities,
                        ct_ecs_ent_chunk_o0 *item,
                        uint32_t n,
                        void *_data) {
     mesh_render_data *data = _data;

    ct_local_to_world_c *transforms = ct_ecs_c_a0->get_all(world, LOCAL_TO_WORLD_COMPONENT, item);
    ct_mesh_component *static_meshs = ct_ecs_c_a0->get_all(world, STATIC_MESH_COMPONENT, item);

    for (int i = 0; i < n; ++i) {
        ct_local_to_world_c t_c = transforms[i];
        ct_mesh_component m_c = static_meshs[i];

        if (!m_c.scene || !m_c.material) {
            continue;
        }

        uint64_t geom_obj = ct_scene_a0->get_geom_obj(m_c.scene, m_c.mesh);

        if (!geom_obj) {
            continue;
        }

        ct_scene_geom_obj_t0 go = {};
        ce_cdb_a0->read_to(ce_cdb_a0->db(), geom_obj, &go, sizeof(go));

        bgfx_index_buffer_handle_t ibh = {.idx = (uint16_t) go.ib};
        bgfx_vertex_buffer_handle_t vbh = {.idx = (uint16_t) go.vb};

        ct_gfx_a0->bgfx_set_transform(t_c.world.m, 1);

        ct_gfx_a0->bgfx_set_vertex_buffer(0, vbh, 0, (uint32_t) go.vb_size);
        ct_gfx_a0->bgfx_set_index_buffer(ibh, 0, (uint32_t) go.ib_size);

        ct_material_a0->submit(m_c.material, data->layer_name, data->viewid);
    }
}


void mesh_combo_items(uint64_t obj,
                      char **items,
                      uint32_t *items_count) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    uint64_t scene_id = ce_cdb_a0->read_ref(reader, PROP_SCENE_ID, 0);

    if (!scene_id) {
        return;
    }

    ct_scene_a0->get_all_geometries(scene_id, items, items_count);
}

void node_combo_items(uint64_t obj,
                      char **items,
                      uint32_t *items_count) {
    const ce_cdb_obj_o0 *reader = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    uint64_t scene_id = ce_cdb_a0->read_ref(reader, PROP_SCENE_ID, 0);

    if (!scene_id) {
        return;
    }

    ct_scene_a0->get_all_nodes(scene_id, items, items_count);
}

static uint64_t cdb_type() {
    return ce_id_a0->id64("static_mesh");
}

static const char *display_name() {
    return ICON_FA_HOUZZ " Static mesh";
}

static void property_editor(ce_cdb_t0 db,
                            uint64_t obj,
                            uint64_t context,
                            const char *filter) {
    ct_editor_ui_a0->prop_asset(obj, "Scene", filter,
                                   PROP_SCENE_ID, PROP_SCENE_ID, context, obj);
    ct_editor_ui_a0->prop_str_combo(obj, "Mesh", filter, PROP_MESH, mesh_combo_items, obj);
    ct_editor_ui_a0->prop_str_combo(obj, "Node", filter, PROP_NODE, node_combo_items, obj);
    ct_editor_ui_a0->prop_asset(obj, "Material", filter, MATERIAL_TYPE, MATERIAL_TYPE, context,
                                   obj + 1);
}

static struct ct_property_editor_i0 property_editor_api = {
        .cdb_type = cdb_type,
        .draw_ui = property_editor,
};

void render(ct_world_t0 world,
            struct ct_rg_builder_t0 *builder) {

    uint8_t viewid = builder->get_layer_viewid(builder, _GBUFFER);

//    ct_dd_a0->begin(viewid);

    mesh_render_data render_data = {
            .viewid = viewid,
            .layer_name = _GBUFFER,
    };

    ct_ecs_q_a0->foreach_serial(world,
                                (ct_ecs_query_t0) {
                                        .all = CT_ECS_ARCHETYPE(STATIC_MESH_COMPONENT,
                                                                LOCAL_TO_WORLD_COMPONENT),
                                }, 0,
                                render_static_mesh, &render_data);
}

static struct ct_renderer_component_i0 ct_renderer_component_i = {
        .render = render
};

static void _mesh_render_on_spawn(ct_world_t0 world,
                                  ce_cdb_t0 db,
                                  uint64_t obj,
                                  void *data) {
    ct_mesh_component *c = data;

    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(ce_cdb_a0->db(), obj);

    const char *mesh = ce_cdb_a0->read_str(r, PROP_MESH, 0);
    const char *node = ce_cdb_a0->read_str(r, PROP_NODE, 0);

    *c = (ct_mesh_component) {
            .mesh = ce_id_a0->id64(mesh),
            .node = ce_id_a0->id64(node),
            .scene = ce_cdb_a0->read_ref(r, PROP_SCENE_ID, 0),
            .material = ce_cdb_a0->read_ref(r, PROP_MATERIAL, 0),
    };

}

static struct ct_ecs_component_i0 ct_component_api = {
        .display_name = display_name,
        .cdb_type = STATIC_MESH_COMPONENT,
        .size = sizeof(ct_mesh_component),
        .from_cdb_obj = _mesh_render_on_spawn,
};

static ce_cdb_prop_def_t0 static_mesh_component_prop[] = {
        {.name = "material", .type = CE_CDB_TYPE_REF},
        {.name = "scene", .type = CE_CDB_TYPE_REF},
        {.name = "node", .type = CE_CDB_TYPE_STR},
        {.name = "mesh", .type = CE_CDB_TYPE_STR},
};


void CE_MODULE_LOAD(static_mesh)(struct ce_api_a0 *api,
                                 int reload) {
    CE_UNUSED(reload);
    CE_INIT_API(api, ce_memory_a0);
    CE_INIT_API(api, ce_id_a0);
    CE_INIT_API(api, ct_material_a0);
    CE_INIT_API(api, ce_yaml_cdb_a0);
    CE_INIT_API(api, ce_yaml_cdb_a0);
    CE_INIT_API(api, ct_scene_a0);
    CE_INIT_API(api, ct_ecs_a0);
    CE_INIT_API(api, ce_cdb_a0);
    CE_INIT_API(api, ct_asset_a0);
    CE_INIT_API(api, ct_renderer_a0);


    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    api->add_impl(CT_ECS_COMPONENT_I0_STR, &ct_component_api, sizeof(ct_component_api));
    api->add_impl(CT_RENDERER_COMPONENT_I0_STR, &ct_renderer_component_i,
                  sizeof(ct_renderer_component_i));

    api->add_impl(CT_PROPERTY_EDITOR_I0_STR, &property_editor_api, sizeof(property_editor_api));

    ce_cdb_a0->reg_obj_type(STATIC_MESH_COMPONENT,
                            static_mesh_component_prop,
                            CE_ARRAY_LEN(static_mesh_component_prop));

}

void CE_MODULE_UNLOAD(static_mesh)(struct ce_api_a0 *api,
                                   int reload) {

    CE_UNUSED(reload);
    CE_UNUSED(api);
}
