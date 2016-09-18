#include <celib/containers/array.h>
#include <celib/yaml/yaml.h>
#include <celib/stringid/types.h>
#include <engine/entcom/entcom.h>
#include <celib/containers/map.h>
#include <engine/memory_system/memory_system.h>
#include <celib/math/quatf.h>
#include <celib/math/mat44f.h>
#include <celib/stringid/stringid.h>
#include "../transform.h"

struct transform_data {
    vec3f_t position;
    vec3f_t scale;
    quatf_t rotation;
};

ARRAY_PROTOTYPE(vec3f_t)

ARRAY_PROTOTYPE(mat44f_t)

ARRAY_PROTOTYPE(quatf_t)

typedef struct {
    MAP_T(u32) ent_idx_map;

    ARRAY_T(u32) first_child;
    ARRAY_T(u32) next_sibling;
    ARRAY_T(u32) parent;

    ARRAY_T(vec3f_t) position;
    ARRAY_T(quatf_t) rotation;
    ARRAY_T(vec3f_t) scale;
    ARRAY_T(mat44f_t) world_matrix;
} world_data_t;

ARRAY_PROTOTYPE(world_data_t)

MAP_PROTOTYPE(world_data_t)


#define _G TransformGlobal
static struct G {
    stringid64_t type;

    MAP_T(world_data_t) world;
} _G = {0};


static void _new_world(world_t world) {
    world_data_t data = {0};

    MAP_INIT(u32, &data.ent_idx_map, memsys_main_allocator());

    ARRAY_INIT(u32, &data.first_child, memsys_main_allocator());
    ARRAY_INIT(u32, &data.next_sibling, memsys_main_allocator());
    ARRAY_INIT(u32, &data.parent, memsys_main_allocator());

    ARRAY_INIT(vec3f_t, &data.position, memsys_main_allocator());
    ARRAY_INIT(quatf_t, &data.rotation, memsys_main_allocator());
    ARRAY_INIT(vec3f_t, &data.scale, memsys_main_allocator());
    ARRAY_INIT(mat44f_t, &data.world_matrix, memsys_main_allocator());

    MAP_SET(world_data_t, &_G.world, world.h.h, data);
}

static world_data_t *_get_world_data(world_t world) {
    return MAP_GET_PTR(world_data_t, &_G.world, world.h.h);
}

static void _destroy_world(world_t world) {
    world_data_t *data = _get_world_data(world);

    MAP_DESTROY(u32, &data->ent_idx_map);

    ARRAY_DESTROY(u32, &data->first_child);
    ARRAY_DESTROY(u32, &data->next_sibling);
    ARRAY_DESTROY(u32, &data->parent);

    ARRAY_DESTROY(vec3f_t, &data->position);
    ARRAY_DESTROY(quatf_t, &data->rotation);
    ARRAY_DESTROY(vec3f_t, &data->scale);
    ARRAY_DESTROY(mat44f_t, &data->world_matrix);
}

int _transform_component_compiler(yaml_node_t body,
                                  ARRAY_T(u8) *data) {

    struct transform_data t_data;

    YAML_NODE_SCOPE(scale, body, "scale", t_data.scale = yaml_as_vec3f_t(scale););
    YAML_NODE_SCOPE(position, body, "position", t_data.position = yaml_as_vec3f_t(position););

    {
        yaml_node_t rotation = yaml_get_node(body, "rotation");
        vec3f_t v_rad = {0};

        vec3f_t v = yaml_as_vec3f_t(rotation);
        vec3f_mul(&v_rad, &v, f32_ToRad);

        quatf_from_euler(&t_data.rotation, v.x, v.y, v.z);

        yaml_node_free(rotation);
    };


    ARRAY_PUSH(u8, data, (u8 *) &t_data, sizeof(t_data));

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
        MAP_REMOVE(u32, &world_data->ent_idx_map, ents[i].idx);
    }
}

