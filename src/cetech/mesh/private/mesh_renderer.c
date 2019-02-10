#include <string.h>

#include "celib/id.h"
#include "celib/config.h"
#include "celib/memory/memory.h"
#include <celib/module.h>
#include <celib/ydb.h>
#include <celib/math/math.h>

#include <celib/macros.h>
#include "celib/api.h"

#include "cetech/resource/resource.h"
#include "cetech/ecs/ecs.h"

#include "cetech/transform/transform.h"
#include <cetech/renderer/renderer.h>
#include <cetech/renderer/gfx.h>
#include <cetech/scene/scene.h>
#include <cetech/material/material.h>
#include <cetech/mesh/mesh_renderer.h>

#include <celib/memory/allocator.h>
#include <celib/macros.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <celib/ydb.h>
#include <cetech/editor/editor_ui.h>
#include <celib/log.h>
#include <celib/containers/buffer.h>
#include <cetech/editor/property.h>
#include <cetech/render_graph/render_graph.h>
#include <cetech/default_rg/default_rg.h>
#include <cetech/camera/camera.h>


#define LOG_WHERE "mesh_renderer"

#define _G mesh_render_global

static struct _G {
    struct ce_alloc_t0 *allocator;
} _G;


struct mesh_render_data {
    uint8_t viewid;
    uint64_t layer_name;
};

void foreach_mesh_renderer(ct_world_t0 world,
                           struct ct_entity_t0 *entities,
                           ct_entity_storage_t *item,
                           uint32_t n,
                           void *_data) {
    struct mesh_render_data *data = _data;

    struct ct_transform_comp *transforms = ct_ecs_a0->get_all(
            TRANSFORM_COMPONENT, item);

    struct ct_mesh_component *mesh_renderers = ct_ecs_a0->get_all(
            MESH_RENDERER_COMPONENT,
            item);

    for (int i = 0; i < n; ++i) {
        struct ct_transform_comp *tr = &transforms[i];
        struct ct_mesh_component *m = &mesh_renderers[i];

        if (!m->scene) {
            continue;
        }

        if (!m->material) {
            continue;
        }

        float final_w[16];
        ce_mat4_identity(final_w);
        ce_mat4_move(final_w, tr->world);

        uint64_t scene_obj = m->scene;
        uint64_t mesh_id = m->mesh;

        const ce_cdb_obj_o0 *scene_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                           scene_obj);

        uint64_t mesh = mesh_id;
        uint64_t geom_obj = ce_cdb_a0->read_ref(scene_reader, mesh, 0);

        if (!geom_obj) {
            continue;
        }

        const ce_cdb_obj_o0 *geom_reader = ce_cdb_a0->read(ce_cdb_a0->db(),
                                                          geom_obj);

        uint64_t ib = ce_cdb_a0->read_uint64(geom_reader, SCENE_IB_PROP, 0);
        uint64_t ib_size = ce_cdb_a0->read_uint64(geom_reader, SCENE_IB_SIZE,
                                                  0);
        uint64_t vb = ce_cdb_a0->read_uint64(geom_reader, SCENE_VB_PROP, 0);
        uint64_t vb_size = ce_cdb_a0->read_uint64(geom_reader, SCENE_VB_SIZE,
                                                  0);

        bgfx_index_buffer_handle_t ibh = {.idx = (uint16_t) ib};
        bgfx_vertex_buffer_handle_t vbh = {.idx = (uint16_t) vb};

        ct_gfx_a0->bgfx_set_transform(&final_w, 1);
        ct_gfx_a0->bgfx_set_vertex_buffer(0, vbh, 0, vb_size);
        ct_gfx_a0->bgfx_set_index_buffer(ibh, 0, ib_size);

        uint64_t material_obj = m->material;

        ct_material_a0->submit(material_obj, data->layer_name, data->viewid);
    }
}

static void _init_api(struct ce_api_a0 *api) {
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
    return MESH_RENDERER_COMPONENT;
}

static const char *display_name() {
    return ICON_FA_HOUZZ " Mesh renderer";
}

