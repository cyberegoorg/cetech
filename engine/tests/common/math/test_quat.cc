#include "catch.hpp"

#include "common/memory/memory.h"
#include "common/math/quat.h"

#include "test_utils.h"

TEST_CASE( "Quat operation", "[quat]" ) {
    cetech1::Quat q1 = {1.0f, 2.0f, 3.0f, 4.0f};
    cetech1::Quat q2 = {5.0f, 6.0f, 7.0f, 8.0f};
    float scalar = 2.0f;

    SECTION( "q1 + q2" ) {
        cetech1::Quat q3 = q1 + q2;

        REQUIRE( q3.x == 6.0f );
        REQUIRE( q3.y == 8.0f );
        REQUIRE( q3.z == 10.0f );
        REQUIRE( q3.w == 12.0f );
    }

    SECTION( "q1 - q2" ) {
        cetech1::Quat q3 = q1 - q2;

        REQUIRE( q3.x == -4.0f );
        REQUIRE( q3.y == -4.0f );
        REQUIRE( q3.z == -4.0f );
        REQUIRE( q3.w == -4.0f );
    }

    SECTION( "q1 * scalar" ) {
        cetech1::Quat q2 = q1 * scalar;

        REQUIRE( q2.x == 2.0f );
        REQUIRE( q2.y == 4.0f );
    }

    SECTION( "q1 * q2" ) {
        cetech1::Quat q3 = q1 * q2;

        REQUIRE( q3.x == -60.0f );
        REQUIRE( q3.y == 12.0f );
        REQUIRE( q3.z == 30.0f );
        REQUIRE( q3.w == 24.0f );
    }

    SECTION( "q1 * v1" ) {
        cetech1::Quat q = cetech1::quat::from_axis_angle({0.0f, 0.0f, 1.0f}, 90.0f);
        cetech1::Vector3 v1 = {1.0f, 0.0f, 0.0f};
        cetech1::Vector3 v2 = q * v1;

        REQUIRE( v2.x == Approx(0.0f).epsilon(0.001f));
        REQUIRE( v2.y == Approx(1.0f).epsilon(0.001f));
        REQUIRE( v2.z == Approx(0.0f).epsilon(0.001f));

        cetech1::Quat qq = (-q);
        cetech1::Vector3 v3 = qq * v2;

        REQUIRE( v3.x == Approx(1.0f).epsilon(0.001f));
        REQUIRE( v3.y == Approx(0.0f).epsilon(0.001f));
        REQUIRE( v3.z == Approx(0.0f).epsilon(0.001f));
    }

    SECTION( "q1 / scalar" ) {
        cetech1::Quat q2 = q1 / scalar;

        REQUIRE( q2.x == 0.5f );
        REQUIRE( q2.y == 1.0f );
    }

    SECTION( "len(q1)" ) {
        float len = cetech1::quat::len(q1);

        REQUIRE_ALMOST3( len, 5.47f, 0.01f);
    }

    SECTION( "len_sq(q1)" ) {
        float len = cetech1::quat::len_sq(q1);

        REQUIRE( len == 30.0f );
    }

    SECTION( "len_inv(q1)" ) {
        float len = cetech1::quat::len_inv(q1);

        REQUIRE_ALMOST( len, 0.18257f);
    }

    SECTION( "normalized(q1)" ) {
        cetech1::Quat v_norm = cetech1::quat::normalized(q1);
        const float len = cetech1::quat::len(v_norm);

        REQUIRE_ALMOST3( len, 1.0f, 0.01f); // TODO: check compare
    }
}