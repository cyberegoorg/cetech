
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

static void on_obj_change(struct ct_cdb_obj_t *obj,
                          uint64_t *prop,
                          uint32_t prop_count) {
    for (int i = 0; i < prop_count; ++i) {
        const uint64_t key = prop[i];

        if ((key >> 32) == (PROP_MATERIAL_ID >> 32)) {
            uint32_t idx = (uint32_t) (key - PROP_MATERIAL_ID);

            uint64_t material = ct_cdb_a0.read_uint64(obj, key, 0);
            ct_cdb_writer_t *w = ct_cdb_a0.write_begin(obj);
            ct_cdb_a0.set_ref(w, PROP_MATERIAL + idx,
                              ct_material_a0.resource_create(material));
            ct_cdb_a0.write_commit(w);
        }
    }
}

void destroy(ct_world world,
             ct_entity ent) {
    ct_entity_a0.remove_component(ent, _G.type);

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
                             uint64_t *comp_keys,
                             uint32_t count,
                             struct ct_cdb_writer_t *writer) {
    struct mesh_data t_data;
    uint64_t keys[count + 3];
    memcpy(keys, comp_keys, sizeof(uint64_t) * count);

    keys[count] = ct_yng_a0.calc_key("scene");
    t_data.scene = CT_ID64_0(ct_ydb_a0.get_string(
            filename, keys, count + 1, ""));

    uint64_t geom[32] = {};
    uint32_t geom_keys_count = 0;

    keys[count] = ct_yng_a0.calc_key("geometries");
    ct_ydb_a0.get_map_keys(filename,
                           keys, count + 1,
                           geom, CETECH_ARRAY_LEN(geom),
                           &geom_keys_count);

    for (uint32_t i = 0; i < geom_keys_count; ++i) {
        keys[count + 1] = geom[i];

        keys[count + 2] = ct_yng_a0.calc_key("mesh");
        t_data.mesh[i] = CT_ID64_0(
                ct_ydb_a0.get_string(filename, keys, count + 3, ""));

        keys[count + 2] = ct_yng_a0.calc_key("material");
        t_data.material[i] = CT_ID64_0(
                ct_ydb_a0.get_string(filename, keys, count + 3, ""));

        keys[count + 2] = ct_yng_a0.calc_key("node");
        t_data.node[i] = CT_ID64_0(
                ct_ydb_a0.get_string(filename, keys, count + 3, ""));


        ct_cdb_a0.set_uint64(writer, PROP_MESH_ID + i, t_data.mesh[i]);
        ct_cdb_a0.set_uint64(writer, PROP_NODE_ID + i, t_data.node[i]);
        ct_cdb_a0.set_uint64(writer, PROP_MATERIAL_ID + i, t_data.material[i]);
    }

    t_data.count = geom_keys_count;

    ct_cdb_a0.set_uint64(writer, PROP_SCENE, t_data.scene);
    ct_cdb_a0.set_uint64(writer, PROP_GEOM_COUNT, t_data.count);

    return 1;
}

void mesh_render_all(ct_world world,
                     uint8_t viewid,
                     uint64_t layer_name) {
    WorldInstance *data = _get_world_instance(world);

    for (uint32_t i = 0; i < data->n; ++i) {
        ct_entity ent = data->entity[i];

        ct_cdb_obj_t *ent_obj = ct_entity_a0.ent_obj(ent);

        uint64_t scene = ct_cdb_a0.read_uint64(ent_obj, PROP_SCENE, 0);

        float wm[16];
        ct_transform_a0.get_world_matrix(ent, wm);

        uint64_t kcount = ct_cdb_a0.read_uint64(ent_obj, PROP_GEOM_COUNT, 0);

        for (int j = 0; j < kcount; ++j) {
            ct_cdb_obj_t *mat = ct_cdb_a0.read_ref(ent_obj, PROP_MATERIAL + j,
                                                   0);

            uint64_t mesh = ct_cdb_a0.read_uint64(ent_obj,
                                                  PROP_MESH_ID + j, 0);

            //mat44f_t t_w = MAT44F_INIT_IDENTITY;//*transform_get_world_matrix(world, t);
            float node_w[16];
            float final_w[16];

            ct_mat4_identity(node_w);
            ct_mat4_identity(final_w);

            if (ct_scenegprah_a0.has(world, ent)) {
                uint64_t name = ct_scene_a0.get_mesh_node(scene, mesh);
                if (name != 0) {
                    ct_scene_node n = ct_scenegprah_a0.node_by_name(world,
                                                                    ent,
                                                                    name);
                    ct_scenegprah_a0.get_world_matrix(n, node_w);
                }
            }

            ct_mat4_mul(final_w, node_w, wm);
            bgfx::setTransform(&final_w, 1);
            ct_scene_a0.setVBIB(scene, mesh);
            ct_material_a0.submit(mat, layer_name, viewid);
        }
    }
}

