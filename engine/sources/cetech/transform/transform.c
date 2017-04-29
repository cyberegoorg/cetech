#include <cetech/array.inl>
#include <cetech/yaml.h>
#include <cetech/entity.h>
#include <cetech/component.h>
#include <cetech/math_quatf.h>
#include <cetech/math_mat44f.inl>
#include <cetech/transform.h>
#include <cetech/memory.h>
#include <cetech/module.h>
#include <cetech/stringid.h>
#include <cetech/math_types.h>


struct transform_data {
    cel_vec3f_t position;
    cel_vec3f_t scale;
    cel_quatf_t rotation;
};

ARRAY_PROTOTYPE(cel_vec3f_t)

ARRAY_PROTOTYPE(cel_mat44f_t)

ARRAY_PROTOTYPE(cel_quatf_t)

typedef struct {
    MAP_T(uint32_t) ent_idx_map;

    ARRAY_T(uint32_t) first_child;
    ARRAY_T(uint32_t) next_sibling;
    ARRAY_T(uint32_t) parent;

    ARRAY_T(cel_vec3f_t) position;
    ARRAY_T(cel_quatf_t) rotation;
    ARRAY_T(cel_vec3f_t) scale;
    ARRAY_T(cel_mat44f_t) world_matrix;
} world_data_t;

ARRAY_PROTOTYPE(world_data_t)

MAP_PROTOTYPE(world_data_t)


#define _G TransformGlobal
static struct G {
    stringid64_t type;

    MAP_T(world_data_t) world;
} _G = {0};

IMPORT_API(memory_api_v0);

int transform_is_valid(transform_t transform);

void transform_transform(world_t world,
                         transform_t transform,
                         cel_mat44f_t *parent);

cel_vec3f_t transform_get_position(world_t world,
                                   transform_t transform);

cel_quatf_t transform_get_rotation(world_t world,
                                   transform_t transform);


cel_vec3f_t transform_get_scale(world_t world,
                                transform_t transform);


cel_mat44f_t *transform_get_world_matrix(world_t world,
                                         transform_t transform);


void transform_set_position(world_t world,
                            transform_t transform,
                            cel_vec3f_t pos);


void transform_set_rotation(world_t world,
                            transform_t transform,
                            cel_quatf_t rot);


void transform_set_scale(world_t world,
                         transform_t transform,
                         cel_vec3f_t scale);

int transform_has(world_t world,
                  entity_t entity);

transform_t transform_get(world_t world,
                          entity_t entity);

transform_t transform_create(world_t world,
                             entity_t entity,
                             entity_t parent,
                             cel_vec3f_t position,
                             cel_quatf_t rotation,
                             cel_vec3f_t scale);


void transform_link(world_t world,
                    entity_t parent,
                    entity_t child);


static void _new_world(world_t world) {
    world_data_t data = {0};

    MAP_INIT(uint32_t, &data.ent_idx_map, memory_api_v0.main_allocator());

    ARRAY_INIT(uint32_t, &data.first_child, memory_api_v0.main_allocator());
    ARRAY_INIT(uint32_t, &data.next_sibling, memory_api_v0.main_allocator());
    ARRAY_INIT(uint32_t, &data.parent, memory_api_v0.main_allocator());

    ARRAY_INIT(cel_vec3f_t, &data.position, memory_api_v0.main_allocator());
    ARRAY_INIT(cel_quatf_t, &data.rotation, memory_api_v0.main_allocator());
    ARRAY_INIT(cel_vec3f_t, &data.scale, memory_api_v0.main_allocator());
    ARRAY_INIT(cel_mat44f_t, &data.world_matrix, memory_api_v0.main_allocator());

    MAP_SET(world_data_t, &_G.world, world.h.id, data);
}

static world_data_t *_get_world_data(world_t world) {
    return MAP_GET_PTR(world_data_t, &_G.world, world.h.id);
}

static void _destroy_world(world_t world) {
    world_data_t *data = _get_world_data(world);

    MAP_DESTROY(uint32_t, &data->ent_idx_map);

    ARRAY_DESTROY(uint32_t, &data->first_child);
    ARRAY_DESTROY(uint32_t, &data->next_sibling);
    ARRAY_DESTROY(uint32_t, &data->parent);

    ARRAY_DESTROY(cel_vec3f_t, &data->position);
    ARRAY_DESTROY(cel_quatf_t, &data->rotation);
    ARRAY_DESTROY(cel_vec3f_t, &data->scale);
    ARRAY_DESTROY(cel_mat44f_t, &data->world_matrix);
}

