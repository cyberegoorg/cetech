#if defined(CETECH_TEST)

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "include/catch/catch.hpp"

extern "C" {
#include <celib/memory/memory.h>
#include "../string.h"
};

/*******************************************************************************
**** Test helpers
*******************************************************************************/

/*******************************************************************************
**** String duplicate
*******************************************************************************/
SCENARIO( "string duplicate", "[string]" ) {
    memsys_init(4*1024*1024);

    GIVEN( "string 'test string'" ) {
        const char* test_string = "test_string";

        WHEN("duplicate sting") {
            char* duplicate_string = str_duplicate(test_string, memsys_main_allocator());

            THEN("duplicate_string == 'test string'") {
                REQUIRE(str_compare(test_string, duplicate_string) == 0);
            }

            allocator_deallocate(memsys_main_allocator(), duplicate_string);
        }

    }

    memsys_shutdown();
}

/*******************************************************************************
**** String compare
*******************************************************************************/
SCENARIO( "string compare", "[string]" ) {
    GIVEN( "string 'test_string'" ) {
        const char* lstr = "test_string";
        const char* rstr_ok = "test_string";
        const char* rstr_dif = "test_s";

        WHEN( "compare, \"test_string\" and \"test_string\"") {
            int cmp = str_compare(lstr, rstr_ok);
            int cmp2 = str_compare(rstr_ok, lstr);

            THEN("are eqaul") {
                REQUIRE( cmp == 0);
                REQUIRE( cmp2 == 0);
            }

        } AND_WHEN( "compare, \"test_string\" and \"test_s\"") {
            int cmp = str_compare(lstr, rstr_dif);
            int cmp2 = str_compare(rstr_dif, lstr);

            THEN("are not equal") {
                REQUIRE( cmp  > 0 );
                REQUIRE( cmp2 < 0 );
            }
        }
    }
}

/*******************************************************************************
**** String length
*******************************************************************************/
SCENARIO( "string length", "[string]" ) {
    GIVEN( "string 'test_string'" ) {
        const char* lstr = "test_string";

        WHEN( "lenght \"test_string\" ") {
            size_t len = str_lenght(lstr);

            THEN("len == 11") {
                REQUIRE( len == 11);
            }
        }
    }
}

#endif
