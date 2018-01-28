#include <cetech/engine/entity/entity.h>
#include <cetech/engine/renderer/renderer.h>
#include <cetech/engine/transform/transform.h>
#include <cetech/engine/camera/camera.h>
#include <cetech/core/yaml/yamlng.h>
#include <cetech/core/yaml/ydb.h>
#include <cetech/macros.h>
#include <cetech/core/containers/array.h>
#include <cetech/core/containers/hash.h>
#include <cetech/core/math/fmath.h>
#include <cetech/core/cdb/cdb.h>

#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/config/config.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"

#include "cetech/core/module/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_component_a0);
CETECH_DECL_API(ct_transform_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_entity_a0);
CETECH_DECL_API(ct_cdb_a0);


uint64_t combine(uint32_t a,
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

struct camera_data {
    float near;
    float far;
    float fov;
};

struct WorldInstance {
    ct_world world;
    uint32_t n;
    uint32_t allocated;
    void *buffer;

    ct_entity *entity;
};

#define _G CameraGlobal
static struct CameraGlobal {
    uint64_t type;

    ct_hash_t world_map;
    ct_hash_t ent_map;

    WorldInstance *world_instances;
    ct_alloc *allocator;
} CameraGlobal;


static void allocate(WorldInstance &_data,
                     ct_alloc *_allocator,
                     uint32_t sz) {
    //assert(sz > _data.n);

    WorldInstance new_data;
    const unsigned bytes = sz * (sizeof(ct_entity) + (3 * sizeof(float)));
    new_data.buffer = CT_ALLOC(_allocator, char, bytes);
    new_data.n = _data.n;
    new_data.allocated = sz;

    new_data.entity = (ct_entity *) (new_data.buffer);

    memcpy(new_data.entity, _data.entity, _data.n * sizeof(ct_entity));

    CT_FREE(_allocator, _data.buffer);

    _data = new_data;
}

//    void destroy(WorldInstance &_data,
//                 unsigned i) {
//
//        unsigned last = _data.n - 1;
////        ct_entity e = _data.entity[i];
////        ct_entity last_e = _data.entity[last];
//
//        _data.entity[i] = _data.entity[last];
//        _data.near[i] = _data.near[last];
//        _data.far[i] = _data.far[last];
//        _data.fov[i] = _data.fov[last];
//
////    map::set(_G._map, last_e.h, i);
////    map::remove(_G._map, e.h);
//
//        --_data.n;
//    }

static void _new_world(ct_world world) {
    uint32_t idx = ct_array_size(_G.world_instances);
    ct_array_push(_G.world_instances, WorldInstance(), _G.allocator);
    _G.world_instances[idx].world = world;
    ct_hash_add(&_G.world_map, world.h, idx, _G.allocator);
}

static void _destroy_world(ct_world world) {
    uint64_t idx = ct_hash_lookup(&_G.world_map, world.h, UINT64_MAX);
    uint32_t last_idx = ct_array_size(_G.world_instances) - 1;

    ct_world last_world = _G.world_instances[last_idx].world;

    CT_FREE(ct_memory_a0.main_allocator(),
             _G.world_instances[idx].buffer);

    _G.world_instances[idx] = _G.world_instances[last_idx];
    ct_hash_add(&_G.world_map, last_world.h, idx, _G.allocator);
    ct_array_pop_back(_G.world_instances);
}

static WorldInstance *_get_world_instance(ct_world world) {
    uint64_t idx = ct_hash_lookup(&_G.world_map, world.h, UINT64_MAX);

    if (idx != UINT64_MAX) {
        return &_G.world_instances[idx];
    }

    return nullptr;
}

static int _camera_component_compiler(const char *filename,
                                      uint64_t *component_key,
                                      uint32_t component_key_count,
                                      char **data) {
    struct camera_data t_data;


    uint64_t keys[component_key_count + 1];
    memcpy(keys, component_key, sizeof(uint64_t) * component_key_count);
    keys[component_key_count] = ct_yng_a0.calc_key("near");

    t_data.near = ct_ydb_a0.get_float(filename, keys, CETECH_ARRAY_LEN(keys),
                                      0.0f);

    keys[component_key_count] = ct_yng_a0.calc_key("far");
    t_data.far = ct_ydb_a0.get_float(filename, keys, CETECH_ARRAY_LEN(keys),
                                     0.0f);

    keys[component_key_count] = ct_yng_a0.calc_key("fov");
    t_data.fov = ct_ydb_a0.get_float(filename, keys, CETECH_ARRAY_LEN(keys),
                                     0.0f);

    ct_array_push_n(*data, (uint8_t *) &t_data, sizeof(t_data), _G.allocator);

    return 1;
}


static int is_valid(ct_camera camera) {
    return camera.idx != UINT32_MAX;
}

static void get_project_view(ct_camera camera,
                             float *proj,
                             float *view,
                             int width,
                             int height) {

    WorldInstance *world_inst = _get_world_instance(camera.world);

    ct_entity e = world_inst->entity[camera.idx];
    ct_transform t = ct_transform_a0.get(camera.world, e);

    ct_cdb_obj_t* ent_obj = ct_entity_a0.ent_obj(e);
    ct_cdb_obj_t* camera_obj = ct_cdb_a0.read_ref(ent_obj, _G.type, NULL);

    float fov = ct_cdb_a0.read_float(camera_obj, CT_ID64_0("fov"), 0.0f);
    float near = ct_cdb_a0.read_float(camera_obj, CT_ID64_0("near"), 0.0f);
    float far = ct_cdb_a0.read_float(camera_obj, CT_ID64_0("far"), 0.0f);

    ct_mat4_proj_fovy(proj, fov, float(width) / float(height), near, far, true);

    float w[16];
    ct_transform_a0.get_world_matrix(t, w);

    //ct_mat4_move(view, w);
    ct_mat4_inverse(view, w);
}

static int has(ct_world world,
               ct_entity entity) {

    uint64_t idx = combine(world.h, entity.h);

    return ct_hash_contain(&_G.ent_map, idx);
}

static ct_camera get(ct_world world,
                     ct_entity entity) {

    uint64_t idx = combine(world.h, entity.h);
    uint32_t component_idx = (uint32_t) ct_hash_lookup(&_G.ent_map, idx,
                                                        UINT32_MAX);

    return (ct_camera) {.idx = component_idx, .world = world};
}

static ct_camera create(ct_world world,
                        ct_entity entity,
                        float near,
                        float far,
                        float fov) {

    WorldInstance *data = _get_world_instance(world);

    uint32_t idx = data->n;
    allocate(*data, ct_memory_a0.main_allocator(), data->n + 1);
    ++data->n;

    data->entity[idx] = entity;

    ct_cdb_obj_t* ent_obj = ct_entity_a0.ent_obj(entity);
    ct_cdb_obj_t* camera_obj = ct_cdb_a0.create_object();
    ct_cdb_writer_t* camera_writer = ct_cdb_a0.write_begin(camera_obj);
    ct_cdb_a0.set_float(camera_writer, CT_ID64_0("near"), near);
    ct_cdb_a0.set_float(camera_writer, CT_ID64_0("far"), far);
    ct_cdb_a0.set_float(camera_writer, CT_ID64_0("fov"), fov);
    ct_cdb_a0.write_commit(camera_writer);

    ct_cdb_writer_t *ent_writer = ct_cdb_a0.write_begin(ent_obj);
    ct_cdb_a0.set_ref(ent_writer, _G.type, camera_obj);
    ct_cdb_a0.write_commit(ent_writer);

    ct_hash_add(&_G.ent_map, combine(world.h, entity.h), idx, _G.allocator);
    return (ct_camera) {.idx = idx, .world = world};
}


static ct_camera_a0 camera_api = {
        .is_valid = is_valid,
        .get_project_view = get_project_view,
        .has = has,
        .get = get,
        .create = create
};

static void _on_world_create(ct_world world) {
    _new_world(world);
}

static void _on_world_destroy(ct_world world) {
    _destroy_world(world);
}

static void _destroyer(ct_world world,
                       ct_entity *ents,
                       uint32_t ent_count) {
    CT_UNUSED(world);

    // TODO: remove from arrays, swap idx -> last AND change size
    for (uint32_t i = 0; i < ent_count; i++) {
        if (has(world, ents[i])) {
            ct_hash_remove(&_G.ent_map, combine(world.h, ents[i].h));
        }
    }
}

static void _spawner(ct_world world,
                     ct_entity *ents,
                     uint32_t *cents,
                     uint32_t *ents_parent,
                     uint32_t ent_count,
                     void *data) {

    CT_UNUSED(ents_parent);

    camera_data *tdata = (camera_data *) data;

    for (uint32_t i = 0; i < ent_count; ++i) {
        create(world,
               ents[cents[i]],
               tdata[i].near,
               tdata[i].far,
               tdata[i].fov);
    }
}

static void _init(ct_api_a0 *api) {
    api->register_api("ct_camera_a0", &camera_api);

    _G = {
            .allocator = ct_memory_a0.main_allocator()
    };

    _G.type = CT_ID64_0("camera");

    ct_component_a0.register_compiler(_G.type,
                                      _camera_component_compiler,
                                      10);

    ct_component_a0.register_type(_G.type, {
            .spawner=_spawner,
            .destroyer=_destroyer,

            .world_clb.on_created = _on_world_create,
            .world_clb.on_destroy = _on_world_destroy
    });
}

static void _shutdown() {
    ct_array_free(_G.world_instances, _G.allocator);
    ct_hash_free(&_G.ent_map, _G.allocator);
    ct_hash_free(&_G.world_map, _G.allocator);
}


CETECH_MODULE_DEF(
        camera,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_component_a0);
            CETECH_GET_API(api, ct_transform_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_entity_a0);
            CETECH_GET_API(api, ct_cdb_a0);
        },
        {
            CT_UNUSED(reload);
            _init(api);
        },
        {
            CT_UNUSED(reload);
            CT_UNUSED(api);
            _shutdown();
        }
)