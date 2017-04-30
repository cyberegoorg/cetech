#include <cetech/array.inl>
#include <cetech/yaml.h>
#include <cetech/map.inl>
#include <cetech/math_mat44f.inl>
#include <cetech/stringid.h>

#include <cetech/entity.h>
#include <cetech/component.h>
#include <cetech/renderer.h>
#include <cetech/transform.h>

#include <cetech/memory.h>
#include <cetech/module.h>


struct camera_data {
    float near;
    float far;
    float fov;
};

typedef struct {
    MAP_T(uint32_t) ent_idx_map;

    ARRAY_T(entity_t) entity;
    ARRAY_T(float) near;
    ARRAY_T(float) far;
    ARRAY_T(float) fov;

} world_data_t;

IMPORT_API(memory_api_v0);
IMPORT_API(component_api_v0);
IMPORT_API(renderer_api_v0);
IMPORT_API(transform_api_v0);

ARRAY_PROTOTYPE(world_data_t)

MAP_PROTOTYPE(world_data_t)

#define _G CameraGlobal
static struct G {
    stringid64_t type;

    MAP_T(world_data_t) world;
} _G = {0};


static void _new_world(world_t world) {
    world_data_t data = {0};

    MAP_INIT(uint32_t, &data.ent_idx_map, memory_api_v0.main_allocator());

    ARRAY_INIT(entity_t, &data.entity, memory_api_v0.main_allocator());
    ARRAY_INIT(float, &data.near, memory_api_v0.main_allocator());
    ARRAY_INIT(float, &data.far, memory_api_v0.main_allocator());
    ARRAY_INIT(float, &data.fov, memory_api_v0.main_allocator());

    MAP_SET(world_data_t, &_G.world, world.h.id, data);
}

static world_data_t *_get_world_data(world_t world) {
    return MAP_GET_PTR(world_data_t, &_G.world, world.h.id);
}

static void _destroy_world(world_t world) {
    world_data_t *data = _get_world_data(world);

    MAP_DESTROY(uint32_t, &data->ent_idx_map);

    ARRAY_DESTROY(entity_t, &data->entity);
    ARRAY_DESTROY(float, &data->near);
    ARRAY_DESTROY(float, &data->far);
    ARRAY_DESTROY(float, &data->fov);

}

int _camera_component_compiler(yaml_node_t body,
                               ARRAY_T(uint8_t) *data) {

    struct camera_data t_data;

    YAML_NODE_SCOPE(near, body, "near", t_data.near = yaml_as_float(near););
    YAML_NODE_SCOPE(far, body, "far", t_data.far = yaml_as_float(far););
    YAML_NODE_SCOPE(fov, body, "fov", t_data.fov = yaml_as_float(fov););

    ARRAY_PUSH(uint8_t, data, (uint8_t *) &t_data, sizeof(t_data));

    return 1;
}

void _on_world_create(world_t world) {
    _new_world(world);
}

void _on_world_destroy(world_t world) {
    _destroy_world(world);
}

void _destroyer(world_t world,
                entity_t *ents,
                size_t ent_count) {
    world_data_t *world_data = _get_world_data(world);

    // TODO: remove from arrays, swap idx -> last AND change size
    for (int i = 0; i < ent_count; i++) {
        MAP_REMOVE(uint32_t, &world_data->ent_idx_map, ents[i].h.id);
    }
}


camera_t camera_create(world_t world,
                       entity_t entity,
                       float near,
                       float far,
                       float fov);


void _spawner(world_t world,
              entity_t *ents,
              uint32_t *cents,
              uint32_t *ents_parent,
              size_t ent_count,
              void *data) {
    struct camera_data *tdata = data;

    for (int i = 0; i < ent_count; ++i) {
        camera_create(world,
                      ents[cents[i]],
                      tdata[i].near,
                      tdata[i].far,
                      tdata[i].fov);
    }
}


static void _init(get_api_fce_t get_engine_api) {
    INIT_API(get_engine_api, memory_api_v0, MEMORY_API_ID);
    INIT_API(get_engine_api, component_api_v0, COMPONENT_API_ID);
    INIT_API(get_engine_api, renderer_api_v0, RENDERER_API_ID);
    INIT_API(get_engine_api, transform_api_v0, TRANSFORM_API_ID);

    _G = (struct G) {0};

    MAP_INIT(world_data_t, &_G.world, memory_api_v0.main_allocator());

    _G.type = stringid64_from_string("camera");

    component_api_v0.component_register_compiler(_G.type,
                                                  _camera_component_compiler,
                                                  10);
    component_api_v0.component_register_type(_G.type, (struct component_clb) {
            .spawner=_spawner, .destroyer=_destroyer,
            .on_world_create=_on_world_create, .on_world_destroy=_on_world_destroy
    });
}

static void _shutdown() {
    MAP_DESTROY(world_data_t, &_G.world);

    _G = (struct G) {0};
}


int camera_is_valid(camera_t camera) {
    return camera.idx != UINT32_MAX;
}

void camera_get_project_view(world_t world,
                             camera_t camera,
                             mat44f_t *proj,
                             mat44f_t *view) {

    world_data_t *world_data = _get_world_data(world);


    vec2f_t size = renderer_api_v0.get_size(); // TODO, to arg... or viewport?
    entity_t e = ARRAY_AT(&world_data->entity, camera.idx);
    transform_t t = transform_api_v0.get(world, e);

    float fov = ARRAY_AT(&world_data->fov, camera.idx);
    float near = ARRAY_AT(&world_data->near, camera.idx);
    float far = ARRAY_AT(&world_data->far, camera.idx);

    mat44f_set_perspective_fov(proj, fov, size.x / size.y, near, far);

    mat44f_t *w = transform_api_v0.get_world_matrix(world, t);
    mat44f_inverse(view, w);
}

int camera_has(world_t world,
               entity_t entity) {
    world_data_t *world_data = _get_world_data(world);
    return MAP_HAS(uint32_t, &world_data->ent_idx_map, entity.h.id);
}

camera_t camera_get(world_t world,
                    entity_t entity) {

    world_data_t *world_data = _get_world_data(world);
    uint32_t idx = MAP_GET(uint32_t, &world_data->ent_idx_map, entity.h.id, UINT32_MAX);
    return (camera_t) {.idx = idx};
}

camera_t camera_create(world_t world,
                       entity_t entity,
                       float near,
                       float far,
                       float fov) {

    world_data_t *data = _get_world_data(world);

    uint32_t idx = (uint32_t) ARRAY_SIZE(&data->near);

    MAP_SET(uint32_t, &data->ent_idx_map, entity.h.id, idx);

    ARRAY_PUSH_BACK(entity_t, &data->entity, entity);
    ARRAY_PUSH_BACK(float, &data->near, near);
    ARRAY_PUSH_BACK(float, &data->far, far);
    ARRAY_PUSH_BACK(float, &data->fov, fov);

    return (camera_t) {.idx = idx};
}

void *camera_get_module_api(int api) {

    switch (api) {
        case PLUGIN_EXPORT_API_ID:
                {
                    static struct module_api_v0 module = {0};

                    module.init = _init;
                    module.shutdown = _shutdown;

                    return &module;
                }

        case CAMERA_API_ID:
             {
                    static struct camera_api_v0 api = {0};

                    api.is_valid = camera_is_valid;
                    api.get_project_view = camera_get_project_view;
                    api.has = camera_has;
                    api.get = camera_get;
                    api.create = camera_create;

                    return &api;
                }

        default:
            return NULL;
    }
}
