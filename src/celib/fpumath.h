/*
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */

#ifndef CELIB_FPUMATH_H
#define CELIB_FPUMATH_H

//==============================================================================
// Includes
//==============================================================================
#include <math.h>

#include "macros.h"
#include "memory.h"

namespace celib {
    ///
    struct Handness {
        enum Enum {
            Left,
            Right,
        };
    };

    ///
    struct NearFar {
        enum Enum {
            Default,
            Reverse,
        };
    };




    template<Handness::Enum HandnessT>
    inline void mat4_proj_xywh(float *result,
                               float x,
                               float y,
                               float width,
                               float height,
                               float near,
                               float far,
                               bool oglNdc);


    inline void mat4_proj(float *result,
                          float ut,
                          float dt,
                          float lt,
                          float rt,
                          float near,
                          float far,
                          bool oglNdc);


    inline void mat4_proj(float *result,
                          const float *fov,
                          float near,
                          float far,
                          bool oglNdc);


    inline void mat4_proj(float *result,
                          float fovy,
                          float aspect,
                          float near,
                          float far,
                          bool oglNdc);


    inline void mat4_proj_lh(float *result,
                             float ut,
                             float dt,
                             float lt,
                             float rt,
                             float near,
                             float far,
                             bool oglNdc);


    inline void mat4_proj_lh(float *result,
                             const float *fov,
                             float near,
                             float far,
                             bool oglNdc);

    inline void mat4_proj_lh(float *result,
                             float fovy,
                             float aspect,
                             float near,
                             float far,
                             bool oglNdc);


    inline void mat4_proj_rh(float *result,
                             float ut,
                             float dt,
                             float lt,
                             float rt,
                             float near,
                             float far,
                             bool oglNdc);

    inline void mat4_proj_rh(float *result,
                             const float *fov,
                             float near,
                             float far,
                             bool oglNdc);

    inline void mat4_proj_rh(float *result,
                             float fovy,
                             float aspect,
                             float near,
                             float far,
                             bool oglNdc);

    template<NearFar::Enum NearFarT, Handness::Enum HandnessT>
    inline void mat4_proj_inf_xywh(float *result,
                                   float x,
                                   float y,
                                   float width,
                                   float height,
                                   float near,
                                   bool oglNdc);


    inline void mat4_proj_inf(float *result,
                              const float *fov,
                              float near,
                              bool oglNdc);


    inline void mat4_proj_inf(float *result,
                              float ut,
                              float dt,
                              float lt,
                              float rt,
                              float near,
                              bool oglNdc);


    inline void mat4_proj_inf(float *result,
                              float fovy,
                              float aspect,
                              float near,
                              bool oglNdc);


    inline void mat4_proj_inf_lh(float *result,
                                 float ut,
                                 float dt,
                                 float lt,
                                 float rt,
                                 float near,
                                 bool oglNdc);


    inline void mat4_proj_inf_lh(float *result,
                                 const float *fov,
                                 float near,
                                 bool oglNdc);


    inline void mat4_proj_inf_lh(float *result,
                                 float fovy,
                                 float aspect,
                                 float near,
                                 bool oglNdc);


    inline void mat4_proj_inf_rh(float *result,
                                 float ut,
                                 float dt,
                                 float lt,
                                 float rt,
                                 float near,
                                 bool oglNdc);


    inline void mat4_proj_inf_rh(float *result,
                                 const float *fov,
                                 float near,
                                 bool oglNdc);


    inline void mat4_proj_inf_rh(float *result,
                                 float fovy,
                                 float aspect,
                                 float near,
                                 bool oglNdc);


    inline void mat4_proj_rev_inf_lh(float *result,
                                     float ut,
                                     float dt,
                                     float lt,
                                     float rt,
                                     float near,
                                     bool oglNdc);


    inline void mat4_proj_rev_inf_lh(float *result,
                                     const float *fov,
                                     float near,
                                     bool oglNdc);


    inline void mat4_proj_rev_inf_lh(float *result,
                                     float fovy,
                                     float aspect,
                                     float near,
                                     bool oglNdc);


    inline void mat4_proj_rev_inf_rh(float *result,
                                     float ut,
                                     float dt,
                                     float lt,
                                     float rt,
                                     float near,
                                     bool oglNdc);

    inline void mat4_proj_rev_inf_rh(float *result,
                                     const float *fov,
                                     float near,
                                     bool oglNdc);

    inline void mat4_proj_rev_inf_rh(float *result,
                                     float fovy,
                                     float aspect,
                                     float near,
                                     bool oglNdc);


}

#include "private/fpumath.inl"

#endif //CELIB_FPUMATH_H
