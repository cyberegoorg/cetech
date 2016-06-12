#ifdef CETECH_TEST

#include "catch/catch.hpp"

#include "celib/memory/memory.h"
#include "celib/math/vector4.inl.h"

#include "celib/test_utils.h"

TEST_CASE( "Vector4 operation", "[vector4]" ) {
    cetech::Vector4 v1 = {1.0f, 2.0f, 3.0f, 4.0f};
    cetech::Vector4 v2 = {5.0f, 6.0f, 7.0f, 8.0f};
    float scalar = 2.0f;

    SECTION( "v1 + v2" ) {
        cetech::Vector4 v3 = v1 + v2;

        REQUIRE( v3.x == 6.0f );
        REQUIRE( v3.y == 8.0f );
        REQUIRE( v3.z == 10.0f );
        REQUIRE( v3.w == 12.0f );
    }

    SECTION( "v1 - v2" ) {
        cetech::Vector4 v3 = v1 - v2;

        REQUIRE( v3.x == -4.0f );
        REQUIRE( v3.y == -4.0f );
        REQUIRE( v3.z == -4.0f );
        REQUIRE( v3.w == -4.0f );
    }

    SECTION( "v1 * v2" ) {
        cetech::Vector4 v3 = v1 * v2;

        REQUIRE( v3.x == 5.0f );
        REQUIRE( v3.y == 12.0f );
        REQUIRE( v3.z == 21.0f );
        REQUIRE( v3.w == 32.0f );
    }

    SECTION( "v1 / v2" ) {
        cetech::Vector4 v3 = v1 / v2;

        REQUIRE_ALMOST( v3.x, 0.2f);
        REQUIRE_ALMOST( v3.y, 0.33334f);
        REQUIRE_ALMOST( v3.z, 0.42857f);
        REQUIRE_ALMOST( v3.w, 0.5f);
    }

    SECTION( "v1 * scalar" ) {
        cetech::Vector4 v2 = v1 * scalar;

        REQUIRE( v2.x == 2.0f );
        REQUIRE( v2.y == 4.0f );
    }

    SECTION( "v1 / scalar" ) {
        cetech::Vector4 v2 = v1 / scalar;

        REQUIRE( v2.x == 0.5f );
        REQUIRE( v2.y == 1.0f );
    }

    SECTION( "len(v1)" ) {
        float len = cetech::vector4::len(v1);

        REQUIRE_ALMOST3( len, 5.47f, 0.01f);
    }

    SECTION( "len_sq(v1)" ) {
        float len = cetech::vector4::len_sq(v1);

        REQUIRE( len == 30.0f );
    }

    SECTION( "len_inv(v1)" ) {
        float len = cetech::vector4::len_inv(v1);

        REQUIRE_ALMOST3( len, 0.18242f, 0.001f);
    }

    SECTION( "normalized(v1)" ) {
        cetech::Vector4 v_norm = cetech::vector4::normalized(v1);
        const float len = cetech::vector4::len(v_norm);

        REQUIRE_ALMOST3( len, 1.0f, 0.01f); // TODO: check compare
    }

    SECTION( "dot(v1, v2)" ) {
        float dot = cetech::vector4::dot(v1, v2);

        REQUIRE( dot == 70.0f );
    }

    SECTION( "distance(v1, v2)" ) {
        float distance = cetech::vector4::distance(v1, v2);

        REQUIRE_ALMOST3( distance, 7.98f, 0.01f);
    }

    SECTION( "distance_sq(v1, v2)" ) {
        float distance_sq = cetech::vector4::distance_sq(v1, v2);

        REQUIRE( distance_sq == 64.0f );
    }

    SECTION( "min(v1)" ) {
        float min = cetech::vector4::min(v1);

        REQUIRE_ALMOST(min, 1.0f);
    }


    SECTION( "max(v1)" ) {
        float max = cetech::vector4::max(v1);

        REQUIRE_ALMOST( max, 4.0f );
    }

    SECTION( "abs_max(DOWN)" ) {
        float abs_max = cetech::vector4::abs_max(cetech::vector4::DOWN);

        REQUIRE_ALMOST( abs_max, 1.0f);
    }

    SECTION( "sign_vector(DOWN)" ) {
        cetech::Vector4 sign = cetech::vector4::sign_vector(cetech::vector4::DOWN);

        REQUIRE_ALMOST( sign.x, 1.0f);
        REQUIRE_ALMOST( sign.y, -1.0f);
    }
}

#endif
