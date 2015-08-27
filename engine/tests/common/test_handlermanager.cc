#include "catch.hpp"

#include "common/handlermanager.h"


typedef cetech::handler::HandlerManager < uint32_t, uint32_t, unsigned char, 22, 8, 1024 > HandlerManager;

SCENARIO( "Handler manager can create/destroy handler", "[handlermanager]" ) {
    cetech::memory_globals::init();

    GIVEN( "New handler managerr" ) {

        cetech::Array < unsigned char > generation(cetech::memory_globals::default_allocator());
        cetech::Queue < uint32_t > free_idx(cetech::memory_globals::default_allocator());

        HandlerManager::HandlerType h1 = HandlerManager::create(free_idx, generation);
        HandlerManager::HandlerType h2 = HandlerManager::create(free_idx, generation);

        WHEN( "Create some handler" ) {
            THEN("Two handlers has diffrent id") {
                REQUIRE( h1 != h2 );
            }

            AND_THEN("Created handlers are alive") {
                REQUIRE( HandlerManager::alive(h1, generation));
                REQUIRE( HandlerManager::alive(h2, generation));
            }
        }

        WHEN( "Destroy handlers" ) {
            HandlerManager::destroy(h1, free_idx, generation);
            HandlerManager::destroy(h2, free_idx, generation);

            THEN("Handlers are dead") {
                REQUIRE_FALSE( HandlerManager::alive(h1, generation));
                REQUIRE_FALSE( HandlerManager::alive(h2, generation));
            }
        }
    }

    cetech::memory_globals::shutdown();
}