static void _spawner(world_t world,
                     entity_t *ents,
                     entity_t *ents_parent,
                     size_t ent_count,
                     void *data) {
    struct transform_data *tdata = data;

    for (int i = 0; i < ent_count; ++i) {
        transform_create(world,
                         ents[i],
                         ents_parent[i].idx != UINT32_MAX ? ents[ents_parent[i].idx] : (entity_t) {.idx = UINT32_MAX},
                         tdata->position,
                         tdata->rotation,
                         tdata->scale);
    }

    mat44f_t m = MAT44F_INIT_IDENTITY;
    for (int i = 0; i < ent_count; ++i) {
        transform_transform(world, transform_get(world, ents[i]), &m);
    }
}


int transform_init() {
    _G = (struct G) {0};

    MAP_INIT(world_data_t, &_G.world, memsys_main_allocator());

    _G.type = stringid64_from_string("transform");

    component_register_compiler(_G.type, _transform_component_compiler, 10);
    component_register_type(_G.type, _spawner, _destroyer, _on_world_create, _on_world_destroy);

    return 1;
}

void transform_shutdown() {

    MAP_DESTROY(world_data_t, &_G.world);

    _G = (struct G) {0};
}

int transform_is_valid(transform_t transform) {
    return transform.idx != UINT32_MAX;
}

void transform_transform(world_t world,
                         transform_t transform,
                         mat44f_t *parent) {
    world_data_t *world_data = _get_world_data(world);

    vec3f_t pos = ARRAY_AT(&world_data->position, transform.idx);
    quatf_t rot = ARRAY_AT(&world_data->rotation, transform.idx);
    vec3f_t sca = ARRAY_AT(&world_data->scale, transform.idx);

    mat44f_t rm = {0};
    quatf_to_mat44f(&rm, &rot);

    mat44f_t sm = {0};
    mat44f_scale(&sm, sca.x, sca.y, sca.z);

    mat44f_t m = {0};
    mat44f_mul(&m, &sm, &rm);

    m.w.x = pos.x;
    m.w.y = pos.y;
    m.w.z = pos.z;

    mat44f_mul(&ARRAY_AT(&world_data->world_matrix, transform.idx), &m, parent);

    u32 child = ARRAY_AT(&world_data->first_child, transform.idx);

    transform_t child_transform = {.idx = child};

    while (transform_is_valid(child_transform)) {
        child_transform.idx = child;

        transform_transform(world, child_transform, &ARRAY_AT(&world_data->world_matrix, transform.idx));

        child = ARRAY_AT(&world_data->next_sibling, transform.idx);
    }
}

vec3f_t transform_get_position(world_t world,
                               transform_t transform) {

    world_data_t *world_data = _get_world_data(world);
    return ARRAY_AT(&world_data->position, transform.idx);
}

quatf_t transform_get_rotation(world_t world,
                               transform_t transform) {

    world_data_t *world_data = _get_world_data(world);
    return ARRAY_AT(&world_data->rotation, transform.idx);
}

vec3f_t transform_get_scale(world_t world,
                            transform_t transform) {

    world_data_t *world_data = _get_world_data(world);
    return ARRAY_AT(&world_data->scale, transform.idx);
}

mat44f_t *transform_get_world_matrix(world_t world,
                                     transform_t transform) {

    world_data_t *world_data = _get_world_data(world);
    return &ARRAY_AT(&world_data->world_matrix, transform.idx);
}

void transform_set_position(world_t world,
                            transform_t transform,
                            vec3f_t pos) {
    world_data_t *world_data = _get_world_data(world);
    u32 parent_idx = ARRAY_AT(&world_data->parent, transform.idx);

    transform_t pt = {.idx = parent_idx};

    mat44f_t m = MAT44F_INIT_IDENTITY;
    mat44f_t *p = parent_idx != UINT32_MAX ? transform_get_world_matrix(world, pt) : &m;

    ARRAY_AT(&world_data->position, transform.idx) = pos;

    transform_transform(world, transform, p);
}

