#ifdef CETECH_TEST

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "include/catch/catch.hpp"

extern "C" {
#include "../types.h"
#include "../vec4f.h"
};

/*******************************************************************************
**** Test helpers
*******************************************************************************/

/*******************************************************************************
**** Basic vector operation
*******************************************************************************/
SCENARIO( "Basic vec4f operation", "[math]" ) {
    GIVEN( "v1 = [1.0f, 2.0f, 3.0f, 4.0f] and v2 = [5.0f, 6.0f, 7.0f]" ) {

        vec4f_t v1 = {1.0f, 2.0f, 3.0f, 4.0f};
        vec4f_t v2 = {5.0f, 6.0f, 7.0f, 8.0f};

        WHEN("v1 + v2") {
            vec4f_t result = {0};

            vec4f_add(result, v1, v2);

            THEN("result == [6.0f, 8.0f, 10.0f, 12.0f]") {
                REQUIRE( result[0] == 6.0f);
                REQUIRE( result[1] == 8.0f);
                REQUIRE( result[2] == 10.0f);
                REQUIRE( result[3] == 12.0f);
            }
        }

        WHEN("v1 - v2") {
            vec4f_t result = {0};

            vec4f_sub(result, v1, v2);

            THEN("result == [-4.0f, -4.0f, -4.0f, -4.0f]") {
                REQUIRE( result[0] == -4.0f);
                REQUIRE( result[1] == -4.0f);
                REQUIRE( result[2] == -4.0f);
                REQUIRE( result[3] == -4.0f);
            }
        }

        WHEN("v1 * 2.0f") {
            vec4f_t result = {0};

            vec4f_mul(result, v1, 2.0f);

            THEN("result == [2.0f, 4.0f, 6.0f, 8.0f]") {
                REQUIRE( result[0] == 2.0f);
                REQUIRE( result[1] == 4.0f);
                REQUIRE( result[2] == 6.0f);
                REQUIRE( result[3] == 8.0f);
            }
        }

        WHEN("v1 / 2.0f") {
            vec4f_t result = {0};

            vec4f_div(result, v1, 2.0f);

            THEN("result == [0.5f, 1.0f, 1.5f, 2.0f]") {
                REQUIRE( result[0] == 0.5f);
                REQUIRE( result[1] == 1.0f);
                REQUIRE( result[2] == 1.5f);
                REQUIRE( result[3] == 2.0f);
            }
        }

        WHEN("v1 dot v2") {
            float dot = vec4f_dot(v1, v2);

            THEN("dot == 70.0f") {
                REQUIRE( dot == 70.0f);
            }
        }

        WHEN("len_squared v1") {
            float len_sq = vec4f_length_squared(v1);

            THEN("len_sq == 30.0f") {
                REQUIRE( len_sq == 30.0f);
            }
        }

        WHEN("len v1") {
            float len_sq = vec4f_length(v1);

            THEN("len == 5.47723f") {
                REQUIRE( len_sq == Approx(5.47723f));
            }
        }

        WHEN("normalize v1") {
            vec4f_t result = {0};
            vec4f_normalized(result, v1);

            THEN("result == [~0.18257f, ~0.36515f, ~0.54772f, ~0.7303f]") {
                REQUIRE( result[0] == Approx(0.18257f));
                REQUIRE( result[1] == Approx(0.36515f));
                REQUIRE( result[2] == Approx(0.54772f));
                REQUIRE( result[3] == Approx(0.7303f));
            }
        }
    }
}

#endif
