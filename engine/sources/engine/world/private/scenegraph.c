#include <celib/containers/array.h>
#include <celib/yaml/yaml.h>
#include <celib/math/quatf.h>
#include <celib/math/mat44f.h>

#include <engine/entcom/entcom.h>
#include <engine/world/scenegraph.h>
#include <engine/memory/memsys.h>
#include <engine/plugin/plugin_api.h>

ARRAY_PROTOTYPE(cel_vec3f_t)

ARRAY_PROTOTYPE(stringid64_t)

ARRAY_PROTOTYPE(cel_mat44f_t)

ARRAY_PROTOTYPE(cel_quatf_t)

typedef struct {
    MAP_T(u32) ent_idx_map;

    ARRAY_T(u32) first_child;
    ARRAY_T(u32) next_sibling;
    ARRAY_T(u32) parent;

    ARRAY_T(stringid64_t) name;
    ARRAY_T(cel_vec3f_t) position;
    ARRAY_T(cel_quatf_t) rotation;
    ARRAY_T(cel_vec3f_t) scale;
    ARRAY_T(cel_mat44f_t) world_matrix;
} world_data_t;

ARRAY_PROTOTYPE(world_data_t)

MAP_PROTOTYPE(world_data_t)


#define _G SceneGraphGlobal
static struct G {
    MAP_T(world_data_t) world;
} _G = {0};


static void _new_world(world_t world) {
    world_data_t data = {0};

    MAP_INIT(u32, &data.ent_idx_map, memsys_main_allocator());

    ARRAY_INIT(u32, &data.first_child, memsys_main_allocator());
    ARRAY_INIT(u32, &data.next_sibling, memsys_main_allocator());
    ARRAY_INIT(u32, &data.parent, memsys_main_allocator());

    ARRAY_INIT(stringid64_t, &data.name, memsys_main_allocator());
    ARRAY_INIT(cel_vec3f_t, &data.position, memsys_main_allocator());
    ARRAY_INIT(cel_quatf_t, &data.rotation, memsys_main_allocator());
    ARRAY_INIT(cel_vec3f_t, &data.scale, memsys_main_allocator());
    ARRAY_INIT(cel_mat44f_t, &data.world_matrix, memsys_main_allocator());

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

    ARRAY_DESTROY(stringid64_t, &data->name);
    ARRAY_DESTROY(cel_vec3f_t, &data->position);
    ARRAY_DESTROY(cel_quatf_t, &data->rotation);
    ARRAY_DESTROY(cel_vec3f_t, &data->scale);
    ARRAY_DESTROY(cel_mat44f_t, &data->world_matrix);
}

static void _on_world_create(world_t world) {
    _new_world(world);
}

static void _on_world_destroy(world_t world) {
    _destroy_world(world);
}


static void _init(get_api_fce_t get_engine_api) {
    _G = (struct G) {0};

    MAP_INIT(world_data_t, &_G.world, memsys_main_allocator());

    world_register_callback((world_callbacks_t) {.on_created=_on_world_create, .on_destroy=_on_world_destroy});

}

static void _shutdown() {

    MAP_DESTROY(world_data_t, &_G.world);

    _G = (struct G) {0};
}

void *scenegraph_get_plugin_api(int api,
                           int version) {

    if (api == PLUGIN_API_ID && version == 0) {
        static struct plugin_api_v0 plugin = {0};

        plugin.init = _init;
        plugin.shutdown = _shutdown;

        return &plugin;
    }

    return 0;
}


int scenegraph_is_valid(scene_node_t transform) {
    return transform.idx != UINT32_MAX;
}

