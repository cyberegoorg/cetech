#include "cetech/core/config/config.h"
#include "cetech/engine/resource/resource.h"
#include "cetech/core/hashlib/hashlib.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"

#include "cetech/core/containers/map.inl"

#include "cetech/engine/entity/entity.h"
#include <cetech/engine/transform/transform.h>
#include <cetech/core/yaml/ydb.h>
#include <cetech/macros.h>
#include <cetech/core/containers/array.h>
#include <cetech/core/containers/hash.h>
#include <cetech/core/math/fmath.h>

#include "cetech/core/module/module.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_component_a0);
CETECH_DECL_API(ct_yng_a0);
CETECH_DECL_API(ct_ydb_a0);
CETECH_DECL_API(ct_entity_a0);
CETECH_DECL_API(ct_cdb_a0);
CETECH_DECL_API(ct_world_a0);

using namespace celib;


//! Transform component
struct ct_transform {
    struct ct_world world;
    uint32_t idx;
};


void transform_transform(ct_transform transform,
                         float *parent);

void transform_get_world_matrix(struct ct_entity transform,
                                float *value);

ct_transform transform_get(ct_world world,
                           ct_entity entity);


void transform_link(ct_world world,
                    ct_entity parent,
                    ct_entity child);

struct transform_data {
    float position[3];
    float scale[3];
    float rotation[3];
};

struct WorldInstance {
    ct_world world;
    uint32_t n;
    uint32_t allocated;
    void *buffer;

    ct_entity *entity;

    uint32_t *first_child;
    uint32_t *next_sibling;
    uint32_t *parent;

    float *position;
    float *rotation;
    float *scale;
    float *world_matrix;
};


#define _G TransformGlobal
static struct _G {
    uint64_t type;

    ct_hash_t world_map;
    WorldInstance *world_instances;
    ct_hash_t ent_map;
    ct_alloc *allocator;
} _G;


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

//#define hash_combine(a, b) (((a) * 11)^(b))


static void allocate(WorldInstance &_data,
                     ct_alloc *_allocator,
                     uint32_t sz) {
    //assert(sz > _data.n);

    WorldInstance new_data;
    const unsigned bytes = sz * (sizeof(ct_entity) +
                                 (3 * sizeof(uint32_t)) +
                                 (2 * sizeof(float) * 3) +
                                 (sizeof(float) * 4) +
                                 (sizeof(float) * 16));
    new_data.buffer = CT_ALLOC(_allocator, char, bytes);
    new_data.n = _data.n;
    new_data.allocated = sz;

    new_data.entity = (ct_entity *) (new_data.buffer);
    new_data.first_child = (uint32_t *) (new_data.entity + sz);
    new_data.next_sibling = (uint32_t *) (new_data.first_child + sz);
    new_data.parent = (uint32_t *) (new_data.next_sibling + sz);
    new_data.position = (float *) (new_data.parent + sz);
    new_data.rotation = (float *) (new_data.position + (sz * 3));
    new_data.scale = (float *) (new_data.rotation + (sz * 4));
    new_data.world_matrix = (float *) (new_data.scale + (sz * 3));

    memcpy(new_data.entity, _data.entity, _data.n * sizeof(ct_entity));

    memcpy(new_data.first_child, _data.first_child, _data.n * sizeof(uint32_t));
    memcpy(new_data.next_sibling, _data.next_sibling,
           _data.n * sizeof(uint32_t));
    memcpy(new_data.parent, _data.parent, _data.n * sizeof(uint32_t));

    memcpy(new_data.position, _data.position, _data.n * sizeof(float) * 3);
    memcpy(new_data.rotation, _data.rotation, _data.n * sizeof(float) * 4);
    memcpy(new_data.scale, _data.scale, _data.n * sizeof(float) * 3);

    memcpy(new_data.world_matrix, _data.world_matrix,
           _data.n * sizeof(float) * 16);

    CT_FREE(_allocator, _data.buffer);

    _data = new_data;
}

static void _new_world(ct_world world) {
    uint32_t idx = ct_array_size(_G.world_instances);
    ct_array_push(_G.world_instances, WorldInstance(), _G.allocator);
    _G.world_instances[idx].world = world;
    ct_hash_add(&_G.world_map, world.h, idx, _G.allocator);
}


