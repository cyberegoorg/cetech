#include "catch.hpp"

#include "common/entitymanager.h"

SCENARIO( "Entity manager can create/destroy entity", "[entity]" ) {
    cetech1::memory_globals::init();

    GIVEN( "New entity manager" ) {
        cetech1::EntityManager em;

        cetech1::Entity ent1 = cetech1::entity_manager::create(em);
        cetech1::Entity ent2 = cetech1::entity_manager::create(em);

        WHEN( "Create some entity" ) {
            THEN("Two entities has diffrent id") {
                REQUIRE( ent1.id != ent2.id );
            }

            AND_THEN("Created entities are alive") {
                REQUIRE( cetech1::entity_manager::alive(em, ent1));
                REQUIRE( cetech1::entity_manager::alive(em, ent2));
            }
        }

        WHEN( "Destroy entities" ) {
            cetech1::entity_manager::destroy(em, ent1);
            cetech1::entity_manager::destroy(em, ent2);

            THEN("Entities are dead") {
                REQUIRE_FALSE( cetech1::entity_manager::alive(em, ent1));
                REQUIRE_FALSE( cetech1::entity_manager::alive(em, ent2));
            }
        }
    }

    cetech1::memory_globals::shutdown();
}
