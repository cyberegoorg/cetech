#ifndef CETECH_CAMERA_H
#define CETECH_CAMERA_H

#include "engine/core/types.h"


typedef struct {
    u32 idx;
} camera_t;

int camera_init(int stage);

void camera_shutdown();

int camera_is_valid(camera_t camera);

void camera_get_project_view(world_t world,
                             camera_t camera,
                             mat44f_t *proj,
                             mat44f_t *view);

int camera_has(world_t world,
               entity_t entity);

camera_t camera_get(world_t world,
                    entity_t entity);

camera_t camera_create(world_t world,
                       entity_t entity,
                       f32 near,
                       f32 far,
                       f32 fov);


#endif //CETECH_CAMERA_H
