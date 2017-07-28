#include <cetech/core/yaml.h>
#include <cetech/core/config.h>
#include <cetech/engine/resource.h>
#include <cetech/core/memory.h>
#include <cetech/core/api_system.h>
#include <celib/array.inl>
#include <celib/map.inl>


#include <cetech/engine/entity.h>
#include <cetech/engine/scenegraph.h>
#include <celib/fpumath.h>


CETECH_DECL_API(ct_memory_a0);
CETECH_DECL_API(ct_world_a0);

#define hash_combine(a, b) ((a)^(b))

using namespace celib;

namespace {
    struct WorldInstance {
        ct_world world;
        uint32_t n;
        uint32_t allocated;
        void *buffer;

        ct_entity *entity;
        uint64_t *name;

        uint32_t *first_child;
        uint32_t *next_sibling;
        uint32_t *parent;

        float *position;
        float *rotation;
        float *scale;

        float *world_matrix;
    };


    static struct SceneGraphGlobal {
        Map<uint32_t> world_map;
        Array<WorldInstance> world_instances;
        Map<uint32_t> ent_map;
    } _G;

    void allocate(WorldInstance &_data,
                  cel_alloc *_allocator,
                  uint32_t sz) {
        //assert(sz > _data.n);

        WorldInstance new_data;
        const unsigned bytes = sz * (
                sizeof(ct_entity)
                + sizeof(uint64_t)
                + (3 * sizeof(uint32_t))
                + (2 * sizeof(float) * 3)
                + sizeof(float) * 4
                + sizeof(float) * 16
        );

        new_data.buffer = CEL_ALLOCATE(_allocator, char, bytes);
        new_data.n = _data.n;
        new_data.allocated = sz;

        new_data.entity = (ct_entity *) (new_data.buffer);
        new_data.name = (uint64_t *) (new_data.entity + sz);
        new_data.first_child = (uint32_t *) (new_data.name + sz);
        new_data.next_sibling = (uint32_t *) (new_data.first_child + sz);
        new_data.parent = (uint32_t *) (new_data.next_sibling + sz);
        new_data.position = (float *) (new_data.parent + sz);
        new_data.rotation = (float *) (new_data.position + (sz*3));
        new_data.scale = (float *) (new_data.rotation + (sz*4));
        new_data.world_matrix = (float *) (new_data.scale + (sz*3));

        memcpy(new_data.entity, _data.entity, _data.n * sizeof(ct_entity));
        memcpy(new_data.name, _data.name, _data.n * sizeof(uint64_t));

        memcpy(new_data.first_child, _data.first_child,
               _data.n * sizeof(uint32_t));
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

    void _new_world(ct_world world) {
        uint32_t idx = array::size(_G.world_instances);
        array::push_back(_G.world_instances, WorldInstance());
        _G.world_instances[idx].world = world;
        map::set(_G.world_map, world.h, idx);
    }

    void _destroy_world(ct_world world) {
        uint32_t idx = map::get(_G.world_map, world.h, UINT32_MAX);
        uint32_t last_idx = array::size(_G.world_instances) - 1;

        ct_world last_world = _G.world_instances[last_idx].world;

        CEL_FREE(ct_memory_a0.main_allocator(),
                    _G.world_instances[idx].buffer);

        _G.world_instances[idx] = _G.world_instances[last_idx];
        map::set(_G.world_map, last_world.h, idx);
        array::pop_back(_G.world_instances);
    }

    WorldInstance *_get_world_instance(ct_world world) {
        uint32_t idx = map::get(_G.world_map, world.h, UINT32_MAX);

        if (idx != UINT32_MAX) {
            return &_G.world_instances[idx];
        }

        return nullptr;
    }


    void _on_world_create(ct_world world) {
        _new_world(world);
    }

    void _on_world_destroy(ct_world world) {
        _destroy_world(world);
    }

}

namespace scenegraph {

    int is_valid(ct_scene_node node) {
        return node.idx != UINT32_MAX;
    }

