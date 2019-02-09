#ifndef BOUNDS_H
#define BOUNDS_H

#ifdef __cplusplus
extern "C" {
#endif

#include "celib/celib_types.h"

typedef struct ce_aabb_t {
    ce_vec3_t min;
    ce_vec3_t max;
} ce_aabb_t;

typedef struct ce_cylinder_t {
    ce_vec3_t pos;
    ce_vec3_t end;
    float radius;
} ce_cylinder_t;

typedef struct ce_capsule_t {
    ce_vec3_t pos;
    ce_vec3_t end;
    float radius;
} ce_capsule_t;

typedef struct ce_cone_t {
    ce_vec3_t pos;
    ce_vec3_t end;
    float radius;
} ce_cone_t;

typedef struct ce_disk_t {
    ce_vec3_t center;
    ce_vec3_t normal;
    float radius;
} ce_disk_t;

typedef struct ce_obb_t {
    float mtx[16];
} ce_obb_t;

typedef struct ce_plane_t {
    ce_vec3_t normal;
    ce_vec3_t dist;
} ce_plane_t;

typedef struct ce_ray_t {
    ce_vec3_t pos;
    ce_vec3_t dir;
} ce_ray_t;

typedef struct ce_sphere_t {
    ce_vec3_t center;
    float radius;
} ce_sphere_t;

typedef struct ce_tris_t {
    ce_vec3_t v0;
    ce_vec3_t v1;
    ce_vec3_t v2;
} ce_tris_t;

typedef struct ce_hit_t {
    ce_vec3_t pos;
    ce_vec3_t normal;
    float dist;
} ce_hit_t;

#ifdef __cplusplus
};
#endif

#endif // BOUNDS_H
