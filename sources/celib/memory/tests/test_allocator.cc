#if defined(CETECH_TEST)

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "include/catch/catch.hpp"

extern "C" {
    #include "../allocator.h"
    #include "../ialocator.h"
    #include "../memory.h"
};


/*******************************************************************************
**** Test helpers
*******************************************************************************/

static struct {
    Alloc_t allocator;
    size_t size;
} LastMallocCall;

static struct {
    Alloc_t allocator;
    void* ptr;
} LastFreeCall;

void *_test_malloc(Alloc_t allocator, size_t size) {

    LastMallocCall.allocator = allocator;
    LastMallocCall.size = size;

    return (void*)1;
}

void _test_free(Alloc_t allocator, void *ptr) {
    LastFreeCall.allocator = allocator;
    LastFreeCall.ptr = ptr;
}

struct TestAllocator {
    struct iallocator i;
};

/*******************************************************************************
**** Allocator alloc/free
*******************************************************************************/
SCENARIO( "Allocator alloc/free", "[memory]" ) {
    GIVEN( "TestAllocator" ) {
        struct TestAllocator ta = {
            .i = { .free = _test_free, .alloc=_test_malloc }
        };

        WHEN("alloc 8B") {
            void* alloc_mem = alloc_alloc(&ta, _8B);

            THEN("memory is allocated") {
                REQUIRE( LastMallocCall.allocator == &ta );
                REQUIRE( LastMallocCall.size == _8B );
                REQUIRE( alloc_mem == (void*)1 );
            }
        }

        AND_WHEN("alloc and free 8B") {
            void* alloc_mem = alloc_alloc(&ta, _8B);
            alloc_free(&ta, alloc_mem);

            THEN("memory is free") {
                REQUIRE( LastFreeCall.allocator == &ta );
                REQUIRE( LastFreeCall.ptr == alloc_mem);
            }
        }
    }
}

#endif
