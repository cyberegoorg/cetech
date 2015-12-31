#pragma once

/*******************************************************************************
**** Includes
*******************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "celib/defines.h"
#include "celib/stacktrace/stacktrace.h"
#include "celib/errors/errors.h"

#include "cetech/log/log.h"


/*******************************************************************************
**** Macros
*******************************************************************************/
#define SHORT_FILE(filename) (strrchr(filename, '/') ? strrchr(filename, '/') + 1 : filename)

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
                       "|\n  when: %s  msg: " #condition msg "\n  file: %s\n  line: %d\n  stacktrace:\n%s", \
                       buffer, \
                       ## __VA_ARGS__, \
                       __FILE__, \
                       __LINE__, \
                       st); \
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
