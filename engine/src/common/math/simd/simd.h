#pragma once

#include "common/math/simd/simd_types.h"

#ifdef CETECH1_SIMD_FPU
  #include "fpu.h"
#endif

namespace cetech1 {
  namespace simd {
    static const SimdVector ZERO = make_simd_vector(0.0f, 0.0f, 0.0f, 0.0f);
    static const SimdVector ONE = make_simd_vector(1.0f, 1.0f, 1.0f, 1.0f);
    static const SimdVector MINUS_ONE = make_simd_vector(-1.0f, -1.0f, -1.0f, -1.0f);
  }
  
  namespace simd {
    FORCE_INLINE SimdVector make_simd_vector(float x, float y, float z, float w);
    
    FORCE_INLINE float get_element(const SimdVector& v1, uint32_t idx);
    
    FORCE_INLINE SimdVector add(const SimdVector& v1, const SimdVector& v2);
    FORCE_INLINE SimdVector sub(const SimdVector& v1, const SimdVector& v2);
    FORCE_INLINE SimdVector mul(const SimdVector& v1, const SimdVector& v2);
    FORCE_INLINE SimdVector div(const SimdVector& v1, const SimdVector& v2);
    
    FORCE_INLINE SimdVector negate(const SimdVector& v1);
    FORCE_INLINE SimdVector abs(const SimdVector& v1);
    
    FORCE_INLINE SimdVector load(const void *src);
    FORCE_INLINE SimdVector load3(const void *src);
    FORCE_INLINE SimdVector load3_w0(const void *src);
    FORCE_INLINE SimdVector load3_w1(const void *src);
    FORCE_INLINE SimdVector load3_w1(const void *src);
    
    FORCE_INLINE void store(const SimdVector& v1, void *dst);
    FORCE_INLINE void store3(const SimdVector& v1, void *dst);
    FORCE_INLINE void store1(const SimdVector& v1, void *dst);
  }
}
