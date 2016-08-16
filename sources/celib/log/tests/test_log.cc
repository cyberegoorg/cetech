#ifdef CETECH_TEST

/*******************************************************************************
**** Includes
*******************************************************************************/

#include "include/catch/catch.hpp"

extern "C" {
#include "celib/log/log.h"
};

/*******************************************************************************
**** Test helpers
*******************************************************************************/
static char get_worker_id() {
    return 0;
}

static struct LogEntry {
    log_level level;
    const char* where;
    const char* msg;
    void* data;
    time_t time;
    char worker_id;
} LastLogEntry;

#define NULL_ENTRY_INIT { \
        .level = LOG_ERROR, \
        .time = 0, \
        .worker_id = 0, \
        .where = 0, \
        .msg = 0, \
        .data = 0, \
};

void test_handler(enum log_level level,
                  time_t time,
                  char worker_id,
                  const char *where,
                  const char *msg,
                  void *data) {

    LogEntry log_entry = {
        .level = level,
        .time = time,
        .worker_id = worker_id,
        .where = where,
        .msg = msg,
        .data = data,
    };

    LastLogEntry = log_entry;
}


/*******************************************************************************
**** Log system can log message =D
*******************************************************************************/
SCENARIO( "Log system can log message =D", "[log]" ) {
    GIVEN( "New logsystem" ) {
        log_init(get_worker_id);
        log_register_handler(test_handler, (void*)(intptr_t)42);

        LastLogEntry = NULL_ENTRY_INIT;

        /***********************************************************************
        **** Info
        ***********************************************************************/
        WHEN("Log info msg") {
            log_info("test.info", "Test msg %d.", 42);

            THEN("Info message is logged") {
                REQUIRE( LastLogEntry.level == LOG_INFO);

                REQUIRE( LastLogEntry.msg == std::string("Test msg 42."));
                REQUIRE( LastLogEntry.where == "test.info");

                REQUIRE( LastLogEntry.time != 0 );
                REQUIRE( LastLogEntry.worker_id == 0);
                REQUIRE( LastLogEntry.data == (void*)(intptr_t)42);
            }
        }


        /***********************************************************************
        **** Warning
        ***********************************************************************/
        WHEN("Log warning msg") {
            log_warning("test.warning", "Test msg %d.", 42);

            THEN("Warnning message is logged") {
                REQUIRE( LastLogEntry.level == LOG_WARNING);

                REQUIRE( LastLogEntry.msg == std::string("Test msg 42."));
                REQUIRE( LastLogEntry.where == "test.warning");

                REQUIRE( LastLogEntry.time != 0 );
                REQUIRE( LastLogEntry.worker_id == 0);
                REQUIRE( LastLogEntry.data == (void*)(intptr_t)42);
            }
        }


        /***********************************************************************
        **** Error
        ***********************************************************************/
        WHEN("Log error msg") {
            log_error("test.error", "Test msg %d.", 42);

            THEN("Error message is logged") {
                REQUIRE( LastLogEntry.level == LOG_ERROR);

                REQUIRE( LastLogEntry.msg == std::string("Test msg 42."));
                REQUIRE( LastLogEntry.where == "test.error");

                REQUIRE( LastLogEntry.time != 0 );
                REQUIRE( LastLogEntry.worker_id == 0);
                REQUIRE( LastLogEntry.data == (void*)(intptr_t)42);
            }
        }

        /***********************************************************************
        **** Debug
        ***********************************************************************/
        WHEN("Log debug msg") {
            log_debug("test.debug", "Test msg %d.", 42);

            THEN("Debug message is logged") {
                REQUIRE( LastLogEntry.level == LOG_DBG);

                REQUIRE( LastLogEntry.msg == std::string("Test msg 42."));
                REQUIRE( LastLogEntry.where == "test.debug");

                REQUIRE( LastLogEntry.time != 0 );
                REQUIRE( LastLogEntry.worker_id == 0);
                REQUIRE( LastLogEntry.data == (void*)(intptr_t)42);
            }
        }

        log_shutdown();
    }
}

#endif
