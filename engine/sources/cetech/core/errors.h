
#ifndef CETECH_ERRORS_H
#define CETECH_ERRORS_H

//==============================================================================
// Includes
//==============================================================================

#include <stdlib.h>

#include "cetech/kernel/log.h"

//==============================================================================
// Iterface
//==============================================================================

#ifdef CETECH_DEBUG
#define _MSG_END "\n  file: %s:%d\n  stacktrace:\n%s"
#define CETECH_ASSERT_MSG(where, condition, msg, ...)                          \
    do {                                                                       \
        if (!(condition)) {                                                    \
            char* st = stacktrace(1);                                          \
            log_error(where ".assert",                                         \
                       "msg: \"%s, " msg "\"" _MSG_END,                        \
                       #condition,                                             \
                       ## __VA_ARGS__,                                         \
                       __FILE__,                                               \
                       __LINE__,                                               \
                       st);                                                    \
            stacktrace_free(st);                                               \
            abort();                                                           \
            /*exit(1);*/                                                       \
        }                                                                      \
    } while (0)                                                                \

#else
#define CETECH_ASSERT_MSG(condition, msg, ...) do {} while (0)
#endif

#define CETECH_ASSERT(where, condition) CETECH_ASSERT_MSG(where, condition, "")

char *stacktrace(int skip);

void stacktrace_free(char *st);


#endif //CETECH_ERRORS_H
