#ifndef CETECH_ERRORS_H
#define CETECH_ERRORS_H

//==============================================================================
// Includes
//==============================================================================

#include <stdlib.h>

#include "celib/log/log.h"
#include "celib/machine/stacktrace.h"

//==============================================================================
// Iterface
//==============================================================================

#ifdef CETECH_DEBUG
#define _MSG_END "\n  file: %s\n  line: %d\n  stacktrace:\n%s"
#define CE_ASSERT_MSG(where, condition, msg, ...)                               \
    do {                                                                        \
        if (!(condition)) {                                                     \
            char* st = machine_stacktrace(1);                                   \
            log_error(where ".assert",                                          \
                       "msg: %s" msg _MSG_END,                                  \
                       #condition,                                              \
                       ## __VA_ARGS__,                                          \
                       __FILE__,                                                \
                       __LINE__,                                                \
                       st);                                                     \
            machine_stacktrace_free(st);                                        \
            abort();                                                            \
        }                                                                       \
    } while (0)                                                                 \

#else
#define CE_ASSERT_MSG(condition, msg, ...) do {} while (0)
#endif

#define CE_ASSERT(where, condition) CE_ASSERT_MSG(where, condition, "")

#endif //CETECH_ERRORS_H
