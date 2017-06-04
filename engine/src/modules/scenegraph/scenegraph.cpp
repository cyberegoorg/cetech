#include <cetech/core/yaml.h>
#include <cetech/celib/quatf.inl>
#include <cetech/celib/mat44f.inl>
#include <cetech/core/config.h>
#include <cetech/modules/resource.h>
#include <cetech/core/memory.h>
#include <cetech/core/module.h>
#include <cetech/core/api.h>
#include <cetech/celib/array.inl>
#include <cetech/celib/array2.inl>
#include <cetech/celib/map2.inl>

#include <cetech/modules/world.h>
#include <cetech/modules/entity.h>
#include <cetech/modules/scenegraph.h>


IMPORT_API(memory_api_v0);
IMPORT_API(world_api_v0);

#define hash_combine(a, b) ((a)^(b))

using namespace cetech;

namespace {
    struct WorldInstance {
        uint32_t n;
        uint32_t allocated;
        void *buffer;

        entity_t *entity;
        uint64_t *name;

        uint32_t *first_child;
        uint32_t *next_sibling;
        uint32_t *parent;

        vec3f_t *position;
        quatf_t *rotation;
        vec3f_t *scale;

        mat44f_t *world_matrix;
    };


    static struct SceneGraphGlobal {
        Map<uint32_t> world_map;
        Array<WorldInstance> world_instances;
        Map<uint32_t> ent_map;
    } _G;

    void allocate(WorldInstance &_data,
                  allocator *_allocator,
                  uint32_t sz) {
        //assert(sz > _data.n);

        WorldInstance new_data;
        const unsigned bytes = sz * (
                sizeof(entity_t)
                + sizeof(uint64_t)
                + (3 * sizeof(uint32_t))
                + (2 * sizeof(vec3f_t))
                + sizeof(quatf_t)
                + sizeof(mat44f_s)
        );

        new_data.buffer = CETECH_ALLOCATE(_allocator, char, bytes);
        new_data.n = _data.n;
        new_data.allocated = sz;

        new_data.entity = (entity_t *) (new_data.buffer);
        new_data.name = (uint64_t *) (new_data.entity + sz);
        new_data.first_child = (uint32_t *) (new_data.name + sz);
        new_data.next_sibling = (uint32_t *) (new_data.first_child + sz);
        new_data.parent = (uint32_t *) (new_data.next_sibling + sz);
        new_data.position = (vec3f_t *) (new_data.parent + sz);
        new_data.rotation = (quatf_t *) (new_data.position + sz);
        new_data.scale = (vec3f_t *) (new_data.rotation + sz);
        new_data.world_matrix = (mat44f_t *) (new_data.scale + sz);

        memcpy(new_data.entity, _data.entity, _data.n * sizeof(entity_t));
        memcpy(new_data.name, _data.name, _data.n * sizeof(uint64_t));

        memcpy(new_data.first_child, _data.first_child,
               _data.n * sizeof(uint32_t));
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

    void _new_world(world_t world) {
        uint32_t idx = array::size(_G.world_instances);
        array::push_back(_G.world_instances, WorldInstance());
        map::set(_G.world_map, world.h, idx);
    }

    void _destroy_world(world_t world) {
        // TODO: impl
    }

    WorldInstance *_get_world_instance(world_t world) {
        uint32_t idx = map::get(_G.world_map, world.h, UINT32_MAX);

        if (idx != UINT32_MAX) {
            return &_G.world_instances[idx];
        }

        return nullptr;
    }


    void _on_world_create(world_t world) {
        _new_world(world);
    }

    void _on_world_destroy(world_t world) {
        _destroy_world(world);
    }

}

namespace scenegraph {

    int is_valid(scene_node_t node) {
        return node.idx != UINT32_MAX;
    }

