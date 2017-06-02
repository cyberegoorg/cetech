#include <cetech/core/config.h>
#include <cetech/core/resource/resource.h>
#include <cetech/core/hash.h>
#include <cetech/core/math/quatf.inl>
#include <cetech/core/math/mat44f.inl>
#include <cetech/core/memory/memory.h>
#include <cetech/core/module.h>
#include <cetech/core/api.h>
#include <cetech/core/container/array2.inl>
#include <cetech/core/container/map2.inl>
#include <cetech/core/yaml.h>

#include <cetech/modules/entity/entity.h>
#include <cetech/modules/world/world.h>
#include <cetech/modules/component/component.h>

#include "../transform.h"

using namespace cetech;

struct transform_data {
    vec3f_t position;
    vec3f_t scale;
    quatf_t rotation;
};

struct WorldInstance {
    uint32_t n;
    uint32_t allocated;
    void *buffer;

    entity_t *entity;

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

IMPORT_API(memory_api_v0);
IMPORT_API(hash_api_v0);

#define hash_combine(a, b) ((a * 11)^(b))

static void allocate(WorldInstance &_data,
                     allocator *_allocator,
                     uint32_t sz) {
    //assert(sz > _data.n);

    WorldInstance new_data;
    const unsigned bytes = sz * (sizeof(entity_t) + (3 * sizeof(uint32_t)) +
                                 (2 * sizeof(vec3f_t)) + sizeof(quatf_t) +
                                 sizeof(mat44f_s));
    new_data.buffer = CETECH_ALLOCATE(_allocator, char, bytes);
    new_data.n = _data.n;
    new_data.allocated = sz;

    new_data.entity = (entity_t *) (new_data.buffer);
    new_data.first_child = (uint32_t *) (new_data.entity + sz);
    new_data.next_sibling = (uint32_t *) (new_data.first_child + sz);
    new_data.parent = (uint32_t *) (new_data.next_sibling + sz);
    new_data.position = (vec3f_t *) (new_data.parent + sz);
    new_data.rotation = (quatf_t *) (new_data.position + sz);
    new_data.scale = (vec3f_t *) (new_data.rotation + sz);
    new_data.world_matrix = (mat44f_t *) (new_data.scale + sz);

    memcpy(new_data.entity, _data.entity, _data.n * sizeof(entity_t));

    memcpy(new_data.first_child, _data.first_child, _data.n * sizeof(uint32_t));
    memcpy(new_data.next_sibling, _data.next_sibling,
           _data.n * sizeof(uint32_t));
    memcpy(new_data.parent, _data.parent, _data.n * sizeof(uint32_t));

    memcpy(new_data.position, _data.position, _data.n * sizeof(vec3f_t));
    memcpy(new_data.rotation, _data.rotation, _data.n * sizeof(quatf_t));
    memcpy(new_data.scale, _data.scale, _data.n * sizeof(vec3f_t));

    memcpy(new_data.world_matrix, _data.world_matrix,
           _data.n * sizeof(mat44f_t));

    CETECH_DEALLOCATE(_allocator, _data.buffer);

    _data = new_data;
}

int transform_is_valid(transform_t transform);

void transform_transform(world_t world,
                         transform_t transform,
                         mat44f_t *parent);

vec3f_t transform_get_position(world_t world,
                               transform_t transform);

quatf_t transform_get_rotation(world_t world,
                               transform_t transform);


vec3f_t transform_get_scale(world_t world,
                            transform_t transform);


mat44f_t *transform_get_world_matrix(world_t world,
                                     transform_t transform);


void transform_set_position(world_t world,
                            transform_t transform,
                            vec3f_t pos);


void transform_set_rotation(world_t world,
                            transform_t transform,
                            quatf_t rot);


void transform_set_scale(world_t world,
                         transform_t transform,
                         vec3f_t scale);

int transform_has(world_t world,
                  entity_t entity);

transform_t transform_get(world_t world,
                          entity_t entity);

transform_t transform_create(world_t world,
                             entity_t entity,
                             entity_t parent,
                             vec3f_t position,
                             quatf_t rotation,
                             vec3f_t scale);


void transform_link(world_t world,
                    entity_t parent,
                    entity_t child);

static void _new_world(world_t world) {
    uint32_t idx = array::size(_G.world_instances);
    array::push_back(_G.world_instances, WorldInstance());
    map::set(_G.world_map, world.h, idx);
}


static WorldInstance *_get_world_instance(world_t world) {
    uint32_t idx = map::get(_G.world_map, world.h, UINT32_MAX);

    if (idx != UINT32_MAX) {
        return &_G.world_instances[idx];
    }

    return nullptr;
}

static void _destroy_world(world_t world) {
// TODO: impl
}

int _transform_component_compiler(yaml_node_t body,
                                  ARRAY_T(uint8_t) *data) {

    struct transform_data t_data;

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
    // TODO: remove from arrays, swap idx -> last AND change size
    for (int i = 0; i < ent_count; i++) {
        map::remove(_G.world_map, ents[i].h);
    }
}

static void _spawner(world_t world,
                     entity_t *ents,
                     uint32_t *cents,
                     uint32_t *ents_parent,
                     size_t ent_count,
                     void *data) {
    struct transform_data *tdata = (transform_data *) data;

    for (int i = 0; i < ent_count; ++i) {
        transform_create(world,
                         ents[cents[i]],
                         ents_parent[cents[i]] != UINT32_MAX
                         ? ents[ents_parent[cents[i]]]
                         : (entity_t) {.h = UINT32_MAX},
                         tdata[i].position,
                         tdata[i].rotation,
                         tdata[i].scale);
    }

    mat44f_t m = MAT44F_INIT_IDENTITY;
    for (int i = 0; i < ent_count; ++i) {
        transform_transform(world, transform_get(world, ents[cents[i]]), &m);
    }
}

void _set_property(world_t world,
                   entity_t entity,
                   uint64_t key,
                   struct property_value value) {

