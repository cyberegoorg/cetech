#ifdef CETECH_TEST

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "catch/catch.hpp"

#include "cetech/log/log.h"
#include "celib/memory/memory.h"

/*******************************************************************************
**** Test helpers
*******************************************************************************/
static uint32_t get_worker_id() {
    return 0;
}

static struct LogEntry {
    cetech::log::LogLevel::Enum level;
    const char* where;
    const char* msg;
    void* data;
    time_t time;
    uint32_t worker_id;
} _LastLogEntry;

#define NULL_ENTRY_INIT { \
        .level = cetech::log::LogLevel::ERROR, \
        .time = 0, \
        .worker_id = 0, \
        .where = 0, \
        .msg = 0, \
        .data = 0, \
};

void test_handler(const cetech::log::LogLevel::Enum level,
                  const time_t time,
                  const uint32_t worker_id,
                  const char* where,
                  const char* msg,
                  void* data) {

    LogEntry log_entry = {
        .level = level,
        .time = time,
        .worker_id = worker_id,
        .where = where,
        .msg = msg,
        .data = data,
    };

    _LastLogEntry = log_entry;
}


/*******************************************************************************
**** Log system can log message =D
*******************************************************************************/
SCENARIO( "Log system can log message =D", "[log]" ) {
    cetech::memory_globals::init();

    GIVEN( "New logsystem" ) {
        cetech::log_globals::init(&get_worker_id);
        cetech::log::register_handler(test_handler, (void*)(intptr_t)42);

        _LastLogEntry = NULL_ENTRY_INIT;

        /***********************************************************************
         **** Info
         ***********************************************************************/
        WHEN("Log info msg") {
            cetech::log::info("test.info", "Test msg %d.", 42);

            THEN("Info message is logged") {
                REQUIRE( _LastLogEntry.level == cetech::log::LogLevel::INFO);

                REQUIRE( _LastLogEntry.msg == std::string("Test msg 42."));
                REQUIRE( _LastLogEntry.where == "test.info");

                REQUIRE( _LastLogEntry.time != 0 );
                REQUIRE( _LastLogEntry.worker_id == 0);
                REQUIRE( _LastLogEntry.data == (void*)(intptr_t)42);
            }
        }


        /***********************************************************************
        **** Warning
        ***********************************************************************/
        WHEN("Log warning msg") {
            cetech::log::warning("test.warning", "Test msg %d.", 42);

            THEN("Warnning message is logged") {
                REQUIRE( _LastLogEntry.level == cetech::log::LogLevel::WARNING);

                REQUIRE( _LastLogEntry.msg == std::string("Test msg 42."));
                REQUIRE( _LastLogEntry.where == "test.warning");

                REQUIRE( _LastLogEntry.time != 0 );
                REQUIRE( _LastLogEntry.worker_id == 0);
                REQUIRE( _LastLogEntry.data == (void*)(intptr_t)42);
            }
        }


        /***********************************************************************
        **** Error
        ***********************************************************************/
        WHEN("Log error msg") {
            cetech::log::error("test.error", "Test msg %d.", 42);

            THEN("Error message is logged") {
                REQUIRE( _LastLogEntry.level == cetech::log::LogLevel::ERROR);

                REQUIRE( _LastLogEntry.msg == std::string("Test msg 42."));
                REQUIRE( _LastLogEntry.where == "test.error");

                REQUIRE( _LastLogEntry.time != 0 );
                REQUIRE( _LastLogEntry.worker_id == 0);
                REQUIRE( _LastLogEntry.data == (void*)(intptr_t)42);
            }
        }

        /***********************************************************************
        **** Debug
        ***********************************************************************/
        WHEN("Log debug msg") {
            cetech::log::debug("test.debug", "Test msg %d.", 42);

            THEN("Debug message is logged") {
                REQUIRE( _LastLogEntry.level == cetech::log::LogLevel::DBG);

                REQUIRE( _LastLogEntry.msg == std::string("Test msg 42."));
                REQUIRE( _LastLogEntry.where == "test.debug");

                REQUIRE( _LastLogEntry.time != 0 );
                REQUIRE( _LastLogEntry.worker_id == 0);
                REQUIRE( _LastLogEntry.data == (void*)(intptr_t)42);
            }
        }

        cetech::log_globals::shutdown();
    }

    cetech::memory_globals::shutdown();
}

#endif
