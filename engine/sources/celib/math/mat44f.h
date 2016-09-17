#ifndef CETECH_MAT44F_H
#define CETECH_MAT44F_H


//==============================================================================
// Includes
//==============================================================================

#include <string.h>

#include "celib/types.h"
#include "fmath.h"
#include "vec4f.h"
#include "types.h"

//==============================================================================
// Defines
//==============================================================================


#define MAT44F_INIT_IDENTITY (mat44f_t)                  \
                             { 1.0f, 0.0f, 0.0f, 0.0f,   \
                               0.0f, 1.0f, 0.0f, 0.0f,   \
                               0.0f, 0.0f, 1.0f, 0.0f,   \
                               0.0f, 0.0f, 0.0f, 1.0f }  \


//==============================================================================
// Interface
//==============================================================================

CE_FORCE_INLINE void mat44f_identity(mat44f_t *result) {
    memset(result, 0, sizeof(f32) * 16);

    result->f[0] = 1.0f;
    result->f[5] = 1.0f;
    result->f[10] = 1.0f;
    result->f[15] = 1.0f;
}

CE_FORCE_INLINE void mat44f_translate(mat44f_t *result,
                                      f32 x,
                                      f32 y,
                                      f32 z) {
    mat44f_identity(result);

    result->f[12] = x;
    result->f[13] = y;
    result->f[14] = z;
}

CE_FORCE_INLINE void mat44f_scale(mat44f_t *result,
                                  f32 x,
                                  f32 y,
                                  f32 z) {
    memset(result, 0, sizeof(f32) * 16);

    result->f[0] = x;
    result->f[5] = y;
    result->f[10] = z;
    result->f[15] = 1.0f;
}

CE_FORCE_INLINE void mat44f_rotate_x(mat44f_t *result,
                                     f32 x) {
    const f32 sx = f32_sin(x);
    const f32 cx = f32_cos(x);

    memset(result, 0, sizeof(f32) * 16);

    result->f[0] = 1.0f;
    result->f[5] = cx;
    result->f[6] = -sx;
    result->f[9] = sx;
    result->f[10] = cx;
    result->f[15] = 1.0f;
}

CE_FORCE_INLINE void mat44f_rotate_y(mat44f_t *result,
                                     f32 y) {
    const f32 sy = f32_sin(y);
    const f32 cy = f32_cos(y);

    memset(result, 0, sizeof(f32) * 16);

    result->f[0] = cy;
    result->f[2] = sy;
    result->f[5] = 1.0f;
    result->f[8] = -sy;
    result->f[10] = cy;
    result->f[15] = 1.0f;
}

CE_FORCE_INLINE void mat44f_rotate_z(mat44f_t *result,
                                     f32 z) {
    const f32 sz = f32_sin(z);
    const f32 cz = f32_cos(z);

    memset(result, 0, sizeof(f32) * 16);

    result->f[0] = cz;
    result->f[1] = -sz;
    result->f[4] = sz;
    result->f[5] = cz;
    result->f[10] = 1.0f;
    result->f[15] = 1.0f;
}

CE_FORCE_INLINE void mat44f_rotate_xy(mat44f_t *result,
                                      f32 x,
                                      f32 y) {
    const f32 sx = f32_sin(x);
    const f32 cx = f32_cos(x);
    const f32 sy = f32_sin(y);
    const f32 cy = f32_cos(y);

    memset(result, 0, sizeof(f32) * 16);

    result->f[0] = cy;
    result->f[2] = sy;
    result->f[4] = sx * sy;
    result->f[5] = cx;
    result->f[6] = -sx * cy;
    result->f[8] = -cx * sy;
    result->f[9] = sx;
    result->f[10] = cx * cy;
    result->f[15] = 1.0f;
}

