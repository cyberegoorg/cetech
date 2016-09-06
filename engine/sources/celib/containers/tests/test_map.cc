#ifdef CETECH_TEST

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "include/catch/catch.hpp"

extern "C" {
#include "celib/memory/memory.h"
#include "../map.h"
}

SCENARIO( "Map can store items", "[hashmap]" ) {
    struct allocator* allocator = malloc_allocator_create();

    GIVEN( "A empty map" ) {
        MAP_T(int) h;
        map_init_int(&h, allocator);

        map_set_int(&h, 22, 23);
        map_set_int(&h, 30, 1231);
        map_set_int(&h, 4324, 432423);

        REQUIRE( map_get_int(&h, 22, 0) == 23);
        REQUIRE( map_get_int(&h, 30, 0) == 1231);
        REQUIRE( map_get_int(&h, 4324, 0) == 432423);

        map_destroy_int(&h);
    }

    malloc_allocator_destroy(allocator);
}

#endif