static WorldInstance *_get_world_instance(ct_world world) {
    uint32_t idx = ct_hash_lookup(&_G.world_map, world.h, UINT32_MAX);

    if (idx != UINT32_MAX) {
        return &_G.world_instances[idx];
    }

    return nullptr;
}

static void _destroy_world(ct_world world) {
    uint32_t idx = ct_hash_lookup(&_G.world_map, world.h, UINT32_MAX);
    uint32_t last_idx = ct_array_size(_G.world_instances) - 1;

    ct_world last_world = _G.world_instances[last_idx].world;

    CT_FREE(ct_memory_a0.main_allocator(),
            _G.world_instances[idx].buffer);

    _G.world_instances[idx] = _G.world_instances[last_idx];
    ct_hash_add(&_G.world_map, last_world.h, idx, _G.allocator);
    ct_array_pop_back(_G.world_instances);
}

int _component_compiler(const char *filename,
                        uint64_t *component_key,
                        uint32_t component_key_count,
                        ct_cdb_writer_t *writer) {
    transform_data t_data;

    uint64_t keys[component_key_count + 1];
    memcpy(keys, component_key, sizeof(uint64_t) * component_key_count);

    keys[component_key_count] = ct_yng_a0.calc_key("scale");
    ct_ydb_a0.get_vec3(filename, keys, CETECH_ARRAY_LEN(keys),
                       t_data.scale, (float[3]) {0});

    keys[component_key_count] = ct_yng_a0.calc_key("position");
    ct_ydb_a0.get_vec3(filename, keys, CETECH_ARRAY_LEN(keys),
                       t_data.position, (float[3]) {0});

    keys[component_key_count] = ct_yng_a0.calc_key("rotation");
    ct_ydb_a0.get_vec3(filename, keys, CETECH_ARRAY_LEN(keys),
                       t_data.rotation, (float[3]) {0});

    ct_cdb_a0.set_vec3(writer, PROP_POSITION, t_data.position);
    ct_cdb_a0.set_vec3(writer, PROP_ROTATION, t_data.rotation);
    ct_cdb_a0.set_vec3(writer, PROP_SCALE, t_data.scale);

    return 1;
}

static void _on_world_create(ct_world world) {
    _new_world(world);
}

static void _on_world_destroy(ct_world world) {
    _destroy_world(world);
}

void on_obj_change(struct ct_cdb_obj_t *obj,
                   uint64_t *prop,
                   uint32_t prop_count) {
    ct_world world = {
            .h = ct_cdb_a0.read_uint64(obj, CT_ID64_0("world"), 0)
    };

    ct_entity ent = {
            .h = ct_cdb_a0.read_uint64(obj, CT_ID64_0("entid"), 0)
    };

    WorldInstance *world_inst = _get_world_instance(world);

    uint32_t idx = ct_cdb_a0.read_uint32(obj, CT_ID64_0("transform.idx"), 0);
    for (int i = 0; i < prop_count; ++i) {
        const uint64_t key = prop[i];

        if (key == PROP_POSITION) {
            float pos[3] = {0};
            ct_cdb_a0.read_vec3(obj, key, pos);
            uint32_t parent_idx = world_inst->parent[idx];

            float p[16];

            if (parent_idx != UINT32_MAX) {
                transform_get_world_matrix(ent, p);
            } else {
                ct_mat4_identity(p);
            }

            ct_vec3_move(&world_inst->position[3 * idx], pos);
            transform_transform({.world = world, .idx = idx}, p);

        } else if (key == PROP_ROTATION) {
            float rot[3] = {0};
            ct_cdb_a0.read_vec3(obj, key, rot);
            uint32_t parent_idx = world_inst->parent[idx];

            float p[16];

            if (parent_idx != UINT32_MAX) {
                transform_get_world_matrix(ent, p);
            } else {
                ct_mat4_identity(p);
            }

            {
                float v_rad[3] = {0};
                float q[4] = {0};

                ct_vec3_mul_s(v_rad, rot, CT_DEG_TO_RAD);
                ct_quat_from_euler(q, v_rad[0], v_rad[1], v_rad[2]);
                ct_quat_move(&world_inst->rotation[4 * idx], q);
            };

            transform_transform({.world = world, .idx = idx}, p);
        } else if (key == PROP_SCALE) {
            float scale[3] = {0};
            ct_cdb_a0.read_vec3(obj, key, scale);
            uint32_t parent_idx = world_inst->parent[idx];

            float p[16];

            if (parent_idx != UINT32_MAX) {
                transform_get_world_matrix(ent, p);
            } else {
                ct_mat4_identity(p);
            }

            ct_vec3_move(&world_inst->scale[3 * idx], scale);
            transform_transform({.world = world, .idx = idx}, p);
        }
    }
}

