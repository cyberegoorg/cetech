#include "catch.hpp"

#include "common/memory/memory.h"
#include "common/math/vector3.inl"

#include "test_utils.h"

TEST_CASE( "Vector3 operation", "[vector3]" ) {
    cetech::Vector3 v1 = {1.0f, 2.0f, 3.0f};
    cetech::Vector3 v2 = {4.0f, 5.0f, 6.0f};
    float scalar = 2.0f;

    SECTION( "v1 + v2" ) {
        cetech::Vector3 v3 = v1 + v2;

        REQUIRE( v3.x == 5.0f );
        REQUIRE( v3.y == 7.0f );
        REQUIRE( v3.z == 9.0f );
    }

    SECTION( "v1 - v2" ) {
        cetech::Vector3 v3 = v1 - v2;

        REQUIRE( v3.x == -3.0f );
        REQUIRE( v3.y == -3.0f );
        REQUIRE( v3.z == -3.0f );
    }

    SECTION( "v1 * v2" ) {
        cetech::Vector3 v3 = v1 * v2;

        REQUIRE( v3.x == 4.0f );
        REQUIRE( v3.y == 10.0f );
        REQUIRE( v3.z == 18.0f );
    }

    SECTION( "v1 / v2" ) {
        cetech::Vector3 v3 = v1 / v2;

        REQUIRE_ALMOST( v3.x, 0.25f);
        REQUIRE_ALMOST( v3.y, 0.4f);
        REQUIRE_ALMOST( v3.z, 0.5f);
    }

    SECTION( "v1 * scalar" ) {
        cetech::Vector3 v2 = v1 * scalar;

        REQUIRE( v2.x == 2.0f );
        REQUIRE( v2.y == 4.0f );
    }

    SECTION( "v1 / scalar" ) {
        cetech::Vector3 v2 = v1 / scalar;

        REQUIRE( v2.x == 0.5f );
        REQUIRE( v2.y == 1.0f );
    }

    SECTION( "len(v1)" ) {
        float len = cetech::vector3::len(v1);

        REQUIRE_ALMOST3( len, 3.74f, 0.01f);
    }

    SECTION( "len_sq(v1)" ) {
        float len = cetech::vector3::len_sq(v1);

        REQUIRE( len == 14.0f );
    }

    SECTION( "len_inv(v1)" ) {
        float len = cetech::vector3::len_inv(v1);

        REQUIRE_ALMOST3( len, 0.26721f, 0.001f);
    }

    SECTION( "normalized(v1)" ) {
        cetech::Vector3 v_norm = cetech::vector3::normalized(v1);
        const float len = cetech::vector3::len(v_norm);

        REQUIRE_ALMOST3( len, 1.0f, 0.01f); // TODO: check compare
    }

    SECTION( "dot(v1, v2)" ) {
        float dot = cetech::vector3::dot(v1, v2);

        REQUIRE( dot == 32.0f );
    }


    SECTION( "cross(v1, v2)" ) {
        // TODO: !!!
        //float cross = cetech::vector3::cross(v1, v2);

        //REQUIRE( cross == -2.0f );
    }

    SECTION( "distance(v1, v2)" ) {
        float distance = cetech::vector3::distance(v1, v2);

        REQUIRE_ALMOST3( distance, 5.18f, 0.01f);
    }

    SECTION( "distance_sq(v1, v2)" ) {
        float distance_sq = cetech::vector3::distance_sq(v1, v2);

        REQUIRE( distance_sq == 27.0f );
    }

    SECTION( "min(v1)" ) {
        float min = cetech::vector3::min(v1);

        REQUIRE_ALMOST(min, 1.0f);
    }


    SECTION( "max(v1)" ) {
        float max = cetech::vector3::max(v1);

        REQUIRE_ALMOST( max, 3.0f );
    }

    SECTION( "abs_max(DOWN)" ) {
        float abs_max = cetech::vector3::abs_max(cetech::vector3::DOWN);

        REQUIRE_ALMOST( abs_max, 1.0f);
    }

    SECTION( "sign_vector(DOWN)" ) {
        cetech::Vector3 sign = cetech::vector3::sign_vector(cetech::vector3::DOWN);

        REQUIRE_ALMOST( sign.x, 1.0f);
        REQUIRE_ALMOST( sign.y, -1.0f);
    }
}