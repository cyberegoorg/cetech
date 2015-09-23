#include "catch.hpp"

#include "common/memory/memory.h"
#include "common/math/matrix33.inl"

#include "test_utils.h"

TEST_CASE( "Matrix33 operation", "[matrix33]" ) {
    cetech::Matrix33 m1 = cetech::matrix33::IDENTITY;

    SECTION( "m1 * v" ) {
        cetech::Vector3 res = m1 * cetech::vector3::UP;

        REQUIRE_ALMOST( res.x, cetech::vector3::UP.x);
        REQUIRE_ALMOST( res.y, cetech::vector3::UP.y);
        REQUIRE_ALMOST( res.z, cetech::vector3::UP.z);
    }

    SECTION( "from_axis_angle()" ) {
        cetech::Matrix33 m = cetech::matrix33::from_axis_angle(cetech::vector3::FORWARDS, 90.0f);
        cetech::Vector3 res = m * cetech::vector3::UP;

        REQUIRE_ALMOST( res.x, cetech::vector3::LEFT.x);
        REQUIRE_ALMOST( res.y, cetech::vector3::LEFT.y);
        REQUIRE_ALMOST( res.z, cetech::vector3::LEFT.z);
    }

}