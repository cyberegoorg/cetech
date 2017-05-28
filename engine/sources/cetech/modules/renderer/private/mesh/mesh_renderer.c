#include <cetech/core/container/array.inl>
#include <cetech/core/yaml.h>
#include <cetech/core/container/map.inl>
#include <cetech/core/hash.h>
#include <cetech/core/config.h>
#include <cetech/modules/resource/resource.h>

#include <cetech/modules/world/world.h>
#include <cetech/modules/entity/entity.h>
#include <cetech/modules/component/component.h>
#include "../../renderer.h"
#include <bgfx/c99/bgfx.h>
#include "../../../transform/transform.h"
#include "../scene/scene.h"
#include "../../../scenegraph/scenegraph.h"
#include <cetech/core/math/mat44f.inl>

#include <cetech/core/memory.h>
#include <cetech/core/module.h>
#include <cetech/core/api.h>


IMPORT_API(memory_api_v0);
IMPORT_API(scenegprah_api_v0);
IMPORT_API(transform_api_v0);
IMPORT_API(component_api_v0);
IMPORT_API(material_api_v0);
IMPORT_API(mesh_renderer_api_v0);
IMPORT_API(hash_api_v0);


#define LOG_WHERE "mesh_renderer"

ARRAY_PROTOTYPE(material_t)

struct mesh_data {
    uint64_t scene;
    uint64_t mesh;
    uint64_t node;
    uint64_t material;
};

typedef struct {
    MAP_T(uint32_t) ent_idx_map;

    ARRAY_T(uint64_t) scene;
    ARRAY_T(uint64_t) mesh;
    ARRAY_T(uint64_t) node;

    ARRAY_T(material_t) material;
} world_data_t;

ARRAY_PROTOTYPE(world_data_t)

MAP_PROTOTYPE(world_data_t)

#define _G meshGlobal
static struct G {
    uint64_t type;

    MAP_T(world_data_t) world;
} _G = {0};

static void _new_world(world_t world) {
    world_data_t data = {0};

    MAP_INIT(uint32_t, &data.ent_idx_map, memory_api_v0.main_allocator());

    ARRAY_INIT(uint64_t, &data.scene, memory_api_v0.main_allocator());
    ARRAY_INIT(uint64_t, &data.mesh, memory_api_v0.main_allocator());
    ARRAY_INIT(uint64_t, &data.node, memory_api_v0.main_allocator());
    ARRAY_INIT(material_t, &data.material, memory_api_v0.main_allocator());

    MAP_SET(world_data_t, &_G.world, world.h, data);
}

static world_data_t *_get_world_data(world_t world) {
    return MAP_GET_PTR(world_data_t, &_G.world, world.h);
}

static void _destroy_world(world_t world) {
    world_data_t *data = _get_world_data(world);

    MAP_DESTROY(uint32_t, &data->ent_idx_map);

    ARRAY_DESTROY(uint64_t, &data->scene);
    ARRAY_DESTROY(uint64_t, &data->mesh);
    ARRAY_DESTROY(uint64_t, &data->node);
    ARRAY_DESTROY(material_t, &data->material);

}

int _mesh_component_compiler(yaml_node_t body,
                             ARRAY_T(uint8_t) *data) {

    struct mesh_data t_data;

    char tmp_buffer[64] = {0};

    YAML_NODE_SCOPE(scene, body, "scene",
                    yaml_as_string(scene, tmp_buffer,
                                   CETECH_ARRAY_LEN(tmp_buffer));
                            t_data.scene = hash_api_v0.id64_from_str(tmp_buffer);
    );
    YAML_NODE_SCOPE(mesh, body, "mesh",
                    yaml_as_string(mesh, tmp_buffer,
                                   CETECH_ARRAY_LEN(tmp_buffer));
                            t_data.mesh = hash_api_v0.id64_from_str(tmp_buffer);
    );

    YAML_NODE_SCOPE(material, body, "material",
                    yaml_as_string(material, tmp_buffer,
                                   CETECH_ARRAY_LEN(tmp_buffer));
                            t_data.material = hash_api_v0.id64_from_str(
                                    tmp_buffer);
    );

    YAML_NODE_SCOPE(node, body, "node",
                    if (yaml_is_valid(node)) {
                        yaml_as_string(node, tmp_buffer,
                                       CETECH_ARRAY_LEN(tmp_buffer));
                        t_data.node = hash_api_v0.id64_from_str(tmp_buffer);
                    }
    );

    ARRAY_PUSH(uint8_t, data, (uint8_t *) &t_data, sizeof(t_data));

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
        if (MAP_HAS(uint32_t, &world_data->ent_idx_map, ents[i].h)) {
            MAP_REMOVE(uint32_t, &world_data->ent_idx_map, ents[i].h);
        }

        //CETECH_ASSERT("mesh_renderer", MAP_HAS(uint32_t, &world_data->ent_idx_map, ents[i].idx));
    }
}


