#include "catch.hpp"

#include "celib/memory/memory.h"
#include "celib/math/matrix44.inl.h"

#include "test_utils.h"

TEST_CASE( "Matrix44 operation", "[matrix4]" ) {
    cetech::Matrix44 m1 = cetech::matrix44::IDENTITY;

    SECTION( "m1 * v" ) {
        cetech::Vector3 res = m1 * cetech::vector3::UP;

        REQUIRE_ALMOST( res.x, cetech::vector3::UP.x);
        REQUIRE_ALMOST( res.y, cetech::vector3::UP.y);
        REQUIRE_ALMOST( res.z, cetech::vector3::UP.z);
    }

    SECTION( "from_axis_angle()" ) {
        cetech::Matrix44 m = cetech::matrix44::from_axis_angle(cetech::vector3::FORWARDS, 90.0f);
        cetech::Vector3 res = m * cetech::vector3::UP;

        REQUIRE_ALMOST( res.x, cetech::vector3::LEFT.x);
        REQUIRE_ALMOST( res.y, cetech::vector3::LEFT.y);
        REQUIRE_ALMOST( res.z, cetech::vector3::LEFT.z);
    }


    SECTION( "determinant(m1)" ) {
        const float det = cetech::matrix44::determinant(m1);
        REQUIRE( det == 1.0f);
    }


}
