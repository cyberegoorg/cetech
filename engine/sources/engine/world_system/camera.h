#ifndef CETECH_CAMERA_H
#define CETECH_CAMERA_H

#include "../world_system/types.h"
#include "../entcom/types.h"
#include "types.h"

int camera_init();

void camera_shutdown();

int camera_is_valid(camera_t camera);

mat44f_t camera_get_project_view(world_t world,
                                 camera_t camera);

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
