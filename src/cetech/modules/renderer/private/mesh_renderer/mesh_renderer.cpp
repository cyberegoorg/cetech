#include "celib/array.inl"
#include "celib/map.inl"

#include "cetech/core/yaml/yaml.h"
#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/config/config.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"

#include "cetech/engine/resource/resource.h"

#include "cetech/engine/entity/entity.h"

#include "cetech/modules/transform/transform.h"
#include "cetech/modules/scenegraph/scenegraph.h"

#include <bgfx/bgfx.h>
#include <cetech/core/macros.h>
#include <cetech/core/blob/blob.h>
#include <cetech/modules/renderer/renderer.h>
#include <cetech/modules/renderer/private/scene/scene.h>
#include "celib/fpumath.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_scenegprah_a0);
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_component_a0);
CETECH_DECL_API(ct_material_a0);
CETECH_DECL_API(ct_mesh_renderer_a0);
CETECH_DECL_API(ct_hash_a0);

using namespace celib;

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
        ct_world world;
        uint32_t n;
        uint32_t allocated;
        void *buffer;

        ct_entity *entity;
        uint64_t *scene;
        uint64_t *mesh;
        uint64_t *node;
        ct_material *material;
    };

    static struct MeshRendererGlobal {
        Map<uint32_t> world_map;
        Array<WorldInstance> world_instances;
        Map<uint32_t> ent_map;
        uint64_t type;
    } _G;

    void allocate(WorldInstance &_data,
                  cel_alloc *_allocator,
                  uint32_t sz) {
        //assert(sz > _data.n);

        WorldInstance new_data;
        const unsigned bytes = sz * (
                sizeof(ct_entity)
                + (3 * sizeof(uint64_t))
                + sizeof(ct_material)
        );

        new_data.buffer = CEL_ALLOCATE(_allocator, char, bytes);
        new_data.n = _data.n;
        new_data.allocated = sz;

        new_data.entity = (ct_entity *) (new_data.buffer);
        new_data.scene = (uint64_t *) (new_data.entity + sz);
        new_data.mesh = (uint64_t *) (new_data.scene + sz);
        new_data.node = (uint64_t *) (new_data.mesh + sz);
        new_data.material = (ct_material *) (new_data.node + sz);

        memcpy(new_data.entity, _data.entity, _data.n * sizeof(ct_entity));
        memcpy(new_data.scene, _data.scene, _data.n * sizeof(uint64_t));
        memcpy(new_data.mesh, _data.mesh, _data.n * sizeof(uint64_t));
        memcpy(new_data.node, _data.node, _data.n * sizeof(uint64_t));
        memcpy(new_data.material, _data.material,
               _data.n * sizeof(ct_material));

        CEL_FREE(_allocator, _data.buffer);

        _data = new_data;
    }

    WorldInstance *_get_world_instance(ct_world world) {
        uint32_t idx = map::get(_G.world_map, world.h, UINT32_MAX);

        if (idx != UINT32_MAX) {
            return &_G.world_instances[idx];
        }

        return nullptr;
    }


    void destroy(ct_world world,
                 ct_entity ent) {
        WorldInstance &_data = *_get_world_instance(world);

        uint32_t id = hash_combine(world.h, ent.h);
        uint32_t i = map::get(_G.ent_map, id, UINT32_MAX);

        if (i == UINT32_MAX) {
            return;
        }

        unsigned last = _data.n - 1;
        ct_entity e = _data.entity[i];
        ct_entity last_e = _data.entity[last];

        _data.entity[i] = _data.entity[last];
        _data.scene[i] = _data.scene[last];
        _data.mesh[i] = _data.mesh[last];
        _data.node[i] = _data.node[last];
        _data.material[i] = _data.material[last];

        map::set(_G.ent_map, hash_combine(world.h, last_e.h), i);
        map::remove(_G.ent_map, hash_combine(world.h, e.h));

        --_data.n;
    }

    static void _new_world(ct_world world) {
        uint32_t idx = array::size(_G.world_instances);
        array::push_back(_G.world_instances, WorldInstance());
        _G.world_instances[idx].world = world;
        map::set(_G.world_map, world.h, idx);
    }


    static void _destroy_world(ct_world world) {
        uint32_t idx = map::get(_G.world_map, world.h, UINT32_MAX);
        uint32_t last_idx = array::size(_G.world_instances) - 1;

        ct_world last_world = _G.world_instances[last_idx].world;

        CEL_FREE(ct_memory_a0.main_allocator(),
                 _G.world_instances[idx].buffer);

        _G.world_instances[idx] = _G.world_instances[last_idx];
        map::set(_G.world_map, last_world.h, idx);
        array::pop_back(_G.world_instances);
    }

    int _mesh_component_compiler(yaml_node_t body,
                                 ct_blob *data) {

        struct mesh_data t_data;

        char tmp_buffer[64] = {};

        YAML_NODE_SCOPE(scene, body, "scene",
                        yaml_as_string(scene, tmp_buffer,
                                       CETECH_ARRAY_LEN(tmp_buffer));
                                t_data.scene = ct_hash_a0.id64_from_str(
                                        tmp_buffer);
        );
        YAML_NODE_SCOPE(mesh, body, "mesh",
                        yaml_as_string(mesh, tmp_buffer,
                                       CETECH_ARRAY_LEN(tmp_buffer));
                                t_data.mesh = ct_hash_a0.id64_from_str(
                                        tmp_buffer);
        );

        YAML_NODE_SCOPE(material, body, "material",
                        yaml_as_string(material, tmp_buffer,
                                       CETECH_ARRAY_LEN(tmp_buffer));
                                t_data.material = ct_hash_a0.id64_from_str(
                                        tmp_buffer);
        );

        YAML_NODE_SCOPE(node, body, "node",
                        if (yaml_is_valid(node)) {
                            yaml_as_string(node, tmp_buffer,
                                           CETECH_ARRAY_LEN(tmp_buffer));
                            t_data.node = ct_hash_a0.id64_from_str(tmp_buffer);
                        }
        );

        data->push(data->inst, (uint8_t *) &t_data, sizeof(t_data));

        return 1;
    }

    static void _on_world_create(ct_world world) {
        _new_world(world);
    }

    static void _on_world_destroy(ct_world world) {
        _destroy_world(world);
    }

    static void _destroyer(ct_world world,
                           ct_entity *ents,
                           uint32_t ent_count) {
        for (uint32_t i = 0; i < ent_count; i++) {
            destroy(world, ents[i]);
        }
    }


    static void _spawner(ct_world world,
                         ct_entity *ents,
                         uint32_t *cents,
                         uint32_t *ents_parent,
                         uint32_t ent_count,
                         void *data) {
        CEL_UNUSED(ents_parent);

        struct mesh_data *tdata = (mesh_data *) data;

        for (uint32_t i = 0; i < ent_count; ++i) {
            ct_mesh_renderer_a0.create(world,
                                       ents[cents[i]],
                                       tdata[i].scene,
                                       tdata[i].mesh,
                                       tdata[i].node,
                                       tdata[i].material);
        }
    }
}