void on_add(struct ct_world world,
            struct ct_entity entity,
            uint64_t comp_mask) {
    if (!(comp_mask & ct_entity_a0.component_mask(_G.type))) {
        return;
    }

    WorldInstance *data = _get_world_instance(world);

    uint32_t idx = data->n;
    allocate(*data, ct_memory_a0.main_allocator(), data->n + 1);
    ++data->n;

    data->entity[idx] = entity;

    ct_cdb_obj_t *ent_obj = ct_entity_a0.ent_obj(entity);

    ct_cdb_a0.register_notify(ent_obj, on_obj_change);

    float position[3] = {0};
    float rotation[3] = {0};
    float scale[3] = {0};
    ct_cdb_a0.read_vec3(ent_obj, PROP_POSITION, position);
    ct_cdb_a0.read_vec3(ent_obj, PROP_ROTATION, rotation);
    ct_cdb_a0.read_vec3(ent_obj, PROP_SCALE, scale);

    ct_vec3_move(&data->position[3 * idx], position);
    ct_vec3_move(&data->scale[3 * idx], scale);

    {
        float v_rad[3] = {0};
        float q[4] = {0};

        ct_vec3_mul_s(v_rad, rotation, CT_DEG_TO_RAD);
        ct_quat_from_euler(q, v_rad[0], v_rad[1], v_rad[2]);
        ct_quat_move(&data->rotation[4 * idx], q);
    }


    data->parent[idx] = UINT32_MAX;
    data->first_child[idx] = UINT32_MAX;
    data->next_sibling[idx] = UINT32_MAX;

    float m[16];
    ct_mat4_identity(m);
    memcpy(&data->world_matrix[16 * idx], m, sizeof(float) * 16);

    ct_transform t = {.idx = idx, .world=world};

    ct_entity parent = (ct_entity) {.h = UINT32_MAX};

    float p[16];
    if (parent.h != UINT32_MAX) {
        transform_get_world_matrix(parent, p);
    } else {
        ct_mat4_identity(p);
    }
    transform_transform(t, p);

    ct_hash_add(&_G.ent_map, hash_combine(world.h, entity.h), idx,
                _G.allocator);

    if (parent.h != UINT32_MAX) {
        uint32_t parent_idx = ct_hash_lookup(&_G.ent_map,
                                             hash_combine(world.h, parent.h),
                                             UINT32_MAX);

        data->parent[idx] = parent_idx;

        if (data->first_child[parent_idx] == UINT32_MAX) {
            data->first_child[parent_idx] = idx;
        } else {
            uint32_t first_child_idx = data->first_child[parent_idx];
            data->first_child[parent_idx] = idx;
            data->next_sibling[idx] = first_child_idx;
        }

        data->parent[idx] = parent_idx;
    }

    ct_cdb_writer_t *ent_writer = ct_cdb_a0.write_begin(ent_obj);
    ct_cdb_a0.set_uint32(ent_writer, CT_ID64_0("transform.idx"), idx);
    ct_cdb_a0.write_commit(ent_writer);
}

void on_remove(struct ct_world world,
               struct ct_entity ent,
               uint64_t comp_mask) {

}

static ct_transform_a0 _api = {
        .get_world_matrix = transform_get_world_matrix,
        .link = transform_link,
};

static void _init(ct_api_a0 *api) {
    api->register_api("ct_transform_a0", &_api);


    _G = {
            .allocator = ct_memory_a0.main_allocator(),
            .type = CT_ID64_0("transform"),
    };

    ct_entity_a0.register_component(_G.type);
    ct_entity_a0.add_components_watch({.on_add=on_add, .on_remove=on_remove});
    ct_world_a0.register_callback({
                    .on_created=_on_world_create,
                    .on_destroy=_on_world_destroy});

    ct_component_a0.register_compiler(_G.type, _component_compiler);
}

