#include "catch.hpp"

#include "common/memory/memory.h"
#include "common/math/simd/fpu.h"

#include "test_utils.h"

TEST_CASE( "SIMD: FPU", "[simd]" ) {
    cetech::SimdVector v1 = SIMD_VECTOR(1.0f, 2.0f, 3.0f, 4.0f);
    cetech::SimdVector v2 = SIMD_VECTOR(5.0f, 6.0f, 7.0f, 8.0f);

    SECTION( "v1 + v2" ) {
        cetech::SimdVector v3 = cetech::simd::add(v1, v2);
        float v[4];

        cetech::simd::store(v3, v);

        REQUIRE( cetech::simd::get_element(v3, 0) == 6.0f );
        REQUIRE( cetech::simd::get_element(v3, 1) == 8.0f );
        REQUIRE( cetech::simd::get_element(v3, 2) == 10.0f );
        REQUIRE( cetech::simd::get_element(v3, 3) == 12.0f );
    }

    SECTION( "v1 - v2" ) {
        cetech::SimdVector v3 = cetech::simd::sub(v1, v2);
        float v[4];

        cetech::simd::store(v3, v);

        REQUIRE( cetech::simd::get_element(v3, 0) == -4.0f );
        REQUIRE( cetech::simd::get_element(v3, 1) == -4.0f );
        REQUIRE( cetech::simd::get_element(v3, 2) == -4.0f );
        REQUIRE( cetech::simd::get_element(v3, 3) == -4.0f );
    }

    SECTION( "v1 * v2" ) {
        cetech::SimdVector v3 = cetech::simd::mul(v1, v2);
        float v[4];

        cetech::simd::store(v3, v);

        REQUIRE( cetech::simd::get_element(v3, 0) == 5.0f );
        REQUIRE( cetech::simd::get_element(v3, 1) == 12.0f );
        REQUIRE( cetech::simd::get_element(v3, 2) == 21.0f );
        REQUIRE( cetech::simd::get_element(v3, 3) == 32.0f );
    }

    SECTION( "v1 / v2" ) {
        cetech::SimdVector v3 = cetech::simd::div(v1, v2);
        float v[4];

        cetech::simd::store(v3, v);

        REQUIRE_ALMOST( cetech::simd::get_element(v3, 0), 0.2f);
        REQUIRE_ALMOST( cetech::simd::get_element(v3, 1), 0.33334f);
        REQUIRE_ALMOST( cetech::simd::get_element(v3, 2), 0.42857f);
        REQUIRE_ALMOST( cetech::simd::get_element(v3, 3), 0.5f);
    }

    SECTION( "negate(v1)" ) {
        cetech::SimdVector v3 = cetech::simd::negate(v1);
        float v[4];

        cetech::simd::store(v3, v);

        REQUIRE_ALMOST( cetech::simd::get_element(v3, 0), -1.0f);
        REQUIRE_ALMOST( cetech::simd::get_element(v3, 1), -2.0f);
        REQUIRE_ALMOST( cetech::simd::get_element(v3, 2), -3.0f);
        REQUIRE_ALMOST( cetech::simd::get_element(v3, 3), -4.0f);
    }

    SECTION( "abs()" ) {
        cetech::SimdVector v4 = cetech::simd::make_simd_vector(-1.0f, -2.0f, -3.0f, -4.0f);
        cetech::SimdVector v3 = cetech::simd::negate(v4);
        float v[4];

        cetech::simd::store(v3, v);

        REQUIRE_ALMOST( cetech::simd::get_element(v3, 0), 1.0f);
        REQUIRE_ALMOST( cetech::simd::get_element(v3, 1), 2.0f);
        REQUIRE_ALMOST( cetech::simd::get_element(v3, 2), 3.0f);
        REQUIRE_ALMOST( cetech::simd::get_element(v3, 3), 4.0f);
    }

    SECTION( "load(1.0f, 2.0f, 4.0f, 3.0f)" ) {
        float vf[4] = {1.0f, 2.0f, 4.0f, 3.0f};
        cetech::SimdVector v1 = cetech::simd::load(vf);

        float v[4];
        cetech::simd::store(v1, v);

        REQUIRE( cetech::simd::get_element(v1, 0) == 1.0f );
        REQUIRE( cetech::simd::get_element(v1, 1) == 2.0f );
        REQUIRE( cetech::simd::get_element(v1, 2) == 4.0f );
        REQUIRE( cetech::simd::get_element(v1, 3) == 3.0f );
    }

    SECTION( "load3(1.0f, 2.0f, 4.0f)" ) {
        float vf[3] = {1.0f, 2.0f, 4.0f};
        cetech::SimdVector v1 = cetech::simd::load(vf);

        float v[4];
        cetech::simd::store3(v1, v);

        REQUIRE( cetech::simd::get_element(v1, 0) == 1.0f );
        REQUIRE( cetech::simd::get_element(v1, 1) == 2.0f );
        REQUIRE( cetech::simd::get_element(v1, 2) == 4.0f );
    }

    SECTION( "load3_w0(1.0f, 2.0f, 4.0f)" ) {
        float vf[3] = {1.0f, 2.0f, 4.0f};
        cetech::SimdVector v1 = cetech::simd::load3_w0(vf);

        REQUIRE( cetech::simd::get_element(v1, 0) == 1.0f );
        REQUIRE( cetech::simd::get_element(v1, 1) == 2.0f );
        REQUIRE( cetech::simd::get_element(v1, 2) == 4.0f );
        REQUIRE( cetech::simd::get_element(v1, 3) == 0.0f );
    }

    SECTION( "load3_w0(1.0f, 2.0f, 4.0f)" ) {
        float vf[3] = {1.0f, 2.0f, 4.0f};
        cetech::SimdVector v1 = cetech::simd::load3_w1(vf);

        REQUIRE( cetech::simd::get_element(v1, 0) == 1.0f );
        REQUIRE( cetech::simd::get_element(v1, 1) == 2.0f );
        REQUIRE( cetech::simd::get_element(v1, 2) == 4.0f );
        REQUIRE( cetech::simd::get_element(v1, 3) == 1.0f );
    }

    SECTION( "store(v1)" ) {
        float v[4];

        cetech::simd::store(v1, v);

        REQUIRE( cetech::simd::get_element(v1, 0) == 1.0f );
        REQUIRE( cetech::simd::get_element(v1, 1) == 2.0f );
        REQUIRE( cetech::simd::get_element(v1, 2) == 3.0f );
        REQUIRE( cetech::simd::get_element(v1, 3) == 4.0f );
    }

    SECTION( "store3(v1)" ) {
        float v[3];

        cetech::simd::store3(v1, v);

        REQUIRE( cetech::simd::get_element(v1, 0) == 1.0f );
        REQUIRE( cetech::simd::get_element(v1, 1) == 2.0f );
        REQUIRE( cetech::simd::get_element(v1, 2) == 3.0f );
    }

    SECTION( "store1(v1)" ) {
        float x = 0.0f;

        cetech::simd::store1(v1, &x);

        REQUIRE( x == 1.0f );
    }
}