    void transform(ct_scene_node node,
                   float *parent) {
        WorldInstance *world_inst = _get_world_instance(node.world);

        float *pos = &world_inst->position[3 * node.idx];
        float *rot = &world_inst->rotation[4 * node.idx];
        float *sca = &world_inst->scale[3 * node.idx];

        float rm[16];
        float sm[16];
        float m[16];

        celib::mat4_quat(rm, rot);
        celib::mat4_scale(sm, sca[0], sca[1], sca[2]);

        mat4_mul(m, rm, sm);

        m[4 * 3 + 0] = pos[0];
        m[4 * 3 + 1] = pos[1];
        m[4 * 3 + 2] = pos[2];

        celib::mat4_mul(&world_inst->world_matrix[16 * node.idx], m, parent);

        uint32_t child = world_inst->first_child[node.idx];

        ct_scene_node child_transform = {.idx = child, .world = node.world};

        while (is_valid(child_transform)) {
            transform(child_transform, &world_inst->world_matrix[16*node.idx]);

            child_transform.idx = world_inst->next_sibling[child_transform.idx];
        }
    }

    void get_position(ct_scene_node node, float* value) {

        WorldInstance *world_inst = _get_world_instance(node.world);

        memcpy(value, &world_inst->position[3*node.idx], sizeof(float)*3);
    }

    void get_rotation(ct_scene_node node, float* value) {

        WorldInstance *world_inst = _get_world_instance(node.world);

        memcpy(value, &world_inst->rotation[4*node.idx], sizeof(float)*4);
}

    void get_scale(ct_scene_node node, float* value) {

        WorldInstance *world_inst = _get_world_instance(node.world);
        memcpy(value, &world_inst->scale[3*node.idx], sizeof(float)*3);
    }

    void get_world_matrix(ct_scene_node node, float* value) {
        WorldInstance *world_inst = _get_world_instance(node.world);
        memcpy(value, &world_inst->world_matrix[16*node.idx], sizeof(float)*16);
    }

    void set_position(ct_scene_node node,
                      float* pos) {

        WorldInstance *world_inst = _get_world_instance(node.world);

        uint32_t parent_idx = world_inst->parent[node.idx];

        ct_scene_node pt = {.idx = parent_idx, .world = node.world};

        float p[16];

        if(parent_idx != UINT32_MAX) {
            get_world_matrix(pt, p);
        } else {
            celib::mat4_identity(p);
        }

        vec3_move(&world_inst->position[3 * node.idx], pos);

        transform(node, p);
    }

    void set_rotation(ct_scene_node node,
                      float* rot) {
        WorldInstance *world_inst = _get_world_instance(node.world);

        uint32_t parent_idx = world_inst->parent[node.idx];

        ct_scene_node pt = {.idx = parent_idx, .world = node.world};

        float p[16];

        if(parent_idx != UINT32_MAX) {
            get_world_matrix(pt, p);
        } else {
            celib::mat4_identity(p);
        }

        float nq[4];
        celib::quat_norm(nq, rot);
        celib::quat_move(&world_inst->rotation[4 * node.idx], nq);

        transform(node, p);
    }

    void set_scale(ct_scene_node node,
                   float* scale) {
        WorldInstance *world_inst = _get_world_instance(node.world);

        uint32_t parent_idx = world_inst->parent[node.idx];

        ct_scene_node pt = {.idx = parent_idx, .world = node.world};

        float p[16];

        if(parent_idx != UINT32_MAX) {
            get_world_matrix(pt, p);
        } else {
            celib::mat4_identity(p);
        }

        vec3_move(&world_inst->scale[3 * node.idx], scale);

        transform(node, p);
    }

    int has(ct_world world,
            ct_entity entity) {
        uint32_t idx = hash_combine(world.h, entity.h);

        return map::has(_G.ent_map, idx);
    }

    ct_scene_node get_root(ct_world world,
                           ct_entity entity) {

        uint32_t idx = hash_combine(world.h, entity.h);

        uint32_t component_idx = map::get(_G.ent_map, idx, UINT32_MAX);

        return (ct_scene_node) {.idx = component_idx, .world = world};
    }