static void _shutdown() {
    ct_hash_free(&_G.ent_map, _G.allocator);
    ct_hash_free(&_G.world_map, _G.allocator);
    ct_array_free(_G.world_instances, _G.allocator);
}


int transform_is_valid(ct_transform transform) {
    return transform.idx != UINT32_MAX;
}

void transform_transform(ct_transform transform,
                         float *parent) {
    WorldInstance *world_inst = _get_world_instance(transform.world);

    float *pos = &world_inst->position[3 * transform.idx];
    float *rot = &world_inst->rotation[4 * transform.idx];
    float *sca = &world_inst->scale[3 * transform.idx];

//    float rm[16];
//    float sm[16];
    float m[16];
    float euler[3];

    ct_quat_to_euler(euler, rot);

//    ct_mat4_quat(rm, rot);
//    ct_mat4_scale(sm, sca[0], sca[1], sca[2]);
//
//    ct_mat4_mul(m, sm, rm);

//    m[4 * 3 + 0] = pos[0];
//    m[4 * 3 + 1] = pos[1];
//    m[4 * 3 + 2] = pos[2];

    ct_mat4_srt(m,
                sca[0], sca[1], sca[2],
                euler[0], euler[1], euler[2],
                pos[0], pos[1], pos[2]);

    ct_mat4_mul(&world_inst->world_matrix[16 * transform.idx], m, parent);

    uint32_t child = world_inst->first_child[transform.idx];

    ct_transform child_transform = {.idx = child, .world = transform.world};

    while (transform_is_valid(child_transform)) {
        transform_transform(child_transform,
                            &world_inst->world_matrix[16 * transform.idx]);

        child_transform.idx = world_inst->next_sibling[child_transform.idx];
    }
}


void transform_get_world_matrix(struct ct_entity transform,
                                float *value) {
    ct_cdb_obj_t *obj = ct_entity_a0.ent_obj(transform);
    const uint32_t idx = ct_cdb_a0.read_uint32(obj, CT_ID64_0("transform.idx"),
                                               UINT32_MAX);

    if (idx == UINT32_MAX) {
        return;
    }

    ct_world world = {
            .h = ct_cdb_a0.read_uint64(obj, CT_ID64_0("world"), 0)
    };

    WorldInstance *world_inst = _get_world_instance(world);
    memcpy(value, &world_inst->world_matrix[16 * idx], sizeof(float) * 16);
}

ct_transform transform_get(ct_world world,
                           ct_entity entity) {
    ct_cdb_obj_t *ent_obj = ct_entity_a0.ent_obj(entity);
    uint32_t idx = ct_cdb_a0.read_uint32(ent_obj, CT_ID64_0("transform.idx"),
                                         UINT32_MAX);
    return (ct_transform) {.idx = idx, .world = world};
}

void transform_link(ct_world world,
                    ct_entity parent,
                    ct_entity child) {

    WorldInstance *data = _get_world_instance(world);

    ct_transform parent_tr = transform_get(world, parent);
    ct_transform child_tr = transform_get(world, child);

    data->parent[child_tr.idx] = parent_tr.idx;

    uint32_t tmp = data->first_child[parent_tr.idx];

    data->first_child[parent_tr.idx] = child_tr.idx;
    data->next_sibling[child_tr.idx] = tmp;

    float p[16];

    if (parent_tr.idx != UINT32_MAX) {
        transform_get_world_matrix(parent, p);
    } else {
        ct_mat4_identity(p);
    }
    transform_transform(parent_tr, p);

    transform_get_world_matrix(parent, p);
    transform_transform(child_tr, p);
}


CETECH_MODULE_DEF(
        transform,
        {
            CETECH_GET_API(api, ct_component_a0);
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_yng_a0);
            CETECH_GET_API(api, ct_ydb_a0);
            CETECH_GET_API(api, ct_cdb_a0);
            CETECH_GET_API(api, ct_entity_a0);
            CETECH_GET_API(api, ct_world_a0);
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