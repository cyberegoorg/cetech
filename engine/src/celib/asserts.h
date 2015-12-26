#pragma once

/*******************************************************************************
**** Includes
*******************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "celib/defines.h"
#include "celib/stacktrace/stacktrace.h"
#include "cetech/log/log.h"


/*******************************************************************************
**** Macros
*******************************************************************************/

/*******************************************************************************
**** Check ptr
*******************************************************************************/
#define CE_CHECK_PTR(ptr) (!ptr ? cetech::ce_check_ptr(__FILE__, __LINE__) : true)

/*******************************************************************************
**** Check
*******************************************************************************/
#define CE_CHECK(condition)\
    (!(condition) ? cetech::ce_check(#condition, \
                                     __FILE__, \
                                     __LINE__, \
                                     __PRETTY_FUNCTION__) : true)

/*******************************************************************************
**** Check with message
*******************************************************************************/
#define CE_CHECKT_MSG(condition, msg) \
    (!(condition) ? cetech::ce_check_msg(msg, \
                                         #condition, \
                                         __FILE__, \
                                         __LINE__, \
                                         __PRETTY_FUNCTION__) : true)

/*******************************************************************************
**** Assert
*******************************************************************************/
#ifdef CETECH_DEBUG
  #define CE_ASSERT(condition) \
    do { \
        !(condition) ? cetech::ce_assert(#condition, \
                                         __FILE__, \
                                         __LINE__, \
                                         __PRETTY_FUNCTION__) : cetech::ce_noop(); \
    } while (0)
#else
  #define CE_ASSERT(condition) do {} while (0)
#endif

/*******************************************************************************
**** Assert with message
*******************************************************************************/
#ifdef CETECH_DEBUG
  #define CE_ASSERT_MSG(condition, msg) \
    do { !(condition) ? cetech::ce_assert_msg(msg, \
                                              #condition, \
                                              __FILE__, \
                                              __LINE__, \
                                              __PRETTY_FUNCTION__) : \
         cetech::ce_noop(); } while (0)
#else
  #define CE_ASSERT_MSG(condition, msg) do {} while (0)
#endif

/*******************************************************************************
**** Assert ptr
*******************************************************************************/
#ifdef CETECH_DEBUG
  #define CE_ASSERT_PTR(condition) \
    do { \
        !(condition) ? cetech::ce_assert_ptr(#condition, \
                                             __FILE__, \
                                             __LINE__, \
                                             __PRETTY_FUNCTION__) : cetech::ce_noop(); \
    } while (0)
#else
  #define CE_ASSERT_PTR(condition) do {} while (0)
#endif

/*******************************************************************************
**** Interface
*******************************************************************************/
namespace cetech {

    /***************************************************************************
    **** Assert
    ***************************************************************************/
    CE_INLINE void ce_assert(const char* condition_str,
                             const char* file,
                             const int line,
                             const char* fce);

    /***************************************************************************
    **** Assert message
    ***************************************************************************/
    CE_INLINE void ce_assert_msg(const char* where,
                                 const char* what,
                                 const char* file,
                                 const int line,
                                 const char* fce);

    /***************************************************************************
    **** Check
    ***************************************************************************/
    CE_INLINE bool ce_check(const char* where,
                            const char* what,
                            const char* file,
                            const int line,
                            const char* fce);

    /***************************************************************************
    **** Check with message
    ***************************************************************************/
    CE_INLINE bool ce_check_msg(const char* where,
                                const char* what,
                                const char* file,
                                const int line,
                                const char* fce);

    /***************************************************************************
    **** Check pointer
    ***************************************************************************/
    CE_INLINE bool ce_check_ptr(const char* file,
                                const int line);

    /***************************************************************************
    **** NOOP
    ***************************************************************************/
    CE_INLINE void ce_noop() {};
}


/******************************************************************************
 **** Implementation
 *******************************************************************************/

namespace cetech {
    #define SHORT_FILE(filename) (strrchr(filename, '/') ? strrchr(filename, '/') + 1 : filename)

    CE_INLINE void ce_assert(const char* condition_str,
                             const char* file,
                             const int line,
                             const char* fce) {
        char* trace = stacktrace(2);

        log::error("assert", "\'%s\' in %s:%i:%s.\n    stacktrace:\n%s\n",
                   condition_str, SHORT_FILE(file), line, fce,
                   trace);

        free(trace);
        abort();
    }

    CE_INLINE void ce_assert_msg(const char* where,
                                 const char* what,
                                 const char* file,
                                 const int line,
                                 const char* fce) {

        char* trace = stacktrace(2);
        log::error("assert", "\'%s\': \"%s\" %s:%i:%s.\n"
                   "    stacktrace:\n%s\n",
                   where, what, SHORT_FILE(file), line, fce, trace);

        free(trace);
        abort();
    }

    CE_INLINE void ce_assert_ptr(const char* file,
                                 const int line) {
        char* trace = stacktrace(2);
        log::error("assert.check_ptr", "in file %s on line %i is invalid pointer.\n"
                   "    stacktrace:\n%s\n \n",
                   SHORT_FILE(file), line, trace);

        free(trace);
        abort();
    }

    CE_INLINE bool ce_check(const char* condition_str,
                            const char* file,
                            const int line,
                            const char* fce) {
        char* trace = stacktrace(2);

        log::error("check", "\'%s\' in %s:%i:%s.\n    stacktrace:\n%s\n",
                   condition_str, SHORT_FILE(file), line, fce,
                   trace);

        free(trace);
        
        return false;
    }

    CE_INLINE bool ce_check_msg(const char* where,
                                const char* what,
                                const char* file,
                                const int line,
                                const char* fce) {

        char* trace = stacktrace(2);
        log::error("check", "\'%s\': \"%s\" %s:%i:%s.\n"
                   "    stacktrace:\n%s\n",
                   where, what, SHORT_FILE(file), line, fce, trace);

        free(trace);
        
        return false;
    }

    CE_INLINE bool ce_check_ptr(const char* file,
                                const int line) {
        char* trace = stacktrace(2);
        log::error("check_ptr", "in file %s on line %i is invalid pointer.\n"
                   "    stacktrace:\n%s\n \n",
                   SHORT_FILE(file), line, trace);

        free(trace);
        return false;
    }

    #undef SHORT_FILE
}