    ct_scene_node create(ct_world world,
                         ct_entity entity,
                         uint64_t *names,
                         uint32_t *parent,
                         float *pose,
                         uint32_t count) {
        CEL_UNUSED(pose);

        WorldInstance *data = _get_world_instance(world);

        uint32_t first_idx = data->n;
        allocate(*data, ct_memory_a0.main_allocator(), data->n + count);
        data->n += count;

        ct_scene_node *nodes = CEL_ALLOCATE(ct_memory_a0.main_allocator(),
                                               ct_scene_node,
                                               sizeof(ct_scene_node) * count);

        for (uint32_t i = 0; i < count; ++i) {
            uint32_t idx = first_idx + i;

            nodes[i] = (ct_scene_node) {.idx = idx, .world = world};

//            float* local_pose = &pose[i*16];

            float position[3] = {0.0f};
            float rotation[4];
            float scale[3] = {1.0f,1.0f, 1.0f};

            celib::quat_identity(rotation);

            data->entity[idx] = entity;
            data->name[idx] = names[i];

            celib::vec3_move(&data->position[3 * idx], position);
            celib::quat_move(&data->rotation[4 * idx], rotation);
            celib::vec3_move(&data->scale[3 * idx], scale);

            data->parent[idx] = UINT32_MAX;
            data->first_child[idx] = UINT32_MAX;
            data->next_sibling[idx] = UINT32_MAX;

            float m[16];
            celib::mat4_identity(m);
            memcpy(&data->world_matrix[16*idx], m, sizeof(float)*16);

            ct_scene_node t = {.idx = idx, .world = world};

            float p[16];
            if(parent[i] != UINT32_MAX) {
                get_world_matrix(nodes[parent[i]], p);
            } else {
                celib::mat4_identity(p);
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

        ct_scene_node root = nodes[0];

        uint64_t hash = hash_combine(world.h, entity.h);

        map::set(_G.ent_map, hash, root.idx);
        CEL_FREE(ct_memory_a0.main_allocator(), nodes);

        return root;
    }

    void link(ct_scene_node parent,
              ct_scene_node child) {
        WorldInstance *data = _get_world_instance(parent.world);

        data->parent[child.idx] = parent.idx;

        uint32_t tmp = data->first_child[parent.idx];

        data->first_child[parent.idx] = child.idx;
        data->next_sibling[child.idx] = tmp;

        float p[16];

        if(parent.idx != UINT32_MAX) {
            get_world_matrix(parent, p);
        } else {
            celib::mat4_identity(p);
        }
        transform(parent, p);

        get_world_matrix(parent, p);
        transform(child, p);
    }

    ct_scene_node _node_by_name(WorldInstance *data,
                                ct_scene_node root,
                                uint64_t name) {
        if (data->name[root.idx] == name) {
            return root;
        }

        ct_scene_node node_it = {.idx = data->first_child[root.idx], .world = root.world};
        while (is_valid(node_it)) {
            ct_scene_node ret = _node_by_name(data, node_it, name);
            if (ret.idx != UINT32_MAX) {
                return ret;
            }

            node_it.idx = data->next_sibling[node_it.idx];
        }

        return (ct_scene_node) {.idx = UINT32_MAX, .world.h = UINT32_MAX};
    }

    ct_scene_node node_by_name(ct_world world,
                               ct_entity entity,
                               uint64_t name) {
        WorldInstance *data = _get_world_instance(world);
        ct_scene_node root = get_root(world, entity);

        return _node_by_name(data, root, name);
    }

}

namespace scenegraph_module {
    static ct_scenegprah_a0 scenegraph_api = {
            .is_valid = scenegraph::is_valid,
            .get_position = scenegraph::get_position,
            .get_rotation = scenegraph::get_rotation,
            .get_scale = scenegraph::get_scale,
            .get_world_matrix = scenegraph::get_world_matrix,
            .set_position = scenegraph::set_position,
            .set_rotation = scenegraph::set_rotation,
            .set_scale = scenegraph::set_scale,
            .has = scenegraph::has,
            .get_root = scenegraph::get_root,
            .create = scenegraph::create,
            .link = scenegraph::link,
            .node_by_name = scenegraph::node_by_name
    };


    static ct_world_callbacks_t world_callbacks = {
            .on_created=_on_world_create,
            .on_destroy=_on_world_destroy
    };

    void _init_api(ct_api_a0 *api) {
        api->register_api("ct_scenegprah_a0", &scenegraph_api);
    }


    void init(ct_api_a0 *api) {
        _init_api(api);

        CETECH_GET_API(api, ct_world_a0);
        CETECH_GET_API(api, ct_memory_a0);

        _G = {};

        _G.world_map.init(ct_memory_a0.main_allocator());
        _G.world_instances.init(ct_memory_a0.main_allocator());
        _G.ent_map.init(ct_memory_a0.main_allocator());

        ct_world_a0.register_callback(world_callbacks);
    }

    void shutdown() {
        _G.world_map.destroy();
        _G.world_instances.destroy();
        _G.ent_map.destroy();
    }

    extern "C" void scenegraph_load_module(ct_api_a0 *api) {
        scenegraph_module::init(api);

    }

    extern "C" void scenegraph_unload_module(ct_api_a0 *api) {
        CEL_UNUSED(api);
        scenegraph_module::shutdown();
    }
}

