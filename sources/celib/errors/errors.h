/*******************************************************************************
**** Erros and assert module
*******************************************************************************/

#ifndef CETECH_ERRORS_H
#define CETECH_ERRORS_H

/*******************************************************************************
**** Include
*******************************************************************************/

#include <stdlib.h>

#include "../log/log.h"
#include "../utils/utils.h"

/*******************************************************************************
**** Assert with message
*******************************************************************************/

#ifdef CETECH_DEBUG
#define _MSG_END "\n  file: %s\n  line: %d\n  stacktrace:\n%s"
#define CE_ASSERT_MSG(where, condition, msg, ...)                               \
    do {                                                                        \
        if (!(condition)) {                                                     \
            char* st = utils_stacktrace(1);                                     \
            log_error(where ".assert",                                          \
                       "msg: " #condition msg _MSG_END,                         \
                       ## __VA_ARGS__,                                          \
                       __FILE__,                                                \
                       __LINE__,                                                \
                       st);                                                     \
            utils_stacktrace_free(st);                                          \
            abort();                                                            \
        }                                                                       \
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

#define CE_STATIC_ASSERT(expr, msg) _Static_assert(expr, msg)

#endif //CETECH_ERRORS_H
