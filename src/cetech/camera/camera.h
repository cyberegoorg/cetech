#ifndef CETECH_CAMERA_H
#define CETECH_CAMERA_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>


#define CT_CAMERA_API \
    CE_ID64_0("ct_camera_a0", 0x66ecb4e7c5973115ULL)

#define CAMERA_COMPONENT \
    CE_ID64_0("camera", 0x60ed8c3931822dc7ULL)

#define PROP_FOV \
    CE_ID64_0("fov", 0x805726a3c8974fbdULL)

#define PROP_NEAR \
    CE_ID64_0("near", 0xa1840af4acd01e77ULL)

#define PROP_FAR \
    CE_ID64_0("far", 0xe8d4ceded45dd6acULL)

#define PROP_PROJECTION \
    CE_ID64_0("projection", 0x765e7da8e9078d46ULL)

#define PROP_VIEW \
    CE_ID64_0("view", 0x3fce6c347b7f5eedULL)


typedef struct ct_world_t0 ct_world_t0;
typedef struct ct_entity_t0 ct_entity_t0;

struct ct_camera_component {
    float near;
    float far;
    float fov;
};

//! Camera API V0
struct ct_camera_a0 {
    //! Get camera project and view matrix
    void (*get_project_view)(ct_world_t0 world,
                             ct_entity_t0 entity,
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
