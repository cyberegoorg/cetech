#ifndef CETECH_PHYSICS3D_H
#define CETECH_PHYSICS3D_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define PHYSICS3D_GROUP \
    CE_ID64_0("physics3d_group", 0xcd34852a751a8e82ULL)

#define PHYSICS3D_SYSTEM \
   CE_ID64_0("physics3d_system", 0x1a502fdb5130e3e7ULL)

#define CT_PHYSICS3D_A0 \
    CE_ID64_0("ct_physics3d_a0", 0x5f3b66dce0aa7c35ULL)

// World
#define PHYSICS3D_WORLD_COMPONENT \
    CE_ID64_0("physics_world3d", 0x95011a75ee1287baULL)

#define PHYSICS_WORLD3D_GRAVITY_PROP\
    CE_ID64_0("gravity", 0xccf0648c76b2a8c2ULL)

// Velocity
#define VELOCITY3D_COMPONENT \
    CE_ID64_0("velocity3d", 0x5bbcb4f3ec532828ULL)

#define VELOCITY3D_LINEAR_PROP \
    CE_ID64_0("linear", 0x4edd18565871c654ULL)

#define VELOCITY3D_ANGULAR_PROP \
    CE_ID64_0("angular", 0x73e2903632f8a84ULL)

// MASS
#define MASS3D_COMPONENT \
    CE_ID64_0("mass3d", 0x723906ab6ba4af64ULL)

#define MASS3D_PROP \
    CE_ID64_0("mass", 0xa06fab7c958a7771ULL)

// Collider
#define COLLIDER3D_COMPONENT \
    CE_ID64_0("collider3d", 0x33787862cf3e5b8aULL)

#define BODY3D_FRICTION_PROP \
    CE_ID64_0("friction", 0xa99da7441f8b5e66ULL)

#define BODY3D_RESTITUTION_PROP \
    CE_ID64_0("restitution", 0x2f06ac1f350e7dbeULL)

#define BODY3D_DENSITY_PROP \
    CE_ID64_0("density", 0xca98f4e6f68d2c8dULL)

#define BODY3D_IS_SENSOR_PROP \
    CE_ID64_0("is_sensor", 0x1d97e62c01dac9c4ULL)

#define COLLIDER3D_SHAPE_PROP \
    CE_ID64_0("shape", 0x3ef87fb905f6d51bULL)

#define COLLIDER3D_BOX \
    CE_ID64_0("collider3d_box", 0xb00dd4c60678fceaULL)

#define COLLIDER3D_BOX_HALF \
    CE_ID64_0("half_size", 0xfae83ca4619fb60aULL)

#define COLLIDER3D_SPHERE \
    CE_ID64_0("collider3d_sphere", 0xff9d5808485c6ffaULL)

#define COLLIDER3D_SPHERE_RADIUS \
    CE_ID64_0("radius", 0x197aa554763f64b0ULL)

typedef struct ct_physics_world3d_c {
    ce_vec3_t gravity;
} ct_physics_world3d_c;

typedef struct ct_velocity3d_c {
    ce_vec3_t linear;
    ce_vec3_t angular;
} ct_velocity3d_c;

typedef struct ct_mass3d_c {
    float mass;
} ct_mass3d_c;

typedef union ct_collider3d_shape_t{
    struct {
        ce_vec3_t half_size;
    } box;

    struct {
        float radius;
    } sphere;
} ct_collider3d_shape_t;

typedef struct ct_collider3d_c {
    uint64_t type;
    ct_collider3d_shape_t shape;
} ct_collider3d_c;

struct ct_physics3d_a0 {
    void (*_)();
};

CE_MODULE(ct_physics_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_PHYSICS3D_H
