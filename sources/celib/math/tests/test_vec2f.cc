#ifdef CETECH_TEST

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "include/catch/catch.hpp"

extern "C" {
#include "../types.h"
#include "../vec2f.h"
};

/*******************************************************************************
**** Test helpers
*******************************************************************************/

/*******************************************************************************
**** Basic vector operation
*******************************************************************************/
SCENARIO( "Basic vec2f operation", "[math]" ) {
    GIVEN( "v1 = [1.0f, 2.0f] and v2 = [3.0f, 4.0f]" ) {

        vec2f_t v1 = {1.0f, 2.0f};
        vec2f_t v2 = {3.0f, 4.0f};

        WHEN("v1 + v2") {
            vec2f_t result = {0};

            vec2f_add(result, v1, v2);

            THEN("result == [4.0f, 6.0f]") {
                REQUIRE( result[0] == 4.0f);
                REQUIRE( result[1] == 6.0f);
            }
        }

        WHEN("v1 - v2") {
            vec2f_t result = {0};

            vec2f_sub(result, v1, v2);

            THEN("result == [-2.0f, -2.0f]") {
                REQUIRE( result[0] == -2.0f);
                REQUIRE( result[1] == -2.0f);
            }
        }

        WHEN("v1 * 2.0f") {
            vec2f_t result = {0};

            vec2f_mul(result, v1, 2.0f);

            THEN("result == [2.0f, 4.0f]") {
                REQUIRE( result[0] == 2.0f);
                REQUIRE( result[1] == 4.0f);
            }
        }

        WHEN("v1 / 2.0f") {
            vec2f_t result = {0};

            vec2f_div(result, v1, 2.0f);

            THEN("result == [0.5f, 1.0f]") {
                REQUIRE( result[0] == 0.5f);
                REQUIRE( result[1] == 1.0f);
            }
        }

        WHEN("v1 dot v2") {
            float dot = vec2f_dot(v1, v2);

            THEN("dot == 11.0f") {
                REQUIRE( dot == 11.0f);
            }
        }

        WHEN("len_squared v1") {
            float len_sq = vec2f_length_squared(v1);

            THEN("len_sq == 5.0") {
                REQUIRE( len_sq == 5.0f);
            }
        }

        WHEN("len v1") {
            float len_sq = vec2f_length(v1);

            THEN("len == 2.23607f") {
                REQUIRE( len_sq == Approx(2.23607f));
            }
        }

        WHEN("normalize v1") {
            vec2f_t result = {0};
            vec2f_normalized(result, v1);

            THEN("result == [~0.44721f, ~0.89443f]") {
                REQUIRE( result[0] == Approx(0.44721f));
                REQUIRE( result[1] == Approx(0.89443f));
            }
        }

        WHEN("v1 eq v2") {
            THEN("v1 != v2") {
                REQUIRE_FALSE( vec2f_eq(v1, v2, f_Epsilon) );
            }
        }

        WHEN("v1 eq v1") {
            THEN("v1 == v1") {
                REQUIRE( vec2f_eq(v1, v1, f_Epsilon) );
            }
        }
    }
}

#endif
