#ifdef CETECH_TEST

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "catch/catch.hpp"

#include "celib/memory/memory.h"
#include "celib/container/hash.inl.h"

using namespace cetech;

/*******************************************************************************
**** Hash
*******************************************************************************/

/*******************************************************************************
**** Hash can map value to key
*******************************************************************************/
SCENARIO( "Hash can map value to key", "[hash]" ) {
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
SCENARIO( "Hash can remove key", "[hash]" ) {
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

/*******************************************************************************
**** Multi hash
*******************************************************************************/

/*******************************************************************************
**** Multi hash can map value to keys
*******************************************************************************/
SCENARIO( "Hash can map value to keys", "[multi_hash]" ) {
    memory_globals::init();

    GIVEN( "Empty hash" ) {
        Hash < int > hash_map(memory_globals::default_allocator());

        WHEN( "Map 1 -> 10, 1 -> 20, 3 -> 30") {
            multi_hash::insert(hash_map, 1, 10);
            multi_hash::insert(hash_map, 1, 20);
            multi_hash::insert(hash_map, 3, 30);

            const Hash<int>::Entry* entry = multi_hash::find_first(hash_map, 1);
            THEN( "First item has value 20" ) {
                REQUIRE(entry->value == 20);

            } AND_THEN( "Second item has value 10" ) {
                entry = multi_hash::find_next(hash_map, entry);
                REQUIRE(entry->value == 10);

            }
        }
    }

    memory_globals::shutdown();
}

/*******************************************************************************
**** Can remove item from multi hash
*******************************************************************************/
SCENARIO( "Can remove item from multi hash", "[multi_hash]" ) {
    memory_globals::init();

    GIVEN( "Map 1 -> 10, 1 -> 20, 3 -> 30" ) {
        Hash < int > hash_map(memory_globals::default_allocator());

        multi_hash::insert(hash_map, 1, 10);
        multi_hash::insert(hash_map, 1, 20);
        multi_hash::insert(hash_map, 3, 30);

        WHEN("Remove key 3") {
            const Hash<int>::Entry* entry = multi_hash::find_first(hash_map, 3);
            multi_hash::remove(hash_map, entry);

            THEN("key 3 does not have values" ) {
                REQUIRE(multi_hash::count(hash_map, 3) == 0);

            }

        } AND_WHEN("Remove key 1") {
            const Hash<int>::Entry* entry = multi_hash::find_first(hash_map, 1);
            multi_hash::remove(hash_map, entry);

            THEN( "key 1 has one value" ) {
                REQUIRE(multi_hash::count(hash_map, 1) == 1);
            }

        } AND_WHEN("Remove key 1 with all values") {
            multi_hash::remove_all(hash_map, 1);

            THEN( "key 1 does not have values" ) {
                REQUIRE(multi_hash::count(hash_map, 1) == 0);
            }
        }
    }

    memory_globals::shutdown();
}

#endif