static void _spawner(world_t world,
                     entity_t *ents,
                     uint32_t *cents,
                     uint32_t *ents_parent,
                     size_t ent_count,
                     void *data) {
    struct mesh_data *tdata = data;

    for (int i = 0; i < ent_count; ++i) {
        mesh_renderer_api_v0.create(world,
                                    ents[cents[i]],
                                    tdata[i].scene,
                                    tdata[i].mesh,
                                    tdata[i].node,
                                    tdata[i].material);
    }
}




int mesh_is_valid(mesh_renderer_t mesh) {
    return mesh.idx != UINT32_MAX;
}

int mesh_has(world_t world,
             entity_t entity) {
    world_data_t *world_data = _get_world_data(world);
    return MAP_HAS(uint32_t, &world_data->ent_idx_map, entity.h);
}

mesh_renderer_t mesh_get(world_t world,
                         entity_t entity) {

    world_data_t *world_data = _get_world_data(world);
    uint32_t idx = MAP_GET(uint32_t, &world_data->ent_idx_map, entity.h,
                           UINT32_MAX);
    return (mesh_renderer_t) {.idx = idx};
}

mesh_renderer_t mesh_create(world_t world,
                            entity_t entity,
                            uint64_t scene,
                            uint64_t mesh,
                            uint64_t node,
                            uint64_t material) {

    world_data_t *data = _get_world_data(world);

    scene_create_graph(world, entity, scene);

    material_t material_instance = material_api_v0.resource_create(material);

    uint32_t idx = (uint32_t) ARRAY_SIZE(&data->material);

    MAP_SET(uint32_t, &data->ent_idx_map, entity.h, idx);

    if (node == 0) {
        node = scene_get_mesh_node(scene, mesh);
    }

    ARRAY_PUSH_BACK(uint64_t, &data->scene, scene);
    ARRAY_PUSH_BACK(uint64_t, &data->mesh, mesh);
    ARRAY_PUSH_BACK(uint64_t, &data->node, node);
    ARRAY_PUSH_BACK(material_t, &data->material, material_instance);

    return (mesh_renderer_t) {.idx = idx};
}

void mesh_render_all(world_t world) {
    world_data_t *data = _get_world_data(world);

    const MAP_ENTRY_T(uint32_t) *ce_it = MAP_BEGIN(uint32_t,
                                                   &data->ent_idx_map);
    const MAP_ENTRY_T(uint32_t) *ce_end = MAP_END(uint32_t, &data->ent_idx_map);
    while (ce_it != ce_end) {
        material_t material = ARRAY_AT(&data->material, ce_it->value);
        uint64_t scene = ARRAY_AT(&data->scene, ce_it->value);
        uint64_t geom = ARRAY_AT(&data->mesh, ce_it->value);

        material_api_v0.use(material);

        entity_t ent = {.h = ce_it->key};

        transform_t t = transform_api_v0.get(world, ent);
        mat44f_t t_w = *transform_api_v0.get_world_matrix(world, t);
        //mat44f_t t_w = MAT44F_INIT_IDENTITY;//*transform_get_world_matrix(world, t);
        mat44f_t node_w = MAT44F_INIT_IDENTITY;
        mat44f_t final_w = MAT44F_INIT_IDENTITY;


        if (scenegprah_api_v0.has(world, ent)) {
            uint64_t name = scene_get_mesh_node(scene, geom);
            if (name != 0) {
                scene_node_t n = scenegprah_api_v0.node_by_name(world, ent,
                                                                name);
                node_w = *scenegprah_api_v0.get_world_matrix(world, n);
            }
        }

        mat44f_mul(&final_w, &node_w, &t_w);

        bgfx_set_transform(&final_w, 1);

        scene_submit(scene, geom);

        material_api_v0.submit(material);

        ++ce_it;
    }
}

material_t mesh_get_material(world_t world,
                             mesh_renderer_t mesh) {
    CETECH_ASSERT(LOG_WHERE, mesh.idx != UINT32_MAX);
    world_data_t *data = _get_world_data(world);

    return ARRAY_AT(&data->material, mesh.idx);
}

void mesh_set_material(world_t world,
                       mesh_renderer_t mesh,
                       uint64_t material) {
    world_data_t *data = _get_world_data(world);

    material_t material_instance = material_api_v0.resource_create(material);
    ARRAY_AT(&data->material, mesh.idx) = material_instance;
}


