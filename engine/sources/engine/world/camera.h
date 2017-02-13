#ifndef CETECH_CAMERA_H
#define CETECH_CAMERA_H

#include "world.h"
#include "engine/entcom/entcom.h"

typedef struct {
    u32 idx;
} camera_t;

struct CameraApiV1 {
    int (*is_valid)(camera_t camera);

    void (*get_project_view)(world_t world,
                             camera_t camera,
                             cel_mat44f_t *proj,
                             cel_mat44f_t *view);

    int (*has)(world_t world,
               entity_t entity);

    camera_t (*get)(world_t world,
                    entity_t entity);

    camera_t (*create)(world_t world,
                       entity_t entity,
                       f32 near,
                       f32 far,
                       f32 fov);
};


//int camera_is_valid(camera_t camera);
//
//void camera_get_project_view(world_t world,
//                             camera_t camera,
//                             cel_mat44f_t *proj,
//                             cel_mat44f_t *view);
//
//int camera_has(world_t world,
//               entity_t entity);
//
//camera_t camera_get(world_t world,
//                    entity_t entity);
//
//camera_t camera_create(world_t world,
//                       entity_t entity,
//                       f32 near,
//                       f32 far,
//                       f32 fov);

#endif //CETECH_CAMERA_H
