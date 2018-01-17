/*
 * Copyright 2011-2017 Branimir Karadzic. All rights reserved.
 * License: https://github.com/bkaradzic/bx#license-bsd-2-clause
 */


#ifndef CELIB_FPUMATH_INL
#define CELIB_FPUMATH_INL

//==============================================================================
// Includes
//==============================================================================

#include <math.h>
#include <celib/fmath.h>

namespace celib {




    template<Handness::Enum HandnessT>
    inline void mat4_proj_xywh(float *_result,
                               float _x,
                               float _y,
                               float _width,
                               float _height,
                               float _near,
                               float _far,
                               bool _oglNdc) {
        const float diff = _far - _near;
        const float aa = _oglNdc ? (_far + _near) / diff : _far / diff;
        const float bb = _oglNdc ? (2.0f * _far * _near) / diff : _near * aa;

        mem_set(_result, 0, sizeof(float) * 16);
        _result[0] = _width;
        _result[5] = _height;
        _result[8] = (Handness::Right == HandnessT) ? _x : -_x;
        _result[9] = (Handness::Right == HandnessT) ? _y : -_y;
        _result[10] = (Handness::Right == HandnessT) ? -aa : aa;
        _result[11] = (Handness::Right == HandnessT) ? -1.0f : 1.0f;
        _result[14] = -bb;
    }

    template<Handness::Enum HandnessT>
    inline void mtxProjImpl(float *_result,
                            float _ut,
                            float _dt,
                            float _lt,
                            float _rt,
                            float _near,
                            float _far,
                            bool _oglNdc) {
        const float invDiffRl = 1.0f / (_rt - _lt);
        const float invDiffUd = 1.0f / (_ut - _dt);
        const float width = 2.0f * _near * invDiffRl;
        const float height = 2.0f * _near * invDiffUd;
        const float xx = (_rt + _lt) * invDiffRl;
        const float yy = (_ut + _dt) * invDiffUd;
        mat4_proj_xywh<HandnessT>(_result, xx, yy, width, height, _near, _far,
                                  _oglNdc);
    }

    template<Handness::Enum HandnessT>
    inline void mtxProjImpl(float *_result,
                            const float _fov[4],
                            float _near,
                            float _far,
                            bool _oglNdc) {
        mtxProjImpl<HandnessT>(_result, _fov[0], _fov[1], _fov[2], _fov[3],
                               _near, _far, _oglNdc);
    }

    template<Handness::Enum HandnessT>
    inline void mtxProjImpl(float *_result,
                            float _fovy,
                            float _aspect,
                            float _near,
                            float _far,
                            bool _oglNdc) {
        const float height = 1.0f / cel_ftan(cel_to_rad(_fovy) * 0.5f);
        const float width = height * 1.0f / _aspect;
        mat4_proj_xywh<HandnessT>(_result, 0.0f, 0.0f, width, height, _near,
                                  _far,
                                  _oglNdc);
    }

    inline void mat4_proj(float *_result,
                          float _ut,
                          float _dt,
                          float _lt,
                          float _rt,
                          float _near,
                          float _far,
                          bool _oglNdc) {
        mtxProjImpl<Handness::Left>(_result, _ut, _dt, _lt, _rt, _near, _far,
                                    _oglNdc);
    }

    inline void mat4_proj(float *_result,
                          const float *_fov,
                          float _near,
                          float _far,
                          bool _oglNdc) {
        mtxProjImpl<Handness::Left>(_result, _fov, _near, _far, _oglNdc);
    }

    inline void mat4_proj(float *_result,
                          float _fovy,
                          float _aspect,
                          float _near,
                          float _far,
                          bool _oglNdc) {
        mtxProjImpl<Handness::Left>(_result, _fovy, _aspect, _near, _far,
                                    _oglNdc);
    }

    inline void mat4_proj_lh(float *_result,
                             float _ut,
                             float _dt,
                             float _lt,
                             float _rt,
                             float _near,
                             float _far,
                             bool _oglNdc) {
        mtxProjImpl<Handness::Left>(_result, _ut, _dt, _lt, _rt, _near, _far,
                                    _oglNdc);
    }

    inline void mat4_proj_lh(float *_result,
                             const float *_fov,
                             float _near,
                             float _far,
                             bool _oglNdc) {
        mtxProjImpl<Handness::Left>(_result, _fov, _near, _far, _oglNdc);
    }