    uint64_t position = hash_api_v0.id64_from_str("position");
    uint64_t rotation = hash_api_v0.id64_from_str("rotation");
    uint64_t scale = hash_api_v0.id64_from_str("scale");

    transform_t transform = transform_get(world, entity);

    if (key == position) {
        transform_set_position(world, transform, value.value.vec3f);

    } else if (key == rotation) {
        quatf_t rot = {0};
        vec3f_t euler_rot = value.value.vec3f;
        vec3f_t euler_rot_rad = {0};

        vec3f_mul(&euler_rot_rad, &euler_rot, CETECH_float_TORAD);
        quatf_from_euler(&rot, euler_rot_rad.x, euler_rot_rad.y,
                         euler_rot_rad.z);

        transform_set_rotation(world, transform, rot);

    } else if (key == scale) {
        transform_set_scale(world, transform, value.value.vec3f);
    }

}

struct property_value _get_property(world_t world,
                                    entity_t entity,
                                    uint64_t key) {
    uint64_t position = hash_api_v0.id64_from_str("position");
    uint64_t rotation = hash_api_v0.id64_from_str("rotation");
    uint64_t scale = hash_api_v0.id64_from_str("scale");

    transform_t transform = transform_get(world, entity);

    if (key == position) {
        return (struct property_value) {
                .type= PROPERTY_VEC3,
                .value.vec3f = transform_get_position(world, transform)
        };
    } else if (key == rotation) {
        vec3f_t euler_rot = {0};
        vec3f_t euler_rot_rad = {0};
        quatf_t rot = transform_get_rotation(world, transform);
        quatf_to_eurel_angle(&euler_rot_rad, &rot);
        vec3f_mul(&euler_rot, &euler_rot_rad, CETECH_float_TODEG);

        return (struct property_value) {
                .type= PROPERTY_VEC3,
                .value.vec3f = euler_rot
        };
    } else if (key == scale) {
        return (struct property_value) {
                .type= PROPERTY_VEC3,
                .value.vec3f = transform_get_scale(world, transform)
        };
    }

    return (struct property_value) {.type= PROPERTY_INVALID};
}

IMPORT_API(component_api_v0);

static void _init_api(struct api_v0 *api) {
    static struct transform_api_v0 _api = {0};

    _api.is_valid = transform_is_valid;
    _api.transform = transform_transform;
    _api.get_position = transform_get_position;
    _api.get_rotation = transform_get_rotation;
    _api.get_scale = transform_get_scale;
    _api.get_world_matrix = transform_get_world_matrix;
    _api.set_position = transform_set_position;
    _api.set_rotation = transform_set_rotation;
    _api.set_scale = transform_set_scale;
    _api.has = transform_has;
    _api.get = transform_get;
    _api.create = transform_create;
    _api.link = transform_link;

    api->register_api("transform_api_v0", &_api);
}

static void _init(struct api_v0 *api) {
    GET_API(api, component_api_v0);
    GET_API(api, memory_api_v0);
    GET_API(api, hash_api_v0);


    _G = (struct _G) {0};

    _G.world_map.init(memory_api_v0.main_allocator());
    _G.world_instances.init(memory_api_v0.main_allocator());
    _G.ent_map.init(memory_api_v0.main_allocator());

    _G.type = hash_api_v0.id64_from_str("transform");

    component_api_v0.register_type(
            _G.type,
            (struct component_clb) {
                    .spawner=_spawner,
                    .destroyer=_destroyer,

                    .on_world_create=_on_world_create,
                    .on_world_destroy=_on_world_destroy,

                    .set_property=_set_property,
                    .get_property=_get_property
            }
    );

    component_api_v0.register_compiler(_G.type,
                                       _transform_component_compiler, 10);
}

static void _shutdown() {
    _G = (struct _G) {0};
}


int transform_is_valid(transform_t transform) {
    return transform.idx != UINT32_MAX;
}

void transform_transform(world_t world,
                         transform_t transform,
                         mat44f_t *parent) {
    WorldInstance *world_inst = _get_world_instance(world);

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

    transform_t child_transform = {.idx = child};

    while (transform_is_valid(child_transform)) {
        transform_transform(world, child_transform,
                            &world_inst->world_matrix[transform.idx]);

        child_transform.idx = world_inst->next_sibling[child_transform.idx];
    }
}

vec3f_t transform_get_position(world_t world,
                               transform_t transform) {

    WorldInstance *world_inst = _get_world_instance(world);
    return world_inst->position[transform.idx];
}

quatf_t transform_get_rotation(world_t world,
                               transform_t transform) {

    WorldInstance *world_inst = _get_world_instance(world);
    return world_inst->rotation[transform.idx];
}

vec3f_t transform_get_scale(world_t world,
                            transform_t transform) {

    WorldInstance *world_inst = _get_world_instance(world);
    return world_inst->scale[transform.idx];
}

mat44f_t *transform_get_world_matrix(world_t world,
                                     transform_t transform) {

    WorldInstance *world_inst = _get_world_instance(world);
    return &world_inst->world_matrix[transform.idx];
}

void transform_set_position(world_t world,
                            transform_t transform,
                            vec3f_t pos) {
    WorldInstance *world_inst = _get_world_instance(world);

    uint32_t parent_idx = world_inst->parent[transform.idx];

    transform_t pt = {.idx = parent_idx};

    mat44f_t m = MAT44F_INIT_IDENTITY;
    mat44f_t *p =
            parent_idx != UINT32_MAX ? transform_get_world_matrix(world, pt)
                                     : &m;

    world_inst->position[transform.idx] = pos;

    transform_transform(world, transform, p);
}

void transform_set_rotation(world_t world,
                            transform_t transform,
                            quatf_t rot) {
    WorldInstance *world_inst = _get_world_instance(world);

    uint32_t parent_idx = world_inst->parent[transform.idx];

    transform_t pt = {.idx = parent_idx};

    mat44f_t m = MAT44F_INIT_IDENTITY;
    mat44f_t *p =
            parent_idx != UINT32_MAX ? transform_get_world_matrix(world, pt)
                                     : &m;

    quatf_t nq = {0};
    quatf_normalized(&nq, &rot);

    world_inst->rotation[transform.idx] = nq;

    transform_transform(world, transform, p);
}

void transform_set_scale(world_t world,
                         transform_t transform,
                         vec3f_t scale) {
    WorldInstance *world_inst = _get_world_instance(world);

    uint32_t parent_idx = world_inst->parent[transform.idx];

    transform_t pt = {.idx = parent_idx};

    mat44f_t m = MAT44F_INIT_IDENTITY;
    mat44f_t *p =
            parent_idx != UINT32_MAX ? transform_get_world_matrix(world, pt)
                                     : &m;

    world_inst->scale[transform.idx] = scale;

    transform_transform(world, transform, p);
}

int transform_has(world_t world,
                  entity_t entity) {
    uint32_t idx = hash_combine(world.h, entity.h);

    return map::has(_G.ent_map, idx);
}

transform_t transform_get(world_t world,
                          entity_t entity) {

    uint32_t idx = hash_combine(world.h, entity.h);

    uint32_t component_idx = map::get(_G.ent_map, idx, UINT32_MAX);

    return (transform_t) {.idx = component_idx};
}

transform_t transform_create(world_t world,
                             entity_t entity,
                             entity_t parent,
                             vec3f_t position,
                             quatf_t rotation,
                             vec3f_t scale) {

    WorldInstance *data = _get_world_instance(world);

    uint32_t idx = data->n;
    allocate(*data, memory_api_v0.main_allocator(), data->n + 1);
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

    transform_t t = {.idx = idx};
    transform_transform(world, t,
                        parent.h != UINT32_MAX ? transform_get_world_matrix(
                                world, transform_get(world, parent))
                                               : &m);

    map::set(_G.ent_map, hash_combine(world.h, entity.h), idx);

    if (parent.h != UINT32_MAX) {
        uint32_t parent_idx = map::get(_G.ent_map, hash_combine(world.h, parent.h),
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


    return (transform_t) {.idx = idx};
}

void transform_link(world_t world,
                    entity_t parent,
                    entity_t child) {

    WorldInstance *data = _get_world_instance(world);

    transform_t parent_tr = transform_get(world, parent);
    transform_t child_tr = transform_get(world, child);

    data->parent[child_tr.idx] = parent_tr.idx;

    uint32_t tmp = data->first_child[parent_tr.idx];

    data->first_child[parent_tr.idx] = child_tr.idx;
    data->next_sibling[child_tr.idx] = tmp;

    mat44f_t m = MAT44F_INIT_IDENTITY;

    mat44f_t *p =
            parent_tr.idx != UINT32_MAX ? transform_get_world_matrix(world,
                                                                     parent_tr)
                                        : &m;

    transform_transform(world, parent_tr, p);
    transform_transform(world, child_tr, transform_get_world_matrix(world,
                                                                    transform_get(
                                                                            world,
                                                                            parent)));
}

extern "C" void *transform_get_module_api(int api) {
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