static void property_editor(uint64_t obj) {

    ct_editor_ui_a0->prop_resource(obj,
                                   PROP_SCENE_ID, "Scene", PROP_SCENE_ID, obj);

    ct_editor_ui_a0->prop_str_combo(obj,
                                    PROP_MESH, "Mesh", mesh_combo_items, obj);

    ct_editor_ui_a0->prop_str_combo(obj,
                                    PROP_NODE, "Node", node_combo_items, obj);

    ct_editor_ui_a0->prop_resource(obj,
                                   ce_id_a0->id64("material"), "Material",
                                   ce_id_a0->id64("material"), obj + 1);

}


static struct ct_property_editor_i0 property_editor_api = {
        .cdb_type = cdb_type,
        .draw_ui = property_editor,
};

void render(ct_world_t0 world,
            struct ct_rg_builder_t0 *builder) {

    uint8_t viewid = builder->get_layer_viewid(builder, _GBUFFER);


    struct mesh_render_data render_data = {
            .viewid = viewid,
            .layer_name = _GBUFFER,
    };

    ct_ecs_a0->process(world,
                       ct_ecs_a0->mask(MESH_RENDERER_COMPONENT) |
                       ct_ecs_a0->mask(TRANSFORM_COMPONENT),
                       foreach_mesh_renderer, &render_data);
}

static struct ct_renderer_component_i0 ct_renderer_component_i = {
        .render = render
};

static void *get_interface(uint64_t name_hash) {
    if (EDITOR_COMPONENT == name_hash) {
        static struct ct_editor_component_i0 ct_editor_component_i0 = {
                .display_name = display_name,
        };

        return &ct_editor_component_i0;
    } else if (CT_RENDERER_COMPONENT_I == name_hash) {
        return &ct_renderer_component_i;
    }

    return NULL;
}

static uint64_t size() {
    return sizeof(ct_mesh_component);
}

static void mesh_spawner(ct_world_t0 world,
                         uint64_t obj,
                         void *data) {
    const ce_cdb_obj_o0 *r = ce_cdb_a0->read(ce_cdb_a0->db(), obj);
    struct ct_mesh_component *m = data;

    *m = (ct_mesh_component) {
            .mesh = ce_id_a0->id64(ce_cdb_a0->read_str(r, PROP_MESH, 0)),
            .node = ce_id_a0->id64(ce_cdb_a0->read_str(r, PROP_NODE, 0)),
            .scene = ce_cdb_a0->read_ref(r, PROP_SCENE_ID, 0),
            .material = ce_cdb_a0->read_ref(r, PROP_MATERIAL, 0),
    };

}


static struct ct_component_i0 ct_component_api = {
        .cdb_type = cdb_type,
        .size = size,
        .get_interface = get_interface,
        .spawner = mesh_spawner,
};

static void _init(struct ce_api_a0 *api) {
    _init_api(api);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    api->register_api(COMPONENT_INTERFACE, &ct_component_api, sizeof(ct_component_api));
    api->register_api(PROPERTY_EDITOR_INTERFACE, &property_editor_api, sizeof(property_editor_api));
}

static void _shutdown() {
}

static void init(struct ce_api_a0 *api) {
    _init(api);
}

static void shutdown() {
    _shutdown();
}


CE_MODULE_DEF(
        mesh_renderer,
        {
            CE_INIT_API(api, ce_memory_a0);
            CE_INIT_API(api, ce_id_a0);
            CE_INIT_API(api, ct_material_a0);
            CE_INIT_API(api, ce_ydb_a0);
            CE_INIT_API(api, ce_ydb_a0);
            CE_INIT_API(api, ct_scene_a0);
            CE_INIT_API(api, ct_ecs_a0);
            CE_INIT_API(api, ce_cdb_a0);
            CE_INIT_API(api, ct_resource_a0);
            CE_INIT_API(api, ct_renderer_a0);

        },
        {
            CE_UNUSED(reload);
            init(api);
        },
        {
            CE_UNUSED(reload);
            CE_UNUSED(api);

            shutdown();
        }
)