#include <cetech/celib/map.inl>

#include <cetech/celib/mat44f.inl>
#include <cetech/kernel/yaml.h>
#include <cetech/kernel/hash.h>
#include <cetech/kernel/config.h>
#include <cetech/modules/resource.h>
#include <cetech/kernel/memory.h>
#include <cetech/kernel/module.h>
#include <cetech/kernel/api_system.h>

#include <cetech/modules/entity.h>


#include <cetech/modules/renderer.h>
#include <cetech/modules/transform.h>

#include "cetech/modules/camera.h"


CETECH_DECL_API(memory_api_v0);
CETECH_DECL_API(component_api_v0);
CETECH_DECL_API(renderer_api_v0);
CETECH_DECL_API(transform_api_v0);
CETECH_DECL_API(hash_api_v0);

using namespace cetech;

namespace {

    struct camera_data {
        float near;
        float far;
        float fov;
    };


    struct WorldInstance {
        world_t world;
        uint32_t n;
        uint32_t allocated;
        void *buffer;

        entity_t *entity;
        float *near;
        float *far;
        float *fov;
    };

#define _G CameraGlobal
    static struct CameraGlobal {
        uint64_t type;

        Map<uint32_t> world_map;
        Array<WorldInstance> world_instances;
        Map<uint32_t> ent_map;
    } CameraGlobal;


    static void allocate(WorldInstance &_data,
                         allocator *_allocator,
                         uint32_t sz) {
        //assert(sz > _data.n);

        WorldInstance new_data;
        const unsigned bytes = sz * (sizeof(entity_t) + (3 * sizeof(float)));
        new_data.buffer = CETECH_ALLOCATE(_allocator, char, bytes);
        new_data.n = _data.n;
        new_data.allocated = sz;

        new_data.entity = (entity_t *) (new_data.buffer);
        new_data.near = (float *) (new_data.entity + sz);
        new_data.far = (float *) (new_data.near + sz);
        new_data.fov = (float *) (new_data.far + sz);

        memcpy(new_data.entity, _data.entity, _data.n * sizeof(entity_t));
        memcpy(new_data.near, _data.near, _data.n * sizeof(float));
        memcpy(new_data.far, _data.far, _data.n * sizeof(float));
        memcpy(new_data.fov, _data.fov, _data.n * sizeof(float));

        CETECH_DEALLOCATE(_allocator, _data.buffer);

        _data = new_data;
    }

    void destroy(WorldInstance &_data,
                 unsigned i) {

        unsigned last = _data.n - 1;
        entity_t e = _data.entity[i];
        entity_t last_e = _data.entity[last];

        _data.entity[i] = _data.entity[last];
        _data.near[i] = _data.near[last];
        _data.far[i] = _data.far[last];
        _data.fov[i] = _data.fov[last];

//    map::set(_G._map, last_e.h, i);
//    map::remove(_G._map, e.h);

        --_data.n;
    }

    static void _new_world(world_t world) {
        uint32_t idx = array::size(_G.world_instances);
        array::push_back(_G.world_instances, WorldInstance());
        _G.world_instances[idx].world = world;
        map::set(_G.world_map, world.h, idx);
    }

    static void _destroy_world(world_t world) {
        uint32_t idx = map::get(_G.world_map, world.h, UINT32_MAX);
        uint32_t last_idx = array::size(_G.world_instances) - 1;

        world_t last_world = _G.world_instances[last_idx].world;

        CETECH_DEALLOCATE(memory_api_v0.main_allocator(),
                          _G.world_instances[idx].buffer);

        _G.world_instances[idx] = _G.world_instances[last_idx];
        map::set(_G.world_map, last_world.h, idx);
        array::pop_back(_G.world_instances);
    }

    static WorldInstance *_get_world_instance(world_t world) {
        uint32_t idx = map::get(_G.world_map, world.h, UINT32_MAX);

        if (idx != UINT32_MAX) {
            return &_G.world_instances[idx];
        }

        return nullptr;
    }

    int _camera_component_compiler(yaml_node_t body,
                                   blob_v0 *data) {

        struct camera_data t_data;

        YAML_NODE_SCOPE(near, body, "near", t_data.near = yaml_as_float(near););
        YAML_NODE_SCOPE(far, body, "far", t_data.far = yaml_as_float(far););
        YAML_NODE_SCOPE(fov, body, "fov", t_data.fov = yaml_as_float(fov););

        data->push(data->inst, (uint8_t *) &t_data, sizeof(t_data));

        return 1;
    }
}

