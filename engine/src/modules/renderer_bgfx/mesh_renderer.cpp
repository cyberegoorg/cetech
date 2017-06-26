#include <cetech/celib/array.inl>
#include <cetech/celib/map.inl>
#include <cetech/celib/mat44f.inl>

#include <cetech/core/yaml.h>
#include <cetech/core/hash.h>
#include <cetech/core/config.h>
#include <cetech/core/memory.h>
#include <cetech/core/module.h>
#include <cetech/core/api.h>

#include <cetech/modules/resource.h>
#include <cetech/modules/world.h>
#include <cetech/modules/entity.h>
#include <cetech/modules/component.h>
#include "cetech/modules/renderer.h"
#include "cetech/modules/transform.h"
#include "cetech/modules/scenegraph.h"

#include <bgfx/c99/bgfx.h>

#include "scene.h"

IMPORT_API(memory_api_v0);
IMPORT_API(scenegprah_api_v0);
IMPORT_API(transform_api_v0);
IMPORT_API(component_api_v0);
IMPORT_API(material_api_v0);
IMPORT_API(mesh_renderer_api_v0);
IMPORT_API(hash_api_v0);

using namespace cetech;

#define LOG_WHERE "mesh_renderer"
#define hash_combine(a, b) ((a * 11)^(b))

struct mesh_data {
    uint64_t scene;
    uint64_t mesh;
    uint64_t node;
    uint64_t material;
};

namespace {
    struct WorldInstance {
        world_t world;
        uint32_t n;
        uint32_t allocated;
        void *buffer;

        entity_t *entity;
        uint64_t *scene;
        uint64_t *mesh;
        uint64_t *node;
        material_t *material;
    };

    static struct MeshRendererGlobal {
        uint64_t type;

        Map<uint32_t> world_map;
        Array<WorldInstance> world_instances;
        Map<uint32_t> ent_map;
    } _G;

    void allocate(WorldInstance &_data,
                  allocator *_allocator,
                  uint32_t sz) {
        //assert(sz > _data.n);

        WorldInstance new_data;
        const unsigned bytes = sz * (
                sizeof(entity_t)
                + (3 * sizeof(uint64_t))
                + sizeof(material_t)
        );

        new_data.buffer = CETECH_ALLOCATE(_allocator, char, bytes);
        new_data.n = _data.n;
        new_data.allocated = sz;

        new_data.entity = (entity_t *) (new_data.buffer);
        new_data.scene = (uint64_t *) (new_data.entity + sz);
        new_data.mesh = (uint64_t *) (new_data.scene + sz);
        new_data.node = (uint64_t *) (new_data.mesh + sz);
        new_data.material = (material_t *) (new_data.node + sz);

        memcpy(new_data.entity, _data.entity, _data.n * sizeof(entity_t));
        memcpy(new_data.scene, _data.scene, _data.n * sizeof(uint64_t));
        memcpy(new_data.mesh, _data.mesh, _data.n * sizeof(uint64_t));
        memcpy(new_data.node, _data.node, _data.n * sizeof(uint64_t));
        memcpy(new_data.material, _data.material, _data.n * sizeof(material_t));

        CETECH_DEALLOCATE(_allocator, _data.buffer);

        _data = new_data;
    }

    WorldInstance *_get_world_instance(world_t world) {
        uint32_t idx = map::get(_G.world_map, world.h, UINT32_MAX);

        if (idx != UINT32_MAX) {
            return &_G.world_instances[idx];
        }

        return nullptr;
    }


    void destroy(world_t world,
                 entity_t ent) {
        WorldInstance &_data = *_get_world_instance(world);

        uint32_t id = hash_combine(world.h, ent.h);
        uint32_t i = map::get(_G.ent_map, id, UINT32_MAX);

        if (i == UINT32_MAX) {
            return;
        }

        unsigned last = _data.n - 1;
        entity_t e = _data.entity[i];
        entity_t last_e = _data.entity[last];

        _data.entity[i] = _data.entity[last];
        _data.scene[i] = _data.scene[last];
        _data.mesh[i] = _data.mesh[last];
        _data.node[i] = _data.node[last];
        _data.material[i] = _data.material[last];

        map::set(_G.ent_map, hash_combine(world.h, last_e.h), i);
        map::remove(_G.ent_map, hash_combine(world.h, e.h));

        --_data.n;
    }

    static void _new_world(world_t world) {
        uint32_t idx = array::size(_G.world_instances);
        array::push_back(_G.world_instances, WorldInstance());
        _G.world_instances[idx].world = world;
        map::set(_G.world_map, world.h, idx);
    }


    static void _destroy_world(world_t world) {
        uint32_t idx = map::get(_G.world_map, world.h, UINT32_MAX);
        uint32_t last_idx = array::size(_G.world_instances) - 1;

        world_t last_world = _G.world_instances[last_idx].world;

        CETECH_DEALLOCATE(memory_api_v0.main_allocator(),
                          _G.world_instances[idx].buffer);

        _G.world_instances[idx] = _G.world_instances[last_idx];
        map::set(_G.world_map, last_world.h, idx);
        array::pop_back(_G.world_instances);
    }

