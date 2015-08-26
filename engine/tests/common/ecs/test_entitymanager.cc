#include "catch.hpp"

#include "common/ecs/entitymanager.h"

SCENARIO( "Entity manager can create/destroy entity", "[entity]" ) {
    cetech::memory_globals::init();

    GIVEN( "New entity manager" ) {
        cetech::EntityManager em;

        cetech::Entity ent1 = cetech::entity_manager::create(em);
        cetech::Entity ent2 = cetech::entity_manager::create(em);

        WHEN( "Create some entity" ) {
            THEN("Two entities has diffrent id") {
                REQUIRE( ent1.id != ent2.id );
            }

            AND_THEN("Created entities are alive") {
                REQUIRE( cetech::entity_manager::alive(em, ent1));
                REQUIRE( cetech::entity_manager::alive(em, ent2));
            }
        }

        WHEN( "Destroy entities" ) {
            cetech::entity_manager::destroy(em, ent1);
            cetech::entity_manager::destroy(em, ent2);

            THEN("Entities are dead") {
                REQUIRE_FALSE( cetech::entity_manager::alive(em, ent1));
                REQUIRE_FALSE( cetech::entity_manager::alive(em, ent2));
            }
        }
    }

    cetech::memory_globals::shutdown();
}
