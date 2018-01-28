
#include "cetech/core/containers/map.inl"
#include "cetech/core/containers/hash.h"

#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/config/config.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"

#include "cetech/engine/resource/resource.h"

#include "cetech/engine/entity/entity.h"

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


CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_scenegprah_a0);
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_component_a0);
CETECH_DECL_API(ct_material_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_scene_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_entity_a0);
CETECH_DECL_API(ct_cdb_a0);
CETECH_DECL_API(ct_resource_a0);

using namespace celib;

#define LOG_WHERE "mesh_renderer"

static uint64_t hash_combine(uint32_t a,
                             uint32_t b) {
    union {
        struct {
            uint32_t a;
            uint32_t b;
        };
        uint64_t ab;
    } c{
            .a = a,
            .b = b,
    };

    return c.ab;
}

struct mesh_data {
    uint64_t scene;
    uint32_t count;
    uint64_t mesh[32];
    uint64_t node[32];
    uint64_t material[32];
    ct_cdb_obj_t *material_inst[32];
};

struct WorldInstance {
    ct_world world;
    uint32_t n;
    uint32_t allocated;
    void *buffer;

    ct_entity *entity;
};

static struct MeshRendererGlobal {
    struct ct_hash_t world_map;
    WorldInstance *world_instances;
    ct_hash_t ent_map;
    uint64_t type;
    ct_alloc *allocator;
} _G;

void allocate(WorldInstance &_data,
              ct_alloc *_allocator,
              uint32_t sz) {
    //assert(sz > _data.n);

    WorldInstance new_data;
    const unsigned bytes = sz * (
            sizeof(ct_entity)
            + sizeof(mesh_data)
    );

    new_data.buffer = CT_ALLOC(_allocator, char, bytes);
    new_data.n = _data.n;
    new_data.allocated = sz;

    new_data.entity = (ct_entity *) (new_data.buffer);

    memcpy(new_data.entity, _data.entity, _data.n * sizeof(ct_entity));

    CT_FREE(_allocator, _data.buffer);

    _data = new_data;
}

WorldInstance *_get_world_instance(ct_world world) {
    uint64_t idx = ct_hash_lookup(&_G.world_map, world.h, UINT64_MAX);

    if (idx != UINT64_MAX) {
        return &_G.world_instances[idx];
    }

    return nullptr;
}


void destroy(ct_world world,
             ct_entity ent) {
    WorldInstance &_data = *_get_world_instance(world);

    uint64_t id = hash_combine(world.h, ent.h);
    uint32_t i = ct_hash_lookup(&_G.ent_map, id, UINT32_MAX);

    if (i == UINT32_MAX) {
        return;
    }

    unsigned last = _data.n - 1;
    ct_entity e = _data.entity[i];
    ct_entity last_e = _data.entity[last];

    _data.entity[i] = _data.entity[last];

    ct_hash_add(&_G.ent_map, hash_combine(world.h, last_e.h), i, _G.allocator);
    ct_hash_remove(&_G.ent_map, hash_combine(world.h, e.h));

    --_data.n;
}

static void _new_world(ct_world world) {

    uint32_t idx = ct_array_size(_G.world_instances);
    ct_array_push(_G.world_instances, WorldInstance(), _G.allocator);
    ct_hash_add(&_G.world_map, world.h, idx, _G.allocator);
    _G.world_instances[idx].world = world;
}


static void _destroy_world(ct_world world) {
    uint64_t idx = ct_hash_lookup(&_G.world_map, world.h, UINT32_MAX);
    uint32_t last_idx = ct_array_size(_G.world_instances) - 1;

    ct_world last_world = _G.world_instances[last_idx].world;

    CT_FREE(ct_memory_a0.main_allocator(),
            _G.world_instances[idx].buffer);

    _G.world_instances[idx] = _G.world_instances[last_idx];
    ct_hash_add(&_G.world_map, last_world.h, idx, _G.allocator);
    ct_array_pop_back(_G.world_instances);
}


