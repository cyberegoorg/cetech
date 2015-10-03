#pragma once

#define SIMD_VECTOR(x, y, z, w) {{x, y, z, w}}

namespace cetech {
    struct SimdVector {
        float v[4];
    };
}