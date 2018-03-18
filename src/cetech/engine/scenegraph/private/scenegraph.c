#include <cetech/core/cdb/cdb.h>
#include "cetech/engine/ecs/ecs.h"
#include <cetech/engine/scenegraph/scenegraph.h>
#include <cetech/core/containers/array.h>
#include <cetech/core/containers/hash.h>
#include <cetech/core/math/fmath.h>
#include <cetech/core/hashlib/hashlib.h>
#include <cetech/core/ebus/ebus.h>
#include "cetech/core/config/config.h"
#include "cetech/engine/resource/resource.h"
#include "cetech/core/memory/memory.h"
#include "cetech/core/api/api_system.h"
#include "cetech/core/module/module.h"


CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_ecs_a0);
CETECH_DECL_API(ct_cdb_a0);
CETECH_DECL_API(ct_hashlib_a0);
CETECH_DECL_API(ct_ebus_a0);

static uint64_t hash_combine(uint32_t a,
                             uint32_t b) {
    union {
        struct {
            uint32_t a;
            uint32_t b;
        };
        uint64_t ab;
    } c = {
            .a = a,
            .b = b,
    };

    return c.ab;
}

struct WorldInstance {
    struct ct_world world;
    uint32_t n;
    uint32_t allocated;
    void *buffer;

    struct ct_entity *entity;
    uint64_t *name;

    uint32_t *first_child;
    uint32_t *next_sibling;
    uint32_t *parent;

    float *position;
    float *rotation;
    float *scale;

    float *world_matrix;
};


#define _G ScenegraphGlobals
static struct _G {
    struct ct_hash_t world_map;
    struct WorldInstance *world_instances;
    struct ct_hash_t ent_map;

    uint64_t type;
    struct ct_alloc *allocator;
} _G;

static void allocate(struct WorldInstance *_data,
                     struct ct_alloc *_allocator,
                     uint32_t sz) {
    //assert(sz > _data->n);

    struct WorldInstance new_data = {};
    const unsigned bytes = sz * (
            sizeof(struct ct_entity)
            + sizeof(uint64_t)
            + (3 * sizeof(uint32_t))
            + (2 * sizeof(float) * 3)
            + sizeof(float) * 4
            + sizeof(float) * 16
    );

    new_data.buffer = CT_ALLOC(_allocator, char, bytes);
    new_data.n = _data->n;
    new_data.allocated = sz;

    new_data.entity = (struct ct_entity *) (new_data.buffer);
    new_data.name = (uint64_t *) (new_data.entity + sz);
    new_data.first_child = (uint32_t *) (new_data.name + sz);
    new_data.next_sibling = (uint32_t *) (new_data.first_child + sz);
    new_data.parent = (uint32_t *) (new_data.next_sibling + sz);
    new_data.position = (float *) (new_data.parent + sz);
    new_data.rotation = (float *) (new_data.position + (sz * 3));
    new_data.scale = (float *) (new_data.rotation + (sz * 4));
    new_data.world_matrix = (float *) (new_data.scale + (sz * 3));

    memcpy(new_data.entity, _data->entity, _data->n * sizeof(struct ct_entity));
    memcpy(new_data.name, _data->name, _data->n * sizeof(uint64_t));

    memcpy(new_data.first_child, _data->first_child,
           _data->n * sizeof(uint32_t));
    memcpy(new_data.next_sibling, _data->next_sibling,
           _data->n * sizeof(uint32_t));
    memcpy(new_data.parent, _data->parent, _data->n * sizeof(uint32_t));

    memcpy(new_data.position, _data->position, _data->n * sizeof(float) * 3);
    memcpy(new_data.rotation, _data->rotation, _data->n * sizeof(float) * 4);
    memcpy(new_data.scale, _data->scale, _data->n * sizeof(float) * 3);

    memcpy(new_data.world_matrix, _data->world_matrix,
           _data->n * sizeof(float) * 16);

    CT_FREE(_allocator, _data->buffer);

    *_data = new_data;
}