CE_FORCE_INLINE void mat44f_rotate_xyz(mat44f_t *result,
                                       f32 x,
                                       f32 y,
                                       f32 z) {
    const f32 sx = f32_sin(x);
    const f32 cx = f32_cos(x);
    const f32 sy = f32_sin(y);
    const f32 cy = f32_cos(y);
    const f32 sz = f32_sin(z);
    const f32 cz = f32_cos(z);

    memset(result, 0, sizeof(f32) * 16);

    result->f[0] = cy * cz;
    result->f[1] = -cy * sz;
    result->f[2] = sy;
    result->f[4] = cz * sx * sy + cx * sz;
    result->f[5] = cx * cz - sx * sy * sz;
    result->f[6] = -cy * sx;
    result->f[8] = -cx * cz * sy + sx * sz;
    result->f[9] = cz * sx + cx * sy * sz;
    result->f[10] = cx * cy;
    result->f[15] = 1.0f;
}

CE_FORCE_INLINE void mat44f_rotate_zyx(mat44f_t *result,
                                       f32 x,
                                       f32 y,
                                       f32 z) {
    const f32 sx = f32_sin(x);
    const f32 cx = f32_cos(x);
    const f32 sy = f32_sin(y);
    const f32 cy = f32_cos(y);
    const f32 sz = f32_sin(z);
    const f32 cz = f32_cos(z);

    memset(result, 0, sizeof(f32) * 16);

    result->f[0] = cy * cz;
    result->f[1] = cz * sx * sy - cx * sz;
    result->f[2] = cx * cz * sy + sx * sz;
    result->f[4] = cy * sz;
    result->f[5] = cx * cz + sx * sy * sz;
    result->f[6] = -cz * sx + cx * sy * sz;
    result->f[8] = -sy;
    result->f[9] = cy * sx;
    result->f[10] = cx * cy;
    result->f[15] = 1.0f;
};

CE_FORCE_INLINE int mat44f_eq(const mat44f_t *__restrict a,
                              const mat44f_t *__restrict b,
                              const f32 epsilon) {
    return f32_equals(a->f, b->f, 4 * 4, epsilon);
}

CE_FORCE_INLINE int mat44f_is_identity(const mat44f_t *__restrict a,
                                       f32 epsilon) {
    static mat44f_t _identity = MAT44F_INIT_IDENTITY;
    return mat44f_eq(a, &_identity, epsilon);
}

CE_FORCE_INLINE void mat44f_mul(mat44f_t *__restrict result,
                                const mat44f_t *__restrict a,
                                const mat44f_t *__restrict b) {

    vec4f_mul_mat44f(&result->x, &a->x, b);
    vec4f_mul_mat44f(&result->y, &a->y, b);
    vec4f_mul_mat44f(&result->z, &a->z, b);
    vec4f_mul_mat44f(&result->w, &a->w, b);
}