    inline void mat4_proj_lh(float *_result,
                             float _fovy,
                             float _aspect,
                             float _near,
                             float _far,
                             bool _oglNdc) {
        mtxProjImpl<Handness::Left>(_result, _fovy, _aspect, _near, _far,
                                    _oglNdc);
    }

    inline void mat4_proj_rh(float *_result,
                             float _ut,
                             float _dt,
                             float _lt,
                             float _rt,
                             float _near,
                             float _far,
                             bool _oglNdc) {
        mtxProjImpl<Handness::Right>(_result, _ut, _dt, _lt, _rt, _near, _far,
                                     _oglNdc);
    }

    inline void mat4_proj_rh(float *_result,
                             const float *_fov,
                             float _near,
                             float _far,
                             bool _oglNdc) {
        mtxProjImpl<Handness::Right>(_result, _fov, _near, _far, _oglNdc);
    }

    inline void mat4_proj_rh(float *_result,
                             float _fovy,
                             float _aspect,
                             float _near,
                             float _far,
                             bool _oglNdc) {
        mtxProjImpl<Handness::Right>(_result, _fovy, _aspect, _near, _far,
                                     _oglNdc);
    }

    template<NearFar::Enum NearFarT, Handness::Enum HandnessT>
    inline void mat4_proj_inf_xywh(float *_result,
                                   float _x,
                                   float _y,
                                   float _width,
                                   float _height,
                                   float _near,
                                   bool _oglNdc) {
        float aa;
        float bb;
        if (CEL_ENABLED(NearFar::Reverse == NearFarT)) {
            aa = _oglNdc ? -1.0f : 0.0f;
            bb = _oglNdc ? -2.0f * _near : -_near;
        } else {
            aa = 1.0f;
            bb = _oglNdc ? 2.0f * _near : _near;
        }

        mem_set(_result, 0, sizeof(float) * 16);
        _result[0] = _width;
        _result[5] = _height;
        _result[8] = (Handness::Right == HandnessT) ? _x : -_x;
        _result[9] = (Handness::Right == HandnessT) ? _y : -_y;
        _result[10] = (Handness::Right == HandnessT) ? -aa : aa;
        _result[11] = (Handness::Right == HandnessT) ? -1.0f : 1.0f;
        _result[14] = -bb;
    }

    template<NearFar::Enum NearFarT, Handness::Enum HandnessT>
    inline void mtxProjInfImpl(float *_result,
                               float _ut,
                               float _dt,
                               float _lt,
                               float _rt,
                               float _near,
                               bool _oglNdc) {
        const float invDiffRl = 1.0f / (_rt - _lt);
        const float invDiffUd = 1.0f / (_ut - _dt);
        const float width = 2.0f * _near * invDiffRl;
        const float height = 2.0f * _near * invDiffUd;
        const float xx = (_rt + _lt) * invDiffRl;
        const float yy = (_ut + _dt) * invDiffUd;
        mat4_proj_inf_xywh<NearFarT, HandnessT>(_result, xx, yy, width, height,
                                                _near, _oglNdc);
    }

    template<NearFar::Enum NearFarT, Handness::Enum HandnessT>
    inline void mtxProjInfImpl(float *_result,
                               const float _fov[4],
                               float _near,
                               bool _oglNdc) {
        mtxProjInfImpl<NearFarT, HandnessT>(_result, _fov[0], _fov[1], _fov[2],
                                            _fov[3], _near, _oglNdc);
    }

    template<NearFar::Enum NearFarT, Handness::Enum HandnessT>
    inline void mtxProjInfImpl(float *_result,
                               float _fovy,
                               float _aspect,
                               float _near,
                               bool _oglNdc) {
        const float height = 1.0f / cel_ftan(cel_to_rad(_fovy) * 0.5f);
        const float width = height * 1.0f / _aspect;
        mat4_proj_inf_xywh<NearFarT, HandnessT>(_result, 0.0f, 0.0f, width,
                                                height,
                                                _near, _oglNdc);
    }

    inline void mat4_proj_inf(float *_result,
                              const float *_fov,
                              float _near,
                              bool _oglNdc) {
        mtxProjInfImpl<NearFar::Default, Handness::Left>(_result, _fov, _near,
                                                         _oglNdc);
    }

    inline void mat4_proj_inf(float *_result,
                              float _ut,
                              float _dt,
                              float _lt,
                              float _rt,
                              float _near,
                              bool _oglNdc) {
        mtxProjInfImpl<NearFar::Default, Handness::Left>(_result, _ut, _dt, _lt,
                                                         _rt, _near, _oglNdc);
    }

