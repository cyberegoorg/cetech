#ifdef CETECH_TEST

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "include/catch/catch.hpp"

extern "C" {
#include "celib/memory/memory.h"
#include "../array.h"
}

/*******************************************************************************
**** Array can store items
*******************************************************************************/
SCENARIO( "Array can store items", "[array]" ) {
    struct allocator* allocator = malloc_allocator_create();

    GIVEN( "A empty array" ) {
        ARRAY_T(int) array;
        ARRAY_INIT(int, &array, allocator);

        REQUIRE( ARRAY_SIZE(&array) == 0 );
        //REQUIRE( array::empty(a));

        WHEN( "some items push back" ) {
            ARRAY_PUSH_BACK(int, &array, 1);
            ARRAY_PUSH_BACK(int, &array, 2);
            ARRAY_PUSH_BACK(int, &array, 3);

            THEN( "size changed" ) {
                REQUIRE( ARRAY_SIZE(&array) == 3 );
            }

            AND_THEN("array has any item") {
                //REQUIRE( array::any(a));
            }

            AND_THEN("item at index 0 is first pushed") {
                REQUIRE( ARRAY_AT(&array, 0) == 1 );
            }

            AND_THEN("item at index 1 is second pushed") {
                REQUIRE( ARRAY_AT(&array, 1) == 2 );
            }

            AND_THEN("item at index 2 is last pushed") {
                REQUIRE( ARRAY_AT(&array, 2) == 3 );
            }
        }

        ARRAY_DESTROY(int, &array);
    }

    malloc_allocator_destroy(allocator);
}

/*******************************************************************************
**** Array can be sized and resized
*******************************************************************************/
SCENARIO( "Array can be sized and resized", "[array]" ) {
    struct allocator* allocator = malloc_allocator_create();


    GIVEN( "A array with some items" ) {
        ARRAY_T(int) array;
        ARRAY_INIT(int, &array, allocator);

        ARRAY_PUSH_BACK(int, &array, 1);
        ARRAY_PUSH_BACK(int, &array, 2);
        ARRAY_PUSH_BACK(int, &array, 3);

        REQUIRE( ARRAY_SIZE(&array) == 3 );
        REQUIRE( ARRAY_CAPACITY(&array) >= 8 );

        WHEN( "size is increased" ) {
            ARRAY_RESIZE(int, &array, 10);

            THEN( "the size and capacity change" ) {
                REQUIRE( ARRAY_SIZE(&array) == 10 );
                REQUIRE( ARRAY_CAPACITY(&array) >= 10 );
            }
        }

        WHEN( "size is reduced" ) {
            ARRAY_RESIZE(int, &array, 5);

            THEN( "the size change but not capacity" ) {
                REQUIRE( ARRAY_SIZE(&array) == 5 );
                REQUIRE( ARRAY_CAPACITY(&array) >= 5 );
            }
        }

        WHEN( "more capacity is reserved" ) {
            ARRAY_RESERVE(int, &array, 10);

            THEN( "the capacity change but not size" ) {
                REQUIRE( ARRAY_SIZE(&array) == 3 );
                REQUIRE( ARRAY_CAPACITY(&array) >= 5 );
            }
        }

        WHEN( "less capacity is reserved" ) {
            ARRAY_RESERVE(int, &array, 0);

            THEN( "the capacity change but not size" ) {
                REQUIRE( ARRAY_SIZE(&array) == 3 );
                REQUIRE( ARRAY_CAPACITY(&array) >= 5 );
            }
        }

        ARRAY_DESTROY(int, &array);
    }

    malloc_allocator_destroy(allocator);
}

/*******************************************************************************
**** Can iterate.
*******************************************************************************/
SCENARIO( "Can iterate array", "[array]" ) {
    struct allocator* allocator = malloc_allocator_create();

    GIVEN( "Array with elements [1, 2, 3, 4, 5]" ) {
        ARRAY_T(int) array;
        ARRAY_INIT(int, &array, allocator);

        static int items [] = {1, 2, 3, 4, 5};
        ARRAY_PUSH(int, &array, items, 5);

        REQUIRE( ARRAY_SIZE(&array) == 5 );

        WHEN( "iterate from begin to end" ) {
            int* it = ARRAY_BEGIN(&array);

            THEN( "item is 1, 2, 3, 4, 5, end" ) {
                REQUIRE( *it == 1 );
                REQUIRE( *++it == 2 );
                REQUIRE( *++it == 3 );
                REQUIRE( *++it == 4 );
                REQUIRE( *++it == 5 );
                REQUIRE( ++it == ARRAY_END(&array));
            }
        }

        WHEN( "iterate from end to begin" ) {
            int* it = ARRAY_END(&array);

            THEN( "item is 5, 4, 3, 2, begin" ) {
                REQUIRE( *--it == 5 );
                REQUIRE( *--it == 4 );
                REQUIRE( *--it == 3 );
                REQUIRE( *--it == 2 );
                REQUIRE( --it == ARRAY_BEGIN(&array));
            }
        }

        ARRAY_DESTROY(int, &array);
    }

    malloc_allocator_destroy(allocator);
}


#endif
