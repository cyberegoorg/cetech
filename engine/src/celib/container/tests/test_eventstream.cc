#ifdef CETECH_TEST

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "catch/catch.hpp"

#include "celib/memory/memory.h"
#include "celib/container/eventstream.inl.h"

using namespace cetech;

/*******************************************************************************
**** Test events
*******************************************************************************/
enum {
    ET_EVENT1 = 1,
    ET_EVENT2 = 2,
};

struct Event1 {
    int index;
};

struct Event2 {
    int index;
};

/*******************************************************************************
**** Event stream can store events
*******************************************************************************/
SCENARIO( "Event sream can store events", "[event_sream]" ) {
    memory_globals::init();

    GIVEN( "Empty stream" ) {
        EventStream estream(memory_globals::default_allocator());

        THEN( "Stream is emtpy" ) {
            REQUIRE( eventstream::size(estream) == 0 );
            REQUIRE( eventstream::empty(estream));
        }

        WHEN( "Push events") {
            Event1 ev1 = {11};
            Event2 ev2 = {22};

            eventstream::write(estream, ET_EVENT1, ev1);
            eventstream::write(estream, ET_EVENT2, ev2);

            THEN( "Stream is not emtpy" ) {
                REQUIRE( eventstream::size(estream) != 0 );
                REQUIRE( not eventstream::empty(estream));

            } AND_THEN( "First event is Event1(11)" ) {
                eventstream::event_it it = 0;

                REQUIRE( eventstream::header(estream, it)->type == ET_EVENT1 );
                REQUIRE( eventstream::event < Event1 > (estream, it)->index == 11);

            } AND_THEN( "Second event is Event2(22)" ) {
                eventstream::event_it it = eventstream::next(estream, 0);

                REQUIRE( eventstream::header(estream, it)->type == ET_EVENT2 );
                REQUIRE( eventstream::event < Event1 > (estream, it)->index == 22);
            }
        }

    }

    memory_globals::shutdown();
}

/*******************************************************************************
**** Can iterate eventstream
*******************************************************************************/
SCENARIO( "Can iterate eventstream", "[event_sream]" ) {
    memory_globals::init();

    GIVEN( "Stream with some events" ) {
        EventStream estream(memory_globals::default_allocator());

        Event1 ev1 = {11};
        Event1 ev12 = {12};

        Event2 ev2 = {21};
        Event2 ev22 = {22};

        eventstream::write(estream, ET_EVENT1, ev1);
        eventstream::write(estream, ET_EVENT2, ev2);
        eventstream::write(estream, ET_EVENT1, ev12);
        eventstream::write(estream, ET_EVENT2, ev22);

        WHEN( "iterate") {
            THEN("event is ev1, ev2, ev12, ev22") {
                eventstream::event_it it = 0;

                REQUIRE( eventstream::header(estream, it)->type == ET_EVENT1 );
                REQUIRE( eventstream::event < Event1 > (estream, it)->index == 11);

                it = eventstream::next(estream, it);
                REQUIRE( eventstream::header(estream, it)->type == ET_EVENT2 );
                REQUIRE( eventstream::event < Event1 > (estream, it)->index == 21);

                it = eventstream::next(estream, it);
                REQUIRE( eventstream::header(estream, it)->type == ET_EVENT1 );
                REQUIRE( eventstream::event < Event1 > (estream, it)->index == 12);

                it = eventstream::next(estream, it);
                REQUIRE( eventstream::header(estream, it)->type == ET_EVENT2 );
                REQUIRE( eventstream::event < Event1 > (estream, it)->index == 22);
            }
        }

    }

    memory_globals::shutdown();
}
#endif
