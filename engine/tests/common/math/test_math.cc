#include "catch.hpp"

#include "common/memory/memory.h"
#include "common/math/math.h"

#include "test_utils.h"

TEST_CASE( "Math operation", "[math]" ) {
    SECTION( "max()" ) {
        float max = cetech1::math::max(1.0f, 2.0f);

        REQUIRE( max == 2.0f );
    }


    SECTION( "min()" ) {
        float min = cetech1::math::min(1.0f, 2.0f);

        REQUIRE( min == 1.0f );
    }

    SECTION( "clamp()" ) {
        float clamp = 0.0f;

        clamp = cetech1::math::clamp(0.5f, 0.0f, 1.0f);
        REQUIRE( clamp == 0.5f );

        clamp = cetech1::math::clamp(2.0f, 0.0f, 1.0f);
        REQUIRE( clamp == 1.0f );

        clamp = cetech1::math::clamp(0.0f, 1.0f, 2.0f);
        REQUIRE( clamp == 1.0f );
    }

    SECTION( "abs()" ) {
        float abs = 0.0f;

        abs = cetech1::math::abs(1.0f);
        REQUIRE( abs == 1.0f );

        abs = cetech1::math::abs(-1.0f);
        REQUIRE( abs == 1.0f );
    }

    SECTION( "almost_equal()" ) {
        REQUIRE( cetech1::math::almost_equal(1.0f, 1.0000001f));
        REQUIRE_FALSE( cetech1::math::almost_equal(1.0f, 1.0001f));
    }

    SECTION( "square()" ) {
        float square = cetech1::math::square(2.0f);

        REQUIRE( square == 4.0f);
    }

    SECTION( "fast_sqrt()" ) {
        float root = cetech1::math::fast_sqrt(4.0f);

        REQUIRE_ALMOST3( root, 2.0f, 0.01f); // TODO: fast_sqrt... numerical error =(
    }

//     SECTION( "inv_sqrt()" ) {
//      float inv_root = cetech1::math::inv_sqrt(4.0f);
//
//      REQUIRE_ALMOST3( inv_root, 0.5f, 0.01f); // TODO: inv_sqrt... numerical error =(
//     }

    SECTION( "deg2rad()" ) {
        float rad = cetech1::math::deg2rad(90);

        REQUIRE_ALMOST( rad, 1.57079637f);
    }

    SECTION( "rad2deg()" ) {
        float deg = cetech1::math::rad2deg(1.57079637f);

        REQUIRE_ALMOST( deg, 90.0f);
    }

    SECTION( "fast_sin()" ) {
        float sin = cetech1::math::fast_sin(90);

        REQUIRE_ALMOST( sin, 1.0f);
    }

    SECTION( "fast_sincos()" ) {
        float sin, cos;
        sin = cos = 0.0;

        cetech1::math::fast_sincos(90, sin, cos);

        REQUIRE_ALMOST( sin, 1.0f);
        REQUIRE_ALMOST( cos, 0.0f);
    }

    SECTION( "float_select()" ) {
        float selected = 0.0f;

        selected = cetech1::math::float_select(2.0f, 1.0f, -1.0f);
        REQUIRE( selected == 1.0f );

        selected = cetech1::math::float_select(-2.0f, 1.0f, -1.0f);
        REQUIRE( selected == -1.0f );
    }
}