    int _mesh_component_compiler(yaml_node_t body,
                                 blob_v0 *data) {

        struct mesh_data t_data;

        char tmp_buffer[64] = {0};

        YAML_NODE_SCOPE(scene, body, "scene",
                        yaml_as_string(scene, tmp_buffer,
                                       CETECH_ARRAY_LEN(tmp_buffer));
                                t_data.scene = hash_api_v0.id64_from_str(
                                        tmp_buffer);
        );
        YAML_NODE_SCOPE(mesh, body, "mesh",
                        yaml_as_string(mesh, tmp_buffer,
                                       CETECH_ARRAY_LEN(tmp_buffer));
                                t_data.mesh = hash_api_v0.id64_from_str(
                                        tmp_buffer);
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

        data->push(data->inst, (uint8_t *) &t_data, sizeof(t_data));

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
        for (int i = 0; i < ent_count; i++) {
            destroy(world, ents[i]);
        }
    }


    static void _spawner(world_t world,
                         entity_t *ents,
                         uint32_t *cents,
                         uint32_t *ents_parent,
                         size_t ent_count,
                         void *data) {
        struct mesh_data *tdata = (mesh_data *) data;

        for (int i = 0; i < ent_count; ++i) {
            mesh_renderer_api_v0.create(world,
                                        ents[cents[i]],
                                        tdata[i].scene,
                                        tdata[i].mesh,
                                        tdata[i].node,
                                        tdata[i].material);
        }
    }
}


int mesh_is_valid(mesh_renderer_t mesh) {
    return mesh.idx != UINT32_MAX;
}

int mesh_has(world_t world,
             entity_t entity) {
    uint32_t idx = hash_combine(world.h, entity.h);

    return map::has(_G.ent_map, idx);
}

mesh_renderer_t mesh_get(world_t world,
                         entity_t entity) {

    uint32_t idx = hash_combine(world.h, entity.h);

    uint32_t component_idx = map::get(_G.ent_map, idx, UINT32_MAX);

    return {.idx = component_idx};
}

mesh_renderer_t mesh_create(world_t world,
                            entity_t entity,
                            uint64_t scene,
                            uint64_t mesh,
                            uint64_t node,
                            uint64_t material) {

    WorldInstance *data = _get_world_instance(world);

    uint32_t idx = data->n;
    allocate(*data, memory_api_v0.main_allocator(), data->n + 1);
    ++data->n;

    scene_create_graph(world, entity, scene);

    material_t material_instance = material_api_v0.resource_create(material);

    map::set(_G.ent_map, hash_combine(world.h, entity.h), idx);


    if (node == 0) {
        node = scene_get_mesh_node(scene, mesh);
    }

    data->entity[idx] = entity;

    data->scene[idx] = scene;
    data->mesh[idx] = mesh;
    data->node[idx] = node;
    data->material[idx] = material_instance;

    return (mesh_renderer_t) {.idx = idx};
}

void mesh_render_all(world_t world) {
    WorldInstance *data = _get_world_instance(world);

    for (int i = 0; i < data->n; ++i) {

        material_t material = data->material[i];
        uint64_t scene = data->scene[i];
        uint64_t geom = data->mesh[i];

        material_api_v0.use(material);

        entity_t ent = data->entity[i];

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
    }
}

material_t mesh_get_material(world_t world,
                             mesh_renderer_t mesh) {
    WorldInstance *data = _get_world_instance(world);
    return data->material[mesh.idx];

}

void mesh_set_material(world_t world,
                       mesh_renderer_t mesh,
                       uint64_t material) {
    WorldInstance *data = _get_world_instance(world);
    material_t material_instance = material_api_v0.resource_create(material);

    data->material[mesh.idx] = material_instance;
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
    WorldInstance *data = _get_world_instance(world);

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


static void _init_api(struct api_v0 *api) {
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


static void _init(struct api_v0 *api) {
    GET_API(api, component_api_v0);
    GET_API(api, memory_api_v0);
    GET_API(api, material_api_v0);
    GET_API(api, mesh_renderer_api_v0);
    GET_API(api, scenegprah_api_v0);
    GET_API(api, transform_api_v0);
    GET_API(api, hash_api_v0);


    _G = {0};

    _G.world_map.init(memory_api_v0.main_allocator());
    _G.world_instances.init(memory_api_v0.main_allocator());
    _G.ent_map.init(memory_api_v0.main_allocator());
    _G.type = hash_api_v0.id64_from_str("mesh_renderer");

    component_api_v0.register_compiler(_G.type, _mesh_component_compiler, 10);

    component_api_v0.register_type(_G.type, {
            .spawner=_spawner,
            .destroyer=_destroyer,
            .on_world_create=_on_world_create,
            .on_world_destroy=_on_world_destroy,
            .set_property=_set_property, .get_property=_get_property
    });
}

static void _shutdown() {
    _G.world_map.destroy();
    _G.world_instances.destroy();
    _G.ent_map.destroy();
}

extern "C" void *mesh_get_module_api(int api) {

    switch (api) {
        case PLUGIN_EXPORT_API_ID: {
            static struct module_export_api_v0 module = {0};

            module.init = _init;
            module.init_api = _init_api;
            module.shutdown = _shutdown;

            return &module;
        }

        default:
            return NULL;
    }
}
