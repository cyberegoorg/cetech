#pragma once

#include <cstdio>
#include <cstdlib>
#include <cstring>

#include "celib/defines.h"
#include "celib/stacktrace.h"

/*!
 * Check ptr macro.
 * \param ptr Pointer.
 */
#ifdef CETECH_DEBUG
  #define CE_CHECK_PTR(ptr) \
    do { if (!ptr) { cetech::ce_check_ptr(__FILE__, __LINE__); } } while (0)
#else
  #define CE_CHECK_PTR(ptr) do {} while (0)
#endif

/*!
 * Assert macro.
 * \param condition Asset condition.
 */
#ifdef CETECH_DEBUG
  #define CE_ASSERT(condition) do { !(condition) ? cetech::ce_assert( #condition, \
                                                                      __FILE__, \
                                                                      __LINE__, \
                                                                      __PRETTY_FUNCTION__) : cetech::ce_noop(); \
} while (0)
#else
  #define CE_ASSERT(condition) do {} while (0)
#endif

/*!
 * Assert macro with message.
 * \param condition Asset condition.
 * \param msg Message.
 */
#ifdef CETECH_DEBUG
  #define CE_ASSERT_MSG(condition, msg) do { !(condition) ? cetech::ce_assert_msg(msg, \
                                                                                  #condition, \
                                                                                  __FILE__, \
                                                                                  __LINE__, \
                                                                                  __PRETTY_FUNCTION__) : \
                                             cetech::ce_noop(); } while (0)
#else
  #define CE_ASSERT_MSG(condition, where, what) do {} while (0)
#endif

#define SHORT_FILE(filename) (strrchr(filename, '/') ? strrchr(filename, '/') + 1 : filename)

namespace cetech {
    /*!
     * No operation fce.
     */
    CE_INLINE void ce_noop();

    /*!
     * This fce print assert message.
     * "ASSERT: 'CONDITION_STR' in file FILE on line LINE"
     * \param condition_str Condition as string.
     * \param file Filename.
     * \param line Line.
     * \param fce Fce name.
     */
    CE_INLINE void ce_assert(const char* condition_str,
                             const char* file,
                             const int line,
                             const char* fce);

    /*!
     * This fce print assert message.
     * "ASSERT failure in 'WHERE': "WHAT", file FILE on line LINE
     * \param where Where did it happend.
     * \param what What happend?
     * \param file Filename.
     * \param line Line.
     * \param fce Fce name.
     */
    CE_INLINE void ce_assert_msg(const char* where,
                                 const char* what,
                                 const char* file,
                                 const int line,
                                 const char* fce);

    /*!
     * Print check_ptr message.
     * \param file File.
     * \param line Line.
     */
    CE_INLINE void ce_check_ptr(const char* file, const int line);
}

namespace cetech {
    CE_INLINE void ce_noop() {}

    CE_INLINE void ce_assert(const char* condition_str,
                             const char* file,
                             const int line,
                             const char* fce) {
        char* trace = stacktrace(2);
        fprintf(stderr, "[A] \'%s\' in %s:%i:%s. stacktrace:\n%s\n", condition_str, SHORT_FILE(file), line, fce, trace);
        free(trace);
        abort();
    }

    CE_INLINE void ce_assert_msg(const char* where,
                                 const char* what,
                                 const char* file,
                                 const int line,
                                 const char* fce) {

        char* trace = stacktrace(2);
        fprintf(stderr, "[A] \'%s\': \"%s\" %s:%i:%s. stacktrace:\n%s\n", where, what, SHORT_FILE(
                    file), line, fce, trace);
        free(trace);
        abort();
    }

    CE_INLINE void ce_check_ptr(const char* file, const int line) {
        char* trace = stacktrace(2);
        fprintf(stderr, "[A][check_ptr] in file %s on line %i is invalid pointer. stacktrace:\n%s\n \n", SHORT_FILE(
                    file), line, trace);
        free(trace);
        abort();
    }
}

