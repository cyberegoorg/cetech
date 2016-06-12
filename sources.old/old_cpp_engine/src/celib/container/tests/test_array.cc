#ifdef CETECH_TEST

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "catch/catch.hpp"

#include "celib/memory/memory.h"
#include "celib/container/array.inl.h"

using namespace cetech;

/*******************************************************************************
**** Array can store items
*******************************************************************************/
SCENARIO( "Array can store items", "[array]" ) {
    memory_globals::init();

    GIVEN( "A empty array" ) {
        Array < int > a(memory_globals::default_allocator());

        REQUIRE( array::size(a) == 0 );
        REQUIRE( array::empty(a));

        WHEN( "some items push back" ) {
            array::push_back(a, 1);
            array::push_back(a, 2);
            array::push_back(a, 3);

            THEN( "size changed" ) {
                REQUIRE( array::size(a) == 3 );
            }

            AND_THEN("array has any item") {
                REQUIRE( array::any(a));
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

    memory_globals::shutdown();
}

/*******************************************************************************
**** Array can be sized and resized
*******************************************************************************/
SCENARIO( "Array can be sized and resized", "[array]" ) {
    memory_globals::init();

    GIVEN( "A array with some items" ) {
        Array < int > a(memory_globals::default_allocator());

        array::push_back(a, 0);
        array::push_back(a, 1);
        array::push_back(a, 2);

        REQUIRE( array::size(a) == 3 );
        REQUIRE( a._capacity >= 8 );

        WHEN( "size is increased" ) {
            array::resize(a, 10);

            THEN( "the size and capacity change" ) {
                REQUIRE( array::size(a) == 10 );
                REQUIRE( a._capacity >= 10 );
            }
        }

        WHEN( "size is reduced" ) {
            array::resize(a, 5);

            THEN( "the size change but not capacity" ) {
                REQUIRE( array::size(a) == 5 );
                REQUIRE( a._capacity >= 5 );
            }
        }

        WHEN( "more capacity is reserved" ) {
            array::reserve(a, 10);

            THEN( "the capacity change but not size" ) {
                REQUIRE( array::size(a) == 3 );
                REQUIRE( a._capacity >= 10 );
            }
        }

        WHEN( "less capacity is reserved" ) {
            array::reserve(a, 0);

            THEN( "the capacity change but not size" ) {
                REQUIRE( array::size(a) == 3 );
                REQUIRE( a._capacity >= 5 );
            }
        }
    }

    memory_globals::shutdown();
}

/*******************************************************************************
**** Can iterate.
*******************************************************************************/
SCENARIO( "Can iterate array", "[array]" ) {
    memory_globals::init();

    GIVEN( "Array with elements [1, 2, 3, 4, 5]" ) {
        Array < int > a(memory_globals::default_allocator());

        static const int items [] = {1, 2, 3, 4, 5};
        array::push(a, items, 5);

        REQUIRE( array::size(a) == 5 );

        WHEN( "iterate from begin to end" ) {
            int* it = array::begin(a);

            THEN( "item is 1, 2, 3, 4, 5, end" ) {
                REQUIRE( *it == 1 );
                REQUIRE( *++it == 2 );
                REQUIRE( *++it == 3 );
                REQUIRE( *++it == 4 );
                REQUIRE( *++it == 5 );
                REQUIRE( ++it == array::end(a));
            }
        }

        WHEN( "iterate from end to begin" ) {
            int* it = array::end(a);

            THEN( "item is 5, 4, 3, 2, begin" ) {
                REQUIRE( *--it == 5 );
                REQUIRE( *--it == 4 );
                REQUIRE( *--it == 3 );
                REQUIRE( *--it == 2 );
                REQUIRE( --it == array::begin(a));
            }
        }

    }

    memory_globals::shutdown();
}


#endif
