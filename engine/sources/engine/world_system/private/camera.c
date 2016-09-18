#include <celib/containers/array.h>
#include <celib/yaml/yaml.h>
#include <celib/stringid/types.h>
#include <celib/containers/map.h>
#include <celib/math/mat44f.h>
#include <celib/stringid/stringid.h>

#include <engine/memory_system/memory_system.h>
#include <engine/entcom/entcom.h>
#include <engine/renderer/renderer.h>
#include <engine/entcom/types.h>
#include <engine/world_system/transform.h>
#include <celib/math/types.h>

#include "../camera.h"

ARRAY_PROTOTYPE(entity_t)


struct camera_data {
    f32 near;
    f32 far;
    f32 fov;
};

typedef struct {
    MAP_T(u32) ent_idx_map;

    ARRAY_T(entity_t) entity;
    ARRAY_T(f32) near;
    ARRAY_T(f32) far;
    ARRAY_T(f32) fov;

} world_data_t;

ARRAY_PROTOTYPE(world_data_t)
MAP_PROTOTYPE(world_data_t)

#define _G CameraGlobal
static struct G {
    stringid64_t type;

    MAP_T(world_data_t) world;
} _G = {0};


static void _new_world(world_t world) {
    world_data_t data = {0};

    MAP_INIT(u32, &data.ent_idx_map, memsys_main_allocator());

    ARRAY_INIT(entity_t, &data.entity, memsys_main_allocator());
    ARRAY_INIT(f32, &data.near, memsys_main_allocator());
    ARRAY_INIT(f32, &data.far, memsys_main_allocator());
    ARRAY_INIT(f32, &data.fov, memsys_main_allocator());

    MAP_SET(world_data_t, &_G.world, world.h.h, data);
}

static world_data_t *_get_world_data(world_t world) {
    return MAP_GET_PTR(world_data_t, &_G.world, world.h.h);
}

static void _destroy_world(world_t world) {
    world_data_t *data = _get_world_data(world);

    MAP_DESTROY(u32, &data->ent_idx_map);

    ARRAY_DESTROY(entity_t, &data->entity);
    ARRAY_DESTROY(f32, &data->near);
    ARRAY_DESTROY(f32, &data->far);
    ARRAY_DESTROY(f32, &data->fov);

}

int _camera_component_compiler(yaml_node_t body,
                               ARRAY_T(u8) *data) {

    struct camera_data t_data;

    YAML_NODE_SCOPE(near, body, "near", t_data.near = yaml_as_float(near););
    YAML_NODE_SCOPE(far, body, "far", t_data.far = yaml_as_float(far););
    YAML_NODE_SCOPE(fov, body, "fov", t_data.fov = yaml_as_float(fov););

    ARRAY_PUSH(u8, data, (u8 *) &t_data, sizeof(t_data));

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
        MAP_REMOVE(u32, &world_data->ent_idx_map, ents[i].idx);
    }
}

void _spawner(world_t world,
              entity_t *ents,
              entity_t *ents_parent,
              size_t ent_count,
              void *data) {
    struct camera_data *tdata = data;

    for (int i = 0; i < ent_count; ++i) {
        camera_create(world,
                      ents[i],
                      tdata->near,
                      tdata->far,
                      tdata->fov);
    }
}


int camera_init() {
    _G = (struct G) {0};

    MAP_INIT(world_data_t, &_G.world, memsys_main_allocator());

    _G.type = stringid64_from_string("camera");

    component_register_compiler(_G.type, _camera_component_compiler, 10);
    component_register_type(_G.type, _spawner, _destroyer, _on_world_create, _on_world_destroy);

    return 1;
}

void camera_shutdown() {

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


    vec2f_t size = renderer_get_size(); // TODO, to arg... or viewport?
    entity_t e = ARRAY_AT(&world_data->entity, camera.idx);
    transform_t t = transform_get(world, e);

    f32 fov = ARRAY_AT(&world_data->fov, camera.idx);
    f32 near = ARRAY_AT(&world_data->near, camera.idx);
    f32 far = ARRAY_AT(&world_data->far, camera.idx);

    mat44f_set_perspective_fov(proj, fov, size.x / size.y, near, far);

    mat44f_t *w = transform_get_world_matrix(world, t);
    mat44f_inverse(view, w);
}

int camera_has(world_t world,
               entity_t entity) {
    world_data_t *world_data = _get_world_data(world);
    return MAP_HAS(u32, &world_data->ent_idx_map, entity.h.h);
}

camera_t camera_get(world_t world,
                    entity_t entity) {

    world_data_t *world_data = _get_world_data(world);
    u32 idx = MAP_GET(u32, &world_data->ent_idx_map, entity.h.h, UINT32_MAX);
    return (camera_t) {.idx = idx};
}

camera_t camera_create(world_t world,
                       entity_t entity,
                       f32 near,
                       f32 far,
                       f32 fov) {

    world_data_t *data = _get_world_data(world);

    log_debug("camera", "create camera %f %f %f", near, far, fov);

    u32 idx = (u32) ARRAY_SIZE(&data->near);

    MAP_SET(u32, &data->ent_idx_map, entity.h.h, idx);

    ARRAY_PUSH_BACK(entity_t, &data->entity, entity);
    ARRAY_PUSH_BACK(f32, &data->near, near);
    ARRAY_PUSH_BACK(f32, &data->far, far);
    ARRAY_PUSH_BACK(f32, &data->fov, fov);

    return (camera_t) {.idx = idx};
}
