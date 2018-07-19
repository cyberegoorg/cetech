#include <string.h>

#include "corelib/hashlib.h"
#include "corelib/config.h"
#include "corelib/memory.h"
#include <corelib/module.h>
#include <corelib/ydb.h>
#include <corelib/fmath.inl>
#include <corelib/ebus.h>
#include <corelib/macros.h>
#include "corelib/api_system.h"

#include "cetech/resource/resource.h"
#include "cetech/ecs/ecs.h"

#include "cetech/transform/transform.h"
#include "cetech/scenegraph/scenegraph.h"
#include <cetech/renderer/renderer.h>
#include <cetech/scene/scene.h>
#include <cetech/material/material.h>
#include <cetech/mesh_renderer/mesh_renderer.h>
#include <cetech/debugdraw/debugdraw.h>
#include <corelib/macros.h>
#include <stdlib.h>
#include <cetech/debugui/private/iconfontheaders/icons_font_awesome.h>
#include <corelib/yng.h>
#include <cetech/editor_ui/editor_ui.h>


#define LOG_WHERE "mesh_renderer"

#define _G mesh_render_global

static struct _G {
    struct ct_alloc *allocator;
} _G;

void _mesh_component_compiler(const char *filename,
                              uint64_t *component_key,
                              uint32_t component_key_count,
                              ct_cdb_obj_o *writer) {

    uint64_t keys[component_key_count + 3];
    memcpy(keys, component_key, sizeof(uint64_t) * component_key_count);

    keys[component_key_count] = ct_yng_a0->key("scene");
    const char *scene = ct_ydb_a0->get_str(filename, keys,
                                           component_key_count + 1,
                                           "");

    keys[component_key_count] = ct_yng_a0->key("mesh");
    const char *mesh = ct_ydb_a0->get_str(filename, keys,
                                          component_key_count + 1, "");

    keys[component_key_count] = ct_yng_a0->key("material");
    const char *mat = ct_ydb_a0->get_str(filename, keys,
                                         component_key_count + 1, "");

    keys[component_key_count] = ct_yng_a0->key("node");
    const char *node = ct_ydb_a0->get_str(filename, keys,
                                          component_key_count + 1, "");

    ct_cdb_a0->set_uint64(writer, PROP_MESH_ID, ct_hashlib_a0->id64(mesh));
    ct_cdb_a0->set_uint64(writer, PROP_NODE_ID, ct_hashlib_a0->id64(node));
    ct_cdb_a0->set_uint64(writer, PROP_MATERIAL_ID, ct_hashlib_a0->id64(mat));
    ct_cdb_a0->set_uint64(writer, PROP_SCENE_ID, ct_hashlib_a0->id64(scene));

    ct_cdb_a0->set_str(writer, PROP_MESH, mesh);
    ct_cdb_a0->set_str(writer, PROP_NODE, node);
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

    struct ct_mesh *mesh_renderers = \
        ct_ecs_a0->component->get_all(MESH_RENDERER_COMPONENT, item);

    struct ct_transform_comp *transforms;
    transforms = ct_ecs_a0->component->get_all(TRANSFORM_COMPONENT, item);

    for (int i = 1; i < n; ++i) {
        struct ct_transform_comp t = transforms[i];
        struct ct_mesh m = mesh_renderers[i];

        uint64_t scene = m.scene_id;

        if (!scene) {
            continue;
        }

        float final_w[16];
        ct_mat4_identity(final_w);
        ct_mat4_move(final_w, t.world);

        struct ct_resource_id rid = (struct ct_resource_id) {
                .type = SCENE_TYPE,
                .name = scene,
        };

        uint64_t scene_obj = ct_resource_a0->get(rid);

        uint64_t mesh = m.mesh_id;
        uint64_t geom_obj = ct_cdb_a0->read_ref(scene_obj, mesh, 0);

        if (!geom_obj) {
            continue;
        }

        uint64_t size = ct_cdb_a0->read_uint64(geom_obj, SCENE_SIZE_PROP, 0);
        uint64_t ib = ct_cdb_a0->read_uint64(geom_obj, SCENE_IB_PROP, 0);
        uint64_t vb = ct_cdb_a0->read_uint64(geom_obj, SCENE_VB_PROP, 0);

        ct_render_index_buffer_handle_t ibh = {.idx = (uint16_t) ib};
        ct_render_vertex_buffer_handle_t vbh = {.idx = (uint16_t) vb};

        ct_renderer_a0->set_transform(&final_w, 1);
        ct_renderer_a0->set_vertex_buffer(0, vbh, 0, size);
        ct_renderer_a0->set_index_buffer(ibh, 0, size);

        ct_material_a0->submit(m.material, data->layer_name, data->viewid);

        ct_dd_a0->set_transform_mtx(t.world);
        ct_dd_a0->draw_axis(0, 0, 0, 1.0f, DD_AXIS_COUNT, 0.0f);
    }
}

