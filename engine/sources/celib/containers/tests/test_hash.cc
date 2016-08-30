#ifdef CETECH_TEST

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "include/catch/catch.hpp"

extern "C" {
#include "celib/memory/memory.h"
#include "../hash.h"
}

SCENARIO( "Hash can store items", "[hashmap]" ) {
    struct allocator* allocator = malloc_allocator_create();

    GIVEN( "A empty array" ) {
        HASH_T(int) h;
        hash_init_int(&h, allocator);

        hash_set_int(&h, 22, 23);
        hash_set_int(&h, 30, 1231);
        hash_set_int(&h, 4324, 432423);

        REQUIRE( hash_get_int(&h, 22, 0) == 23);
        REQUIRE( hash_get_int(&h, 30, 0) == 1231);
        REQUIRE( hash_get_int(&h, 4324, 0) == 432423);

        hash_destroy_int(&h);
    }

    malloc_allocator_destroy(allocator);
}

#endif
