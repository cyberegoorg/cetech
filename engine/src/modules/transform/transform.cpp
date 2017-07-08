#include <cetech/kernel/config.h>
#include <cetech/modules/resource.h>
#include <cetech/kernel/hash.h>
#include <cetech/celib/quatf.inl>
#include <cetech/celib/mat44f.inl>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/api_system.h>
#include <cetech/celib/array.inl>
#include <cetech/celib/map.inl>
#include <cetech/kernel/yaml.h>

#include <cetech/modules/entity.h>


#include "cetech/modules/transform.h"

CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_hash_a0);
CETECH_DECL_API(ct_component_a0);

using namespace cetech;

struct transform_data {
    vec3f_t position;
    vec3f_t scale;
    quatf_t rotation;
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

    vec3f_t *position;
    quatf_t *rotation;
    vec3f_t *scale;

    mat44f_t *world_matrix;
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
                     ct_allocator *_allocator,
                     uint32_t sz) {
    //assert(sz > _data.n);

    WorldInstance new_data;
    const unsigned bytes = sz * (sizeof(ct_entity) + (3 * sizeof(uint32_t)) +
                                 (2 * sizeof(vec3f_t)) + sizeof(quatf_t) +
                                 sizeof(mat44f_s));
    new_data.buffer = CETECH_ALLOCATE(_allocator, char, bytes);
    new_data.n = _data.n;
    new_data.allocated = sz;

    new_data.entity = (ct_entity *) (new_data.buffer);
    new_data.first_child = (uint32_t *) (new_data.entity + sz);
    new_data.next_sibling = (uint32_t *) (new_data.first_child + sz);
    new_data.parent = (uint32_t *) (new_data.next_sibling + sz);
    new_data.position = (vec3f_t *) (new_data.parent + sz);
    new_data.rotation = (quatf_t *) (new_data.position + sz);
    new_data.scale = (vec3f_t *) (new_data.rotation + sz);
    new_data.world_matrix = (mat44f_t *) (new_data.scale + sz);

    memcpy(new_data.entity, _data.entity, _data.n * sizeof(ct_entity));

    memcpy(new_data.first_child, _data.first_child, _data.n * sizeof(uint32_t));
    memcpy(new_data.next_sibling, _data.next_sibling,
           _data.n * sizeof(uint32_t));
    memcpy(new_data.parent, _data.parent, _data.n * sizeof(uint32_t));

    memcpy(new_data.position, _data.position, _data.n * sizeof(vec3f_t));
    memcpy(new_data.rotation, _data.rotation, _data.n * sizeof(quatf_t));
    memcpy(new_data.scale, _data.scale, _data.n * sizeof(vec3f_t));

    memcpy(new_data.world_matrix, _data.world_matrix,
           _data.n * sizeof(mat44f_t));

    CETECH_FREE(_allocator, _data.buffer);

    _data = new_data;
}

int transform_is_valid(ct_transform transform);

void transform_transform(ct_transform transform,
                         mat44f_t *parent);

vec3f_t transform_get_position(ct_transform transform);

quatf_t transform_get_rotation(ct_transform transform);


vec3f_t transform_get_scale(ct_transform transform);


mat44f_t *transform_get_world_matrix(ct_transform transform);


void transform_set_position(ct_transform transform,
                            vec3f_t pos);


void transform_set_rotation(ct_transform transform,
                            quatf_t rot);


void transform_set_scale(ct_transform transform,
                         vec3f_t scale);

int transform_has(ct_world world,
                  ct_entity entity);

ct_transform transform_get(ct_world world,
                           ct_entity entity);

ct_transform transform_create(ct_world world,
                              ct_entity entity,
                              ct_entity parent,
                              vec3f_t position,
                              quatf_t rotation,
                              vec3f_t scale);


void transform_link(ct_world world,
                    ct_entity parent,
                    ct_entity child);

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

    CETECH_FREE(ct_memory_a0.main_allocator(),
                _G.world_instances[idx].buffer);

    _G.world_instances[idx] = _G.world_instances[last_idx];
    map::set(_G.world_map, last_world.h, idx);
    array::pop_back(_G.world_instances);
}

