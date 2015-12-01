#ifdef CETECH_TEST

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "catch/catch.hpp"

#include "celib/memory/memory.h"
#include "celib/container/hash.inl.h"

using namespace cetech;

/*******************************************************************************
**** Hash can map value to key
*******************************************************************************/
SCENARIO( "Hash can map value to key", "[hash_map]" ) {
    memory_globals::init();

    GIVEN( "Empty hash" ) {
        Hash < int > hash_map(memory_globals::default_allocator());

        WHEN( "Map 1 -> 10, 2 -> 20, 3 -> 30") {
            hash::set(hash_map, 1, 10);
            hash::set(hash_map, 2, 20);
            hash::set(hash_map, 3, 30);

            THEN( "Key 1 has value 10" ) {
                REQUIRE(hash::get(hash_map, 1, 0) == 10);

            } AND_THEN( "Key 2 has value 20" ) {
                REQUIRE(hash::get(hash_map, 2, 0) == 20);

            } AND_THEN( "Key 3 has value 30" ) {
                REQUIRE(hash::get(hash_map, 3, 0) == 30);
            }
        }
    }

    memory_globals::shutdown();
}

/*******************************************************************************
**** Hash can remove key
*******************************************************************************/
SCENARIO( "Hash can remove key", "[hash_map]" ) {
    memory_globals::init();

    GIVEN( "Map 1 -> 10, 2 -> 20, 3 -> 30" ) {
        Hash < int > hash_map(memory_globals::default_allocator());
        hash::set(hash_map, 1, 10);
        hash::set(hash_map, 2, 20);
        hash::set(hash_map, 3, 30);

        WHEN("Remove key 2") {
            hash::remove(hash_map, 2);

            THEN( "Key 2 has default value" ) {
                REQUIRE(hash::get(hash_map, 2, 0) == 0);

            } AND_THEN( "Key 1 has value 10" ) {
                REQUIRE(hash::get(hash_map, 1, 0) == 10);

            } AND_THEN( "Key 3 has value 30" ) {
                REQUIRE(hash::get(hash_map, 3, 0) == 30);

            }
        }
    }

    memory_globals::shutdown();
}
#endif
