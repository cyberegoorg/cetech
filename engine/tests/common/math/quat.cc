#include "catch.hpp"

#include "common/memory/memory.h"
#include "common/math/quat.h"

#include "test_utils.h"

TEST_CASE( "Quat operation", "[quat]" ) {
    cetech1::Quat v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    cetech1::Quat v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    float scalar = 2.0f;

    SECTION( "v1 + v2" ) {
        cetech1::Quat v3 = v1 + v2;

        REQUIRE( v3.x == 6.0f );
        REQUIRE( v3.y == 8.0f );
        REQUIRE( v3.z == 10.0f );
	REQUIRE( v3.w == 12.0f );
    }

    SECTION( "v1 - v2" ) {
        cetech1::Quat v3 = v1 - v2;

        REQUIRE( v3.x == -4.0f );
        REQUIRE( v3.y == -4.0f );
        REQUIRE( v3.z == -4.0f );
	REQUIRE( v3.w == -4.0f );
    }

    SECTION( "v1 * scalar" ) {
        cetech1::Quat v2 = v1 * scalar;

        REQUIRE( v2.x == 2.0f );
        REQUIRE( v2.y == 4.0f );
    }

    SECTION( "q1 * q2" ) {
        cetech1::Quat v3 = v1 * v2;

        REQUIRE( v3.x == -60.0f );
        REQUIRE( v3.y == 12.0f );
        REQUIRE( v3.z == 30.0f );
	REQUIRE( v3.w == 24.0f );
    }
    
    SECTION( "v1 / scalar" ) {
        cetech1::Quat v2 = v1 / scalar;

        REQUIRE( v2.x == 0.5f );
        REQUIRE( v2.y == 1.0f );
    }

    SECTION( "len(v1)" ) {
        float len = cetech1::quat::len(v1);

        REQUIRE_ALMOST( len, 5.47265f);
    }

    SECTION( "len_sq(v1)" ) {
        float len = cetech1::quat::len_sq(v1);

        REQUIRE( len == 30.0f );
    }

    SECTION( "len_inv(v1)" ) {
        float len = cetech1::quat::len_inv(v1);

        REQUIRE_ALMOST( len, 0.18242f);
    }

    SECTION( "normalized(v1)" ) {
        cetech1::Quat v_norm = cetech1::quat::normalized(v1);
        const float len = cetech1::quat::len(v_norm);

        REQUIRE_ALMOST3( len, 1.0f, 0.01f); // TODO: check compare
    }
}