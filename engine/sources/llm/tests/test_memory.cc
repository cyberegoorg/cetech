#if defined(CETECH_TEST)

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "include/catch/catch.hpp"

extern "C" {
#include "celib/memory/memory.h"
#include "llm/llm.h"
};


/*******************************************************************************
**** Test helpers
*******************************************************************************/

/*******************************************************************************
**** Malloc/Free
*******************************************************************************/
SCENARIO( "Malloc/Free", "[memory]" ) {
    GIVEN( "8B memory" ) {
        char* test_memory = (char*) os_malloc(_8B);

        WHEN("write some data") {
            test_memory[0] = 8;
            test_memory[1] = 7;
            test_memory[2] = 6;
            test_memory[3] = 5;
            test_memory[4] = 4;
            test_memory[5] = 3;
            test_memory[6] = 2;
            test_memory[7] = 1;

            THEN("data is writen == 'test string'") {
                REQUIRE(test_memory[0] == 8);
                REQUIRE(test_memory[1] == 7);
                REQUIRE(test_memory[2] == 6);
                REQUIRE(test_memory[3] == 5);
                REQUIRE(test_memory[4] == 4);
                REQUIRE(test_memory[5] == 3);
                REQUIRE(test_memory[6] == 2);
                REQUIRE(test_memory[7] == 1);
            }
        }

        os_free(test_memory);
    }
}

/*******************************************************************************
**** Memory copy
*******************************************************************************/
SCENARIO( "Memory copy", "[memory]" ) {
    GIVEN( "2 x 8B memory" ) {
        char* src_memory = (char*) os_malloc(_8B);
        char* dst_memory = (char*) os_malloc(_8B);

        src_memory[0] = 8;
        src_memory[1] = 7;
        src_memory[2] = 6;
        src_memory[3] = 5;
        src_memory[4] = 4;
        src_memory[5] = 3;
        src_memory[6] = 2;
        src_memory[7] = 1;

        WHEN("copy src -> dst") {
            memory_copy(dst_memory, src_memory, _8B);

            THEN("src == dst") {
                REQUIRE(dst_memory[0] == 8);
                REQUIRE(dst_memory[1] == 7);
                REQUIRE(dst_memory[2] == 6);
                REQUIRE(dst_memory[3] == 5);
                REQUIRE(dst_memory[4] == 4);
                REQUIRE(dst_memory[5] == 3);
                REQUIRE(dst_memory[6] == 2);
                REQUIRE(dst_memory[7] == 1);
            }
        }

        os_free(src_memory);
        os_free(dst_memory);
    }
}

#endif
