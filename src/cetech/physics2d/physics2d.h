#ifndef CETECH_PHYSICS2D_H
#define CETECH_PHYSICS2D_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define PHYSICS2D_GROUP \
    CE_ID64_0("physics2d_group", 0x9ab6f3cfdc87fe30ULL)

#define PHYSICS2D_SYSTEM \
    CE_ID64_0("physics2d", 0x4fdc4a291d00e7f5ULL)

#define CT_PHYSICS_A0 \
    CE_ID64_0("ct_physics_a0", 0x3ef93fecf67b99b8ULL)

// World
#define PHYSICS_WORLD2D_COMPONENT \
    CE_ID64_0("physics_world2d", 0xcaa7b937d52834fcULL)

#define PHYSICS_WORLD2D_GRAVITY_PROP\
    CE_ID64_0("gravity", 0xccf0648c76b2a8c2ULL)

// Velocity
#define VELOCITY2D_COMPONENT \
    CE_ID64_0("velocity2d", 0x223ef3bf4e53ae66ULL)

#define VELOCITY2D_LINEAR_PROP \
    CE_ID64_0("linear", 0x4edd18565871c654ULL)

#define VELOCITY2D_ANGULAR_PROP \
    CE_ID64_0("angular", 0x73e2903632f8a84ULL)

// MASS
#define MASS2D_COMPONENT \
    CE_ID64_0("mass2d", 0x8cb74b2aa7143c7bULL)

#define MASS2D_PROP \
    CE_ID64_0("mass", 0xa06fab7c958a7771ULL)

// Collider
#define COLLIDER2D_COMPONENT \
    CE_ID64_0("collider2d", 0x412adb7f43ed22b8ULL)

#define BODY2D_FRICTION_PROP \
    CE_ID64_0("friction", 0xa99da7441f8b5e66ULL)

#define BODY2D_RESTITUTION_PROP \
    CE_ID64_0("restitution", 0x2f06ac1f350e7dbeULL)

#define BODY2D_DENSITY_PROP \
    CE_ID64_0("density", 0xca98f4e6f68d2c8dULL)

#define BODY2D_IS_SENSOR_PROP \
    CE_ID64_0("is_sensor", 0x1d97e62c01dac9c4ULL)

#define PHYSICS_COLLIDER2D_SHAPE_PROP \
    CE_ID64_0("shape", 0x3ef87fb905f6d51bULL)

#define COLLIDER2D_RECTANGLE \
    CE_ID64_0("collider2d_shape_rectangle", 0x5b61668f7f402343ULL)

#define PHYSICS_COLLIDER2D_RECTANGLE_HALF_SIZE \
    CE_ID64_0("half_size", 0xfae83ca4619fb60aULL)

typedef struct ct_physics_world2d_c {
    ce_vec2_t gravity;
} ct_physics_world2d_c;

typedef struct ct_velocity2d_c {
    ce_vec2_t linear;
    float angular;
} ct_velocity2d_c;

typedef struct ct_mass2d_c {
    float mass;
} ct_mass2d_c;

typedef struct ct_collider2d_c {
    uint64_t type;
    union {
        struct {
            ce_vec2_t half_size;
        } rectangle;
    };
} ct_collider2d_c;

struct ct_physics2d_a0 {
    void (*_)();
};

CE_MODULE(ct_physics2d_a0);

#ifdef __cplusplus
};
#endif

#endif //CETECH_PHYSICS2D_H
