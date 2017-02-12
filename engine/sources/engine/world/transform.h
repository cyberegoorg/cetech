#ifndef CETECH_TRANSFORM_H
#define CETECH_TRANSFORM_H

#include "celib/math/types.h"
#include "engine/entcom/entcom.h"


typedef struct {
    u32 idx;
} transform_t;

struct TransformApiV1 {
    int (*is_valid)(transform_t transform);

    void (*transform)(world_t world,
                      transform_t transform,
                      cel_mat44f_t *parent);

    cel_vec3f_t (*get_position)(world_t world,
                                transform_t transform);

    cel_quatf_t (*get_rotation)(world_t world,
                                transform_t transform);

    cel_vec3f_t (*get_scale)(world_t world,
                             transform_t transform);

    cel_mat44f_t *(*get_world_matrix)(world_t world,
                                      transform_t transform);

    void (*set_position)(world_t world,
                         transform_t transform,
                         cel_vec3f_t pos);

    void (*set_rotation)(world_t world,
                         transform_t transform,
                         cel_quatf_t rot);

    void (*set_scale)(world_t world,
                      transform_t transform,
                      cel_vec3f_t scale);

    int (*has)(world_t world,
               entity_t entity);

    transform_t (*get)(world_t world,
                       entity_t entity);

    transform_t (*create)(world_t world,
                          entity_t entity,
                          entity_t parent,
                          cel_vec3f_t position,
                          cel_quatf_t rotation,
                          cel_vec3f_t scale);

    void (*link)(world_t world,
                 entity_t parent,
                 entity_t child);
};
//int transform_is_valid(transform_t transform);
//
//void transform_transform(world_t world,
//                         transform_t transform,
//                         cel_mat44f_t *parent);
//
//cel_vec3f_t transform_get_position(world_t world,
//                                   transform_t transform);
//
//cel_quatf_t transform_get_rotation(world_t world,
//                                   transform_t transform);
//
//
//cel_vec3f_t transform_get_scale(world_t world,
//                                transform_t transform);
//
//
//cel_mat44f_t *transform_get_world_matrix(world_t world,
//                                         transform_t transform);
//
//
//void transform_set_position(world_t world,
//                            transform_t transform,
//                            cel_vec3f_t pos);
//
//
//void transform_set_rotation(world_t world,
//                            transform_t transform,
//                            cel_quatf_t rot);
//
//
//void transform_set_scale(world_t world,
//                         transform_t transform,
//                         cel_vec3f_t scale);
//
//int transform_has(world_t world,
//                  entity_t entity);
//
//transform_t transform_get(world_t world,
//                          entity_t entity);
//
//transform_t transform_create(world_t world,
//                             entity_t entity,
//                             entity_t parent,
//                             cel_vec3f_t position,
//                             cel_quatf_t rotation,
//                             cel_vec3f_t scale);
//
//
//void transform_link(world_t world,
//                    entity_t parent,
//                    entity_t child);
//

#endif //CETECH_TRANSFORM_H