static void _new_world(uint32_t bus_name,
                       void *event) {
    struct ct_world world = ((struct ct_ecs_world_ev*)event)->world;

    uint32_t idx = ct_array_size(_G.world_instances);
    ct_array_push(_G.world_instances, (struct WorldInstance) {}, _G.allocator);
    _G.world_instances[idx].world = world;
    ct_hash_add(&_G.world_map, world.h, idx, _G.allocator);
}

static void _destroy_world(uint32_t bus_name,
                           void *event) {
    struct ct_world world = ((struct ct_ecs_world_ev*)event)->world;

    uint32_t idx = ct_hash_lookup(&_G.world_map, world.h, UINT32_MAX);
    uint32_t last_idx = ct_array_size(_G.world_instances) - 1;

    struct ct_world last_world = _G.world_instances[last_idx].world;

    CT_FREE(ct_memory_a0.main_allocator(),
            _G.world_instances[idx].buffer);

    _G.world_instances[idx] = _G.world_instances[last_idx];
    ct_hash_add(&_G.world_map, last_world.h, idx, _G.allocator);
    ct_array_pop_back(_G.world_instances);
}

static struct WorldInstance *_get_world_instance(struct ct_world world) {
    uint32_t idx = ct_hash_lookup(&_G.world_map, world.h, UINT32_MAX);

    if (idx != UINT32_MAX) {
        return &_G.world_instances[idx];
    }

    return NULL;
}


static int is_valid(struct ct_scene_node node) {
    return node.idx != UINT32_MAX;
}

static void transform(struct ct_scene_node node,
                      float *parent) {
    struct WorldInstance *world_inst = _get_world_instance(node.world);

    float *pos = &world_inst->position[3 * node.idx];
    float *rot = &world_inst->rotation[4 * node.idx];
    float *sca = &world_inst->scale[3 * node.idx];

    float rm[16];
    float sm[16];
    float m[16];

    ct_mat4_quat(rm, rot);
    ct_mat4_scale(sm, sca[0], sca[1], sca[2]);

    ct_mat4_mul(m, rm, sm);

    m[4 * 3 + 0] = pos[0];
    m[4 * 3 + 1] = pos[1];
    m[4 * 3 + 2] = pos[2];

    ct_mat4_mul(&world_inst->world_matrix[16 * node.idx], m, parent);

    uint32_t child = world_inst->first_child[node.idx];

    struct ct_scene_node child_transform = {.idx = child, .world = node.world};

    while (is_valid(child_transform)) {
        transform(child_transform,
                  &world_inst->world_matrix[16 * node.idx]);

        child_transform.idx = world_inst->next_sibling[child_transform.idx];
    }
}

static void get_position(struct ct_scene_node node,
                         float *value) {

    struct WorldInstance *world_inst = _get_world_instance(node.world);

    memcpy(value, &world_inst->position[3 * node.idx], sizeof(float) * 3);
}

static void get_rotation(struct ct_scene_node node,
                         float *value) {

    struct WorldInstance *world_inst = _get_world_instance(node.world);

    memcpy(value, &world_inst->rotation[4 * node.idx], sizeof(float) * 4);
}

static void get_scale(struct ct_scene_node node,
                      float *value) {

    struct WorldInstance *world_inst = _get_world_instance(node.world);
    memcpy(value, &world_inst->scale[3 * node.idx], sizeof(float) * 3);
}

static void get_world_matrix(struct ct_scene_node node,
                             float *value) {
    struct WorldInstance *world_inst = _get_world_instance(node.world);
    memcpy(value, &world_inst->world_matrix[16 * node.idx],
           sizeof(float) * 16);
}

static void set_position(struct ct_scene_node node,
                         float *pos) {

    struct WorldInstance *world_inst = _get_world_instance(node.world);

    uint32_t parent_idx = world_inst->parent[node.idx];

    struct ct_scene_node pt = {.idx = parent_idx, .world = node.world};

    float p[16];

    if (parent_idx != UINT32_MAX) {
        get_world_matrix(pt, p);
    } else {
        ct_mat4_identity(p);
    }

    ct_vec3_move(&world_inst->position[3 * node.idx], pos);

    transform(node, p);
}