    void transform(world_t world,
                   scene_node_t node,
                   mat44f_t *parent) {
        WorldInstance *world_inst = _get_world_instance(world);

        vec3f_t pos = world_inst->position[node.idx];
        quatf_t rot = world_inst->rotation[node.idx];
        vec3f_t sca = world_inst->scale[node.idx];

        mat44f_t rm = {0};
        mat44f_t sm = {0};
        mat44f_t m = {0};

        quatf_to_mat44f(&rm, &rot);
        mat44f_scale(&sm, sca.x, sca.y, sca.z);
        mat44f_mul(&m, &sm, &rm);

        m.w.x = pos.x;
        m.w.y = pos.y;
        m.w.z = pos.z;

        mat44f_mul(&world_inst->world_matrix[node.idx], &m, parent);

        uint32_t child = world_inst->first_child[node.idx];

        scene_node_t child_transform = {.idx = child};

        while (is_valid(child_transform)) {
            transform(world, child_transform,
                      &world_inst->world_matrix[node.idx]);

            child_transform.idx = world_inst->next_sibling[child_transform.idx];
        }
    }

    vec3f_t get_position(world_t world,
                         scene_node_t node) {

        WorldInstance *world_inst = _get_world_instance(world);
        return world_inst->position[node.idx];
    }

    quatf_t get_rotation(world_t world,
                         scene_node_t node) {

        WorldInstance *world_inst = _get_world_instance(world);
        return world_inst->rotation[node.idx];
    }

    vec3f_t get_scale(world_t world,
                      scene_node_t node) {

        WorldInstance *world_inst = _get_world_instance(world);
        return world_inst->scale[node.idx];
    }

    mat44f_t *get_world_matrix(world_t world,
                               scene_node_t node) {
        WorldInstance *world_inst = _get_world_instance(world);
        return &world_inst->world_matrix[node.idx];
    }

    void set_position(world_t world,
                      scene_node_t node,
                      vec3f_t pos) {
        WorldInstance *world_inst = _get_world_instance(world);

        uint32_t parent_idx = world_inst->parent[node.idx];

        scene_node_t pt = {.idx = parent_idx};

        mat44f_t m = MAT44F_INIT_IDENTITY;
        mat44f_t *p =
                parent_idx != UINT32_MAX ? get_world_matrix(world, pt)
                                         : &m;

        world_inst->position[node.idx] = pos;

        transform(world, node, p);
    }

    void set_rotation(world_t world,
                      scene_node_t node,
                      quatf_t rot) {
        WorldInstance *world_inst = _get_world_instance(world);

        uint32_t parent_idx = world_inst->parent[node.idx];

        scene_node_t pt = {.idx = parent_idx};

        mat44f_t m = MAT44F_INIT_IDENTITY;
        mat44f_t *p =
                parent_idx != UINT32_MAX ? get_world_matrix(world, pt)
                                         : &m;

        quatf_t nq = {0};
        quatf_normalized(&nq, &rot);

        world_inst->rotation[node.idx] = nq;

        transform(world, node, p);
    }

    void set_scale(world_t world,
                   scene_node_t node,
                   vec3f_t scale) {
        WorldInstance *world_inst = _get_world_instance(world);

        uint32_t parent_idx = world_inst->parent[node.idx];

        scene_node_t pt = {.idx = parent_idx};

        mat44f_t m = MAT44F_INIT_IDENTITY;
        mat44f_t *p =
                parent_idx != UINT32_MAX ? get_world_matrix(world, pt)
                                         : &m;

        world_inst->scale[node.idx] = scale;

        transform(world, node, p);
    }

    int has(world_t world,
            entity_t entity) {
        uint32_t idx = hash_combine(world.h, entity.h);

        return map::has(_G.ent_map, idx);
    }

    scene_node_t get_root(world_t world,
                          entity_t entity) {

        uint32_t idx = hash_combine(world.h, entity.h);

        uint32_t component_idx = map::get(_G.ent_map, idx, UINT32_MAX);

        return (scene_node_t) {.idx = component_idx};
    }