void scene_node_transform(world_t world,
                          scene_node_t transform,
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

    cel_mat44f_mul(&ARRAY_AT(&world_data->world_matrix, transform.idx), &m, parent);

    u32 child = ARRAY_AT(&world_data->first_child, transform.idx);

    scene_node_t child_transform = {.idx = child};

    while (scenegraph_is_valid(child_transform)) {
        scene_node_transform(world, child_transform, &ARRAY_AT(&world_data->world_matrix, transform.idx));
        child_transform.idx = ARRAY_AT(&world_data->next_sibling, child_transform.idx);
    }
}

cel_vec3f_t scenegraph_get_position(world_t world,
                                    scene_node_t transform) {

    world_data_t *world_data = _get_world_data(world);
    return ARRAY_AT(&world_data->position, transform.idx);
}

cel_quatf_t scenegraph_get_rotation(world_t world,
                                    scene_node_t transform) {

    world_data_t *world_data = _get_world_data(world);
    return ARRAY_AT(&world_data->rotation, transform.idx);
}

cel_vec3f_t scenegraph_get_scale(world_t world,
                                 scene_node_t transform) {

    world_data_t *world_data = _get_world_data(world);
    return ARRAY_AT(&world_data->scale, transform.idx);
}

cel_mat44f_t *scenegraph_get_world_matrix(world_t world,
                                          scene_node_t transform) {
    static cel_mat44f_t _n = MAT44F_INIT_IDENTITY;
    world_data_t *world_data = _get_world_data(world);
    return &ARRAY_AT(&world_data->world_matrix, transform.idx);
}

void scenegraph_set_position(world_t world,
                             scene_node_t transform,
                             cel_vec3f_t pos) {
    world_data_t *world_data = _get_world_data(world);
    u32 parent_idx = ARRAY_AT(&world_data->parent, transform.idx);

    scene_node_t pt = {.idx = parent_idx};

    cel_mat44f_t m = MAT44F_INIT_IDENTITY;
    cel_mat44f_t *p = parent_idx != UINT32_MAX ? scenegraph_get_world_matrix(world, pt) : &m;

    ARRAY_AT(&world_data->position, transform.idx) = pos;

    scene_node_transform(world, transform, p);
}

void scenegraph_set_rotation(world_t world,
                             scene_node_t transform,
                             cel_quatf_t rot) {
    world_data_t *world_data = _get_world_data(world);
    u32 parent_idx = ARRAY_AT(&world_data->parent, transform.idx);

    scene_node_t pt = {.idx = parent_idx};

    cel_mat44f_t m = MAT44F_INIT_IDENTITY;
    cel_mat44f_t *p = parent_idx != UINT32_MAX ? scenegraph_get_world_matrix(world, pt) : &m;

    cel_quatf_t nq = {0};
    cel_quatf_normalized(&nq, &rot);

    ARRAY_AT(&world_data->rotation, transform.idx) = nq;

    scene_node_transform(world, transform, p);
}

void scenegraph_set_scale(world_t world,
                          scene_node_t transform,
                          cel_vec3f_t scale) {
    world_data_t *world_data = _get_world_data(world);
    u32 parent_idx = ARRAY_AT(&world_data->parent, transform.idx);

    scene_node_t pt = {.idx = parent_idx};

    cel_mat44f_t m = MAT44F_INIT_IDENTITY;
    cel_mat44f_t *p = parent_idx != UINT32_MAX ? scenegraph_get_world_matrix(world, pt) : &m;

    ARRAY_AT(&world_data->scale, transform.idx) = scale;

    scene_node_transform(world, transform, p);
}

int scenegraph_has(world_t world,
                   entity_t entity) {
    world_data_t *world_data = _get_world_data(world);
    return MAP_HAS(u32, &world_data->ent_idx_map, entity.h.h);
}

scene_node_t scenegraph_get_root(world_t world,
                                 entity_t entity) {

    world_data_t *world_data = _get_world_data(world);
    u32 idx = MAP_GET(u32, &world_data->ent_idx_map, entity.h.h, UINT32_MAX);
    return (scene_node_t) {.idx = idx};
}