static void set_rotation(struct ct_scene_node node,
                         float *rot) {
    struct WorldInstance *world_inst = _get_world_instance(node.world);

    uint32_t parent_idx = world_inst->parent[node.idx];

    struct ct_scene_node pt = {.idx = parent_idx, .world = node.world};

    float p[16];

    if (parent_idx != UINT32_MAX) {
        get_world_matrix(pt, p);
    } else {
        ct_mat4_identity(p);
    }

    float nq[4];
    ct_quat_norm(nq, rot);
    ct_quat_move(&world_inst->rotation[4 * node.idx], nq);

    transform(node, p);
}

static void set_scale(struct ct_scene_node node,
                      float *scale) {
    struct WorldInstance *world_inst = _get_world_instance(node.world);

    uint32_t parent_idx = world_inst->parent[node.idx];

    struct ct_scene_node pt = {.idx = parent_idx, .world = node.world};

    float p[16];

    if (parent_idx != UINT32_MAX) {
        get_world_matrix(pt, p);
    } else {
        ct_mat4_identity(p);
    }

    ct_vec3_move(&world_inst->scale[3 * node.idx], scale);

    transform(node, p);
}

static int has(struct ct_world world,
               struct ct_entity entity) {
    uint64_t idx = hash_combine(world.h, entity.h);

    return ct_hash_contain(&_G.ent_map, idx);
}

static struct ct_scene_node get_root(struct ct_world world,
                                     struct ct_entity entity) {
    struct ct_scenegraph_component *scene;
    scene = ct_ecs_a0.entity_data(world, SCENEGRAPH_COMPONENT,
                                              entity);

    return (struct ct_scene_node) {.idx = scene->idx, .world = world};
}

