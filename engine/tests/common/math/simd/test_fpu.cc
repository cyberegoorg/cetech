#include "catch.hpp"

#include "common/memory/memory.h"
#include "common/math/simd/fpu.h"

#include "test_utils.h"

TEST_CASE( "SIMD: FPU", "[simd]" ) {
    cetech1::SimdVector v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    cetech1::SimdVector v2 = {5.0f, 6.0f, 7.0f, 8.0f};

    SECTION( "v1 + v2" ) {
        cetech1::SimdVector v3 = cetech1::simd::add(v1, v2);
        float v[4];

        cetech1::simd::store(v3, v);

        REQUIRE( cetech1::simd::get_element(v3, 0) == 6.0f );
        REQUIRE( cetech1::simd::get_element(v3, 1) == 8.0f );
        REQUIRE( cetech1::simd::get_element(v3, 2) == 10.0f );
        REQUIRE( cetech1::simd::get_element(v3, 3) == 12.0f );
    }

    SECTION( "v1 - v2" ) {
        cetech1::SimdVector v3 = cetech1::simd::sub(v1, v2);
        float v[4];

        cetech1::simd::store(v3, v);

        REQUIRE( cetech1::simd::get_element(v3, 0) == -4.0f );
        REQUIRE( cetech1::simd::get_element(v3, 1) == -4.0f );
        REQUIRE( cetech1::simd::get_element(v3, 2) == -4.0f );
        REQUIRE( cetech1::simd::get_element(v3, 3) == -4.0f );
    }

    SECTION( "v1 * v2" ) {
        cetech1::SimdVector v3 = cetech1::simd::mul(v1, v2);
        float v[4];

        cetech1::simd::store(v3, v);

        REQUIRE( cetech1::simd::get_element(v3, 0) == 5.0f );
        REQUIRE( cetech1::simd::get_element(v3, 1) == 12.0f );
        REQUIRE( cetech1::simd::get_element(v3, 2) == 21.0f );
        REQUIRE( cetech1::simd::get_element(v3, 3) == 32.0f );
    }

    SECTION( "v1 / v2" ) {
        cetech1::SimdVector v3 = cetech1::simd::div(v1, v2);
        float v[4];

        cetech1::simd::store(v3, v);

        REQUIRE_ALMOST( cetech1::simd::get_element(v3, 0), 0.2f);
        REQUIRE_ALMOST( cetech1::simd::get_element(v3, 1), 0.33334f);
        REQUIRE_ALMOST( cetech1::simd::get_element(v3, 2), 0.42857f);
        REQUIRE_ALMOST( cetech1::simd::get_element(v3, 3), 0.5f);
    }

    SECTION( "negate(v1)" ) {
        cetech1::SimdVector v3 = cetech1::simd::negate(v1);
        float v[4];

        cetech1::simd::store(v3, v);

        REQUIRE_ALMOST( cetech1::simd::get_element(v3, 0), -1.0f);
        REQUIRE_ALMOST( cetech1::simd::get_element(v3, 1), -2.0f);
        REQUIRE_ALMOST( cetech1::simd::get_element(v3, 2), -3.0f);
        REQUIRE_ALMOST( cetech1::simd::get_element(v3, 3), -4.0f);
    }

    SECTION( "abs()" ) {
        cetech1::SimdVector v4 = cetech1::simd::make_simd_vector(-1.0f, -2.0f, -3.0f, -4.0f);
        cetech1::SimdVector v3 = cetech1::simd::negate(v4);
        float v[4];

        cetech1::simd::store(v3, v);

        REQUIRE_ALMOST( cetech1::simd::get_element(v3, 0), 1.0f);
        REQUIRE_ALMOST( cetech1::simd::get_element(v3, 1), 2.0f);
        REQUIRE_ALMOST( cetech1::simd::get_element(v3, 2), 3.0f);
        REQUIRE_ALMOST( cetech1::simd::get_element(v3, 3), 4.0f);
    }

    SECTION( "load(1.0f, 2.0f, 4.0f, 3.0f)" ) {
        float vf[4] = {1.0f, 2.0f, 4.0f, 3.0f};
        cetech1::SimdVector v1 = cetech1::simd::load(vf);

        float v[4];
        cetech1::simd::store(v1, v);

        REQUIRE( cetech1::simd::get_element(v1, 0) == 1.0f );
        REQUIRE( cetech1::simd::get_element(v1, 1) == 2.0f );
        REQUIRE( cetech1::simd::get_element(v1, 2) == 4.0f );
        REQUIRE( cetech1::simd::get_element(v1, 3) == 3.0f );
    }

    SECTION( "load3(1.0f, 2.0f, 4.0f)" ) {
        float vf[3] = {1.0f, 2.0f, 4.0f};
        cetech1::SimdVector v1 = cetech1::simd::load(vf);

        float v[4];
        cetech1::simd::store3(v1, v);

        REQUIRE( cetech1::simd::get_element(v1, 0) == 1.0f );
        REQUIRE( cetech1::simd::get_element(v1, 1) == 2.0f );
        REQUIRE( cetech1::simd::get_element(v1, 2) == 4.0f );
    }

    SECTION( "load3_w0(1.0f, 2.0f, 4.0f)" ) {
        float vf[3] = {1.0f, 2.0f, 4.0f};
        cetech1::SimdVector v1 = cetech1::simd::load3_w0(vf);

        REQUIRE( cetech1::simd::get_element(v1, 0) == 1.0f );
        REQUIRE( cetech1::simd::get_element(v1, 1) == 2.0f );
        REQUIRE( cetech1::simd::get_element(v1, 2) == 4.0f );
        REQUIRE( cetech1::simd::get_element(v1, 3) == 0.0f );
    }

    SECTION( "load3_w0(1.0f, 2.0f, 4.0f)" ) {
        float vf[3] = {1.0f, 2.0f, 4.0f};
        cetech1::SimdVector v1 = cetech1::simd::load3_w1(vf);

        REQUIRE( cetech1::simd::get_element(v1, 0) == 1.0f );
        REQUIRE( cetech1::simd::get_element(v1, 1) == 2.0f );
        REQUIRE( cetech1::simd::get_element(v1, 2) == 4.0f );
        REQUIRE( cetech1::simd::get_element(v1, 3) == 1.0f );
    }

    SECTION( "store(v1)" ) {
        float v[4];

        cetech1::simd::store(v1, v);

        REQUIRE( cetech1::simd::get_element(v1, 0) == 1.0f );
        REQUIRE( cetech1::simd::get_element(v1, 1) == 2.0f );
        REQUIRE( cetech1::simd::get_element(v1, 2) == 3.0f );
        REQUIRE( cetech1::simd::get_element(v1, 3) == 4.0f );
    }

    SECTION( "store3(v1)" ) {
        float v[3];

        cetech1::simd::store3(v1, v);

        REQUIRE( cetech1::simd::get_element(v1, 0) == 1.0f );
        REQUIRE( cetech1::simd::get_element(v1, 1) == 2.0f );
        REQUIRE( cetech1::simd::get_element(v1, 2) == 3.0f );
    }

    SECTION( "store1(v1)" ) {
        float x = 0.0f;

        cetech1::simd::store1(v1, &x);

        REQUIRE( x == 1.0f );
    }
}