void mesh_render_all(struct ct_world world,
                     uint8_t viewid,
                     uint64_t layer_name) {
    struct mesh_render_data render_data = {.viewid = viewid, .layer_name = layer_name};
    ct_ecs_a0->system->process(
            world,
            ct_ecs_a0->component->mask(MESH_RENDERER_COMPONENT) |
            ct_ecs_a0->component->mask(TRANSFORM_COMPONENT),
            foreach_mesh_renderer, &render_data);
}


static struct ct_mesh_renderer_a0 _api = {
        .render_all = mesh_render_all,
};

struct ct_mesh_renderer_a0 *ct_mesh_renderer_a0 = &_api;

static void _init_api(struct ct_api_a0 *api) {

    api->register_api("ct_mesh_renderer_a0", &_api);
}

static void _on_obj_change(uint64_t obj,
                           uint64_t *prop,
                           uint32_t prop_count) {

    uint64_t ent_obj = ct_cdb_a0->parent(ct_cdb_a0->parent(obj));

    struct ct_world world = {
            .h = ct_cdb_a0->read_uint64(ent_obj, ENTITY_WORLD, 0)
    };

    struct ct_entity ent = {
            .h = ent_obj
    };

    struct ct_mesh *mr;
    mr = ct_ecs_a0->component->get_one(world, MESH_RENDERER_COMPONENT, ent);

    ct_cdb_obj_o *writer = NULL;
    for (int k = 0; k < prop_count; ++k) {
        switch (prop[k]) {
            case PROP_NODE : {
                const char *str = ct_cdb_a0->read_str(obj, PROP_NODE, "");

                if (!writer) {
                    writer = ct_cdb_a0->write_begin(obj);
                }

                ct_cdb_a0->set_uint64(writer, PROP_NODE_ID,
                                      ct_hashlib_a0->id64(str));
                break;
            }
            case PROP_MESH : {
                const char *str = ct_cdb_a0->read_str(obj, PROP_MESH, "");

                if (!writer) {
                    writer = ct_cdb_a0->write_begin(obj);
                }

                ct_cdb_a0->set_uint64(writer, PROP_MESH_ID,
                                      ct_hashlib_a0->id64(str));
                break;

            }
            case PROP_MATERIAL_ID: {
                uint64_t material_id = ct_cdb_a0->read_uint64(obj,
                                                              PROP_MATERIAL_ID,
                                                              0);

                if (!writer) {
                    writer = ct_cdb_a0->write_begin(obj);
                }

                ct_cdb_a0->set_ref(writer,
                                   PROP_MATERIAL_REF,
                                   ct_material_a0->create(material_id));
                break;
            }

            case PROP_MATERIAL_REF: {
                mr->material = ct_cdb_a0->read_uint64(obj,
                                                      PROP_MATERIAL_REF, 0);
                break;
            }

            case PROP_MESH_ID: {
                mr->mesh_id = ct_cdb_a0->read_uint64(obj, PROP_MESH_ID, 0);
                break;
            }

            case PROP_SCENE_ID: {
                mr->scene_id = ct_cdb_a0->read_uint64(obj, PROP_SCENE_ID, 0);
                break;
            }

        }
    }

    if (writer) {
        ct_cdb_a0->write_commit(writer);
    }
}

