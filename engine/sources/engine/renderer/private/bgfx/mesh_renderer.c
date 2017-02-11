#include <celib/containers/array.h>
#include <celib/yaml/yaml.h>
#include <celib/containers/map.h>
#include <celib/string/stringid.h>

#include <engine/entcom/entcom.h>
#include <engine/renderer/material.h>
#include <bgfx/c99/bgfx.h>
#include <engine/world/transform.h>
#include <engine/renderer/mesh_renderer.h>
#include <engine/renderer/private/bgfx/scene.h>
#include <engine/world/scenegraph.h>
#include <celib/math/mat44f.h>

#include <engine/memory/memsys.h>
#include <engine/plugin/plugin_api.h>
#include "../../types.h"


#define LOG_WHERE "mesh_renderer"

ARRAY_PROTOTYPE(stringid64_t)

ARRAY_PROTOTYPE(material_t)


struct mesh_data {
    stringid64_t scene;
    stringid64_t mesh;
    stringid64_t node;
    stringid64_t material;
};

typedef struct {
    MAP_T(u32) ent_idx_map;

    ARRAY_T(stringid64_t) scene;
    ARRAY_T(stringid64_t) mesh;
    ARRAY_T(stringid64_t) node;

    ARRAY_T(material_t) material;
} world_data_t;

ARRAY_PROTOTYPE(world_data_t)

MAP_PROTOTYPE(world_data_t)

#define _G meshGlobal
static struct G {
    stringid64_t type;

    MAP_T(world_data_t) world;
} _G = {0};


static void _new_world(world_t world) {
    world_data_t data = {0};

    MAP_INIT(u32, &data.ent_idx_map, memsys_main_allocator());

    ARRAY_INIT(stringid64_t, &data.scene, memsys_main_allocator());
    ARRAY_INIT(stringid64_t, &data.mesh, memsys_main_allocator());
    ARRAY_INIT(stringid64_t, &data.node, memsys_main_allocator());
    ARRAY_INIT(material_t, &data.material, memsys_main_allocator());

    MAP_SET(world_data_t, &_G.world, world.h.h, data);
}

static world_data_t *_get_world_data(world_t world) {
    return MAP_GET_PTR(world_data_t, &_G.world, world.h.h);
}

static void _destroy_world(world_t world) {
    world_data_t *data = _get_world_data(world);

    MAP_DESTROY(u32, &data->ent_idx_map);

    ARRAY_DESTROY(stringid64_t, &data->scene);
    ARRAY_DESTROY(stringid64_t, &data->mesh);
    ARRAY_DESTROY(stringid64_t, &data->node);
    ARRAY_DESTROY(material_t, &data->material);

}

int _mesh_component_compiler(yaml_node_t body,
                             ARRAY_T(u8) *data) {

    struct mesh_data t_data;

    char tmp_buffer[64] = {0};

    YAML_NODE_SCOPE(scene, body, "scene",
                    yaml_as_string(scene, tmp_buffer, CEL_ARRAY_LEN(tmp_buffer));
                            t_data.scene = stringid64_from_string(tmp_buffer);
    );
    YAML_NODE_SCOPE(mesh, body, "mesh",
                    yaml_as_string(mesh, tmp_buffer, CEL_ARRAY_LEN(tmp_buffer));
                            t_data.mesh = stringid64_from_string(tmp_buffer);
    );

    YAML_NODE_SCOPE(material, body, "material",
                    yaml_as_string(material, tmp_buffer, CEL_ARRAY_LEN(tmp_buffer));
                            t_data.material = stringid64_from_string(tmp_buffer);
    );

    YAML_NODE_SCOPE(node, body, "node",
                    if (yaml_is_valid(node)) {
                        yaml_as_string(node, tmp_buffer, CEL_ARRAY_LEN(tmp_buffer));
                        t_data.node = stringid64_from_string(tmp_buffer);
                    }
    );

    ARRAY_PUSH(u8, data, (u8 *) &t_data, sizeof(t_data));

    return 1;
}

static void _on_world_create(world_t world) {
    _new_world(world);
}

static void _on_world_destroy(world_t world) {
    _destroy_world(world);
}

static void _destroyer(world_t world,
                       entity_t *ents,
                       size_t ent_count) {
    world_data_t *world_data = _get_world_data(world);

    // TODO: remove from arrays, swap idx -> last AND change size
    for (int i = 0; i < ent_count; ++i) {
        if (MAP_HAS(u32, &world_data->ent_idx_map, ents[i].idx)) {
            MAP_REMOVE(u32, &world_data->ent_idx_map, ents[i].idx);
        }

        //CEL_ASSERT("mesh_renderer", MAP_HAS(u32, &world_data->ent_idx_map, ents[i].idx));
    }
}

static void _spawner(world_t world,
                     entity_t *ents,
                     u32 *cents,
                     u32 *ents_parent,
                     size_t ent_count,
                     void *data) {
    struct mesh_data *tdata = data;

    for (int i = 0; i < ent_count; ++i) {
        mesh_create(world,
                    ents[cents[i]],
                    tdata[i].scene,
                    tdata[i].mesh,
                    tdata[i].node,
                    tdata[i].material);
    }
}

