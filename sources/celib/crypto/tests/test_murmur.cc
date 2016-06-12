#ifdef CETECH_TEST

#include <string.h>

#include "include/catch/catch.hpp"

extern "C" {
    #include "../hash.h"
};

SCENARIO( "Murmur hash", "[hash]" ) {
    GIVEN( "String to hash" ) {
        const char* string_to_hash = "hash me";

        WHEN( "when seed is 22" ) {
            uint64_t hash = hash_murmur2_64(string_to_hash, strlen(string_to_hash), 22);

            THEN( "final hash is 15828218858260124487" ) {
                REQUIRE( hash == 15828218858260124487ul );
            }
        }

        AND_WHEN( "when seed is 33" ) {
            uint64_t hash = hash_murmur2_64(string_to_hash, strlen(string_to_hash), 330);

            THEN( "final hash is 11490980199443804233" ) {
                REQUIRE( hash == 11490980199443804233ul );
            }
        }
    }
}

#endif
