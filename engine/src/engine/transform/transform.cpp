#include <cetech/core/config.h>
#include <cetech/engine/resource.h>
#include <cetech/core/hash.h>
#include <cetech/core/memory.h>
#include <cetech/core/api_system.h>
#include <celib/array.inl>
#include <celib/map.inl>
#include <cetech/core/yaml.h>

#include <cetech/engine/entity.h>
#include <celib/fpumath.h>


#include "cetech/engine/transform.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_component_a0);

using namespace celib;


int transform_is_valid(ct_transform transform);

void transform_transform(ct_transform transform,
                         float *parent);

void transform_get_position(ct_transform transform,
                            float *value);

void transform_get_rotation(ct_transform transform,
                            float *value);


void transform_get_scale(ct_transform transform,
                         float *value);


void transform_get_world_matrix(ct_transform transform,
                                float *value);


void transform_set_position(ct_transform transform,
                            float *pos);


void transform_set_rotation(ct_transform transform,
                            float *rot);


void transform_set_scale(ct_transform transform,
                         float *scale);

int transform_has(ct_world world,
                  ct_entity entity);

ct_transform transform_get(ct_world world,
                           ct_entity entity);

ct_transform transform_create(ct_world world,
                              ct_entity entity,
                              ct_entity parent,
                              float *position,
                              float *rotation,
                              float *scale);


void transform_link(ct_world world,
                    ct_entity parent,
                    ct_entity child);

struct transform_data {
    float position[3];
    float scale[3];
    float rotation[4];
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

    Map<uint32_t> world_map;
    Array<WorldInstance> world_instances;
    Map<uint32_t> ent_map;
} _G;


#define hash_combine(a, b) ((a * 11)^(b))


static void allocate(WorldInstance &_data,
                     cel_alloc *_allocator,
                     uint32_t sz) {
    //assert(sz > _data.n);

    WorldInstance new_data;
    const unsigned bytes = sz * (sizeof(ct_entity) +
                                 (3 * sizeof(uint32_t)) +
                                 (2 * sizeof(float) * 3) +
                                (sizeof(float) * 4) +
                                 (sizeof(float) * 16));
    new_data.buffer = CEL_ALLOCATE(_allocator, char, bytes);
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

    CEL_FREE(_allocator, _data.buffer);

    _data = new_data;
}

static void _new_world(ct_world world) {
    uint32_t idx = array::size(_G.world_instances);
    array::push_back(_G.world_instances, WorldInstance());
    _G.world_instances[idx].world = world;
    map::set(_G.world_map, world.h, idx);
}