int mesh_is_valid(ct_mesh_renderer mesh) {
    return mesh.idx != UINT32_MAX;
}

int mesh_has(ct_world world,
             ct_entity entity) {
    uint32_t idx = hash_combine(world.h, entity.h);

    return map::has(_G.ent_map, idx);
}

ct_mesh_renderer mesh_get(ct_world world,
                          ct_entity entity) {

    uint32_t idx = hash_combine(world.h, entity.h);

    uint32_t component_idx = map::get(_G.ent_map, idx, UINT32_MAX);

    return {.idx = component_idx, .world = world};
}

ct_mesh_renderer mesh_create(ct_world world,
                             ct_entity entity,
                             uint64_t scene,
                             uint64_t mesh,
                             uint64_t node,
                             uint64_t material) {

    WorldInstance *data = _get_world_instance(world);

    uint32_t idx = data->n;
    allocate(*data, ct_memory_a0.main_allocator(), data->n + 1);
    ++data->n;

    scene::create_graph(world, entity, scene);

    ct_material material_instance = ct_material_a0.resource_create(material);

    map::set(_G.ent_map, hash_combine(world.h, entity.h), idx);

    if (node == 0) {
        node = scene::get_mesh_node(scene, mesh);
    }

    data->entity[idx] = entity;

    data->scene[idx] = scene;
    data->mesh[idx] = mesh;
    data->node[idx] = node;
    data->material[idx] = material_instance;

    return (ct_mesh_renderer) {.idx = idx, .world = world};
}

void mesh_render_all(ct_world world) {
    WorldInstance *data = _get_world_instance(world);

    for (uint32_t i = 0; i < data->n; ++i) {

        ct_material material = data->material[i];
        uint64_t scene = data->scene[i];
        uint64_t geom = data->mesh[i];

        ct_material_a0.use(material);

        ct_entity ent = data->entity[i];

        ct_transform t = ct_transform_a0.get(world, ent);

        float wm[16];

        ct_transform_a0.get_world_matrix(t, wm);

        //mat44f_t t_w = MAT44F_INIT_IDENTITY;//*transform_get_world_matrix(world, t);
        float node_w[16];
        float final_w[16];

        celib::mat4_identity(node_w);
        celib::mat4_identity(final_w);

        if (ct_scenegprah_a0.has(world, ent)) {
            uint64_t name = scene::get_mesh_node(scene, geom);
            if (name != 0) {
                ct_scene_node n = ct_scenegprah_a0.node_by_name(world, ent,
                                                                name);
                ct_scenegprah_a0.get_world_matrix(n, node_w);
            }
        }

        celib::mat4_mul(final_w, node_w, wm);

        bgfx::setTransform(&final_w, 1);

        scene::submit(scene, geom);

        ct_material_a0.submit(material);
    }
}

ct_material mesh_get_material(ct_mesh_renderer mesh) {
    WorldInstance *data = _get_world_instance(mesh.world);
    return data->material[mesh.idx];

}

