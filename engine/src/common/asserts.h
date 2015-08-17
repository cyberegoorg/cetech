#pragma once

#include <cstdio>
#include <cstdlib>

#include "platform/platform.h"

/*!
 * Check ptr macro.
 * \param ptr Pointer.
 */
#ifdef CETECH1_DEBUG
  #define CE_CHECK_PTR(ptr) \
    do { if (!ptr) { cetech1::ce_check_ptr(__FILE__, __LINE__); } } while (0)
#else
  #define CE_CHECK_PTR(ptr) do {} while (0)
#endif

/*!
 * Assert macro.
 * \param condition Asset condition.
 */
#ifdef CETECH1_DEBUG
  #define CE_ASSERT(condition) do { !(condition) ? cetech1::ce_assert( #condition, \
                                                                       __FILE__, \
                                                                       __LINE__, \
                                                                       __PRETTY_FUNCTION__) : cetech1::ce_noop(); \
} while (0)
#else
  #define CE_ASSERT(condition) do {} while (0)
#endif

/*!
 * Assert macro with message.
 * \param condition Asset condition.
 * \param where Where did it happend.
 * \param what What happend?
 */
#ifdef CETECH1_DEBUG
  #define CE_ASSERT_MSG(condition, where, what) do { !(condition) ? cetech1::ce_assert_msg(where, \
                                                                                           what, \
                                                                                           __FILE__, \
                                                                                           __LINE__, \
                                                                                           __PRETTY_FUNCTION__) : \
                                                     cetech1::ce_noop(); } while (0)
#else
  #define CE_ASSERT_MSG(condition, where, what) do {} while (0)
#endif

#define SHORT_FILE(filename) (strrchr(filename, '/') ? strrchr(filename, '/') + 1 : filename)

namespace cetech1 {
    /*!
     * No operation fce.
     */
    FORCE_INLINE void ce_noop();

    /*!
     * This fce print assert message.
     * "ASSERT: 'CONDITION_STR' in file FILE on line LINE"
     * \param condition_str Condition as string.
     * \param file Filename.
     * \param line Line.
     * \param fce Fce name.
     */
    FORCE_INLINE void ce_assert(const char* condition_str, const char* file, const int line, const char* fce);

    /*!
     * This fce print assert message.
     * "ASSERT failure in 'WHERE': "WHAT", file FILE on line LINE
     * \param where Where did it happend.
     * \param what What happend?
     * \param file Filename.
     * \param line Line.
     * \param fce Fce name.
     */
    FORCE_INLINE void ce_assert_msg(const char* where, const char* what, const char* file, const int line, const char* fce);

    /*!
     * Print check_ptr message.
     * \param file File.
     * \param line Line.
     */
    FORCE_INLINE void ce_check_ptr(const char* file, const int line);
}

namespace cetech1 {
    FORCE_INLINE void ce_noop() {}

    FORCE_INLINE void ce_assert(const char* condition_str, const char* file, const int line, const char* fce) {
        std::fprintf(stderr, "ASSERT: \'%s\' in %s:%i:%s().", condition_str, SHORT_FILE(file), line, fce);
        abort();
    }

    FORCE_INLINE void ce_assert_msg(const char* where, const char* what, const char* file, const int line, const char* fce) {
        std::fprintf(stderr, "ASSERT:\'%s\': \"%s\" %s:%i:%s().", where, what, SHORT_FILE(file), line, fce);
        abort();
    }

    FORCE_INLINE void ce_check_ptr(const char* file, const int line) {
        std::fprintf(stderr, "CHECK POINTER: in file %s on line %i is invalid pointer.", SHORT_FILE(file), line);
        abort();
    }
}