static WorldInstance *_get_world_instance(ct_world world) {
    uint32_t idx = map::get(_G.world_map, world.h, UINT32_MAX);

    if (idx != UINT32_MAX) {
        return &_G.world_instances[idx];
    }

    return nullptr;
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

int _component_compiler(yaml_node_t body,
                        ct_blob *data) {
    transform_data t_data;

    YAML_NODE_SCOPE(scale, body, "scale",
                    yaml_as_vec3(scale, t_data.scale););
    YAML_NODE_SCOPE(position, body, "position",
                    yaml_as_vec3(position, t_data.position););

    {
        float v[3] = {};
        float v_rad[3] = {};

        yaml_node_t rotation = yaml_get_node(body, "rotation");
        yaml_as_vec3(rotation, v);

        celib::vec3_mul(v_rad, v, celib::DEG_TO_RAD);
        celib::quatFromEuler(t_data.rotation, v_rad[0], v_rad[1], v_rad[2]);

        yaml_node_free(rotation);
    };

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

    CEL_UNUSED(world);

    // TODO: remove from arrays, swap idx -> last AND change size
    for (uint32_t i = 0; i < ent_count; i++) {
        map::remove(_G.world_map, ents[i].h);
    }
}

static void _spawner(ct_world world,
                     ct_entity *ents,
                     uint32_t *cents,
                     uint32_t *ents_parent,
                     uint32_t ent_count,
                     void *data) {
    transform_data *tdata = (transform_data *) data;

    for (uint32_t i = 0; i < ent_count; ++i) {
        transform_create(world,
                         ents[cents[i]],
                         ents_parent[cents[i]] != UINT32_MAX
                         ? ents[ents_parent[cents[i]]]
                         : (ct_entity) {.h = UINT32_MAX},
                         tdata[i].position,
                         tdata[i].rotation,
                         tdata[i].scale);
    }

    float m[16];
    celib::mat4_identity(m);

    for (uint32_t i = 0; i < ent_count; ++i) {
        transform_transform(transform_get(world, ents[cents[i]]), m);
    }
}

void _set_property(ct_world world,
                   ct_entity entity,
                   uint64_t key,
                   ct_property_value value) {

    uint64_t position = ct_hash_a0.id64_from_str("position");
    uint64_t rotation = ct_hash_a0.id64_from_str("rotation");
    uint64_t scale = ct_hash_a0.id64_from_str("scale");

    ct_transform transform = transform_get(world, entity);

    if (key == position) {
        transform_set_position(transform, value.value.vec3f);

    } else if (key == rotation) {
        float rot[4];
        float euler_rot[3];
        float euler_rot_rad[3];

        celib::vec3_move(euler_rot, value.value.vec3f);
        celib::vec3_mul(euler_rot_rad, euler_rot, celib::DEG_TO_RAD);
        celib::quatFromEuler(rot,
                              euler_rot_rad[0],
                              euler_rot_rad[1],
                              euler_rot_rad[2]);

        transform_set_rotation(transform, rot);

    } else if (key == scale) {
        transform_set_scale(transform, value.value.vec3f);
    }

}

ct_property_value _get_property(ct_world world,
                                ct_entity entity,
                                uint64_t key) {
    uint64_t position = ct_hash_a0.id64_from_str("position");
    uint64_t rotation = ct_hash_a0.id64_from_str("rotation");
    uint64_t scale = ct_hash_a0.id64_from_str("scale");

    ct_transform transform = transform_get(world, entity);

    if (key == position) {
        ct_property_value v = {
                .type= PROPERTY_VEC3
        };

        transform_get_position(transform, v.value.vec3f);

    } else if (key == rotation) {
        float euler_rot[3];
        float euler_rot_rad[4];
        float rot[3];

        ct_property_value v = {
                .type= PROPERTY_VEC3,
        };

        transform_get_rotation(transform, rot);

        celib::quat_to_euler(euler_rot_rad, rot);
        celib::vec3_mul(euler_rot, euler_rot_rad, celib::RAD_TO_DEG);

        celib::vec3_move(v.value.vec3f, euler_rot);
        return v;

    } else if (key == scale) {
        float scal[3];

        ct_property_value v = {
                .type= PROPERTY_VEC3,
        };


        transform_get_scale(transform, scal);
        celib::vec3_move(v.value.vec3f, scal);

        return v;
    }

    return (ct_property_value) {.type= PROPERTY_INVALID};
}

static ct_transform_a0 _api = {
        .is_valid = transform_is_valid,
        .transform = transform_transform,
        .get_position = transform_get_position,
        .get_rotation = transform_get_rotation,
        .get_scale = transform_get_scale,
        .get_world_matrix = transform_get_world_matrix,
        .set_position = transform_set_position,
        .set_rotation = transform_set_rotation,
        .set_scale = transform_set_scale,
        .has = transform_has,
        .get = transform_get,
        .create = transform_create,
        .link = transform_link,
};

static void _init(ct_api_a0 *api) {
    api->register_api("ct_transform_a0", &_api);

    CETECH_GET_API(api, ct_component_a0);
    CETECH_GET_API(api, ct_memory_a0);
    CETECH_GET_API(api, ct_hash_a0);


    _G = {};

    _G.world_map.init(ct_memory_a0.main_allocator());
    _G.world_instances.init(ct_memory_a0.main_allocator());
    _G.ent_map.init(ct_memory_a0.main_allocator());

    _G.type = ct_hash_a0.id64_from_str("transform");

    ct_component_a0.register_type(
            _G.type,
            (ct_component_clb) {
                    .spawner=_spawner,
                    .destroyer=_destroyer,

                    .world_clb.on_created=_on_world_create,
                    .world_clb.on_destroy=_on_world_destroy,

                    .set_property=_set_property,
                    .get_property=_get_property
            }
    );

    ct_component_a0.register_compiler(_G.type, _component_compiler, 10);
}

static void _shutdown() {
    _G.world_map.destroy();
    _G.world_instances.destroy();
    _G.ent_map.destroy();
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

    float rm[16];
    float sm[16];
    float m[16];

    celib::mat4_quat(rm, rot);
    celib::mat4_scale(sm, sca[0], sca[1], sca[2]);

    celib::mat4_mul(m, sm, rm);

    m[4 * 3 + 0] = pos[0];
    m[4 * 3 + 1] = pos[1];
    m[4 * 3 + 2] = pos[2];

    celib::mat4_mul(&world_inst->world_matrix[16 * transform.idx], m, parent);

    uint32_t child = world_inst->first_child[transform.idx];

    ct_transform child_transform = {.idx = child, .world = transform.world};

    while (transform_is_valid(child_transform)) {
        transform_transform(child_transform,
                            &world_inst->world_matrix[16 * transform.idx]);

        child_transform.idx = world_inst->next_sibling[child_transform.idx];
    }
}

void transform_get_position(ct_transform node,
                            float *value) {

    WorldInstance *world_inst = _get_world_instance(node.world);
    celib::vec3_move(value, &world_inst->position[3 * node.idx]);
}

void transform_get_rotation(ct_transform node,
                            float *value) {

    WorldInstance *world_inst = _get_world_instance(node.world);

    celib::quat_move(value, &world_inst->rotation[4 * node.idx]);
}

void transform_get_scale(ct_transform node,
                         float *value) {

    WorldInstance *world_inst = _get_world_instance(node.world);
    celib::vec3_move(value, &world_inst->scale[3 * node.idx]);
}

void transform_get_world_matrix(ct_transform node,
                                float *value) {
    WorldInstance *world_inst = _get_world_instance(node.world);

    memcpy(value, &world_inst->world_matrix[16 * node.idx], sizeof(float) * 16);
}

void transform_set_position(ct_transform node,
                            float *pos) {

    WorldInstance *world_inst = _get_world_instance(node.world);

    uint32_t parent_idx = world_inst->parent[node.idx];

    ct_transform pt = {.idx = parent_idx, .world = node.world};

    float p[16];

    if (parent_idx != UINT32_MAX) {
        transform_get_world_matrix(pt, p);
    } else {
        celib::mat4_identity(p);
    }

    vec3_move(&world_inst->position[3 * node.idx], pos);

    transform_transform(node, p);
}

void transform_set_rotation(ct_transform node,
                            float *rot) {
    WorldInstance *world_inst = _get_world_instance(node.world);

    uint32_t parent_idx = world_inst->parent[node.idx];

    ct_transform pt = {.idx = parent_idx, .world = node.world};

    float p[16];

    if (parent_idx != UINT32_MAX) {
        transform_get_world_matrix(pt, p);
    } else {
        celib::mat4_identity(p);
    }

    float nq[4];
    celib::quat_norm(nq, rot);
    celib::quat_move(&world_inst->rotation[4 * node.idx], nq);

    transform_transform(node, p);
}

void transform_set_scale(ct_transform node,
                         float *scale) {
    WorldInstance *world_inst = _get_world_instance(node.world);

    uint32_t parent_idx = world_inst->parent[node.idx];

    ct_transform pt = {.idx = parent_idx, .world = node.world};

    float p[16];

    if (parent_idx != UINT32_MAX) {
        transform_get_world_matrix(pt, p);
    } else {
        celib::mat4_identity(p);
    }

    vec3_move(&world_inst->scale[3 * node.idx], scale);

    transform_transform(node, p);
}

int transform_has(ct_world world,
                  ct_entity entity) {
    uint32_t idx = hash_combine(world.h, entity.h);

    return map::has(_G.ent_map, idx);
}

ct_transform transform_get(ct_world world,
                           ct_entity entity) {

    uint32_t idx = hash_combine(world.h, entity.h);

    uint32_t component_idx = map::get(_G.ent_map, idx, UINT32_MAX);

    return (ct_transform) {.idx = component_idx, .world = world};
}

ct_transform transform_create(ct_world world,
                              ct_entity entity,
                              ct_entity parent,
                              float *position,
                              float *rotation,
                              float *scale) {

    WorldInstance *data = _get_world_instance(world);

    uint32_t idx = data->n;
    allocate(*data, ct_memory_a0.main_allocator(), data->n + 1);
    ++data->n;

    data->entity[idx] = entity;

    celib::vec3_move(&data->position[3 * idx], position);
    celib::quat_move(&data->rotation[4 * idx], rotation);
    celib::vec3_move(&data->scale[3 * idx], scale);

    data->parent[idx] = UINT32_MAX;
    data->first_child[idx] = UINT32_MAX;
    data->next_sibling[idx] = UINT32_MAX;

    float m[16];
    celib::mat4_identity(m);
    memcpy(&data->world_matrix[16 * idx], m, sizeof(float) * 16);

    ct_transform t = {.idx = idx, .world=world};

    float p[16];
    if (parent.h != UINT32_MAX) {
        transform_get_world_matrix(transform_get(world, parent), p);
    } else {
        celib::mat4_identity(p);
    }
    transform_transform(t, p);


    map::set(_G.ent_map, hash_combine(world.h, entity.h), idx);

    if (parent.h != UINT32_MAX) {
        uint32_t parent_idx = map::get(_G.ent_map,
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


    return t;
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
        transform_get_world_matrix(parent_tr, p);
    } else {
        celib::mat4_identity(p);
    }
    transform_transform(parent_tr, p);

    transform_get_world_matrix(parent_tr, p);
    transform_transform(child_tr, p);
}

extern "C" void transform_load_module(ct_api_a0 *api) {
    _init(api);
}

extern "C" void transform_unload_module(ct_api_a0 *api) {
    CEL_UNUSED(api)
    _shutdown();
}