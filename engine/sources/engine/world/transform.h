#ifndef CETECH_TRANSFORM_H
#define CETECH_TRANSFORM_H

#include "types.h"
#include "celib/math/types.h"
#include "engine/entcom/types.h"

int transform_init(int stage);

void transform_shutdown();

int transform_is_valid(transform_t transform);

void transform_transform(world_t world,
                         transform_t transform,
                         mat44f_t *parent);

vec3f_t transform_get_position(world_t world,
                               transform_t transform);

quatf_t transform_get_rotation(world_t world,
                               transform_t transform);


vec3f_t transform_get_scale(world_t world,
                            transform_t transform);


mat44f_t *transform_get_world_matrix(world_t world,
                                     transform_t transform);


void transform_set_position(world_t world,
                            transform_t transform,
                            vec3f_t pos);


void transform_set_rotation(world_t world,
                            transform_t transform,
                            quatf_t rot);


void transform_set_scale(world_t world,
                         transform_t transform,
                         vec3f_t scale);

int transform_has(world_t world,
                  entity_t entity);

transform_t transform_get(world_t world,
                          entity_t entity);

transform_t transform_create(world_t world,
                             entity_t entity,
                             entity_t parent,
                             vec3f_t position,
                             quatf_t rotation,
                             vec3f_t scale);


void transform_link(world_t world,
                    entity_t parent,
                    entity_t child);


#endif //CETECH_TRANSFORM_H
