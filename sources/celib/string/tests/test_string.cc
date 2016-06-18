#ifdef CETECH_TEST

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "include/catch/catch.hpp"

extern "C" {
    #include "../string.h"
    #include "../../memory/mallocator.h"
    #include "../../log/log.h"
    #include "../../log/handlers.h"
};

/*******************************************************************************
**** Test helpers
*******************************************************************************/

/*******************************************************************************
**** String duplicate
*******************************************************************************/
SCENARIO( "string duplicate", "[string]" ) {
    GIVEN( "string 'test string'" ) {
        ALLOCATOR_CREATE(allocator, mallocator);

        const char* test_string = "test_string";

        WHEN("duplicate sting") {
            char* duplicate_string = string_duplicate(test_string, allocator); // TODO: LEAK

            THEN("duplicate_string == 'test string'") {
                REQUIRE( string_compare(test_string, duplicate_string) == 0);
            }

            alloc_free(allocator, duplicate_string);
        }

        ALLOCATOR_DESTROY(allocator, mallocator);
    }
}

/*******************************************************************************
**** String compare
*******************************************************************************/
SCENARIO( "string compare", "[string]" ) {
    GIVEN( "string 'test string'" ) {
        const char* lstr = "test_string";
        const char* rstr_ok = "test_string";
        const char* rstr_dif = "test_s";

        WHEN( "compare, \"test_string\" and \"test_string\"") {
            int cmp = string_compare(lstr, rstr_ok);
            int cmp2 = string_compare(rstr_ok, lstr);

            THEN("are eqaul") {
                REQUIRE( cmp == 0);
                REQUIRE( cmp2 == 0);
            }

        } AND_WHEN( "compare, \"test_string\" and \"test_s\"") {
            int cmp = string_compare(lstr, rstr_dif);
            int cmp2 = string_compare(rstr_dif, lstr);

            THEN("are not equal") {
                REQUIRE( cmp  > 0 );
                REQUIRE( cmp2 < 0 );
            }
        }
    }
}

#endif