CE_FORCE_INLINE void mat44f_inverse(mat44f_t *__restrict result,
                                    const mat44f_t *__restrict a) {
    f32 xx = a->f[0];
    f32 xy = a->f[1];
    f32 xz = a->f[2];
    f32 xw = a->f[3];
    f32 yx = a->f[4];
    f32 yy = a->f[5];
    f32 yz = a->f[6];
    f32 yw = a->f[7];
    f32 zx = a->f[8];
    f32 zy = a->f[9];
    f32 zz = a->f[10];
    f32 zw = a->f[11];
    f32 wx = a->f[12];
    f32 wy = a->f[13];
    f32 wz = a->f[14];
    f32 ww = a->f[15];

    f32 det = 0.0f;
    det += xx * (yy * (zz * ww - zw * wz) - yz * (zy * ww - zw * wy) + yw * (zy * wz - zz * wy));
    det -= xy * (yx * (zz * ww - zw * wz) - yz * (zx * ww - zw * wx) + yw * (zx * wz - zz * wx));
    det += xz * (yx * (zy * ww - zw * wy) - yy * (zx * ww - zw * wx) + yw * (zx * wy - zy * wx));
    det -= xw * (yx * (zy * wz - zz * wy) - yy * (zx * wz - zz * wx) + yz * (zx * wy - zy * wx));

    f32 inv_det = 1.0f / det;

    result->f[0] = +(yy * (zz * ww - wz * zw) - yz * (zy * ww - wy * zw) + yw * (zy * wz - wy * zz)) * inv_det;
    result->f[1] = -(xy * (zz * ww - wz * zw) - xz * (zy * ww - wy * zw) + xw * (zy * wz - wy * zz)) * inv_det;
    result->f[2] = +(xy * (yz * ww - wz * yw) - xz * (yy * ww - wy * yw) + xw * (yy * wz - wy * yz)) * inv_det;
    result->f[3] = -(xy * (yz * zw - zz * yw) - xz * (yy * zw - zy * yw) + xw * (yy * zz - zy * yz)) * inv_det;

    result->f[4] = -(yx * (zz * ww - wz * zw) - yz * (zx * ww - wx * zw) + yw * (zx * wz - wx * zz)) * inv_det;
    result->f[5] = +(xx * (zz * ww - wz * zw) - xz * (zx * ww - wx * zw) + xw * (zx * wz - wx * zz)) * inv_det;
    result->f[6] = -(xx * (yz * ww - wz * yw) - xz * (yx * ww - wx * yw) + xw * (yx * wz - wx * yz)) * inv_det;
    result->f[7] = +(xx * (yz * zw - zz * yw) - xz * (yx * zw - zx * yw) + xw * (yx * zz - zx * yz)) * inv_det;

    result->f[8] = +(yx * (zy * ww - wy * zw) - yy * (zx * ww - wx * zw) + yw * (zx * wy - wx * zy)) * inv_det;
    result->f[9] = -(xx * (zy * ww - wy * zw) - xy * (zx * ww - wx * zw) + xw * (zx * wy - wx * zy)) * inv_det;
    result->f[10] = +(xx * (yy * ww - wy * yw) - xy * (yx * ww - wx * yw) + xw * (yx * wy - wx * yy)) * inv_det;
    result->f[11] = -(xx * (yy * zw - zy * yw) - xy * (yx * zw - zx * yw) + xw * (yx * zy - zx * yy)) * inv_det;

    result->f[12] = -(yx * (zy * wz - wy * zz) - yy * (zx * wz - wx * zz) + yz * (zx * wy - wx * zy)) * inv_det;
    result->f[13] = +(xx * (zy * wz - wy * zz) - xy * (zx * wz - wx * zz) + xz * (zx * wy - wx * zy)) * inv_det;
    result->f[14] = -(xx * (yy * wz - wy * yz) - xy * (yx * wz - wx * yz) + xz * (yx * wy - wx * yy)) * inv_det;
    result->f[15] = +(xx * (yy * zz - zy * yz) - xy * (yx * zz - zx * yz) + xz * (yx * zy - zx * yy)) * inv_det;
}

CE_FORCE_INLINE void mat44f_transpose(mat44f_t *__restrict result,
                                      const mat44f_t *__restrict a) {
    result->f[0] = a->f[0];
    result->f[4] = a->f[1];
    result->f[8] = a->f[2];
    result->f[12] = a->f[3];

    result->f[1] = a->f[4];
    result->f[5] = a->f[5];
    result->f[9] = a->f[6];
    result->f[13] = a->f[7];

    result->f[2] = a->f[8];
    result->f[6] = a->f[9];
    result->f[10] = a->f[10];
    result->f[14] = a->f[11];

    result->f[3] = a->f[12];
    result->f[7] = a->f[13];
    result->f[11] = a->f[14];
    result->f[15] = a->f[15];
}

CE_FORCE_INLINE void mat44f_set_perspective_fov(mat44f_t *__restrict result,
                                                const f32 fov,
                                                const f32 aspect_ratio,
                                                f32 near,
                                                f32 far) {

    f32 yScale = 1.0f / f32_tan(fov * f32_ToRad * 0.5f);
    f32 xScale = yScale / aspect_ratio;

    result->x.x = xScale;
    result->x.y = result->x.z = result->x.w = 0.0f;

    result->y.y = yScale;
    result->y.x = result->y.z = result->y.w = 0.0f;

    result->z.x = result->z.y = 0.0f;
    result->z.z = far / (near - far);
    result->z.w = -1.0f;

    result->w.x = result->w.y = result->w.w = 0.0f;
    result->w.z = near * far / (near - far);
}

#endif //CETECH_MAT44F_H