void transform_set_rotation(world_t world,
                            transform_t transform,
                            quatf_t rot) {
    world_data_t *world_data = _get_world_data(world);
    u32 parent_idx = ARRAY_AT(&world_data->parent, transform.idx);

    transform_t pt = {.idx = parent_idx};

    mat44f_t m = MAT44F_INIT_IDENTITY;
    mat44f_t *p = parent_idx != UINT32_MAX ? transform_get_world_matrix(world, pt) : &m;

    quatf_t nq = {0};
    quatf_normalized(&nq, &rot);

    ARRAY_AT(&world_data->rotation, transform.idx) = nq;

    transform_transform(world, transform, p);
}

void transform_set_scale(world_t world,
                         transform_t transform,
                         vec3f_t scale) {
    world_data_t *world_data = _get_world_data(world);
    u32 parent_idx = ARRAY_AT(&world_data->parent, transform.idx);

    transform_t pt = {.idx = parent_idx};

    mat44f_t m = MAT44F_INIT_IDENTITY;
    mat44f_t *p = parent_idx != UINT32_MAX ? transform_get_world_matrix(world, pt) : &m;

    ARRAY_AT(&world_data->scale, transform.idx) = scale;

    transform_transform(world, transform, p);
}

int transform_has(world_t world,
                  entity_t entity) {
    world_data_t *world_data = _get_world_data(world);
    return MAP_HAS(u32, &world_data->ent_idx_map, entity.h.h);
}

transform_t transform_get(world_t world,
                          entity_t entity) {

    world_data_t *world_data = _get_world_data(world);
    u32 idx = MAP_GET(u32, &world_data->ent_idx_map, entity.h.h, UINT32_MAX);
    return (transform_t) {.idx = idx};
}

transform_t transform_create(world_t world,
                             entity_t entity,
                             entity_t parent,
                             vec3f_t position,
                             quatf_t rotation,
                             vec3f_t scale) {

    world_data_t *data = _get_world_data(world);

    u32 idx = (u32) ARRAY_SIZE(&data->position);

    ARRAY_PUSH_BACK(vec3f_t, &data->position, position);
    ARRAY_PUSH_BACK(quatf_t, &data->rotation, rotation);
    ARRAY_PUSH_BACK(vec3f_t, &data->scale, scale);

    ARRAY_PUSH_BACK(u32, &data->parent, UINT32_MAX);
    ARRAY_PUSH_BACK(u32, &data->first_child, UINT32_MAX);
    ARRAY_PUSH_BACK(u32, &data->next_sibling, UINT32_MAX);

    mat44f_t m = MAT44F_INIT_IDENTITY;
    ARRAY_PUSH_BACK(mat44f_t, &data->world_matrix, m);

    transform_t t = {.idx = idx};
    transform_transform(world, t,
                        parent.h.h != UINT32_MAX ? transform_get_world_matrix(world, transform_get(world, parent))
                                                 : &m);

    MAP_SET(u32, &data->ent_idx_map, entity.h.h, idx);

    if (parent.h.h != UINT32_MAX) {
        u32 parent_idx = MAP_GET(u32, &data->ent_idx_map, entity.h.h, UINT32_MAX);

        ARRAY_AT(&data->parent, idx) = parent_idx;

        if (ARRAY_AT(&data->first_child, parent_idx) == UINT32_MAX) {
            ARRAY_AT(&data->first_child, parent_idx) = idx;
        } else {
            u32 first_child_idx = ARRAY_AT(&data->first_child, parent_idx);
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

    u32 tmp = ARRAY_AT(&data->first_child, parent_tr.idx);

    ARRAY_AT(&data->first_child, parent_tr.idx) = child_tr.idx;
    ARRAY_AT(&data->next_sibling, child_tr.idx) = tmp;

    mat44f_t m = MAT44F_INIT_IDENTITY;

    mat44f_t *p = parent_tr.idx != UINT32_MAX ? transform_get_world_matrix(world, parent_tr) : &m;

    transform_transform(world, parent_tr, p);
    transform_transform(world, child_tr, transform_get_world_matrix(world, transform_get(world, parent)));
}