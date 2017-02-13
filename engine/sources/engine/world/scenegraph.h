#ifndef CETECH_SCENE_GAPH_H
#define CETECH_SCENE_GAPH_H

#import "engine/entcom/entcom.h"
#import "engine/world/world.h"
#import "celib/math/types.h"
#import "celib/string/stringid.h"

typedef struct {
    u32 idx;
} scene_node_t;


struct SceneGprahApiV1 {
    int (*is_valid)(scene_node_t transform);

    //void (*transform)(world_t world,scene_node_t transform,cel_mat44f_t *parent);
    cel_vec3f_t (*get_position)(world_t world,
                                scene_node_t transform);

    cel_quatf_t (*get_rotation)(world_t world,
                                scene_node_t transform);

    cel_vec3f_t (*get_scale)(world_t world,
                             scene_node_t transform);

    cel_mat44f_t *(*get_world_matrix)(world_t world,
                                      scene_node_t transform);

    void (*set_position)(world_t world,
                         scene_node_t transform,
                         cel_vec3f_t pos);

    void (*set_rotation)(world_t world,
                         scene_node_t transform,
                         cel_quatf_t rot);

    void (*set_scale)(world_t world,
                      scene_node_t transform,
                      cel_vec3f_t scale);

    int (*has)(world_t world,
               entity_t entity);

    scene_node_t (*get_root)(world_t world,
                             entity_t entity);

    scene_node_t (*create)(world_t world,
                           entity_t entity,
                           stringid64_t *names,
                           u32 *parent,
                           cel_mat44f_t *pose,
                           u32 count);

    void (*link)(world_t world,
                 scene_node_t parent,
                 scene_node_t child);

    scene_node_t (*node_by_name)(world_t world,
                                 entity_t entity,
                                 stringid64_t name);
};


#endif //CETECH_SCENE_GAPH_H
