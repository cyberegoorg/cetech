#ifndef CETECH_MATH_TYPES_H
#define CETECH_MATH_TYPES_H

//==============================================================================
// Includes
//==============================================================================


//==============================================================================
// Vectors
//==============================================================================

typedef struct vec2f_s {
    union {
        float f[2];
        struct {
            float x;
            float y;
        };
    };
} vec2f_t;


typedef struct vec3f_s {
    union {
        float f[3];
        struct {
            float x;
            float y;
            float z;
        };
    };
} vec3f_t;

typedef struct vec4f_s {
    union {
        float f[4];
        struct {
            float x;
            float y;
            float z;
            float w;
        };
    };
} vec4f_t;

//==============================================================================
// Quaternion
//==============================================================================

typedef vec4f_t quatf_t;


//==============================================================================
// Matrix
//==============================================================================

typedef struct mat33f_s {
    union {
        float f[3 * 3];
        struct {
            vec3f_t x;
            vec3f_t y;
            vec3f_t z;
        };
    };
} mat33f_t;


typedef struct mat44f_s {
    union {
        float f[4 * 4];
        struct {
            vec4f_t x;
            vec4f_t y;
            vec4f_t z;
            vec4f_t w;
        };
    };
} mat44f_t;

#endif //CETECH_MATH_TYPES_H