namespace camera {
    int is_valid(camera_t camera) {
        return camera.idx != UINT32_MAX;
    }

    void get_project_view(world_t world,
                          camera_t camera,
                          mat44f_t *proj,
                          mat44f_t *view) {

        WorldInstance *world_inst = _get_world_instance(world);

        vec2f_t size = renderer_api_v0.get_size(); // TODO, to arg... or viewport?
        entity_t e = world_inst->entity[camera.idx];
        transform_t t = transform_api_v0.get(world, e);

        float fov = world_inst->fov[camera.idx];
        float near = world_inst->near[camera.idx];
        float far = world_inst->far[camera.idx];

        mat44f_set_perspective_fov(proj, fov, size.x / size.y, near, far);

        mat44f_t *w = transform_api_v0.get_world_matrix(world, t);
        mat44f_inverse(view, w);
    }

    int has(world_t world,
            entity_t entity) {

        uint32_t idx = world.h ^entity.h;

        return map::has(_G.ent_map, idx);
    }

    camera_t get(world_t world,
                 entity_t entity) {

        uint32_t idx = world.h ^entity.h;

        uint32_t component_idx = map::get(_G.ent_map, idx, UINT32_MAX);

        return (camera_t) {.idx = component_idx};
    }

    camera_t create(world_t world,
                    entity_t entity,
                    float near,
                    float far,
                    float fov) {

        WorldInstance *data = _get_world_instance(world);

        uint32_t idx = data->n;
        allocate(*data, memory_api_v0.main_allocator(), data->n + 1);
        ++data->n;

        data->entity[idx] = entity;
        data->near[idx] = near;
        data->far[idx] = far;
        data->fov[idx] = fov;

        map::set(_G.ent_map, world.h ^ entity.h, idx);

        return (camera_t) {.idx = idx};
    }

}

namespace camera_module {

    static struct camera_api_v0 camera_api = {
            .is_valid = camera::is_valid,
            .get_project_view = camera::get_project_view,
            .has = camera::has,
            .get = camera::get,
            .create = camera::create
    };

    void _on_world_create(world_t world) {
        _new_world(world);
    }

    void _on_world_destroy(world_t world) {
        _destroy_world(world);
    }

    void _destroyer(world_t world,
                    entity_t *ents,
                    size_t ent_count) {
        // TODO: remove from arrays, swap idx -> last AND change size
        for (int i = 0; i < ent_count; i++) {
            map::remove(_G.world_map, ents[i].h);
        }
    }

    void _spawner(world_t world,
                  entity_t *ents,
                  uint32_t *cents,
                  uint32_t *ents_parent,
                  size_t ent_count,
                  void *data) {
        camera_data *tdata = (camera_data *) data;

        for (int i = 0; i < ent_count; ++i) {
            camera::create(world,
                           ents[cents[i]],
                           tdata[i].near,
                           tdata[i].far,
                           tdata[i].fov);
        }
    }


    static void _init_api(struct api_v0 *api) {
        api->register_api("camera_api_v0", &camera_api);
    }


    static void _init(struct api_v0 *api_v0) {
        _init_api(api_v0);

        CETECH_GET_API(api_v0, memory_api_v0);
        CETECH_GET_API(api_v0, component_api_v0);
        CETECH_GET_API(api_v0, renderer_api_v0);
        CETECH_GET_API(api_v0, transform_api_v0);
        CETECH_GET_API(api_v0, hash_api_v0);

        _G = {0};

        _G.world_map.init(memory_api_v0.main_allocator());
        _G.world_instances.init(memory_api_v0.main_allocator());
        _G.ent_map.init(memory_api_v0.main_allocator());

        _G.type = hash_api_v0.id64_from_str("camera");

        component_api_v0.register_compiler(_G.type,
                                           _camera_component_compiler,
                                           10);

        component_api_v0.register_type(_G.type, {
                .spawner=_spawner,
                .destroyer=_destroyer,
                .on_world_create=_on_world_create,
                .on_world_destroy=_on_world_destroy
        });
    }

    static void _shutdown() {
        _G.ent_map.destroy();
        _G.world_instances.destroy();
        _G.world_map.destroy();
    }


    extern "C" void camera_load_module(struct api_v0 *api) {
        _init(api);
    }

    extern "C" void camera_unload_module(struct api_v0 *api) {
        _shutdown();
    }

}