#ifndef BOUNDS_H
#define BOUNDS_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ce_aabb_t {
    float min[3];
    float max[3];
} ce_aabb_t;

typedef struct ce_cylinder_t {
    float pos[3];
    float end[3];
    float radius;
} ce_cylinder_t;

typedef struct ce_capsule_t {
    float pos[3];
    float end[3];
    float radius;
} ce_capsule_t;

typedef struct ce_cone_t {
    float pos[3];
    float end[3];
    float radius;
} ce_cone_t;

typedef struct ce_disk_t {
    float center[3];
    float normal[3];
    float radius;
} ce_disk_t;

typedef struct ce_obb_t {
    float mtx[16];
} ce_obb_t;

typedef struct ce_plane_t {
    float normal[3];
    float dist;
} ce_plane_t;

typedef struct ce_ray_t {
    float pos[3];
    float dir[3];
} ce_ray_t;

typedef struct ce_sphere_t {
    float center[3];
    float radius;
} ce_sphere_t;

typedef struct ce_tris_t {
    float v0[3];
    float v1[3];
    float v2[3];
} ce_tris_t;

typedef struct ce_hit_t {
    float pos[3];
    float normal[3];
    float dist;
} ce_hit_t;

#ifdef __cplusplus
};
#endif

#endif // BOUNDS_H
