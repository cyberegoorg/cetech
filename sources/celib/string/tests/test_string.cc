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

        /***********************************************************************
        **** Info
        ***********************************************************************/
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

#endif