void mesh_set_material(ct_entity mesh,
                       uint32_t idx,
                       uint64_t material) {
    ct_cdb_obj_t *ent_obj = ct_entity_a0.ent_obj(mesh);

    ct_cdb_obj_t *material_instance = ct_material_a0.resource_create(material);

    ct_cdb_writer_t *wr = ct_cdb_a0.write_begin(ent_obj);
    ct_cdb_a0.set_ref(wr, PROP_MATERIAL + idx, material_instance);
    ct_cdb_a0.write_commit(wr);
}

static void on_add(struct ct_world world,
                   struct ct_entity entity,
                   uint64_t comp_mask) {
    if (!(comp_mask & ct_entity_a0.component_mask(_G.type))) {
        return;
    }

    ct_cdb_obj_t *ent_obj = ct_entity_a0.ent_obj(entity);
    uint64_t scene = ct_cdb_a0.read_uint64(ent_obj, PROP_SCENE, 0);
    uint64_t geom_count = ct_cdb_a0.read_uint64(ent_obj, PROP_GEOM_COUNT, 0);

    WorldInstance *data = _get_world_instance(world);

    uint32_t idx = data->n;
    allocate(*data, ct_memory_a0.main_allocator(), data->n + 1);
    ++data->n;

    ct_hash_add(&_G.ent_map, hash_combine(world.h, entity.h), idx,
                _G.allocator);

    ct_scene_a0.create_graph(world, entity, scene);

    data->entity[idx] = entity;

    ct_cdb_writer_t *ent_writer = ct_cdb_a0.write_begin(ent_obj);
    for (uint32_t i = 0; i < geom_count; ++i) {
        uint64_t n = ct_cdb_a0.read_uint64(ent_obj, PROP_NODE_ID + i, 0);

        uint64_t mesh = ct_cdb_a0.read_uint64(ent_obj, PROP_MESH_ID + i, 0);

        if (n == 0) {
            n = ct_scene_a0.get_mesh_node(scene, mesh);
        }

        ct_cdb_a0.set_uint64(ent_writer, PROP_NODE + i, n);

        uint64_t material = ct_cdb_a0.read_uint64(ent_obj, PROP_MATERIAL_ID + i,
                                                  0);

        ct_cdb_a0.set_ref(ent_writer, PROP_MATERIAL + i,
                          ct_material_a0.resource_create(material));

    }

    ct_cdb_a0.write_commit(ent_writer);
    ct_cdb_a0.register_notify(ent_obj, on_obj_change);
}


static void on_remove(struct ct_world world,
                      struct ct_entity ent,
                      uint64_t comp_mask) {
    if (!(comp_mask & ct_entity_a0.component_mask(_G.type))) {
        return;
    }

    destroy(world, ent);
}

static struct ct_mesh_renderer_a0 _api = {
        .set_material = mesh_set_material,
        .render_all = mesh_render_all,
};


static void _init_api(struct ct_api_a0 *api) {

    api->register_api("ct_mesh_renderer_a0", &_api);
}


static void _init(ct_api_a0 *api) {
    _init_api(api);

    _G = {
            .allocator = ct_memory_a0.main_allocator(),
            .type = CT_ID64_0("mesh_renderer"),
    };

    ct_entity_a0.register_component(_G.type);
    ct_entity_a0.add_components_watch({.on_add=on_add, .on_remove=on_remove});

    ct_component_a0.register_compiler(_G.type, _mesh_component_compiler, 10);
    ct_component_a0.register_type(_G.type, {
            .world_clb.on_created =  _new_world,
            .world_clb.on_destroy = _destroy_world,
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