void mesh_set_material(ct_mesh_renderer mesh,
                       uint64_t material) {
    WorldInstance *data = _get_world_instance(mesh.world);
    ct_material material_instance = ct_material_a0.resource_create(material);

    data->material[mesh.idx] = material_instance;
}


static void _set_property(ct_world world,
                          ct_entity entity,
                          uint64_t key,
                          ct_property_value value) {
//
//    uint64_t scene = ct_hash_a0.id64_from_str("scene");
//    uint64_t mesh = ct_hash_a0.id64_from_str("mesh");
//    uint64_t node = ct_hash_a0.id64_from_str("node");
    uint64_t material = ct_hash_a0.id64_from_str("material");

    ct_mesh_renderer mesh_renderer = mesh_get(world, entity);

    if (key == material) {
        mesh_set_material(mesh_renderer,
                          ct_hash_a0.id64_from_str(value.value.str));
    }
}

static ct_property_value _get_property(ct_world world,
                                       ct_entity entity,
                                       uint64_t key) {
    CEL_UNUSED(world, entity, key);
//    uint64_t scene = ct_hash_a0.id64_from_str("scene");
//    uint64_t mesh = ct_hash_a0.id64_from_str("mesh");
//    uint64_t node = ct_hash_a0.id64_from_str("node");
//    uint64_t material = ct_hash_a0.id64_from_str("material");
//
//    ct_mesh_renderer mesh_r = mesh_get(world, entity);
//    WorldInstance *data = _get_world_instance(world);
//
//    char name_buff[256] = {};

//    if (key.id == scene.id) {
//        ResourceApiV0.get_filename(name_buff, CEL_ARRAY_LEN(name_buff), scene, ARRAY_AT(&data->scene, mesh_r.idx));
//        char* name = cel_strdup(name_buff, MemSysApiV0.main_scratch_allocator());
//
//        return (ct_property_value) {
//                .type= PROPERTY_STRING,
//                .value.str = name
//        };
//    } else if (key.id == mesh.id) {
//        ResourceApiV0.compiler_get_filename(name_buff, CEL_ARRAY_LEN(name_buff), scene, ARRAY_AT(&data->mesh, mesh_r.idx));
//        char* name = cel_strdup(name_buff, MemSysApiV0.main_scratch_allocator());
//
//        return (ct_property_value) {
//                .type= PROPERTY_STRING,
//                .value.str = name
//        };
//    } else if (key.id == node.id) {
//        ResourceApiV0.compiler_get_filename(name_buff, CEL_ARRAY_LEN(name_buff), scene, ARRAY_AT(&data->node, mesh_r.idx));
//        char* name = cel_strdup(name_buff, MemSysApiV0.main_scratch_allocator());
//
//        return (ct_property_value) {
//                .type= PROPERTY_STRING,
//                .value.str = name
//        };
//    } else if (key.id == material.id) {
//        ResourceApiV0.compiler_get_filename(name_buff, CEL_ARRAY_LEN(name_buff), scene, ARRAY_AT(&data->scene, mesh_r.idx));
//        char* name = cel_strdup(name_buff, MemSysApiV0.main_scratch_allocator());
//
//        return (ct_property_value) {
//                .type= PROPERTY_STRING,
//                .value.str = name
//        };
//    }

    return (ct_property_value) {.type= PROPERTY_INVALID};
}


static void _init_api(struct ct_api_a0 *api) {
    static struct ct_mesh_renderer_a0 _api = {};

    _api.is_valid = mesh_is_valid;
    _api.has = mesh_has;
    _api.get = mesh_get;
    _api.create = mesh_create;
    _api.get_material = mesh_get_material;
    _api.set_material = mesh_set_material;
    _api.render_all = mesh_render_all;

    api->register_api("ct_mesh_renderer_a0", &_api);
}


static void _init(ct_api_a0 *api) {
    _init_api(api);

    CETECH_GET_API(api, ct_component_a0);
    CETECH_GET_API(api, ct_memory_a0);
    CETECH_GET_API(api, ct_material_a0);
    CETECH_GET_API(api, ct_mesh_renderer_a0);
    CETECH_GET_API(api, ct_scenegprah_a0);
    CETECH_GET_API(api, ct_transform_a0);
    CETECH_GET_API(api, ct_hash_a0);

    _G = {};

    _G.world_map.init(ct_memory_a0.main_allocator());
    _G.world_instances.init(ct_memory_a0.main_allocator());
    _G.ent_map.init(ct_memory_a0.main_allocator());
    _G.type = ct_hash_a0.id64_from_str("mesh_renderer");

    ct_component_a0.register_compiler(_G.type, _mesh_component_compiler, 10);

    ct_component_a0.register_type(_G.type, {
            .spawner=_spawner,
            .destroyer=_destroyer,

            .world_clb.on_created = _on_world_create,
            .world_clb.on_destroy = _on_world_destroy,

            .set_property=_set_property, .get_property=_get_property
    });
}

static void _shutdown() {
    _G.world_map.destroy();
    _G.world_instances.destroy();
    _G.ent_map.destroy();
}

namespace mesh {
    void init(ct_api_a0 *api) {
        _init(api);
    }

    void shutdown() {
        _shutdown();
    }
}
