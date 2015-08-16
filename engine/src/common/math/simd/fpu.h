#pragma once

#include "common/math/math.h"
#include "common/memory/memory.h"

#include "fpu_types.h"

namespace cetech1 {
  namespace simd {
    FORCE_INLINE SimdVector make_simd_vector(float x, float y, float z, float w) {
      SimdVector ret = {{x, y, z, w}};
      return ret;
    }
    
    FORCE_INLINE float get_element(const SimdVector& v1, uint32_t idx) {
      return v1.v[idx];
    }
    
    FORCE_INLINE SimdVector add(const SimdVector& v1, const SimdVector& v2) {
      SimdVector ret;
      
      ret.v[0] = v1.v[0] + v2.v[0];
      ret.v[1] = v1.v[1] + v2.v[1];
      ret.v[2] = v1.v[2] + v2.v[2];
      ret.v[3] = v1.v[3] + v2.v[3];
	
      return ret;
    }
    
    FORCE_INLINE SimdVector sub(const SimdVector& v1, const SimdVector& v2) {
      SimdVector ret;
      
      ret.v[0] = v1.v[0] - v2.v[0];
      ret.v[1] = v1.v[1] - v2.v[1];
      ret.v[2] = v1.v[2] - v2.v[2];
      ret.v[3] = v1.v[3] - v2.v[3];
	
      return ret;
    }
    
    FORCE_INLINE SimdVector mul(const SimdVector& v1, const SimdVector& v2) {
      SimdVector ret;
      
      ret.v[0] = v1.v[0] * v2.v[0];
      ret.v[1] = v1.v[1] * v2.v[1];
      ret.v[2] = v1.v[2] * v2.v[2];
      ret.v[3] = v1.v[3] * v2.v[3];
	
      return ret;
    }

    FORCE_INLINE SimdVector div(const SimdVector& v1, const SimdVector& v2) {
      SimdVector ret;
      
      ret.v[0] = v1.v[0] / v2.v[0];
      ret.v[1] = v1.v[1] / v2.v[1];
      ret.v[2] = v1.v[2] / v2.v[2];
      ret.v[3] = v1.v[3] / v2.v[3];
	
      return ret;
    }
    
    FORCE_INLINE SimdVector negate(const SimdVector& v1) {
      return make_simd_vector(
	-v1.v[0],
	-v1.v[1],
	-v1.v[2],
	-v1.v[3]
      );
    }
    
    FORCE_INLINE SimdVector abs(const SimdVector& v1) {
      return make_simd_vector(
	math::abs(v1.v[0]),
	math::abs(v1.v[1]),
	math::abs(v1.v[2]),
	math::abs(v1.v[3])
      );
    }
    
    FORCE_INLINE SimdVector load(const void *src) {
      return make_simd_vector(
	((const float*) src)[0],
	((const float*) src)[1],
	((const float*) src)[2],
	((const float*) src)[3]
      );
    }
    
    FORCE_INLINE SimdVector load3(const void *src) {
      return make_simd_vector(
	((const float*) src)[0],
	((const float*) src)[1],
	((const float*) src)[2],
	0.0f
      );
    }
    
    FORCE_INLINE SimdVector load3_w0(const void *src) {
      return make_simd_vector(
	((const float*) src)[0],
	((const float*) src)[1],
	((const float*) src)[2],
	0.0f
      );
    }
    
    FORCE_INLINE SimdVector load3_w1(const void *src) {
      return make_simd_vector(
	((const float*) src)[0],
	((const float*) src)[1],
	((const float*) src)[2],
	1.0f
      );
    }
    
    FORCE_INLINE void store(const SimdVector& v1, void *dst) {
      memory::memcpy(dst, v1.v, 16);
    }
    
    FORCE_INLINE void store3(const SimdVector& v1, void *dst) {
      memory::memcpy(dst, v1.v, 12);
    }
    
    FORCE_INLINE void store1(const SimdVector& v1, void *dst) {
      memory::memcpy(dst, v1.v, 4);
    }
    
    FORCE_INLINE void quat_mult(const void* q1, const void* q2, void* dst) {
	const float* q1f = (const float*)q1;
	const float* q2f = (const float*)q2;
	float* dstf = (float*)dst;
    
	dstf[0] = q1f[0]*q2f[0] - q1f[1]*q2f[1] - q1f[2]*q2f[2] - q1f[3]*q2f[3];
	dstf[1] = q1f[0]*q2f[1] + q1f[1]*q2f[0] + q1f[2]*q2f[3] - q1f[3]*q2f[2];
	dstf[2] = q1f[0]*q2f[2] - q1f[1]*q2f[3] + q1f[2]*q2f[0] + q1f[3]*q2f[1];
	dstf[3] = q1f[0]*q2f[3] + q1f[1]*q2f[2] - q1f[2]*q2f[1] + q1f[3]*q2f[0];
    }
  }
};