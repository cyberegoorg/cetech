#include <string.h>

#include "celib/hashlib.h"
#include "celib/config.h"
#include "celib/memory.h"
#include <celib/module.h>
#include <celib/ydb.h>
#include <celib/fmath.inl>
#include <celib/ebus.h>
#include <celib/macros.h>
#include "celib/api_system.h"

#include "cetech/resource/resource.h"
#include "cetech/ecs/ecs.h"

#include "cetech/transform/transform.h"
#include <cetech/renderer/renderer.h>
#include <cetech/scene/scene.h>
#include <cetech/material/material.h>
#include <cetech/mesh/mesh_renderer.h>
#include <cetech/debugdraw/debugdraw.h>
#include <celib/macros.h>
#include <stdlib.h>
#include <cetech/debugui/icons_font_awesome.h>
#include <celib/ydb.h>
#include <cetech/editor/resource_ui.h>
#include <celib/log.h>
#include <celib/buffer.inl>
#include <cetech/editor/property.h>


#define LOG_WHERE "mesh_renderer"

#define _G mesh_render_global

static struct _G {
    struct ce_alloc *allocator;
} _G;


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


    for (int i = 1; i < n; ++i) {
        uint64_t transform = ct_ecs_a0->get_one(world, TRANSFORM_COMPONENT,
                                                entities[i]);
        uint64_t mesh_renderer = ct_ecs_a0->get_one(world,
                                                    MESH_RENDERER_COMPONENT,
                                                    entities[i]);

        const ce_cdb_obj_o *t_reader = ce_cdb_a0->read(transform);
        const ce_cdb_obj_o *mr_reader = ce_cdb_a0->read(mesh_renderer);

        uint64_t scene = ce_id_a0->id64(ce_cdb_a0->read_str(mr_reader,
                                                            PROP_SCENE_ID,
                                                            0));

        if (!scene) {
            continue;
        }

        uint64_t material = ce_id_a0->id64(ce_cdb_a0->read_str(mr_reader,
                                                               PROP_MATERIAL,
                                                               0));

        if (!material) {
            continue;
        }

        float *world = ce_cdb_a0->read_blob(t_reader, PROP_WORLD, NULL, 0);

        float final_w[16];
        ce_mat4_identity(final_w);

        if(world) {
            ce_mat4_move(final_w, world);
        }

        struct ct_resource_id rid = (struct ct_resource_id) {
                .type = SCENE_TYPE,
                .name = scene,
        };

        uint64_t scene_obj = ct_resource_a0->get(rid);

        uint64_t mesh_id = ce_id_a0->id64(ce_cdb_a0->read_str(mr_reader,
                                                           PROP_MESH,
                                                           0));

        const ce_cdb_obj_o *scene_reader = ce_cdb_a0->read(scene_obj);


        uint64_t mesh = mesh_id;
        uint64_t geom_obj = ce_cdb_a0->read_ref(scene_reader, mesh, 0);

        if (!geom_obj) {
            continue;
        }

        const ce_cdb_obj_o *geom_reader = ce_cdb_a0->read(geom_obj);

        uint64_t ib = ce_cdb_a0->read_uint64(geom_reader, SCENE_IB_PROP, 0);
        uint64_t ib_size = ce_cdb_a0->read_uint64(geom_reader, SCENE_IB_SIZE, 0);
        uint64_t vb = ce_cdb_a0->read_uint64(geom_reader, SCENE_VB_PROP, 0);
        uint64_t vb_size = ce_cdb_a0->read_uint64(geom_reader, SCENE_VB_SIZE, 0);

        ct_render_index_buffer_handle_t ibh = {.idx = (uint16_t) ib};
        ct_render_vertex_buffer_handle_t vbh = {.idx = (uint16_t) vb};

        ct_renderer_a0->set_transform(&final_w, 1);
        ct_renderer_a0->set_vertex_buffer(0, vbh, 0, vb_size);
        ct_renderer_a0->set_index_buffer(ibh, 0, ib_size);

        struct ct_resource_id material_resource = {.type = MATERIAL_TYPE, .name = material};
        uint64_t material_obj = ct_resource_a0->get(material_resource);

        ct_material_a0->submit(material_obj, data->layer_name, data->viewid);

        ct_dd_a0->set_transform_mtx(world);
        ct_dd_a0->draw_axis(0, 0, 0, 1.0f, DD_AXIS_COUNT, 0.0f);
    }
}