    scene_node_t create(world_t world,
                        entity_t entity,
                        uint64_t *names,
                        uint32_t *parent,
                        mat44f_t *pose,
                        uint32_t count) {
        WorldInstance *data = _get_world_instance(world);

        uint32_t first_idx = data->n;
        allocate(*data, memory_api_v0.main_allocator(), data->n + count);
        data->n += count;

        scene_node_t *nodes = CETECH_ALLOCATE(memory_api_v0.main_allocator(),
                                              scene_node_t, count);

        for (int i = 0; i < count; ++i) {
            uint32_t idx = first_idx + i;

            nodes[i] = (scene_node_t) {.idx = idx};

            mat44f_t local_pose = pose[i];

            vec3f_t position = {0};
            quatf_t rotation = QUATF_IDENTITY;
            vec3f_t scale = {.x = 1.0f, .y = 1.0f, .z = 1.0f};

            data->entity[idx] = entity;
            data->name[idx] = names[i];

            data->position[idx] = position;
            data->rotation[idx] = rotation;
            data->scale[idx] = scale;

            data->parent[idx] = UINT32_MAX;
            data->first_child[idx] = UINT32_MAX;
            data->next_sibling[idx] = UINT32_MAX;

            mat44f_t m = MAT44F_INIT_IDENTITY;
            memcpy(data->world_matrix[idx].f, m.f, sizeof(m));

            scene_node_t t = {.idx = idx};
            transform(world, t,
                      parent[i] != UINT32_MAX
                      ? get_world_matrix(world,
                                         nodes[parent[i]])
                      : &m);

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

        scene_node_t root = nodes[0];

        uint64_t hash = hash_combine(world.h, entity.h);

        map::set(_G.ent_map, hash, root.idx);
        CETECH_DEALLOCATE(memory_api_v0.main_allocator(), nodes);

        return root;
    }

    void link(world_t world,
              scene_node_t parent,
              scene_node_t child) {
        WorldInstance *data = _get_world_instance(world);

        data->parent[child.idx] = parent.idx;

        uint32_t tmp = data->first_child[parent.idx];

        data->first_child[parent.idx] = child.idx;
        data->next_sibling[child.idx] = tmp;

        mat44f_t m = MAT44F_INIT_IDENTITY;

        mat44f_t *p =
                parent.idx != UINT32_MAX ? get_world_matrix(world,
                                                            parent)
                                         : &m;

        transform(world, parent, p);
        transform(world, child, get_world_matrix(world,
                                                 parent));
    }

    scene_node_t _node_by_name(WorldInstance *data,
                               scene_node_t root,
                               uint64_t name) {
        if (data->name[root.idx] == name) {
            return root;
        }

        scene_node_t node_it = {.idx = data->first_child[root.idx]};
        while (is_valid(node_it)) {
            scene_node_t ret = _node_by_name(data, node_it, name);
            if (ret.idx != UINT32_MAX) {
                return ret;
            }

            node_it.idx = data->next_sibling[node_it.idx];
        }

        return (scene_node_t) {.idx=UINT32_MAX};
    }

    scene_node_t node_by_name(world_t world,
                              entity_t entity,
                              uint64_t name) {
        WorldInstance *data = _get_world_instance(world);
        scene_node_t root = get_root(world, entity);

        return _node_by_name(data, root, name);
    }

}

namespace scenegraph_module {
    static struct scenegprah_api_v0 scenegraph_api = {
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


    static world_callbacks_t world_callbacks = {
            .on_created=_on_world_create,
            .on_destroy=_on_world_destroy
    };

    void _init_api(struct api_v0 *api) {
        api->register_api("scenegprah_api_v0", &scenegraph_api);
    }


    void init(struct api_v0 *api) {
        GET_API(api, world_api_v0);
        GET_API(api, memory_api_v0);

        _G = {0};

        _G.world_map.init(memory_api_v0.main_allocator());
        _G.world_instances.init(memory_api_v0.main_allocator());
        _G.ent_map.init(memory_api_v0.main_allocator());

        world_api_v0.register_callback(world_callbacks);
    }

    void shutdown() {
        _G = {0};
    }

    extern "C" void *scenegraph_get_module_api(int api) {

        switch (api) {
            case PLUGIN_EXPORT_API_ID: {
                static struct module_api_v0 module = {0};

                module.init = scenegraph_module::init;
                module.init_api = scenegraph_module::_init_api;
                module.shutdown = scenegraph_module::shutdown;

                return &module;
            }

            default:
                return NULL;
        }
    }
}

