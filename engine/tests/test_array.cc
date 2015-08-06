#include "catch.hpp"

#include "runtime/memory.h"
#include "runtime/array.h"

SCENARIO( "Array can store items", "[array]" ) {
    cetech1::memory_globals::init();

    GIVEN( "A empty array" ) {
        cetech1::Array<int> a(cetech1::memory_globals::default_allocator());

        REQUIRE( cetech1::array::size(a) == 0 );

        WHEN( "some items push back" ) {
            cetech1::array::push_back(a, 1);
            cetech1::array::push_back(a, 2);
            cetech1::array::push_back(a, 3);

            THEN( "size changed" ) {
                REQUIRE( cetech1::array::size(a) == 3 );
            }

            AND_THEN("item at index 0 is first pushed") {
                REQUIRE( a[0] == 1 );
            }

            AND_THEN("item at index 1 is second pushed") {
                REQUIRE( a[1] == 2 );
            }

            AND_THEN("item at index 2 is last pushed") {
                REQUIRE( a[2] == 3 );
            }
        }

    }

    cetech1::memory_globals::shutdown();
}

SCENARIO( "Array can be sized and resized", "[array]" ) {
    cetech1::memory_globals::init();

    GIVEN( "A array with some items" ) {
        cetech1::Array<int> a(cetech1::memory_globals::default_allocator());

        cetech1::array::push_back(a, 0);
        cetech1::array::push_back(a, 1);
        cetech1::array::push_back(a, 2);

        REQUIRE( cetech1::array::size(a) == 3 );
        REQUIRE( a._capacity >= 8 );

        WHEN( "size is increased" ) {
            cetech1::array::resize(a, 10);

            THEN( "the size and capacity change" ) {
                REQUIRE( cetech1::array::size(a) == 10 );
                REQUIRE( a._capacity >= 10 );
            }
        }

        WHEN( "size is reduced" ) {
            cetech1::array::resize(a, 5);

            THEN( "the size change but not capacity" ) {
                REQUIRE( cetech1::array::size(a) == 5 );
                REQUIRE( a._capacity >= 5 );
            }
        }

        WHEN( "more capacity is reserved" ) {
            cetech1::array::reserve(a, 10);

            THEN( "the capacity change but not size" ) {
                REQUIRE( cetech1::array::size(a) == 3 );
                REQUIRE( a._capacity >= 10 );
            }
        }

        WHEN( "less capacity is reserved" ) {
            cetech1::array::reserve(a, 0);

            THEN( "the capacity change but not size" ) {
                REQUIRE( cetech1::array::size(a) == 3 );
                REQUIRE( a._capacity >= 5 );
            }
        }
    }

    cetech1::memory_globals::shutdown();
}