void mesh_render_all(struct ct_world world,
                     uint8_t viewid,
                     uint64_t layer_name) {

    struct mesh_render_data render_data = {
            .viewid = viewid,
            .layer_name = layer_name
    };

    ct_ecs_a0->process(
            world,
            ct_ecs_a0->mask(MESH_RENDERER_COMPONENT) |
            ct_ecs_a0->mask(TRANSFORM_COMPONENT),
            foreach_mesh_renderer, &render_data);
}


static struct ct_mesh_renderer_a0 _api = {
        .render_all = mesh_render_all,
};

struct ct_mesh_renderer_a0 *ct_mesh_renderer_a0 = &_api;

static void _init_api(struct ce_api_a0 *api) {

    api->register_api("ct_mesh_renderer_a0", &_api);
}

void mesh_combo_items(uint64_t obj,
                      char **items,
                      uint32_t *items_count) {
    const ce_cdb_obj_o *reader = ce_cdb_a0->read(obj);

    const char *scene = ce_cdb_a0->read_str(reader, PROP_SCENE_ID, 0);
    uint64_t scene_id = ce_id_a0->id64(scene);

    if (!scene_id) {
        return;
    }

    ct_scene_a0->get_all_geometries(scene_id, items, items_count);
}

void node_combo_items(uint64_t obj,
                      char **items,
                      uint32_t *items_count) {
    const ce_cdb_obj_o *reader = ce_cdb_a0->read(obj);
    const char *scene = ce_cdb_a0->read_str(reader, PROP_SCENE_ID, 0);
    uint64_t scene_id = ce_id_a0->id64(scene);

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

    ct_resource_ui_a0->ui_resource(obj,
                                   PROP_SCENE_ID, "Scene",
                                   PROP_SCENE_ID,
                                   obj);

    ct_resource_ui_a0->ui_str_combo(obj,
                                    PROP_MESH, "Mesh",
                                    mesh_combo_items,
                                    obj);

    ct_resource_ui_a0->ui_str_combo(obj,
                                    PROP_NODE, "Node",
                                    node_combo_items,
                                    obj);

    ct_resource_ui_a0->ui_resource(obj,
                                   ce_id_a0->id64("material"),
                                   "Material",
                                   ce_id_a0->id64("material"),
                                   obj);

}


static struct ct_property_editor_i0 ct_property_editor_i0 = {
        .cdb_type = cdb_type,
        .draw_ui = property_editor,
};

static void *get_interface(uint64_t name_hash) {
    if (EDITOR_COMPONENT == name_hash) {
        static struct ct_editor_component_i0 ct_editor_component_i0 = {
                .display_name = display_name,
        };

        return &ct_editor_component_i0;
    }

    return NULL;
}

//static uint64_t size() {
//    return sizeof(struct ct_mesh);
//}

static void mesh_spawner(struct ct_world world,
                         uint64_t obj) {
//    struct ct_mesh *m = data;
//    *m = (struct ct_mesh) {
//            .mesh_id = ce_id_a0->id64(ce_cdb_a0->read_str(obj, PROP_MESH, 0)),
//            .node_id = ce_id_a0->id64(ce_cdb_a0->read_str(obj, PROP_NODE, 0)),
//            .scene_id = ce_id_a0->id64(ce_cdb_a0->read_str(obj,
//                                                           PROP_SCENE_ID, 0)),
//            .material = ce_id_a0->id64(ce_cdb_a0->read_str(obj,
//                                                           PROP_MATERIAL, 0)),
//    };
}


static struct ct_component_i0 ct_component_i0 = {
        .cdb_type = cdb_type,
        .get_interface = get_interface,
        .spawner = mesh_spawner,
};

static void _init(struct ce_api_a0 *api) {
    _init_api(api);

    _G = (struct _G) {
            .allocator = ce_memory_a0->system,
    };

    api->register_api(COMPONENT_INTERFACE_NAME, &ct_component_i0);
    api->register_api(PROPERTY_EDITOR_INTERFACE_NAME, &ct_property_editor_i0);

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
            CE_INIT_API(api, ce_ebus_a0);
            CE_INIT_API(api, ct_dd_a0);
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