static void _component_spawner(uint64_t obj,
                               void *data) {
    struct ct_mesh *mesh = data;

    *mesh = (struct ct_mesh) {
            .material = ct_material_a0->create(
                    ct_cdb_a0->read_uint64(obj, PROP_MATERIAL_ID, 0)),

            .mesh_id = ct_cdb_a0->read_uint64(obj, PROP_MESH_ID, 0),
            .node_id = ct_cdb_a0->read_uint64(obj, PROP_NODE_ID, 0),
            .scene_id = ct_cdb_a0->read_uint64(obj, PROP_SCENE_ID, 0),
    };

    ct_cdb_a0->register_notify(obj, (ct_cdb_notify) _on_obj_change, NULL);
}


void mesh_combo_items(uint64_t obj,
                      char **items,
                      uint32_t *items_count) {
    uint64_t scene_id = ct_cdb_a0->read_uint64(obj, PROP_SCENE_ID, 0);
    ct_scene_a0->get_all_geometries(scene_id, items, items_count);
}

void node_combo_items(uint64_t obj,
                      char **items,
                      uint32_t *items_count) {
    uint64_t scene_id = ct_cdb_a0->read_uint64(obj, PROP_SCENE_ID, 0);
    ct_scene_a0->get_all_nodes(scene_id, items, items_count);
}

static uint64_t cdb_type() {
    return MESH_RENDERER_COMPONENT;
}

static const char *display_name() {
    return ICON_FA_HOUZZ " Mesh renderer";
}

static void property_editor(uint64_t obj) {
    ct_editor_ui_a0->ui_resource(obj,
                                 PROP_SCENE_ID, "Scene",
                                 ct_hashlib_a0->id64("scene"),
                                 obj);

    ct_editor_ui_a0->ui_resource(obj,
                                 PROP_MATERIAL_ID, "Material",
                                 ct_hashlib_a0->id64("material"),
                                 obj);

    ct_editor_ui_a0->ui_str_combo(obj,
                                  PROP_MESH, "Mesh",
                                  mesh_combo_items,
                                  obj);

    ct_editor_ui_a0->ui_str_combo(obj,
                                  PROP_NODE, "Node",
                                  node_combo_items,
                                  obj);
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
    return sizeof(struct ct_mesh);
}

static struct ct_component_i0 ct_component_i0 = {
        .size = size,
        .cdb_type = cdb_type,
        .get_interface = get_interface,
        .compiler = _mesh_component_compiler,
        .spawner = _component_spawner,
};

static void _init(struct ct_api_a0 *api) {
    _init_api(api);

    _G = (struct _G) {
            .allocator = ct_memory_a0->system,
    };

    api->register_api("ct_component_i0", &ct_component_i0);

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
            CT_INIT_API(api, ct_memory_a0);
            CT_INIT_API(api, ct_scenegprah_a0);
            CT_INIT_API(api, ct_hashlib_a0);
            CT_INIT_API(api, ct_material_a0);
            CT_INIT_API(api, ct_yng_a0);
            CT_INIT_API(api, ct_ydb_a0);
            CT_INIT_API(api, ct_scene_a0);
            CT_INIT_API(api, ct_ecs_a0);
            CT_INIT_API(api, ct_cdb_a0);
            CT_INIT_API(api, ct_resource_a0);
            CT_INIT_API(api, ct_ebus_a0);
            CT_INIT_API(api, ct_dd_a0);
            CT_INIT_API(api, ct_renderer_a0);

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