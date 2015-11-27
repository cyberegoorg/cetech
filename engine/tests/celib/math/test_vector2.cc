#include "catch.hpp"

#include "celib/memory/memory.h"
#include "celib/math/vector2.inl.h"

#include "test_utils.h"

TEST_CASE( "Vector2 operation", "[vector2]" ) {
    cetech::Vector2 v1 = {1.0f, 2.0f};
    cetech::Vector2 v2 = {3.0f, 4.0f};
    float scalar = 2.0f;

    SECTION( "v1 + v2" ) {
        cetech::Vector2 v3 = v1 + v2;

        REQUIRE( v3.x == 4.0f );
        REQUIRE( v3.y == 6.0f );
    }

    SECTION( "v1 - v2" ) {
        cetech::Vector2 v3 = v1 - v2;

        REQUIRE( v3.x == -2.0f );
        REQUIRE( v3.y == -2.0f );
    }

    SECTION( "v1 * v2" ) {
        cetech::Vector2 v3 = v1 * v2;

        REQUIRE( v3.x == 3.0f );
        REQUIRE( v3.y == 8.0f );
    }

    SECTION( "v1 / v2" ) {
        cetech::Vector2 v3 = v1 / v2;

        REQUIRE_ALMOST( v3.x, 0.33333f);
        REQUIRE_ALMOST( v3.y, 0.5f);
    }

    SECTION( "v1 * scalar" ) {
        cetech::Vector2 v2 = v1 * scalar;

        REQUIRE( v2.x == 2.0f );
        REQUIRE( v2.y == 4.0f );
    }

    SECTION( "v1 / scalar" ) {
        cetech::Vector2 v2 = v1 / scalar;

        REQUIRE( v2.x == 0.5f );
        REQUIRE( v2.y == 1.0f );
    }

    SECTION( "len(v1)" ) {
        float len = cetech::vector2::len(v1);

        REQUIRE_ALMOST3( len, 2.23f, 0.01f);
    }

    SECTION( "len_sq(v1)" ) {
        float len = cetech::vector2::len_sq(v1);

        REQUIRE( len == 5.0f );
    }

    SECTION( "len_inv(v1)" ) {
        float len = cetech::vector2::len_inv(v1);

        REQUIRE_ALMOST3( len, 0.447f, 0.001f);
    }

    SECTION( "normalized(v1)" ) {
        cetech::Vector2 v_norm = cetech::vector2::normalized(v1);

        REQUIRE( cetech::vector2::is_normalized(v_norm)); // TODO: check compare
    }

    SECTION( "dot(v1, v2)" ) {
        float dot = cetech::vector2::dot(v1, v2);

        REQUIRE( dot == 11.0f );
    }


    SECTION( "cross(v1, v2)" ) {
        float cross = cetech::vector2::cross(v1, v2);

        REQUIRE( cross == -2.0f );
    }

    SECTION( "distance(v1, v2)" ) {
        float distance = cetech::vector2::distance(v1, v2);

        REQUIRE_ALMOST3( distance, 2.82f, 0.01f);
    }

    SECTION( "distance_sq(v1, v2)" ) {
        float distance_sq = cetech::vector2::distance_sq(v1, v2);

        REQUIRE( distance_sq == 8.0f );
    }

    SECTION( "rotated(UP, 90)" ) {
        cetech::Vector2 rotated = cetech::vector2::rotated(cetech::vector2::UP, 90);

        REQUIRE_ALMOST( rotated.x, cetech::vector2::LEFT.x );
        REQUIRE_ALMOST( rotated.y, cetech::vector2::LEFT.y );
    }

    SECTION( "min(v1)" ) {
        float min = cetech::vector2::min(v1);

        REQUIRE_ALMOST(min, 1.0f);
    }


    SECTION( "max(v1)" ) {
        float max = cetech::vector2::max(v1);

        REQUIRE_ALMOST( max, 2.0f );
    }

    SECTION( "abs_max(DOWN)" ) {
        float abs_max = cetech::vector2::abs_max(cetech::vector2::DOWN);

        REQUIRE_ALMOST( abs_max, 1.0f);
    }

    SECTION( "sign_vector(DOWN)" ) {
        cetech::Vector2 sign = cetech::vector2::sign_vector(cetech::vector2::DOWN);

        REQUIRE_ALMOST( sign.x, 1.0f);
        REQUIRE_ALMOST( sign.y, -1.0f);
    }
}
