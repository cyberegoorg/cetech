#ifndef CELIB_ERRORS_H
#define CELIB_ERRORS_H

//==============================================================================
// Includes
//==============================================================================

#include <stdlib.h>

#include "log.h"
#include "stacktrace.h"

//==============================================================================
// Iterface
//==============================================================================

#ifdef CELIB_DEBUG
#define _MSG_END "\n  file: %s\n  line: %d\n  stacktrace:\n%s"
#define CEL_ASSERT_MSG(where, condition, msg, ...)                             \
    do {                                                                       \
        if (!(condition)) {                                                    \
            char* st = cel_stacktrace(1);                                      \
            log_error(where ".assert",                                         \
                       "msg: \"%s, " msg "\"" _MSG_END,                        \
                       #condition,                                             \
                       ## __VA_ARGS__,                                         \
                       __FILE__,                                               \
                       __LINE__,                                               \
                       st);                                                    \
            cel_stacktrace_free(st);                                           \
            abort();                                                           \
            /*exit(1);*/                                                       \
        }                                                                      \
    } while (0)                                                                \

#else
#define CEL_ASSERT_MSG(condition, msg, ...) do {} while (0)
#endif

#define CEL_ASSERT(where, condition) CEL_ASSERT_MSG(where, condition, "")

#endif //CELIB_ERRORS_H
