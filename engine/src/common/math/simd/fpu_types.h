#pragma once

#define SIMD_VECTOR(x, y, z, w) {{x, y, z, w}}

namespace cetech1 {
    struct SimdVector {
        float v[4];
    };
}