static void _set_property(world_t world,
                          entity_t entity,
                          uint64_t key,
                          struct property_value value) {

    uint64_t scene = hash_api_v0.id64_from_str("scene");
    uint64_t mesh = hash_api_v0.id64_from_str("mesh");
    uint64_t node = hash_api_v0.id64_from_str("node");
    uint64_t material = hash_api_v0.id64_from_str("material");

    mesh_renderer_t mesh_renderer = mesh_get(world, entity);

    if (key == material) {
        mesh_set_material(world, mesh_renderer,
                          hash_api_v0.id64_from_str(value.value.str));
    }
}

static struct property_value _get_property(world_t world,
                                           entity_t entity,
                                           uint64_t key) {
    uint64_t scene = hash_api_v0.id64_from_str("scene");
    uint64_t mesh = hash_api_v0.id64_from_str("mesh");
    uint64_t node = hash_api_v0.id64_from_str("node");
    uint64_t material = hash_api_v0.id64_from_str("material");

    mesh_renderer_t mesh_r = mesh_get(world, entity);
    world_data_t *data = _get_world_data(world);

    char name_buff[256] = {0};

//    if (key.id == scene.id) {
//        ResourceApiV0.get_filename(name_buff, CEL_ARRAY_LEN(name_buff), scene, ARRAY_AT(&data->scene, mesh_r.idx));
//        char* name = cel_strdup(name_buff, MemSysApiV0.main_scratch_allocator());
//
//        return (struct property_value) {
//                .type= PROPERTY_STRING,
//                .value.str = name
//        };
//    } else if (key.id == mesh.id) {
//        ResourceApiV0.compiler_get_filename(name_buff, CEL_ARRAY_LEN(name_buff), scene, ARRAY_AT(&data->mesh, mesh_r.idx));
//        char* name = cel_strdup(name_buff, MemSysApiV0.main_scratch_allocator());
//
//        return (struct property_value) {
//                .type= PROPERTY_STRING,
//                .value.str = name
//        };
//    } else if (key.id == node.id) {
//        ResourceApiV0.compiler_get_filename(name_buff, CEL_ARRAY_LEN(name_buff), scene, ARRAY_AT(&data->node, mesh_r.idx));
//        char* name = cel_strdup(name_buff, MemSysApiV0.main_scratch_allocator());
//
//        return (struct property_value) {
//                .type= PROPERTY_STRING,
//                .value.str = name
//        };
//    } else if (key.id == material.id) {
//        ResourceApiV0.compiler_get_filename(name_buff, CEL_ARRAY_LEN(name_buff), scene, ARRAY_AT(&data->scene, mesh_r.idx));
//        char* name = cel_strdup(name_buff, MemSysApiV0.main_scratch_allocator());
//
//        return (struct property_value) {
//                .type= PROPERTY_STRING,
//                .value.str = name
//        };
//    }

    return (struct property_value) {.type= PROPERTY_INVALID};
}


static void _init_api(struct api_v0* api){
    static struct mesh_renderer_api_v0 _api = {0};

    _api.is_valid = mesh_is_valid;
    _api.has = mesh_has;
    _api.get = mesh_get;
    _api.create = mesh_create;
    _api.get_material = mesh_get_material;
    _api.set_material = mesh_set_material;
    _api.render_all = mesh_render_all;

    api->register_api("mesh_renderer_api_v0", &_api);
}


static void _init( struct api_v0* api) {
    GET_API(api, component_api_v0);
    GET_API(api, memory_api_v0);
    GET_API(api, material_api_v0);
    GET_API(api, mesh_renderer_api_v0);
    GET_API(api, scenegprah_api_v0);
    GET_API(api, transform_api_v0);
    GET_API(api, hash_api_v0);



    _G = (struct G) {0};

    MAP_INIT(world_data_t, &_G.world, memory_api_v0.main_allocator());

    _G.type = hash_api_v0.id64_from_str("mesh_renderer");

    component_api_v0.component_register_compiler(_G.type,
                                                 _mesh_component_compiler, 10);

    component_api_v0.component_register_type(_G.type, (struct component_clb) {
            .spawner=_spawner,
            .destroyer=_destroyer,
            .on_world_create=_on_world_create,
            .on_world_destroy=_on_world_destroy,
            .set_property=_set_property, .get_property=_get_property
    });
}

static void _shutdown() {
    MAP_DESTROY(world_data_t, &_G.world);

    _G = (struct G) {0};
}

void *mesh_get_module_api(int api) {

    switch (api) {
        case PLUGIN_EXPORT_API_ID: {
            static struct module_api_v0 module = {0};

            module.init = _init;
            module.init_api = _init_api;
            module.shutdown = _shutdown;

            return &module;
        }

        default:
            return NULL;
    }
}