static void _init(get_api_fce_t get_engine_api) {
    _G = (struct G) {0};

    MAP_INIT(world_data_t, &_G.world, memsys_main_allocator());

    _G.type = stringid64_from_string("mesh_renderer");

    component_register_compiler(_G.type, _mesh_component_compiler, 10);

    component_register_type(_G.type, (struct component_clb) {
            .spawner=_spawner, .destroyer=_destroyer,
            .on_world_create=_on_world_create, .on_world_destroy=_on_world_destroy
    });
}

static void _shutdown() {
    MAP_DESTROY(world_data_t, &_G.world);

    _G = (struct G) {0};
}


int mesh_is_valid(mesh_t mesh) {
    return mesh.idx != UINT32_MAX;
}

int mesh_has(world_t world,
             entity_t entity) {
    world_data_t *world_data = _get_world_data(world);
    return MAP_HAS(u32, &world_data->ent_idx_map, entity.h.h);
}

mesh_t mesh_get(world_t world,
                entity_t entity) {

    world_data_t *world_data = _get_world_data(world);
    u32 idx = MAP_GET(u32, &world_data->ent_idx_map, entity.h.h, UINT32_MAX);
    return (mesh_t) {.idx = idx};
}

mesh_t mesh_create(world_t world,
                   entity_t entity,
                   stringid64_t scene,
                   stringid64_t mesh,
                   stringid64_t node,
                   stringid64_t material) {

    world_data_t *data = _get_world_data(world);

    scene_create_graph(world, entity, scene);

    material_t material_instance = material_resource_create(material);

    u32 idx = (u32) ARRAY_SIZE(&data->material);

    MAP_SET(u32, &data->ent_idx_map, entity.h.h, idx);

    if (node.id == 0) {
        node = scene_get_mesh_node(scene, mesh);
    }

    ARRAY_PUSH_BACK(stringid64_t, &data->scene, scene);
    ARRAY_PUSH_BACK(stringid64_t, &data->mesh, mesh);
    ARRAY_PUSH_BACK(stringid64_t, &data->node, node);
    ARRAY_PUSH_BACK(material_t, &data->material, material_instance);

    return (mesh_t) {.idx = idx};
}

void mesh_render_all(world_t world) {
    world_data_t *data = _get_world_data(world);

    const MAP_ENTRY_T(u32) *ce_it = MAP_BEGIN(u32, &data->ent_idx_map);
    const MAP_ENTRY_T(u32) *ce_end = MAP_END(u32, &data->ent_idx_map);
    while (ce_it != ce_end) {
        material_t material = ARRAY_AT(&data->material, ce_it->value);
        stringid64_t scene = ARRAY_AT(&data->scene, ce_it->value);
        stringid64_t geom = ARRAY_AT(&data->mesh, ce_it->value);

        material_use(material);

        entity_t ent = {.idx = ce_it->key};

        transform_t t = transform_get(world, ent);
        cel_mat44f_t t_w = *transform_get_world_matrix(world, t);
        //cel_mat44f_t t_w = MAT44F_INIT_IDENTITY;//*transform_get_world_matrix(world, t);
        cel_mat44f_t node_w = MAT44F_INIT_IDENTITY;
        cel_mat44f_t final_w = MAT44F_INIT_IDENTITY;


        if (scenegraph_has(world, ent)) {
            stringid64_t name = scene_get_mesh_node(scene, geom);
            if (name.id != 0) {
                scene_node_t n = scenegraph_node_by_name(world, ent, name);
                node_w = *scenegraph_get_world_matrix(world, n);
            }
        }

        cel_mat44f_mul(&final_w, &node_w, &t_w);

        bgfx_set_transform(&final_w, 1);

        scene_resource_submit(scene, geom);

        material_submit(material);

        ++ce_it;
    }
}

material_t mesh_get_material(world_t world,
                             mesh_t mesh) {
    CEL_ASSERT(LOG_WHERE, mesh.idx != UINT32_MAX);
    world_data_t *data = _get_world_data(world);

    return ARRAY_AT(&data->material, mesh.idx);
}

void mesh_set_material(world_t world,
                       mesh_t mesh,
                       stringid64_t material) {
    world_data_t *data = _get_world_data(world);

    material_t material_instance = material_resource_create(material);
    ARRAY_AT(&data->material, mesh.idx) = material_instance;
}


void *mesh_get_plugin_api(int api,
                          int version) {

    switch (api) {
        case PLUGIN_API_ID:
            switch (version) {
                case 0: {
                    static struct plugin_api_v0 plugin = {0};

                    plugin.init = _init;
                    plugin.shutdown = _shutdown;

                    return &plugin;
                }

                default:
                    return NULL;
            };
        case MESH_API_ID:
            switch (version) {
                case 0: {
                    static struct MeshApiV1 api = {0};

                    api.is_valid = mesh_is_valid;
                    api.has = mesh_has;
                    api.get = mesh_get;
                    api.create = mesh_create;
                    api.get_material = mesh_get_material;
                    api.set_material = mesh_set_material;
                    api.render_all = mesh_render_all;

                    return &api;
                }

                default:
                    return NULL;
            };

        default:
            return NULL;
    }
}