static struct ct_scene_node create(struct ct_world world,
                                   struct ct_entity entity,
                                   uint64_t *names,
                                   uint32_t *parent,
                                   float *pose,
                                   uint32_t count) {
    CT_UNUSED(pose);

    ct_ecs_a0.add_components(world, entity, &_G.type, 1);

    struct WorldInstance *data = _get_world_instance(world);

    uint32_t first_idx = data->n;
    allocate(data, ct_memory_a0.main_allocator(), data->n + count);
    data->n += count;

    struct ct_scene_node *nodes = CT_ALLOC(ct_memory_a0.main_allocator(),
                                           struct ct_scene_node,
                                           sizeof(struct ct_scene_node) *
                                           count);

    for (uint32_t i = 0; i < count; ++i) {
        uint32_t idx = first_idx + i;

        nodes[i] = (struct ct_scene_node) {.idx = idx, .world = world};

//            float* local_pose = &pose[i*16];

        float position[3] = {0.0f};
        float rotation[4];
        float scale[3] = {1.0f, 1.0f, 1.0f};

        ct_quat_identity(rotation);

        data->entity[idx] = entity;
        data->name[idx] = names[i];

        ct_vec3_move(&data->position[3 * idx], position);
        ct_quat_move(&data->rotation[4 * idx], rotation);
        ct_vec3_move(&data->scale[3 * idx], scale);

        data->parent[idx] = UINT32_MAX;
        data->first_child[idx] = UINT32_MAX;
        data->next_sibling[idx] = UINT32_MAX;

        float m[16];
        ct_mat4_identity(m);
        memcpy(&data->world_matrix[16 * idx], m, sizeof(float) * 16);

        struct ct_scene_node t = {.idx = idx, .world = world};

        float p[16];
        if (parent[i] != UINT32_MAX) {
            get_world_matrix(nodes[parent[i]], p);
        } else {
            ct_mat4_identity(p);
        }
        transform(t, p);

        if (parent[i] != UINT32_MAX) {
            uint32_t parent_idx = nodes[parent[i]].idx;

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
    }

    struct ct_scene_node root = nodes[0];

    uint64_t hash = hash_combine(world.h, entity.h);

    ct_hash_add(&_G.ent_map, hash, root.idx, _G.allocator);
    CT_FREE(ct_memory_a0.main_allocator(), nodes);

    struct ct_scenegraph_component *scene;
    scene = ct_ecs_a0.entity_data(world, SCENEGRAPH_COMPONENT,
                                              entity);

    scene->idx = root.idx;

    return root;
}

static void link(struct ct_scene_node parent,
                 struct ct_scene_node child) {
    struct WorldInstance *data = _get_world_instance(parent.world);

    data->parent[child.idx] = parent.idx;

    uint32_t tmp = data->first_child[parent.idx];

    data->first_child[parent.idx] = child.idx;
    data->next_sibling[child.idx] = tmp;

    float p[16];

    if (parent.idx != UINT32_MAX) {
        get_world_matrix(parent, p);
    } else {
        ct_mat4_identity(p);
    }
    transform(parent, p);

    get_world_matrix(parent, p);
    transform(child, p);
}

static struct ct_scene_node _node_by_name(struct WorldInstance *data,
                                          struct ct_scene_node root,
                                          uint64_t name) {
    if (data->name[root.idx] == name) {
        return root;
    }

    struct ct_scene_node node_it = {
            .idx = data->first_child[root.idx],
            .world = root.world
    };

    while (is_valid(node_it)) {
        struct ct_scene_node ret = _node_by_name(data, node_it, name);
        if (ret.idx != UINT32_MAX) {
            return ret;
        }

        node_it.idx = data->next_sibling[node_it.idx];
    }

    return (struct ct_scene_node) {.idx = UINT32_MAX, .world.h = UINT32_MAX};
}

static struct ct_scene_node node_by_name(struct ct_world world,
                                         struct ct_entity entity,
                                         uint64_t name) {
    struct WorldInstance *data = _get_world_instance(world);
    struct ct_scene_node root = get_root(world, entity);

    return _node_by_name(data, root, name);
}

static struct ct_scenegprah_a0 scenegraph_api = {
        .is_valid = is_valid,
        .get_position = get_position,
        .get_rotation = get_rotation,
        .get_scale = get_scale,
        .get_world_matrix = get_world_matrix,
        .set_position = set_position,
        .set_rotation = set_rotation,
        .set_scale = set_scale,
        .has = has,
        .get_root = get_root,
        .create = create,
        .link = link,
        .node_by_name = node_by_name
};


static void _init_api(struct ct_api_a0 *api) {
    api->register_api("ct_scenegprah_a0", &scenegraph_api);
}

static void _component_spawner(uint32_t ebus,
                               void *event) {
}

static void init(struct ct_api_a0 *api) {
    _init_api(api);


    _G = (struct _G){
            .allocator = ct_memory_a0.main_allocator(),
            .type = CT_ID64_0("scenegraph"),
    };

    ct_ecs_a0.register_component((struct ct_component_info) {
            .size = sizeof(struct ct_scenegraph_component),
            .component_name = "scenegraph",
    });

    ct_ebus_a0.connect_addr(ECS_EBUS, ECS_COMPONENT_SPAWN,
                            CT_ID64_0("scenegraph"), _component_spawner);

    ct_ebus_a0.connect(ECS_EBUS, ECS_WORLD_CREATE, _new_world);
    ct_ebus_a0.connect(ECS_EBUS, ECS_WORLD_DESTROY, _destroy_world);

}

static void shutdown() {
    ct_hash_free(&_G.ent_map, _G.allocator);
    ct_hash_free(&_G.world_map, _G.allocator);
    ct_array_free(_G.world_instances, _G.allocator);
}

CETECH_MODULE_DEF(
        scenegraph,
        {
            CETECH_GET_API(api, ct_memory_a0);
            CETECH_GET_API(api, ct_ecs_a0);
            CETECH_GET_API(api, ct_cdb_a0);
            CETECH_GET_API(api, ct_hashlib_a0);
            CETECH_GET_API(api, ct_ebus_a0);
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
