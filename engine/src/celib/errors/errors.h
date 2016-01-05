#pragma once

#include <cstdlib>
#include <unistd.h>

#include "celib/stacktrace/stacktrace.h"
#include "celib/log/log.h"

/*******************************************************************************
**** Assert with message
*******************************************************************************/
#ifdef CETECH_DEBUG
  #define CE_ASSERT_MSG(where, condition, msg, ...) \
    do { \
        if (!(condition)) { \
            char* st = stacktrace(1); \
            char buffer[4096] = {0}; \
            error::to_yaml(buffer); \
            log::error(where ".assert", \
                       "when: %s  msg: " #condition msg "\n  file: %s\n  line: %d\n  stacktrace:\n%s", \
                       buffer, \
                       ## __VA_ARGS__, \
                       __FILE__, \
                       __LINE__, \
                       st); \
            sleep(1); \
            free(st); \
            abort(); \
        } \
    } while (0)
#else
  #define CE_ASSERT_MSG(condition, msg, ...) do {} while (0)
#endif

/*******************************************************************************
**** Assert
*******************************************************************************/
#ifdef CETECH_DEBUG
  #define CE_ASSERT(where, condition) CE_ASSERT_MSG(where, condition, "")
#else
  #define CE_ASSERT(where, condition) do {} while (0)
#endif

namespace cetech {
    namespace error {
        void enter_scope(const char* name,
                         const char* data);
        void leave_scope();

        struct ErrorScope {
            ErrorScope(const char* name, const char* data) {
                error::enter_scope(name, data);
            }

            ~ErrorScope() {
                error::leave_scope();
            }
        };
        
        void to_yaml(char* buffer);
    }

    namespace error_globals {
        void init();
        void shutdown();
    }
}
