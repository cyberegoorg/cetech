#ifdef CETECH_TEST

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "include/catch/catch.hpp"

extern "C" {
#include "../types.h"
#include "../quatf.h"
#include "../mat44f.h"
};

/*******************************************************************************
**** Test helpers
*******************************************************************************/

/*******************************************************************************
**** Basic quaternion operation
*******************************************************************************/
SCENARIO("Basic quatf operation", "[math]") {
    GIVEN("q1 = [1.0f, 2.0f, 3.0f, 4.0f] and q2 = [5.0f, 6.0f, 7.0f, 8.0f]") {

        quatf_t q1 = {1.0f, 2.0f, 3.0f, 4.0f};
        quatf_t q2 = {5.0f, 6.0f, 7.0f, 8.0f};

        WHEN("q1 + q2") {
            quatf_t result = {0};

            quatf_add(result, q1, q2);

            THEN("result == [6.0f, 8.0f, 10.0f, 12.0f]") {
                REQUIRE(result[0] == 6.0f);
                REQUIRE(result[1] == 8.0f);
                REQUIRE(result[2] == 10.0f);
                REQUIRE(result[3] == 12.0f);
            }
        }

        WHEN("q1 - q2") {
            quatf_t result = {0};

            quatf_sub(result, q1, q2);

            THEN("result == [-4.0f, -4.0f, -4.0f, -4.0f]") {
                REQUIRE(result[0] == -4.0f);
                REQUIRE(result[1] == -4.0f);
                REQUIRE(result[2] == -4.0f);
                REQUIRE(result[3] == -4.0f);
            }
        }

        WHEN("q1 * 2.0f") {
            quatf_t result = {0};

            quatf_mul_s(result, q1, 2.0f);

            THEN("result == [2.0f, 4.0f, 6.0f, 8.0f]") {
                REQUIRE(result[0] == 2.0f);
                REQUIRE(result[1] == 4.0f);
                REQUIRE(result[2] == 6.0f);
                REQUIRE(result[3] == 8.0f);
            }
        }

        WHEN("q1 / 2.0f") {
            quatf_t result = {0};

            quatf_div_s(result, q1, 2.0f);

            THEN("result == [0.5f, 1.0f, 1.5f, 2.0f]") {
                REQUIRE(result[0] == 0.5f);
                REQUIRE(result[1] == 1.0f);
                REQUIRE(result[2] == 1.5f);
                REQUIRE(result[3] == 2.0f);
            }
        }
    }
}

//SCENARIO( "quatf can create from eulers angle", "[math]" ) {
//    GIVEN( "angles" ) {
//        WHEN("create quatf form angles") {
//            quatf_t result = {0};
//            quatf_from_euler(result,
//                             f_to_rad(0.0f),
//                             f_to_rad(45.0f),
//                             f_to_rad(90.0f));
//
//            THEN("resault to euler => angles") {
//                vec3f_t angles = {0};
//                quatf_to_eurel_angle(angles, result);
//
//                REQUIRE(angles[0] == Approx(f_to_rad(0.0f)));
//                REQUIRE(angles[1] == Approx(f_to_rad(45.0f)));
//                REQUIRE(angles[2] == Approx(f_to_rad(90.0f)));
//            }
//        }
//    }
//}

SCENARIO("identity quaternion == identiy matrix", "[math]") {
    GIVEN("Identity quaternion") {
        quatf_t quat_identity = QUATF_IDENTITY;

        WHEN("convert quat to mat") THEN("mat is identity") {
            mat44f_t mat = {0};

            quatf_to_mat44f(mat, quat_identity);

            REQUIRE(mat44f_is_identity(mat, f32_Epsilon));
        }
    }
}

#endif
