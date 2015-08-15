#include "catch.hpp"

#include "common/memory/memory.h"
#include "common/math/vector3.h"

#include "test_utils.h"

TEST_CASE( "Vector3 operation", "[vector3]" ) {
    cetech1::Vector3 v1 = {1.0f, 2.0f, 3.0f};
    cetech1::Vector3 v2 = {4.0f, 5.0f, 6.0f};
    float scalar = 2.0f;

    SECTION( "v1 + v2" ) {
        cetech1::Vector3 v3 = v1 + v2;

        REQUIRE( v3.x == 5.0f );
        REQUIRE( v3.y == 7.0f );
        REQUIRE( v3.z == 9.0f );
    }

    SECTION( "v1 - v2" ) {
        cetech1::Vector3 v3 = v1 - v2;

        REQUIRE( v3.x == -3.0f );
        REQUIRE( v3.y == -3.0f );
        REQUIRE( v3.z == -3.0f );
    }

    SECTION( "v1 * v2" ) {
        cetech1::Vector3 v3 = v1 * v2;

        REQUIRE( v3.x == 4.0f );
        REQUIRE( v3.y == 10.0f );
        REQUIRE( v3.z == 18.0f );
    }

    SECTION( "v1 / v2" ) {
        cetech1::Vector3 v3 = v1 / v2;

        REQUIRE_ALMOST( v3.x, 0.25f);
        REQUIRE_ALMOST( v3.y, 0.4f);
        REQUIRE_ALMOST( v3.z, 0.5f);
    }

    SECTION( "v1 * scalar" ) {
        cetech1::Vector3 v2 = v1 * scalar;

        REQUIRE( v2.x == 2.0f );
        REQUIRE( v2.y == 4.0f );
    }

    SECTION( "v1 / scalar" ) {
        cetech1::Vector3 v2 = v1 / scalar;

        REQUIRE( v2.x == 0.5f );
        REQUIRE( v2.y == 1.0f );
    }

    SECTION( "len(v1)" ) {
        float len = cetech1::vector3::len(v1);

        REQUIRE_ALMOST( len, 3.741f);
    }

    SECTION( "len_sq(v1)" ) {
        float len = cetech1::vector3::len_sq(v1);

        REQUIRE( len == 14.0f );
    }

    SECTION( "len_inv(v1)" ) {
        float len = cetech1::vector3::len_inv(v1);

        REQUIRE_ALMOST( len, 0.26721f);
    }

    SECTION( "normalized(v1)" ) {
        cetech1::Vector3 v_norm = cetech1::vector3::normalized(v1);
        const float len = cetech1::vector3::len(v_norm);

        REQUIRE_ALMOST3( len, 1.0f, 0.01f); // TODO: check compare
    }

    SECTION( "dot(v1, v2)" ) {
        float dot = cetech1::vector3::dot(v1, v2);

        REQUIRE( dot == 32.0f );
    }


    SECTION( "cross(v1, v2)" ) {
        // TODO: !!!
        //float cross = cetech1::vector3::cross(v1, v2);

        //REQUIRE( cross == -2.0f );
    }

    SECTION( "distance(v1, v2)" ) {
        float distance = cetech1::vector3::distance(v1, v2);

        REQUIRE_ALMOST( distance, 5.18815f );
    }

    SECTION( "distance_sq(v1, v2)" ) {
        float distance_sq = cetech1::vector3::distance_sq(v1, v2);

        REQUIRE( distance_sq == 27.0f );
    }

    SECTION( "min(v1)" ) {
        float min = cetech1::vector3::min(v1);

        REQUIRE_ALMOST(min, 1.0f);
    }


    SECTION( "max(v1)" ) {
        float max = cetech1::vector3::max(v1);

        REQUIRE_ALMOST( max, 3.0f );
    }

    SECTION( "abs_max(DOWN)" ) {
        float abs_max = cetech1::vector3::abs_max(cetech1::vector3::DOWN);

        REQUIRE_ALMOST( abs_max, 1.0f);
    }

    SECTION( "sign_vector(DOWN)" ) {
        cetech1::Vector3 sign = cetech1::vector3::sign_vector(cetech1::vector3::DOWN);

        REQUIRE_ALMOST( sign.x, 1.0f);
        REQUIRE_ALMOST( sign.y, -1.0f);
    }
}