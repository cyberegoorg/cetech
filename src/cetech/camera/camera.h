#ifndef CETECH_CAMERA_H
#define CETECH_CAMERA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>


#define CT_CAMERA_API \
    CE_ID64_0("ct_camera_a0", 0x66ecb4e7c5973115ULL)

#define CT_CAMERA_COMPONENT \
    CE_ID64_0("camera", 0x60ed8c3931822dc7ULL)

#define CT_ACTIVE_CAMERA_COMPONENT \
    CE_ID64_0("active_camera", 0xa0430cee11242caULL)

#define PROP_CAMERA_TYPE \
    CE_ID64_0("camera_type", 0xaf56273bba8c279bULL)

#define CAMERA_TYPE_ORTHO \
    CE_ID64_0("ortho", 0x747f66809b7ad519ULL)

#define CAMERA_TYPE_PERSPECTIVE \
    CE_ID64_0("perspective", 0x281d823258cc7c6eULL)

#define PROP_FOV \
    CE_ID64_0("fov", 0x805726a3c8974fbdULL)

#define PROP_NEAR \
    CE_ID64_0("near", 0xa1840af4acd01e77ULL)

#define PROP_FAR \
    CE_ID64_0("far", 0xe8d4ceded45dd6acULL)


#define PROP_CAMERA_ENT \
    CE_ID64_0("camera_ent", 0x83b9039ed72a162fULL)

typedef struct ct_world_t0 ct_world_t0;
typedef struct ct_entity_t0 ct_entity_t0;

typedef struct ct_camera_component {
    float near;
    float far;
    float fov;
    uint64_t camera_type;
} ct_camera_component;

typedef struct ct_camera_data_t0 {
    ct_camera_component camera;
    ce_mat4_t world;
} ct_camera_data_t0;

typedef struct ct_active_camera {
    ct_entity_t0 camera_ent;
} ct_active_camera;

//! Camera API V0
struct ct_camera_a0 {
    //! Get camera project and view matrix
    void (*get_project_view)(ce_mat4_t world,
                             ct_camera_component camera,
                             float *proj,
                             float *view,
                             int width,
                             int height);
};


CE_MODULE(ct_camera_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_CAMERA_H