scene_node_t scenegraph_create(world_t world,
                               entity_t entity,
                               stringid64_t *names,
                               u32 *parent,
                               cel_mat44f_t *pose,
                               u32 count) {
    world_data_t *data = _get_world_data(world);

    scene_node_t *nodes = CEL_ALLOCATE(memsys_main_allocator(), scene_node_t, count);

    for (int i = 0; i < count; ++i) {
        u32 idx = (u32) ARRAY_SIZE(&data->position);
        nodes[i] = (scene_node_t) {.idx = idx};

        cel_mat44f_t local_pose = pose[i];

        cel_vec3f_t position = {0};
        cel_quatf_t rotation = QUATF_IDENTITY;
        cel_vec3f_t scale = {.x = 1.0f, .y = 1.0f, .z = 1.0f};

        ARRAY_PUSH_BACK(cel_vec3f_t, &data->position, position);
        ARRAY_PUSH_BACK(cel_quatf_t, &data->rotation, rotation);
        ARRAY_PUSH_BACK(cel_vec3f_t, &data->scale, scale);

        ARRAY_PUSH_BACK(stringid64_t, &data->name, names[i]);
        ARRAY_PUSH_BACK(u32, &data->parent, UINT32_MAX);
        ARRAY_PUSH_BACK(u32, &data->first_child, UINT32_MAX);
        ARRAY_PUSH_BACK(u32, &data->next_sibling, UINT32_MAX);

        cel_mat44f_t m = MAT44F_INIT_IDENTITY;
        ARRAY_PUSH_BACK(cel_mat44f_t, &data->world_matrix, m);

        scene_node_t t = {.idx = idx};
        scene_node_transform(world, t,
                             parent[i] != UINT32_MAX ? scenegraph_get_world_matrix(world, nodes[parent[i]]) : &m);

        if (parent[i] != UINT32_MAX) {
            u32 parent_idx = nodes[parent[i]].idx;

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
    }

    scene_node_t root = nodes[0];
    MAP_SET(u32, &data->ent_idx_map, entity.h.h, root.idx);
    CEL_DEALLOCATE(memsys_main_allocator(), nodes);
    return root;
}

void scenegraph_link(world_t world,
                     scene_node_t parent,
                     scene_node_t child) {
    world_data_t *data = _get_world_data(world);

    ARRAY_AT(&data->parent, child.idx) = parent.idx;

    u32 tmp = ARRAY_AT(&data->first_child, parent.idx);

    ARRAY_AT(&data->first_child, parent.idx) = child.idx;
    ARRAY_AT(&data->next_sibling, child.idx) = tmp;

    cel_mat44f_t m = MAT44F_INIT_IDENTITY;

    cel_mat44f_t *p = parent.idx != UINT32_MAX ? scenegraph_get_world_matrix(world, parent) : &m;

    scene_node_transform(world, parent, p);
    scene_node_transform(world, child, scenegraph_get_world_matrix(world, parent));
}

scene_node_t _scenegraph_node_by_name(world_data_t *data,
                                      scene_node_t root,
                                      stringid64_t name) {
    if (ARRAY_AT(&data->name, root.idx).id == name.id) {
        return root;
    }

    scene_node_t node_it = {.idx = ARRAY_AT(&data->first_child, root.idx)};
    while (scenegraph_is_valid(node_it)) {
        scene_node_t ret = _scenegraph_node_by_name(data, node_it, name);
        if (ret.idx != UINT32_MAX) {
            return ret;
        }

        node_it.idx = ARRAY_AT(&data->next_sibling, node_it.idx);
    }

    return (scene_node_t) {.idx=UINT32_MAX};
}

scene_node_t scenegraph_node_by_name(world_t world,
                                     entity_t entity,
                                     stringid64_t name) {
    world_data_t *data = _get_world_data(world);
    scene_node_t root = scenegraph_get_root(world, entity);

    return _scenegraph_node_by_name(data, root, name);
}