#ifndef CETECH_MAT44F_H
#define CETECH_MAT44F_H


//==============================================================================
// Includes
//==============================================================================

#include <string.h>

#include "celib/types.h"
#include "fmath.h"
#include "vec4f.h"

//==============================================================================
// Defines
//==============================================================================


#define MAT44F_INIT_IDENTITY { 1.0f, 0.0f, 0.0f, 0.0f,   \
                               0.0f, 1.0f, 0.0f, 0.0f,   \
                               0.0f, 0.0f, 1.0f, 0.0f,   \
                               0.0f, 0.0f, 0.0f, 1.0f }  \


//==============================================================================
// Interface
//==============================================================================

CE_FORCE_INLINE void mat44f_identity(f32 *result) {
    memset(result, 0, sizeof(f32) * 16);

    result[0] = 1.0f;
    result[5] = 1.0f;
    result[10] = 1.0f;
    result[15] = 1.0f;
}

CE_FORCE_INLINE void mat44f_translate(f32 *result, f32 x, f32 y, f32 z) {
    mat44f_identity(result);

    result[12] = x;
    result[13] = y;
    result[14] = z;
}

CE_FORCE_INLINE void mat44f_scale(f32 *result, f32 x, f32 y, f32 z) {
    memset(result, 0, sizeof(f32) * 16);

    result[0] = x;
    result[5] = y;
    result[10] = z;
    result[15] = 1.0f;
}

CE_FORCE_INLINE void mat44f_rotate_x(f32 *result, f32 x) {
    const f32 sx = f32_sin(x);
    const f32 cx = f32_cos(x);

    memset(result, 0, sizeof(f32) * 16);

    result[0] = 1.0f;
    result[5] = cx;
    result[6] = -sx;
    result[9] = sx;
    result[10] = cx;
    result[15] = 1.0f;
}

CE_FORCE_INLINE void mat44f_rotate_y(f32 *result, f32 y) {
    const f32 sy = f32_sin(y);
    const f32 cy = f32_cos(y);

    memset(result, 0, sizeof(f32) * 16);

    result[0] = cy;
    result[2] = sy;
    result[5] = 1.0f;
    result[8] = -sy;
    result[10] = cy;
    result[15] = 1.0f;
}

CE_FORCE_INLINE void mat44f_rotate_z(f32 *result, f32 z) {
    const f32 sz = f32_sin(z);
    const f32 cz = f32_cos(z);

    memset(result, 0, sizeof(f32) * 16);

    result[0] = cz;
    result[1] = -sz;
    result[4] = sz;
    result[5] = cz;
    result[10] = 1.0f;
    result[15] = 1.0f;
}

CE_FORCE_INLINE void mat44f_rotate_xy(f32 *result, f32 x, f32 y) {
    const f32 sx = f32_sin(x);
    const f32 cx = f32_cos(x);
    const f32 sy = f32_sin(y);
    const f32 cy = f32_cos(y);

    memset(result, 0, sizeof(f32) * 16);

    result[0] = cy;
    result[2] = sy;
    result[4] = sx * sy;
    result[5] = cx;
    result[6] = -sx * cy;
    result[8] = -cx * sy;
    result[9] = sx;
    result[10] = cx * cy;
    result[15] = 1.0f;
}

CE_FORCE_INLINE void mat44f_rotate_xyz(f32 *result, f32 x, f32 y, f32 z) {
    const f32 sx = f32_sin(x);
    const f32 cx = f32_cos(x);
    const f32 sy = f32_sin(y);
    const f32 cy = f32_cos(y);
    const f32 sz = f32_sin(z);
    const f32 cz = f32_cos(z);

    memset(result, 0, sizeof(f32) * 16);

    result[0] = cy * cz;
    result[1] = -cy * sz;
    result[2] = sy;
    result[4] = cz * sx * sy + cx * sz;
    result[5] = cx * cz - sx * sy * sz;
    result[6] = -cy * sx;
    result[8] = -cx * cz * sy + sx * sz;
    result[9] = cz * sx + cx * sy * sz;
    result[10] = cx * cy;
    result[15] = 1.0f;
}

CE_FORCE_INLINE void mat44f_rotate_zyx(f32 *result, f32 x, f32 y, f32 z) {
    const f32 sx = f32_sin(x);
    const f32 cx = f32_cos(x);
    const f32 sy = f32_sin(y);
    const f32 cy = f32_cos(y);
    const f32 sz = f32_sin(z);
    const f32 cz = f32_cos(z);

    memset(result, 0, sizeof(f32) * 16);

    result[0] = cy * cz;
    result[1] = cz * sx * sy - cx * sz;
    result[2] = cx * cz * sy + sx * sz;
    result[4] = cy * sz;
    result[5] = cx * cz + sx * sy * sz;
    result[6] = -cz * sx + cx * sy * sz;
    result[8] = -sy;
    result[9] = cy * sx;
    result[10] = cx * cy;
    result[15] = 1.0f;
};

CE_FORCE_INLINE bool mat44f_eq(const f32 *__restrict a,
                               const f32 *__restrict b,
                               const f32 epsilon) {
    return f32_equals(a, b, 4 * 4, epsilon);
}