int _mesh_component_compiler(const char *filename,
                             uint64_t *component_key,
                             uint32_t component_key_count,
                             char **data) {

    struct mesh_data t_data;

    uint64_t keys[component_key_count + 3];
    memcpy(keys, component_key, sizeof(uint64_t) * component_key_count);

    keys[component_key_count] = ct_yng_a0.calc_key("scene");
    t_data.scene = CT_ID64_0(ct_ydb_a0.get_string(
            filename, keys, component_key_count + 1, ""));

    uint64_t geom[32] = {};
    uint32_t geom_keys_count = 0;

    keys[component_key_count] = ct_yng_a0.calc_key("geometries");
    ct_ydb_a0.get_map_keys(filename,
                           keys, component_key_count + 1,
                           geom, CETECH_ARRAY_LEN(geom),
                           &geom_keys_count);

    for (uint32_t i = 0; i < geom_keys_count; ++i) {
        keys[component_key_count + 1] = geom[i];

        keys[component_key_count + 2] = ct_yng_a0.calc_key("mesh");
        t_data.mesh[i] = CT_ID64_0(
                ct_ydb_a0.get_string(filename, keys,
                                     component_key_count + 3, ""));

        keys[component_key_count + 2] = ct_yng_a0.calc_key("material");
        t_data.material[i] = CT_ID64_0(
                ct_ydb_a0.get_string(filename, keys,
                                     component_key_count + 3, ""));

        keys[component_key_count + 2] = ct_yng_a0.calc_key("node");
        t_data.node[i] = CT_ID64_0(
                ct_ydb_a0.get_string(filename, keys,
                                     component_key_count + 3, ""));
    }

    t_data.count = geom_keys_count;

    ct_array_push_n(*data, (uint8_t *) &t_data, sizeof(struct mesh_data),
                    _G.allocator);

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

ct_mesh_renderer mesh_create(ct_world world,
                             ct_entity entity,
                             uint64_t scene,
                             uint64_t *mesh,
                             uint64_t *node,
                             uint64_t *material,
                             uint32_t geom_count) {

    WorldInstance *data = _get_world_instance(world);

    uint32_t idx = data->n;
    allocate(*data, ct_memory_a0.main_allocator(), data->n + 1);
    ++data->n;

    ct_hash_add(&_G.ent_map, hash_combine(world.h, entity.h), idx, _G.allocator);
    ct_scene_a0.create_graph(world, entity, scene);

    data->entity[idx] = entity;

    ct_cdb_obj_t *renderer_obj = ct_cdb_a0.create_object();
    ct_cdb_writer_t *renderer_writer = ct_cdb_a0.write_begin(renderer_obj);

    ct_cdb_a0.set_uint64(renderer_writer, CT_ID64_0("scene"), scene);

    ct_cdb_obj_t *geom_obj = ct_cdb_a0.create_object();
    ct_cdb_writer_t *geom_writer = ct_cdb_a0.write_begin(geom_obj);
    for (uint32_t i = 0; i < geom_count; ++i) {
        ct_cdb_obj_t *gobj = ct_cdb_a0.create_object();
        ct_cdb_writer_t *gobj_writer = ct_cdb_a0.write_begin(gobj);

        uint64_t n = node[i];
        if (n == 0) {
            n = ct_scene_a0.get_mesh_node(scene, mesh[i]);
        }

        ct_cdb_a0.set_uint64(gobj_writer, CT_ID64_0("mesh"), mesh[i]);
        ct_cdb_a0.set_uint64(gobj_writer, CT_ID64_0("node"), n);
        ct_cdb_a0.set_ref(gobj_writer, CT_ID64_0("material"),
                          ct_material_a0.resource_create(material[i]));

        ct_cdb_a0.write_commit(gobj_writer);

        ct_cdb_a0.set_ref(geom_writer, i + 1, gobj);
    }
    ct_cdb_a0.write_commit(geom_writer);
    ct_cdb_a0.set_ref(renderer_writer, CT_ID64_0("geometries"), geom_obj);
    ct_cdb_a0.write_commit(renderer_writer);


    ct_cdb_obj_t *ent_obj = ct_entity_a0.ent_obj(entity);
    ct_cdb_writer_t *ent_writer = ct_cdb_a0.write_begin(ent_obj);
    ct_cdb_a0.set_ref(ent_writer, _G.type, renderer_obj);
    ct_cdb_a0.write_commit(ent_writer);


    return (ct_mesh_renderer) {.idx = idx, .world = world};
}

static void _spawner(ct_world world,
                     ct_entity *ents,
                     uint32_t *cents,
                     uint32_t *ents_parent,
                     uint32_t ent_count,
                     void *data) {
    CT_UNUSED(ents_parent);

    struct mesh_data *tdata = (mesh_data *) data;

    for (uint32_t i = 0; i < ent_count; ++i) {
        mesh_create(world,
                    ents[cents[i]],
                    tdata[i].scene,
                    tdata[i].mesh,
                    tdata[i].node,
                    tdata[i].material,
                    tdata[i].count
        );
    }
}

int mesh_is_valid(ct_mesh_renderer mesh) {
    return mesh.idx != UINT32_MAX;
}

int mesh_has(ct_world world,
             ct_entity entity) {
    uint64_t idx = hash_combine(world.h, entity.h);

    return ct_hash_contain(&_G.ent_map, idx);
}

ct_mesh_renderer mesh_get(ct_world world,
                          ct_entity entity) {

    uint64_t idx = hash_combine(world.h, entity.h);

    uint32_t component_idx = ct_hash_lookup(&_G.ent_map, idx, UINT32_MAX);

    return {.idx = component_idx, .world = world};
}

void mesh_render_all(ct_world world,
                     uint8_t viewid,
                     uint64_t layer_name) {
    WorldInstance *data = _get_world_instance(world);

    for (uint32_t i = 0; i < data->n; ++i) {
        ct_entity ent = data->entity[i];

        ct_cdb_obj_t *ent_obj = ct_entity_a0.ent_obj(ent);
        ct_cdb_obj_t *renderer_obj = ct_cdb_a0.read_ref(ent_obj, _G.type, NULL);
        ct_cdb_obj_t *geom_obj = ct_cdb_a0.read_ref(renderer_obj,
                                                    CT_ID64_0("geometries"),
                                                    NULL);

        uint64_t scene = ct_cdb_a0.read_uint64(renderer_obj, CT_ID64_0("scene"),
                                               0);

        float wm[16];
        ct_transform t = ct_transform_a0.get(world, ent);
        ct_transform_a0.get_world_matrix(t, wm);

        uint64_t *keys = ct_cdb_a0.prop_keys(geom_obj);
        uint64_t kcount = ct_cdb_a0.prop_count(geom_obj);
        for (int j = 0; j < kcount; ++j) {
            ct_cdb_obj_t *geom = ct_cdb_a0.read_ref(geom_obj, keys[j],
                                                    NULL);

            ct_cdb_obj_t *material = ct_cdb_a0.read_ref(geom,
                                                        CT_ID64_0("material"),
                                                        0);

            uint64_t mesh = ct_cdb_a0.read_uint64(geom,
                                                  CT_ID64_0("mesh"), 0);

            //mat44f_t t_w = MAT44F_INIT_IDENTITY;//*transform_get_world_matrix(world, t);
            float node_w[16];
            float final_w[16];

            ct_mat4_identity(node_w);
            ct_mat4_identity(final_w);

            if (ct_scenegprah_a0.has(world, ent)) {
                uint64_t name = ct_scene_a0.get_mesh_node(scene, mesh);
                if (name != 0) {
                    ct_scene_node n = ct_scenegprah_a0.node_by_name(world, ent,
                                                                    name);
                    ct_scenegprah_a0.get_world_matrix(n, node_w);
                }
            }

            ct_mat4_mul(final_w, node_w, wm);

            bgfx::setTransform(&final_w, 1);

            ct_scene_a0.setVBIB(scene, mesh);

            ct_material_a0.submit(material, layer_name, viewid);
        }
    }
}

ct_cdb_obj_t *mesh_get_material(ct_mesh_renderer mesh,
                                uint32_t idx) {
    WorldInstance *data = _get_world_instance(mesh.world);

    ct_cdb_obj_t *ent_obj = ct_entity_a0.ent_obj(data->entity[mesh.idx]);
    ct_cdb_obj_t *renderer_obj = ct_cdb_a0.read_ref(ent_obj, _G.type, NULL);
    ct_cdb_obj_t *geom_obj = ct_cdb_a0.read_ref(renderer_obj,
                                                CT_ID64_0("geometries"),
                                                NULL);
    ct_cdb_obj_t *gobj = ct_cdb_a0.read_ref(geom_obj, idx+1, NULL);
    return ct_cdb_a0.read_ref(gobj, CT_ID64_0("material"), NULL);
}

void mesh_set_material(ct_mesh_renderer mesh,
                       uint32_t idx,
                       uint64_t material) {
    WorldInstance *data = _get_world_instance(mesh.world);
    ct_cdb_obj_t *material_instance = ct_material_a0.resource_create(material);

    ct_cdb_obj_t *ent_obj = ct_entity_a0.ent_obj(data->entity[mesh.idx]);
    ct_cdb_obj_t *renderer_obj = ct_cdb_a0.read_ref(ent_obj, _G.type, NULL);
    ct_cdb_obj_t *geom_obj = ct_cdb_a0.read_ref(renderer_obj,
                                                CT_ID64_0("geometries"),
                                                NULL);
    ct_cdb_obj_t *gobj = ct_cdb_a0.read_ref(geom_obj, idx+1, NULL);

    ct_cdb_writer_t* wr = ct_cdb_a0.write_begin(gobj);
    ct_cdb_a0.set_ref(wr, CT_ID64_0("material"), material_instance);
    ct_cdb_a0.write_commit(wr);
}

void set_geometry(struct ct_mesh_renderer mesh,
                  uint32_t idx,
                  uint64_t geometry) {
    WorldInstance *data = _get_world_instance(mesh.world);
    ct_cdb_obj_t *ent_obj = ct_entity_a0.ent_obj(data->entity[mesh.idx]);
    ct_cdb_obj_t *renderer_obj = ct_cdb_a0.read_ref(ent_obj, _G.type, NULL);
    ct_cdb_obj_t *geom_obj = ct_cdb_a0.read_ref(renderer_obj,
                                                CT_ID64_0("geometries"),
                                                NULL);
    ct_cdb_obj_t *gobj = ct_cdb_a0.read_ref(geom_obj, idx+1, NULL);

    ct_cdb_writer_t* wr = ct_cdb_a0.write_begin(gobj);
    ct_cdb_a0.set_uint64(wr, CT_ID64_0("mesh"), geometry);
    ct_cdb_a0.write_commit(wr);
}

void set_node(struct ct_mesh_renderer mesh,
              uint32_t idx,
              uint64_t node) {
    WorldInstance *data = _get_world_instance(mesh.world);
    ct_cdb_obj_t *ent_obj = ct_entity_a0.ent_obj(data->entity[mesh.idx]);
    ct_cdb_obj_t *renderer_obj = ct_cdb_a0.read_ref(ent_obj, _G.type, NULL);
    ct_cdb_obj_t *geom_obj = ct_cdb_a0.read_ref(renderer_obj,
                                                CT_ID64_0("geometries"),
                                                NULL);
    ct_cdb_obj_t *gobj = ct_cdb_a0.read_ref(geom_obj, idx+1, NULL);

    ct_cdb_writer_t* wr = ct_cdb_a0.write_begin(gobj);
    ct_cdb_a0.set_uint64(wr, CT_ID64_0("node"), node);
    ct_cdb_a0.write_commit(wr);
}

void set_scene(struct ct_mesh_renderer mesh,
               uint64_t scene) {
    WorldInstance *data = _get_world_instance(mesh.world);
    ct_cdb_obj_t *ent_obj = ct_entity_a0.ent_obj(data->entity[mesh.idx]);
    ct_cdb_obj_t *renderer_obj = ct_cdb_a0.read_ref(ent_obj, _G.type, NULL);

    ct_cdb_writer_t* wr = ct_cdb_a0.write_begin(renderer_obj);
    ct_cdb_a0.set_uint64(wr, CT_ID64_0("scene"), scene);
    ct_cdb_a0.write_commit(wr);
}


static void _init_api(struct ct_api_a0 *api) {
    static struct ct_mesh_renderer_a0 _api = {
            .is_valid = mesh_is_valid,
            .has = mesh_has,
            .get = mesh_get,
            .create = mesh_create,
            .get_material = mesh_get_material,
            .set_material = mesh_set_material,
            .set_geometry = set_geometry,
            .set_node = set_node,
            .set_scene = set_scene,
            .render_all = mesh_render_all,
    };


    api->register_api("ct_mesh_renderer_a0", &_api);
}


static void _init(ct_api_a0 *api) {
    _init_api(api);

    _G = {
            .allocator = ct_memory_a0.main_allocator(),
            .type = CT_ID64_0("mesh_renderer"),
    };

    ct_component_a0.register_compiler(_G.type, _mesh_component_compiler, 10);
    ct_component_a0.register_type(_G.type, {
            .spawner=_spawner,
            .destroyer=_destroyer,

            .world_clb.on_created = _on_world_create,
            .world_clb.on_destroy = _on_world_destroy,

    });
}

static void _shutdown() {
    ct_hash_free(&_G.world_map, _G.allocator);
    ct_hash_free(&_G.ent_map, _G.allocator);
    ct_array_free(_G.world_instances, _G.allocator);
}

static void init(ct_api_a0 *api) {
    _init(api);
}

static void shutdown() {
    _shutdown();
}


CETECH_MODULE_DEF(
        mesh_renderer,
        {
            CETECH_GET_API(api, ct_component_a0);
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_scenegprah_a0);
            CETECH_GET_API(api, ct_transform_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_material_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_scene_a0);
            CETECH_GET_API(api, ct_entity_a0);
            CETECH_GET_API(api, ct_cdb_a0);
            CETECH_GET_API(api, ct_resource_a0);

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