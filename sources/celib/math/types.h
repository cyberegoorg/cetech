#ifndef CETECH_MATH_TYPES_H
#define CETECH_MATH_TYPES_H

/*******************************************************************************
**** Vectors
*******************************************************************************/

typedef union {
    // POSITION
    struct {
        float x;
        float y;
    };

    // COLOR
    struct {
        float r;
        float g;
    };

    // RAW
    float v[2];
} vec2f_t;

typedef union {
    // POSITION
    struct {
        float x;
        float y;
        float z;
    };

    // COLOR
    struct {
        float r;
        float g;
        float b;
    };

    // RAW
    float v[3];
} vec3f_t;

typedef union {
    // POSITION
    struct {
        float x;
        float y;
        float z;
        float w;
    };

    // COLOR
    struct {
        float r;
        float g;
        float b;
        float a;
    };

    // RAW
    float v[4];
} vec4f_t;

/*******************************************************************************
**** Quaternion
*******************************************************************************/

typedef vec4f_t quatf_t;


/*******************************************************************************
**** Matrix
*******************************************************************************/

typedef union {
    // Rows
    struct {
        vec3f_t x;
        vec3f_t y;
        vec3f_t z;
    };


    // RAW
    float m[3*3];
} mat33f_t;

typedef union {
    // Rows
    struct {
        vec4f_t x;
        vec4f_t y;
        vec4f_t z;
        vec4f_t t;
    };


    // RAW
    float m[4*4];
} mat44f_t;

#endif //CETECH_MATH_TYPES_H