CE_FORCE_INLINE bool mat44f_is_identity(const f32 *__restrict a, f32 epsilon) {
    static mat44f_t _identity = MAT44F_INIT_IDENTITY;
    return mat44f_eq(a, _identity, epsilon);
}

CE_FORCE_INLINE void mat44f_mul(f32 *__restrict result,
                                const f32 *__restrict a,
                                const f32 *__restrict b) {

    vec4f_mul_mat44f(&result[0], &a[0], b);
    vec4f_mul_mat44f(&result[4], &a[4], b);
    vec4f_mul_mat44f(&result[8], &a[8], b);
    vec4f_mul_mat44f(&result[12], &a[12], b);
}

CE_FORCE_INLINE void mat44f_inverse(f32 *__restrict result, const f32 *__restrict a) {
    f32 xx = a[0];
    f32 xy = a[1];
    f32 xz = a[2];
    f32 xw = a[3];
    f32 yx = a[4];
    f32 yy = a[5];
    f32 yz = a[6];
    f32 yw = a[7];
    f32 zx = a[8];
    f32 zy = a[9];
    f32 zz = a[10];
    f32 zw = a[11];
    f32 wx = a[12];
    f32 wy = a[13];
    f32 wz = a[14];
    f32 ww = a[15];

    f32 det = 0.0f;
    det += xx * (yy * (zz * ww - zw * wz) - yz * (zy * ww - zw * wy) + yw * (zy * wz - zz * wy));
    det -= xy * (yx * (zz * ww - zw * wz) - yz * (zx * ww - zw * wx) + yw * (zx * wz - zz * wx));
    det += xz * (yx * (zy * ww - zw * wy) - yy * (zx * ww - zw * wx) + yw * (zx * wy - zy * wx));
    det -= xw * (yx * (zy * wz - zz * wy) - yy * (zx * wz - zz * wx) + yz * (zx * wy - zy * wx));

    f32 inv_det = 1.0f / det;

    result[0] = +(yy * (zz * ww - wz * zw) - yz * (zy * ww - wy * zw) + yw * (zy * wz - wy * zz)) * inv_det;
    result[1] = -(xy * (zz * ww - wz * zw) - xz * (zy * ww - wy * zw) + xw * (zy * wz - wy * zz)) * inv_det;
    result[2] = +(xy * (yz * ww - wz * yw) - xz * (yy * ww - wy * yw) + xw * (yy * wz - wy * yz)) * inv_det;
    result[3] = -(xy * (yz * zw - zz * yw) - xz * (yy * zw - zy * yw) + xw * (yy * zz - zy * yz)) * inv_det;

    result[4] = -(yx * (zz * ww - wz * zw) - yz * (zx * ww - wx * zw) + yw * (zx * wz - wx * zz)) * inv_det;
    result[5] = +(xx * (zz * ww - wz * zw) - xz * (zx * ww - wx * zw) + xw * (zx * wz - wx * zz)) * inv_det;
    result[6] = -(xx * (yz * ww - wz * yw) - xz * (yx * ww - wx * yw) + xw * (yx * wz - wx * yz)) * inv_det;
    result[7] = +(xx * (yz * zw - zz * yw) - xz * (yx * zw - zx * yw) + xw * (yx * zz - zx * yz)) * inv_det;

    result[8] = +(yx * (zy * ww - wy * zw) - yy * (zx * ww - wx * zw) + yw * (zx * wy - wx * zy)) * inv_det;
    result[9] = -(xx * (zy * ww - wy * zw) - xy * (zx * ww - wx * zw) + xw * (zx * wy - wx * zy)) * inv_det;
    result[10] = +(xx * (yy * ww - wy * yw) - xy * (yx * ww - wx * yw) + xw * (yx * wy - wx * yy)) * inv_det;
    result[11] = -(xx * (yy * zw - zy * yw) - xy * (yx * zw - zx * yw) + xw * (yx * zy - zx * yy)) * inv_det;

    result[12] = -(yx * (zy * wz - wy * zz) - yy * (zx * wz - wx * zz) + yz * (zx * wy - wx * zy)) * inv_det;
    result[13] = +(xx * (zy * wz - wy * zz) - xy * (zx * wz - wx * zz) + xz * (zx * wy - wx * zy)) * inv_det;
    result[14] = -(xx * (yy * wz - wy * yz) - xy * (yx * wz - wx * yz) + xz * (yx * wy - wx * yy)) * inv_det;
    result[15] = +(xx * (yy * zz - zy * yz) - xy * (yx * zz - zx * yz) + xz * (yx * zy - zx * yy)) * inv_det;
}

CE_FORCE_INLINE void mat44f_transpose(f32 *__restrict result, const f32 *__restrict a) {
    result[0] = a[0];
    result[4] = a[1];
    result[8] = a[2];
    result[12] = a[3];

    result[1] = a[4];
    result[5] = a[5];
    result[9] = a[6];
    result[13] = a[7];

    result[2] = a[8];
    result[6] = a[9];
    result[10] = a[10];
    result[14] = a[11];

    result[3] = a[12];
    result[7] = a[13];
    result[11] = a[14];
    result[15] = a[15];
}


#endif //CETECH_MAT44F_H