int _transform_component_compiler(yaml_node_t body,
                                  ARRAY_T(uint8_t) *data) {

    struct transform_data t_data;

    YAML_NODE_SCOPE(scale, body, "scale",
                    t_data.scale = yaml_as_cel_vec3f_t(scale););
    YAML_NODE_SCOPE(position, body, "position",
                    t_data.position = yaml_as_cel_vec3f_t(position););

    {
        yaml_node_t rotation = yaml_get_node(body, "rotation");
        cel_vec3f_t v_rad = {0};

        cel_vec3f_t v = yaml_as_cel_vec3f_t(rotation);
        cel_vec3f_mul(&v_rad, &v, CEL_float_TORAD);

        cel_quatf_from_euler(&t_data.rotation, v_rad.x, v_rad.y, v_rad.z);

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
    world_data_t *world_data = _get_world_data(world);

    // TODO: remove from arrays, swap idx -> last AND change size
    for (int i = 0; i < ent_count; i++) {
        CEL_ASSERT("transform",
                   MAP_HAS(uint32_t, &world_data->ent_idx_map, ents[i].h.id));
        MAP_REMOVE(uint32_t, &world_data->ent_idx_map, ents[i].h.id);
    }
}

static void _spawner(world_t world,
                     entity_t *ents,
                     uint32_t *cents,
                     uint32_t *ents_parent,
                     size_t ent_count,
                     void *data) {
    struct transform_data *tdata = data;

    for (int i = 0; i < ent_count; ++i) {
        transform_create(world,
                         ents[cents[i]],
                         ents_parent[cents[i]] != UINT32_MAX
                         ? ents[ents_parent[cents[i]]]
                         : (entity_t) {.h.id = UINT32_MAX},
                         tdata[i].position,
                         tdata[i].rotation,
                         tdata[i].scale);
    }

    cel_mat44f_t m = MAT44F_INIT_IDENTITY;
    for (int i = 0; i < ent_count; ++i) {
        transform_transform(world, transform_get(world, ents[cents[i]]), &m);
    }
}

void _set_property(world_t world,
                     entity_t entity,
                     stringid64_t key,
                     struct property_value value) {

    stringid64_t position = stringid64_from_string("position");
    stringid64_t rotation = stringid64_from_string("rotation");
    stringid64_t scale = stringid64_from_string("scale");

    transform_t transform = transform_get(world, entity);

    if(key.id == position.id){
        transform_set_position(world, transform, value.value.vec3f);

    } else if (key.id == rotation.id) {
        cel_quatf_t rot = {0};
        cel_vec3f_t euler_rot = value.value.vec3f;
        cel_vec3f_t euler_rot_rad = {0};

        cel_vec3f_mul(&euler_rot_rad, &euler_rot, CEL_float_TORAD);
        cel_quatf_from_euler(&rot, euler_rot_rad.x, euler_rot_rad.y, euler_rot_rad.z);

        transform_set_rotation(world, transform, rot);

    } else if (key.id == scale.id) {
        transform_set_scale(world, transform, value.value.vec3f);
    }

}

struct property_value _get_property(world_t world,
                                    entity_t entity,
                                    stringid64_t key) {
    stringid64_t position = stringid64_from_string("position");
    stringid64_t rotation = stringid64_from_string("rotation");
    stringid64_t scale = stringid64_from_string("scale");

    transform_t transform = transform_get(world, entity);

    if(key.id == position.id){
        return  (struct property_value){
                .type= PROPERTY_VEC3,
                .value.vec3f = transform_get_position(world, transform)
        };
    } else if (key.id == rotation.id) {
        cel_vec3f_t euler_rot = {0};
        cel_vec3f_t euler_rot_rad = {0};
        cel_quatf_t rot = transform_get_rotation(world, transform);
        cel_quatf_to_eurel_angle(&euler_rot_rad, &rot);
        cel_vec3f_mul(&euler_rot, &euler_rot_rad, CEL_float_TODEG);

        return  (struct property_value){
                .type= PROPERTY_VEC3,
                .value.vec3f = euler_rot
        };
    } else if (key.id == scale.id) {
        return  (struct property_value){
                .type= PROPERTY_VEC3,
                .value.vec3f = transform_get_scale(world, transform)
        };
    }

    return (struct property_value){.type= PROPERTY_INVALID};
}

IMPORT_API(component_api_v0);

static void _init(get_api_fce_t get_engine_api) {
    INIT_API(get_engine_api, component_api_v0, COMPONENT_API_ID);
    INIT_API(get_engine_api, memory_api_v0, MEMORY_API_ID);

    _G = (struct G) {0};

    MAP_INIT(world_data_t, &_G.world, memory_api_v0.main_allocator());

    _G.type = stringid64_from_string("transform");

    component_api_v0.component_register_compiler(_G.type,
                                                     _transform_component_compiler,
                                                     10);
    component_api_v0.component_register_type(
            _G.type,
            (struct component_clb) {
                    .spawner=_spawner, .destroyer=_destroyer,
                    .on_world_create=_on_world_create, .on_world_destroy=_on_world_destroy,
                    .set_property=_set_property, .get_property=_get_property
            }
    );
}

static void _shutdown() {
    MAP_DESTROY(world_data_t, &_G.world);

    _G = (struct G) {0};
}


int transform_is_valid(transform_t transform) {
    return transform.idx != UINT32_MAX;
}

void transform_transform(world_t world,
                         transform_t transform,
                         cel_mat44f_t *parent) {
    world_data_t *world_data = _get_world_data(world);

    cel_vec3f_t pos = ARRAY_AT(&world_data->position, transform.idx);
    cel_quatf_t rot = ARRAY_AT(&world_data->rotation, transform.idx);
    cel_vec3f_t sca = ARRAY_AT(&world_data->scale, transform.idx);

    cel_mat44f_t rm = {0};
    cel_mat44f_t sm = {0};
    cel_mat44f_t m = {0};

    cel_quatf_to_mat44f(&rm, &rot);
    cel_mat44f_scale(&sm, sca.x, sca.y, sca.z);
    cel_mat44f_mul(&m, &sm, &rm);

    m.w.x = pos.x;
    m.w.y = pos.y;
    m.w.z = pos.z;

    cel_mat44f_mul(&ARRAY_AT(&world_data->world_matrix, transform.idx), &m,
                   parent);

    uint32_t child = ARRAY_AT(&world_data->first_child, transform.idx);

    transform_t child_transform = {.idx = child};

    while (transform_is_valid(child_transform)) {
        transform_transform(world, child_transform,
                            &ARRAY_AT(&world_data->world_matrix,
                                      transform.idx));
        child_transform.idx = ARRAY_AT(&world_data->next_sibling,
                                       child_transform.idx);
    }
}

cel_vec3f_t transform_get_position(world_t world,
                                   transform_t transform) {

    world_data_t *world_data = _get_world_data(world);
    return ARRAY_AT(&world_data->position, transform.idx);
}

cel_quatf_t transform_get_rotation(world_t world,
                                   transform_t transform) {

    world_data_t *world_data = _get_world_data(world);
    return ARRAY_AT(&world_data->rotation, transform.idx);
}

cel_vec3f_t transform_get_scale(world_t world,
                                transform_t transform) {

    world_data_t *world_data = _get_world_data(world);
    return ARRAY_AT(&world_data->scale, transform.idx);
}

cel_mat44f_t *transform_get_world_matrix(world_t world,
                                         transform_t transform) {

    world_data_t *world_data = _get_world_data(world);
    return &ARRAY_AT(&world_data->world_matrix, transform.idx);
}

void transform_set_position(world_t world,
                            transform_t transform,
                            cel_vec3f_t pos) {
    world_data_t *world_data = _get_world_data(world);
    uint32_t parent_idx = ARRAY_AT(&world_data->parent, transform.idx);

    transform_t pt = {.idx = parent_idx};

    cel_mat44f_t m = MAT44F_INIT_IDENTITY;
    cel_mat44f_t *p =
            parent_idx != UINT32_MAX ? transform_get_world_matrix(world, pt)
                                     : &m;

    ARRAY_AT(&world_data->position, transform.idx) = pos;

    transform_transform(world, transform, p);
}

void transform_set_rotation(world_t world,
                            transform_t transform,
                            cel_quatf_t rot) {
    world_data_t *world_data = _get_world_data(world);
    uint32_t parent_idx = ARRAY_AT(&world_data->parent, transform.idx);

    transform_t pt = {.idx = parent_idx};

    cel_mat44f_t m = MAT44F_INIT_IDENTITY;
    cel_mat44f_t *p =
            parent_idx != UINT32_MAX ? transform_get_world_matrix(world, pt)
                                     : &m;

    cel_quatf_t nq = {0};
    cel_quatf_normalized(&nq, &rot);

    ARRAY_AT(&world_data->rotation, transform.idx) = nq;

    transform_transform(world, transform, p);
}

void transform_set_scale(world_t world,
                         transform_t transform,
                         cel_vec3f_t scale) {
    world_data_t *world_data = _get_world_data(world);
    uint32_t parent_idx = ARRAY_AT(&world_data->parent, transform.idx);

    transform_t pt = {.idx = parent_idx};

    cel_mat44f_t m = MAT44F_INIT_IDENTITY;
    cel_mat44f_t *p =
            parent_idx != UINT32_MAX ? transform_get_world_matrix(world, pt)
                                     : &m;

    ARRAY_AT(&world_data->scale, transform.idx) = scale;

    transform_transform(world, transform, p);
}

int transform_has(world_t world,
                  entity_t entity) {
    world_data_t *world_data = _get_world_data(world);
    return MAP_HAS(uint32_t, &world_data->ent_idx_map, entity.h.id);
}

transform_t transform_get(world_t world,
                          entity_t entity) {

    world_data_t *world_data = _get_world_data(world);
    uint32_t idx = MAP_GET(uint32_t, &world_data->ent_idx_map, entity.h.id, UINT32_MAX);
    return (transform_t) {.idx = idx};
}

transform_t transform_create(world_t world,
                             entity_t entity,
                             entity_t parent,
                             cel_vec3f_t position,
                             cel_quatf_t rotation,
                             cel_vec3f_t scale) {

    world_data_t *data = _get_world_data(world);

    uint32_t idx = (uint32_t) ARRAY_SIZE(&data->position);

    ARRAY_PUSH_BACK(cel_vec3f_t, &data->position, position);
    ARRAY_PUSH_BACK(cel_quatf_t, &data->rotation, rotation);
    ARRAY_PUSH_BACK(cel_vec3f_t, &data->scale, scale);

    ARRAY_PUSH_BACK(uint32_t, &data->parent, UINT32_MAX);
    ARRAY_PUSH_BACK(uint32_t, &data->first_child, UINT32_MAX);
    ARRAY_PUSH_BACK(uint32_t, &data->next_sibling, UINT32_MAX);

    cel_mat44f_t m = MAT44F_INIT_IDENTITY;
    ARRAY_PUSH_BACK(cel_mat44f_t, &data->world_matrix, m);

    transform_t t = {.idx = idx};
    transform_transform(world, t,
                        parent.h.id != UINT32_MAX ? transform_get_world_matrix(
                                world, transform_get(world, parent))
                                                 : &m);

    MAP_SET(uint32_t, &data->ent_idx_map, entity.h.id, idx);

    if (parent.h.id != UINT32_MAX) {
        uint32_t parent_idx = MAP_GET(uint32_t, &data->ent_idx_map, parent.h.id,
                                 UINT32_MAX);

        ARRAY_AT(&data->parent, idx) = parent_idx;

        if (ARRAY_AT(&data->first_child, parent_idx) == UINT32_MAX) {
            ARRAY_AT(&data->first_child, parent_idx) = idx;
        } else {
            uint32_t first_child_idx = ARRAY_AT(&data->first_child, parent_idx);
            ARRAY_AT(&data->first_child, parent_idx) = idx;
            ARRAY_AT(&data->next_sibling, idx) = first_child_idx;
        }

        ARRAY_AT(&data->parent, idx) = parent_idx;

    }


    return (transform_t) {.idx = idx};
}

void transform_link(world_t world,
                    entity_t parent,
                    entity_t child) {
    world_data_t *data = _get_world_data(world);

    transform_t parent_tr = transform_get(world, parent);
    transform_t child_tr = transform_get(world, child);

    ARRAY_AT(&data->parent, child_tr.idx) = parent_tr.idx;

    uint32_t tmp = ARRAY_AT(&data->first_child, parent_tr.idx);

    ARRAY_AT(&data->first_child, parent_tr.idx) = child_tr.idx;
    ARRAY_AT(&data->next_sibling, child_tr.idx) = tmp;

    cel_mat44f_t m = MAT44F_INIT_IDENTITY;

    cel_mat44f_t *p =
            parent_tr.idx != UINT32_MAX ? transform_get_world_matrix(world,
                                                                     parent_tr)
                                        : &m;

    transform_transform(world, parent_tr, p);
    transform_transform(world, child_tr, transform_get_world_matrix(world,
                                                                    transform_get(
                                                                            world,
                                                                            parent)));
}

void *transform_get_module_api(int api) {
    switch (api) {
        case PLUGIN_EXPORT_API_ID:
                {
                    static struct module_api_v0 module = {0};

                    module.init = _init;
                    module.shutdown = _shutdown;

                    return &module;
                }

        case TRANSFORM_API_ID:
                 {
                    static struct transform_api_v0 api = {0};

                    api.is_valid = transform_is_valid;
                    api.transform = transform_transform;
                    api.get_position = transform_get_position;
                    api.get_rotation = transform_get_rotation;
                    api.get_scale = transform_get_scale;
                    api.get_world_matrix = transform_get_world_matrix;
                    api.set_position = transform_set_position;
                    api.set_rotation = transform_set_rotation;
                    api.set_scale = transform_set_scale;
                    api.has = transform_has;
                    api.get = transform_get;
                    api.create = transform_create;
                    api.link = transform_link;

                    return &api;
                }


        default:
            return NULL;
    }
}