    inline void mat4_proj_inf(float *_result,
                              float _fovy,
                              float _aspect,
                              float _near,
                              bool _oglNdc) {
        mtxProjInfImpl<NearFar::Default, Handness::Left>(_result, _fovy,
                                                         _aspect, _near,
                                                         _oglNdc);
    }

    inline void mat4_proj_inf_lh(float *_result,
                                 float _ut,
                                 float _dt,
                                 float _lt,
                                 float _rt,
                                 float _near,
                                 bool _oglNdc) {
        mtxProjInfImpl<NearFar::Default, Handness::Left>(_result, _ut, _dt, _lt,
                                                         _rt, _near, _oglNdc);
    }

    inline void mat4_proj_inf_lh(float *_result,
                                 const float *_fov,
                                 float _near,
                                 bool _oglNdc) {
        mtxProjInfImpl<NearFar::Default, Handness::Left>(_result, _fov, _near,
                                                         _oglNdc);
    }

    inline void mat4_proj_inf_lh(float *_result,
                                 float _fovy,
                                 float _aspect,
                                 float _near,
                                 bool _oglNdc) {
        mtxProjInfImpl<NearFar::Default, Handness::Left>(_result, _fovy,
                                                         _aspect, _near,
                                                         _oglNdc);
    }

    inline void mat4_proj_inf_rh(float *_result,
                                 float _ut,
                                 float _dt,
                                 float _lt,
                                 float _rt,
                                 float _near,
                                 bool _oglNdc) {
        mtxProjInfImpl<NearFar::Default, Handness::Right>(_result, _ut, _dt,
                                                          _lt, _rt, _near,
                                                          _oglNdc);
    }

    inline void mat4_proj_inf_rh(float *_result,
                                 const float *_fov,
                                 float _near,
                                 bool _oglNdc) {
        mtxProjInfImpl<NearFar::Default, Handness::Right>(_result, _fov, _near,
                                                          _oglNdc);
    }

    inline void mat4_proj_inf_rh(float *_result,
                                 float _fovy,
                                 float _aspect,
                                 float _near,
                                 bool _oglNdc) {
        mtxProjInfImpl<NearFar::Default, Handness::Right>(_result, _fovy,
                                                          _aspect, _near,
                                                          _oglNdc);
    }

    inline void mat4_proj_rev_inf_lh(float *_result,
                                     float _ut,
                                     float _dt,
                                     float _lt,
                                     float _rt,
                                     float _near,
                                     bool _oglNdc) {
        mtxProjInfImpl<NearFar::Reverse, Handness::Left>(_result, _ut, _dt, _lt,
                                                         _rt, _near, _oglNdc);
    }

    inline void mat4_proj_rev_inf_lh(float *_result,
                                     const float *_fov,
                                     float _near,
                                     bool _oglNdc) {
        mtxProjInfImpl<NearFar::Reverse, Handness::Left>(_result, _fov, _near,
                                                         _oglNdc);
    }

    inline void mat4_proj_rev_inf_lh(float *_result,
                                     float _fovy,
                                     float _aspect,
                                     float _near,
                                     bool _oglNdc) {
        mtxProjInfImpl<NearFar::Reverse, Handness::Left>(_result, _fovy,
                                                         _aspect, _near,
                                                         _oglNdc);
    }

    inline void mat4_proj_rev_inf_rh(float *_result,
                                     float _ut,
                                     float _dt,
                                     float _lt,
                                     float _rt,
                                     float _near,
                                     bool _oglNdc) {
        mtxProjInfImpl<NearFar::Reverse, Handness::Right>(_result, _ut, _dt,
                                                          _lt, _rt, _near,
                                                          _oglNdc);
    }

    inline void mat4_proj_rev_inf_rh(float *_result,
                                     const float *_fov,
                                     float _near,
                                     bool _oglNdc) {
        mtxProjInfImpl<NearFar::Reverse, Handness::Right>(_result, _fov, _near,
                                                          _oglNdc);
    }

    inline void mat4_proj_rev_inf_rh(float *_result,
                                     float _fovy,
                                     float _aspect,
                                     float _near,
                                     bool _oglNdc) {
        mtxProjInfImpl<NearFar::Reverse, Handness::Right>(_result, _fovy,
                                                          _aspect, _near,
                                                          _oglNdc);
    }





}

#endif //CELIB_FPUMATH_INL