int _transform_component_compiler(yaml_node_t body,
                                  ct_blob *data) {
    transform_data t_data;

    YAML_NODE_SCOPE(scale, body, "scale",
                    t_data.scale = yaml_as_vec3f_t(scale););
    YAML_NODE_SCOPE(position, body, "position",
                    t_data.position = yaml_as_vec3f_t(position););

    {
        yaml_node_t rotation = yaml_get_node(body, "rotation");
        vec3f_t v_rad = {0};

        vec3f_t v = yaml_as_vec3f_t(rotation);
        vec3f_mul(&v_rad, &v, CETECH_float_TORAD);

        quatf_from_euler(&t_data.rotation, v_rad.x, v_rad.y, v_rad.z);

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
                       size_t ent_count) {
    // TODO: remove from arrays, swap idx -> last AND change size
    for (int i = 0; i < ent_count; i++) {
        map::remove(_G.world_map, ents[i].h);
    }
}

static void _spawner(ct_world world,
                     ct_entity *ents,
                     uint32_t *cents,
                     uint32_t *ents_parent,
                     size_t ent_count,
                     void *data) {
    transform_data *tdata = (transform_data *) data;

    for (int i = 0; i < ent_count; ++i) {
        transform_create(world,
                         ents[cents[i]],
                         ents_parent[cents[i]] != UINT32_MAX
                         ? ents[ents_parent[cents[i]]]
                         : (ct_entity) {.h = UINT32_MAX},
                         tdata[i].position,
                         tdata[i].rotation,
                         tdata[i].scale);
    }

    mat44f_t m = MAT44F_INIT_IDENTITY;
    for (int i = 0; i < ent_count; ++i) {
        transform_transform(transform_get(world, ents[cents[i]]), &m);
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
        quatf_t rot = {0};
        vec3f_t euler_rot = value.value.vec3f;
        vec3f_t euler_rot_rad = {0};

        vec3f_mul(&euler_rot_rad, &euler_rot, CETECH_float_TORAD);
        quatf_from_euler(&rot, euler_rot_rad.x, euler_rot_rad.y,
                         euler_rot_rad.z);

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
        return (ct_property_value) {
                .type= PROPERTY_VEC3,
                .value.vec3f = transform_get_position(transform)
        };
    } else if (key == rotation) {
        vec3f_t euler_rot = {0};
        vec3f_t euler_rot_rad = {0};
        quatf_t rot = transform_get_rotation(transform);
        quatf_to_eurel_angle(&euler_rot_rad, &rot);
        vec3f_mul(&euler_rot, &euler_rot_rad, CETECH_float_TODEG);

        return (ct_property_value) {
                .type= PROPERTY_VEC3,
                .value.vec3f = euler_rot
        };
    } else if (key == scale) {
        return (ct_property_value) {
                .type= PROPERTY_VEC3,
                .value.vec3f = transform_get_scale(transform)
        };
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


    _G = {0};

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

    ct_component_a0.register_compiler(_G.type,
                                      _transform_component_compiler, 10);
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
                         mat44f_t *parent) {

    WorldInstance *world_inst = _get_world_instance(transform.world);

    vec3f_t pos = world_inst->position[transform.idx];
    quatf_t rot = world_inst->rotation[transform.idx];
    vec3f_t sca = world_inst->scale[transform.idx];

    mat44f_t rm = {0};
    mat44f_t sm = {0};
    mat44f_t m = {0};

    quatf_to_mat44f(&rm, &rot);
    mat44f_scale(&sm, sca.x, sca.y, sca.z);
    mat44f_mul(&m, &sm, &rm);

    m.w.x = pos.x;
    m.w.y = pos.y;
    m.w.z = pos.z;

    mat44f_mul(&world_inst->world_matrix[transform.idx], &m, parent);

    uint32_t child = world_inst->first_child[transform.idx];

    ct_transform child_transform = {.idx = child, .world = transform.world};

    while (transform_is_valid(child_transform)) {
        transform_transform(child_transform,
                            &world_inst->world_matrix[transform.idx]);

        child_transform.idx = world_inst->next_sibling[child_transform.idx];
    }
}

vec3f_t transform_get_position(ct_transform transform) {

    WorldInstance *world_inst = _get_world_instance(transform.world);
    return world_inst->position[transform.idx];
}

quatf_t transform_get_rotation(ct_transform transform) {

    WorldInstance *world_inst = _get_world_instance(transform.world);
    return world_inst->rotation[transform.idx];
}

vec3f_t transform_get_scale(ct_transform transform) {

    WorldInstance *world_inst = _get_world_instance(transform.world);
    return world_inst->scale[transform.idx];
}

mat44f_t *transform_get_world_matrix(ct_transform transform) {

    WorldInstance *world_inst = _get_world_instance(transform.world);
    return &world_inst->world_matrix[transform.idx];
}

void transform_set_position(ct_transform transform,
                            vec3f_t pos) {

    WorldInstance *world_inst = _get_world_instance(transform.world);

    uint32_t parent_idx = world_inst->parent[transform.idx];

    ct_transform pt = {.idx = parent_idx, .world = transform.world};

    mat44f_t m = MAT44F_INIT_IDENTITY;
    mat44f_t *p =
            parent_idx != UINT32_MAX ? transform_get_world_matrix(pt)
                                     : &m;

    world_inst->position[transform.idx] = pos;

    transform_transform(transform, p);
}

void transform_set_rotation(ct_transform transform,
                            quatf_t rot) {

    WorldInstance *world_inst = _get_world_instance(transform.world);

    uint32_t parent_idx = world_inst->parent[transform.idx];

    ct_transform pt = {.idx = parent_idx, .world = transform.world};

    mat44f_t m = MAT44F_INIT_IDENTITY;
    mat44f_t *p =
            parent_idx != UINT32_MAX ? transform_get_world_matrix(pt)
                                     : &m;

    quatf_t nq = {0};
    quatf_normalized(&nq, &rot);

    world_inst->rotation[transform.idx] = nq;

    transform_transform(transform, p);
}

void transform_set_scale(ct_transform transform,
                         vec3f_t scale) {

    WorldInstance *world_inst = _get_world_instance(transform.world);

    uint32_t parent_idx = world_inst->parent[transform.idx];

    ct_transform pt = {.idx = parent_idx, .world = transform.world};

    mat44f_t m = MAT44F_INIT_IDENTITY;
    mat44f_t *p =
            parent_idx != UINT32_MAX ? transform_get_world_matrix(pt)
                                     : &m;

    world_inst->scale[transform.idx] = scale;

    transform_transform(transform, p);
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
                              vec3f_t position,
                              quatf_t rotation,
                              vec3f_t scale) {

    WorldInstance *data = _get_world_instance(world);

    uint32_t idx = data->n;
    allocate(*data, ct_memory_a0.main_allocator(), data->n + 1);
    ++data->n;

    data->entity[idx] = entity;

    data->position[idx] = position;
    data->rotation[idx] = rotation;
    data->scale[idx] = scale;

    data->parent[idx] = UINT32_MAX;
    data->first_child[idx] = UINT32_MAX;
    data->next_sibling[idx] = UINT32_MAX;

    mat44f_t m = MAT44F_INIT_IDENTITY;
    memcpy(data->world_matrix[idx].f, m.f, sizeof(m));

    ct_transform t = {.idx = idx, .world=world};

    transform_transform(t, parent.h != UINT32_MAX ? transform_get_world_matrix(
            transform_get(world, parent)) : &m);

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

    mat44f_t m = MAT44F_INIT_IDENTITY;

    mat44f_t *p =
            parent_tr.idx != UINT32_MAX ? transform_get_world_matrix(parent_tr)
                                        : &m;

    transform_transform(parent_tr, p);
    transform_transform(child_tr,
                        transform_get_world_matrix(
                                transform_get(world, parent)));
}

extern "C" void transform_load_module(ct_api_a0 *api) {
    _init(api);
}

extern "C" void transform_unload_module(ct_api_a0 *api) {
    _shutdown();
}