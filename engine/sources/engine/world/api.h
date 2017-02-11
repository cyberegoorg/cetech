#ifndef CETECH_WORLD_API_H
#define CETECH_WORLD_API_H

#include <celib/math/types.h>
#include <celib/string/stringid.h>
#include <celib/yaml/yaml.h>

#include "engine/entcom/types.h"
#include "engine/resource/types.h"

struct CameraApiV1 {
    int (*is_valid)(camera_t camera);
    void (*get_project_view)(world_t world, camera_t camera, cel_mat44f_t *proj, cel_mat44f_t *view);
    int (*has)(world_t world,entity_t entity);
    camera_t (*get)(world_t world,entity_t entity);
    camera_t (*create)(world_t world, entity_t entity, f32 near, f32 far, f32 fov);
};

struct LevelApiV1 {
    level_t (*load_level)(world_t world, stringid64_t name);
    void (*destroy)(world_t world,level_t level);
    entity_t (*unit_by_id)(level_t level,stringid64_t name);
    entity_t (*unit)(level_t level);
};

struct SceneGprahApiV1 {
    int (*is_valid)(scene_node_t transform);
    //void (*transform)(world_t world,scene_node_t transform,cel_mat44f_t *parent);
    cel_vec3f_t (*get_position)(world_t world,scene_node_t transform);
    cel_quatf_t (*get_rotation)(world_t world,scene_node_t transform);
    cel_vec3f_t (*get_scale)(world_t world,scene_node_t transform);
    cel_mat44f_t *(*get_world_matrix)(world_t world, scene_node_t transform);
    void (*set_position)(world_t world, scene_node_t transform, cel_vec3f_t pos);
    void (*set_rotation)(world_t world,scene_node_t transform, cel_quatf_t rot);
    void (*set_scale)(world_t world, scene_node_t transform, cel_vec3f_t scale);
    int (*has)(world_t world, entity_t entity);
    scene_node_t (*get_root)(world_t world, entity_t entity);
    scene_node_t (*create)(world_t world, entity_t entity, stringid64_t *names, u32 *parent, cel_mat44f_t *pose, u32 count);
    void (*link)(world_t world, scene_node_t parent, scene_node_t child);
    scene_node_t (*node_by_name)(world_t world,  entity_t entity, stringid64_t name);
};

struct TransformApiV1 {
    int (*is_valid)(transform_t transform);
    void (*transform)(world_t world, transform_t transform, cel_mat44f_t *parent);
    cel_vec3f_t (*get_position)(world_t world, transform_t transform);
    cel_quatf_t (*get_rotation)(world_t world,transform_t transform);
    cel_vec3f_t (*get_scale)(world_t world, transform_t transform);
    cel_mat44f_t *(*get_world_matrix)(world_t world, transform_t transform);
    void (*set_position)(world_t world, transform_t transform, cel_vec3f_t pos);
    void (*set_rotation)(world_t world, transform_t transform, cel_quatf_t rot);
    void (*set_scale)(world_t world, transform_t transform, cel_vec3f_t scale);
    int (*has)(world_t world, entity_t entity);
    transform_t (*get)(world_t world, entity_t entity);
    transform_t (*create)(world_t world, entity_t entity, entity_t parent, cel_vec3f_t position, cel_quatf_t rotation, cel_vec3f_t scale);
    void (*link)(world_t world, entity_t parent, entity_t child);
};

struct UnitApiv1 {
    ARRAY_T(entity_t) *(*spawn_from_resource)(world_t world, void *resource);
    entity_t (*spawn)(world_t world, stringid64_t name);
    void (*destroy)(world_t world,entity_t *unit, u32 count);
    struct entity_compile_output *(*compiler_create_output)();
    void (*compiler_destroy_output)(struct entity_compile_output *output);
    void (*compiler_compile_unit)(struct entity_compile_output *output, yaml_node_t root, const char *filename, struct compilator_api *compilator_api);
    u32 (*compiler_ent_counter)(struct entity_compile_output *output);
    void (*compiler_write_to_build)(struct entity_compile_output *output, ARRAY_T(u8) *build);
    void (*resource_compiler)(yaml_node_t root, const char *filename, ARRAY_T(u8) *build, struct compilator_api *compilator_api);
};

struct WorldApiV1 {
    void (*register_callback)(world_callbacks_t clb);
    world_t (*create)();
    void (*destroy)(world_t world);
    void (*update)(world_t world, float dt);
    level_t (*load_level)(world_t world, stringid64_t name);
    entity_t (*unit_by_id)(level_t level, stringid64_t name);
    entity_t (*unit)(level_t level);
};

#endif //CETECH_WORLD_API_H
